#include "Base/ueBase.h"

#if defined(GL_D3D11)

#include "Graphics/glLib_Private.h"

void glTextureBuffer_DetermineD3D11Params(const glTextureBufferDesc* desc, DXGI_FORMAT& format, D3D11_USAGE& usage, UINT& bindFlags, UINT& miscFlags, UINT& CPUAccessFlags)
{
	UE_ASSERT_FUNC(glUtils_ToD3DFORMAT(desc->m_format, format));

	usage = (desc->m_flags & glTextureBufferFlags_IsDynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

	bindFlags = (desc->m_flags & glTextureBufferFlags_IsTexture) ? D3D11_BIND_SHADER_RESOURCE : 0;
	if (desc->m_flags & glTextureBufferFlags_IsRenderTarget)
		bindFlags |= glUtils_IsDepthFormat(desc->m_format) ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET;

	miscFlags = (desc->m_type == glTexType_Cube) ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

	if (((desc->m_flags & glTextureBufferFlags_IsRenderTarget) && desc->m_enableAutoMips) || desc->m_enableManualMips)
	{
		bindFlags |= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		miscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}

	CPUAccessFlags = (desc->m_flags & glTextureBufferFlags_IsDynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
}

void glTextureBuffer_CreateInPlace(glTextureBuffer* tb, const glTextureInitData* initData)
{
	UE_ASSERT(glTextureBuffer_IsRenderTarget(tb) || glTextureBuffer_IsTexture(tb));

	tb->m_textureHandle = NULL;
	tb->m_shaderView = NULL;
	tb->m_renderTargetView = NULL;
	tb->m_depthStencilView = NULL;

	// Load / convert from source format?

	if (initData && initData->m_storage != glTexStorage_GPU)
	{
		UE_ASSERT(initData->m_dataArrayLength == 1);

		D3DX11_IMAGE_LOAD_INFO info;
		ueMemZeroS(info);
		info.Width = tb->m_desc.m_width;
		info.Height = tb->m_desc.m_height;
		info.Depth = tb->m_desc.m_depth;
		info.MipLevels = tb->m_desc.m_numLevels;
		info.Filter = D3DX11_FILTER_NONE; // No resizing at runtime
		info.MipFilter = D3DX11_DEFAULT;
		glTextureBuffer_DetermineD3D11Params(&tb->m_desc, info.Format, info.Usage, info.BindFlags, info.MiscFlags, info.CpuAccessFlags);

		UE_ASSERTWinCall(D3DX11CreateTextureFromMemory(
			D3DEV,
			initData->m_dataArray[0].m_data,
			initData->m_dataArray[0].m_rowPitch,
			&info,
			NULL, // ID3DX11ThreadPump
			&tb->m_textureHandle,
			NULL));

		glTextureBuffer_CreateViews(tb);
		return;
	}

	// Set up MSAA

#if 0 // TODO
	UINT numQualityLevels;
verify that msaa level is supported - see ID3D11Device::CheckMultisampleQualityLevels(format, tb->m_desc.m_rt.m_MSAALevel, &numQualityLevels) Method
 and also see D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT
#endif

	DXGI_SAMPLE_DESC sampleDesc;
	sampleDesc.Count = tb->m_desc.m_rt.m_MSAALevel;
	sampleDesc.Quality = 0;

	// Set up subresources descriptions

	D3D11_SUBRESOURCE_DATA subresourceDataArray[16 /* max mip levels */ * 6 /* cube texture case */ ];
	if (!tb->m_desc.m_enableAutoMips && initData)
		for (u32 i = 0; i < initData->m_dataArrayLength; i++)
		{
			subresourceDataArray[i].pSysMem = initData->m_dataArray[i].m_data;
			subresourceDataArray[i].SysMemPitch = initData->m_dataArray[i].m_rowPitch;
			subresourceDataArray[i].SysMemSlicePitch = initData->m_dataArray[i].m_slicePitch;
		}

	// Create texture

	switch (tb->m_desc.m_type)
	{
		case glTexType_2D:
		case glTexType_Cube:
		{
			D3D11_TEXTURE2D_DESC desc;
			ueMemZeroS(desc);

			desc.Width = tb->m_desc.m_width;
			desc.Height = tb->m_desc.m_height;
			desc.MipLevels = tb->m_desc.m_enableAutoMips ? 0 : tb->m_desc.m_numLevels;
			desc.ArraySize = tb->m_desc.m_type == glTexType_Cube ? 6 : 1;

			desc.SampleDesc = sampleDesc;

			glTextureBuffer_DetermineD3D11Params(&tb->m_desc, desc.Format, desc.Usage, desc.BindFlags, desc.MiscFlags, desc.CPUAccessFlags);

			UE_ASSERTWinCall(D3DEV->CreateTexture2D(&desc, (initData && initData->m_dataArrayLength && !tb->m_desc.m_enableAutoMips) ? subresourceDataArray : NULL, (ID3D11Texture2D**) &tb->m_textureHandle));
			break;
		}
		case glTexType_3D:
		{
			UE_ASSERT(!"Not yet supported");
			break;
		}
	}

	// Set top mip level data for textures with auto-generation enabled

	if (tb->m_desc.m_enableAutoMips && initData && initData->m_dataArrayLength != 0)
	{
		UE_ASSERT(initData->m_storage == glTexStorage_GPU);
		UE_ASSERT(initData->m_dataArrayLength == 1);
		D3DCTX->UpdateSubresource(tb->m_textureHandle, 0, NULL, initData->m_dataArray[0].m_data, initData->m_dataArray[0].m_rowPitch, initData->m_dataArray[0].m_slicePitch);
	}

	// Create shader / render target views

	glTextureBuffer_CreateViews(tb);
}

void glTextureBuffer_CreateViews(glTextureBuffer* tb)
{
	tb->m_shaderView = NULL;
	tb->m_renderTargetView = NULL;
	tb->m_depthStencilView = NULL;

	// Get D3D format

	DXGI_FORMAT d3dFormat;
	UE_ASSERT_FUNC(glUtils_ToD3DFORMAT(tb->m_desc.m_format, d3dFormat));

	// Create shader view

	if (glTextureBuffer_IsTexture(tb))
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
		ueMemZeroS(viewDesc);

		viewDesc.Format = d3dFormat;
		switch (tb->m_desc.m_type)
		{
			case glTexType_2D:
				viewDesc.ViewDimension = (tb->m_desc.m_rt.m_MSAALevel > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
				viewDesc.Texture2D.MipLevels = tb->m_desc.m_numLevels;
				break;
			case glTexType_Cube:
				UE_ASSERT(tb->m_desc.m_rt.m_MSAALevel == 1);
				viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
				viewDesc.TextureCube.MipLevels = tb->m_desc.m_numLevels;
				break;
			case glTexType_3D:
				UE_ASSERT(tb->m_desc.m_rt.m_MSAALevel == 1);
				viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
				viewDesc.Texture3D.MipLevels = tb->m_desc.m_numLevels;
				break;
			UE_INVALID_CASE(tb->m_desc.m_type);
		}
		UE_ASSERTWinCall(D3DEV->CreateShaderResourceView(tb->m_textureHandle, &viewDesc, &tb->m_shaderView));
	}

	// Create render target view

	if (glTextureBuffer_IsRenderTarget(tb))
	{
		UE_ASSERT(tb->m_desc.m_type == glTexType_2D);

		if (glUtils_IsDepthFormat(tb->m_desc.m_format))
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;
			ueMemZeroS(viewDesc);

			viewDesc.Format = d3dFormat;
			viewDesc.ViewDimension = tb->m_desc.m_rt.m_MSAALevel > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipSlice = 0;
			UE_ASSERTWinCall(D3DEV->CreateDepthStencilView(tb->m_textureHandle, &viewDesc, &tb->m_depthStencilView));
		}
		else
		{
			D3D11_RENDER_TARGET_VIEW_DESC viewDesc;
			ueMemZeroS(viewDesc);

			viewDesc.Format = d3dFormat;
			viewDesc.ViewDimension = tb->m_desc.m_rt.m_MSAALevel > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipSlice = 0;
			UE_ASSERTWinCall(D3DEV->CreateRenderTargetView(tb->m_textureHandle, &viewDesc, &tb->m_renderTargetView));
		}
	}
}

void glTextureBuffer_DestroyInPlace(glTextureBuffer* tb)
{
	if (tb->m_shaderView)
	{
		const UINT refCount = tb->m_shaderView->Release();
		UE_ASSERT(refCount == 0);
		tb->m_shaderView = NULL;
	}

	if (tb->m_depthStencilView)
	{
		const UINT refCount = tb->m_depthStencilView->Release();
		UE_ASSERT(refCount == 0);
		tb->m_depthStencilView = NULL;
	}

	if (tb->m_renderTargetView)
	{
		const UINT refCount = tb->m_renderTargetView->Release();
		UE_ASSERT(refCount == 0);
		tb->m_renderTargetView = NULL;
	}

	if (tb->m_textureHandle)
	{
		const UINT refCount = tb->m_textureHandle->Release();
		UE_ASSERT(refCount == 0);
		tb->m_textureHandle = NULL;
	}
}

ueBool glTextureBuffer_Lock(glTextureBuffer* tb, glTextureData* result, u32 face, u32 level, u32 lockFlags, const glTextureArea* area)
{
	UE_ASSERT(!area); // For now only whole level mapping supported

	// Determine mapping type

	D3D11_MAP mapType;
	switch (lockFlags)
	{
		case glBufferLockFlags_Read: mapType = D3D11_MAP_READ; break;
		case glBufferLockFlags_Write: mapType = D3D11_MAP_WRITE; break;
		case (glBufferLockFlags_Read | glBufferLockFlags_Write): mapType = D3D11_MAP_READ_WRITE; break;
		case (glBufferLockFlags_Write | glBufferLockFlags_Discard): mapType = D3D11_MAP_WRITE_DISCARD; break;
		UE_INVALID_CASE(lockFlags);
	}

	// Map

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	UE_ASSERTWinCall(D3DCTX->Map(tb->m_textureHandle, level, mapType, 0, &mappedResource));

	result->m_rowPitch = mappedResource.RowPitch;
	result->m_slicePitch = mappedResource.DepthPitch;
	result->m_data = mappedResource.pData;

	return UE_TRUE;
}

void glTextureBuffer_Unlock(glTextureBuffer* tb, u32 face, u32 level)
{
	D3DCTX->Unmap(tb->m_textureHandle, level);
}

void glTextureBuffer_GenerateMips(glTextureBuffer* tb)
{
	UE_ASSERT(glTextureBuffer_IsTexture(tb) && !tb->m_desc.m_enableAutoMips);
	D3DCTX->GenerateMips(tb->m_shaderView);
}

#else // defined(GL_D3D11)
	UE_NO_EMPTY_FILE
#endif
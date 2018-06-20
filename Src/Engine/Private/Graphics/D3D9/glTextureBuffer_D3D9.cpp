#include "Base/ueBase.h"

#if defined(GL_D3D9)

#include "Graphics/glLib_Private.h"

UE_INLINE IDirect3DTexture9* glTextureBuffer_GetHandle2D(glTextureBuffer* tb)
{
	UE_ASSERT(glTextureBuffer_IsTexture(tb) && tb->m_desc.m_type == glTexType_2D);
	return static_cast<IDirect3DTexture9*>(tb->m_textureHandle);
}

UE_INLINE IDirect3DCubeTexture9* glTextureBuffer_GetHandleCube(glTextureBuffer* tb)
{
	UE_ASSERT(glTextureBuffer_IsTexture(tb) && tb->m_desc.m_type == glTexType_Cube);
	return static_cast<IDirect3DCubeTexture9*>(tb->m_textureHandle);
}

UE_INLINE IDirect3DVolumeTexture9* glTextureBuffer_GetHandle3D(glTextureBuffer* tb)
{
	UE_ASSERT(glTextureBuffer_IsTexture(tb) && tb->m_desc.m_type == glTexType_3D);
	return static_cast<IDirect3DVolumeTexture9*>(tb->m_textureHandle);
}

void glTextureBuffer_CreateInPlace(glTextureBuffer* tb, const glTextureInitData* initData)
{
	UE_ASSERT(glTextureBuffer_IsRenderTarget(tb) || glTextureBuffer_IsTexture(tb));

	tb->m_textureHandle = NULL;
	tb->m_surfaceHandle = NULL;

	// Get D3D format

	D3DFORMAT d3dFormat;
	UE_ASSERT_FUNC(glUtils_ToD3DFORMAT(tb->m_desc.m_format, d3dFormat));

	// Load / convert from source format?

	if (initData && initData->m_storage != glTexStorage_GPU)
	{
		UE_ASSERT(initData->m_dataArrayLength == 1);

		switch (tb->m_desc.m_type)
		{
		case glTexType_2D:
			UE_ASSERTWinCall(D3DXCreateTextureFromFileInMemoryEx(
				D3DEV,
				initData->m_dataArray[0].m_data,
				initData->m_dataArray[0].m_rowPitch,
				tb->m_desc.m_width,
				tb->m_desc.m_height,
				tb->m_desc.m_numLevels,
				0, // Usage
				d3dFormat,
				D3DPOOL_MANAGED,
				D3DX_FILTER_NONE, // No resizing at runtime  // D3DX_FILTER_POINT
				D3DX_DEFAULT, // Mip-filter
				0, // Color key
				NULL, // Image info
				NULL, // Palette entry
				(IDirect3DTexture9**) &tb->m_textureHandle));
			break;
		case glTexType_3D:
			UE_ASSERTWinCall(D3DXCreateVolumeTextureFromFileInMemoryEx(
				D3DEV,
				initData->m_dataArray[0].m_data,
				initData->m_dataArray[0].m_rowPitch,
				tb->m_desc.m_width,
				tb->m_desc.m_height,
				tb->m_desc.m_depth,
				tb->m_desc.m_numLevels,
				0, // Usage
				d3dFormat,
				D3DPOOL_MANAGED,
				D3DX_FILTER_NONE, // No resizing at runtime  // D3DX_FILTER_POINT
				D3DX_DEFAULT, // Mip-filter
				0, // Color key
				NULL, // Image info
				NULL, // Palette entry
				(IDirect3DVolumeTexture9**) &tb->m_textureHandle));
			break;
		case glTexType_Cube:
			UE_ASSERTWinCall(D3DXCreateCubeTextureFromFileInMemoryEx(
				D3DEV,
				initData->m_dataArray[0].m_data,
				initData->m_dataArray[0].m_rowPitch,
				tb->m_desc.m_width,
				tb->m_desc.m_numLevels,
				0, // Usage
				d3dFormat,
				D3DPOOL_MANAGED,
				D3DX_FILTER_NONE, // No resizing at runtime  // D3DX_FILTER_POINT
				D3DX_DEFAULT, // Mip-filter
				0, // Color key
				NULL, // Image info
				NULL, // Palette entry
				(IDirect3DCubeTexture9**) &tb->m_textureHandle));
			break;
		}

		tb->m_desc.m_numLevels = tb->m_textureHandle->GetLevelCount();
		return;
	}

	// Create texture (possibly also render target)

	if (glTextureBuffer_IsTexture(tb))
	{
		// All non-dynamic non-render-target textures are stored in managed pool

		const D3DPOOL pool = (glTextureBuffer_IsRenderTarget(tb) || (tb->m_desc.m_flags & glTextureBufferFlags_IsDynamic)) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;

		const DWORD usage =
			(glTextureBuffer_IsRenderTarget(tb) ? D3DUSAGE_RENDERTARGET : 0) |
			((!glTextureBuffer_IsRenderTarget(tb) && (tb->m_desc.m_flags & glTextureBufferFlags_IsDynamic)) ? D3DUSAGE_DYNAMIC : 0) |
			(tb->m_desc.m_enableAutoMips ? D3DUSAGE_AUTOGENMIPMAP : 0);

		const u32 d3dNumLevelsParam = tb->m_desc.m_enableAutoMips ? 0 : tb->m_desc.m_numLevels; // D3D9 requires 0 levels for ueto mipmapped textures

		switch (tb->m_desc.m_type)
		{
		case glTexType_2D:
			UE_ASSERTWinCall(D3DEV->CreateTexture(tb->m_desc.m_width, tb->m_desc.m_height, d3dNumLevelsParam, usage, d3dFormat, pool, (IDirect3DTexture9**) &tb->m_textureHandle, NULL));
			break;
		case glTexType_3D:
			UE_ASSERTWinCall(D3DEV->CreateVolumeTexture(tb->m_desc.m_width, tb->m_desc.m_height, tb->m_desc.m_depth, d3dNumLevelsParam, usage, d3dFormat, pool, (IDirect3DVolumeTexture9**) &tb->m_textureHandle, NULL));
			break;
		case glTexType_Cube:
			UE_ASSERTWinCall(D3DEV->CreateCubeTexture(tb->m_desc.m_width, d3dNumLevelsParam, usage, d3dFormat, pool, (IDirect3DCubeTexture9**) &tb->m_textureHandle, NULL));
			break;
		}

		// Obtain pointer to the top render target surface

		if (glTextureBuffer_IsRenderTarget(tb))
		{
			UE_ASSERT(tb->m_desc.m_type == glTexType_2D);
			UE_ASSERTWinCall(glTextureBuffer_GetHandle2D(tb)->GetSurfaceLevel(0, &tb->m_surfaceHandle));
		}
	}

	// Create render target

	else if (glUtils_IsDepthFormat(tb->m_desc.m_format))
	{
		UE_ASSERT(tb->m_desc.m_numLevels == 1);
		UE_ASSERTWinCall(D3DEV->CreateDepthStencilSurface(
			tb->m_desc.m_width,
			tb->m_desc.m_height,
			d3dFormat,
			glUtils_ToD3DMULTISAMPLE_TYPE(tb->m_desc.m_rt.m_MSAALevel),
			0,
			FALSE,
			&tb->m_surfaceHandle,
			NULL));
	}
	else
	{
		UE_ASSERT(tb->m_desc.m_numLevels == 1);
		UE_ASSERTWinCall(D3DEV->CreateRenderTarget(
			tb->m_desc.m_width,
			tb->m_desc.m_height,
			d3dFormat,
			glUtils_ToD3DMULTISAMPLE_TYPE(tb->m_desc.m_rt.m_MSAALevel),
			0,
			FALSE,
			&tb->m_surfaceHandle,
			NULL));
	}

	// Copy initial data to texture

	if (initData)
	{
		const u32 numFaces = (tb->m_desc.m_type == glTexType_Cube ? 6 : 1);
		const u32 numInitDataLevels = (tb->m_desc.m_enableAutoMips ? 1 : tb->m_desc.m_numLevels);
		const u32 expectedArrayLength = numFaces * numInitDataLevels;

		UE_ASSERT(initData->m_dataArrayLength == expectedArrayLength);

		for (u32 i = 0; i < expectedArrayLength; i++)
		{
			const u32 face = i / numInitDataLevels;
			const u32 level = i % numInitDataLevels;
			glTextureBuffer_Update(tb, face, level, NULL, &initData->m_dataArray[i]);
		}
	}
}

u32 glTextureBuffer_Update(glTextureBuffer* tb, u32 face, u32 level, const glTextureArea* area, const glTextureData* src)
{
	UE_ASSERT(!area);

	glTextureData dst;
	if (!glTextureBuffer_Lock(tb, &dst, face, level, glBufferLockFlags_Write, area))
		return 0;

	u32 areaDepth;
	u32 rowSize, numRows;
	if (area)
	{
		glUtils_CalcSurfaceRowParams(area->m_width, area->m_height, area->m_depth, tb->m_desc.m_format, rowSize, numRows);
		areaDepth = area->m_depth;
	}
	else
	{
		u32 levelWidth, levelHeight, levelDepth;
		glUtils_CalcMipLevelDimensions(tb->m_desc.m_width, tb->m_desc.m_height, tb->m_desc.m_depth, level, levelWidth, levelHeight, levelDepth);
		glUtils_CalcSurfaceRowParams(levelWidth, levelHeight, levelDepth, tb->m_desc.m_format, rowSize, numRows);
		areaDepth = levelDepth;
	}

	u8* srcData = (u8*) src->m_data;
	u8* dstData = (u8*) dst.m_data;
	for (u32 i = 0; i < areaDepth; i++)
	{
		if (src->m_rowPitch == dst.m_rowPitch && src->m_rowPitch == rowSize)
			ueMemCpy(dstData, srcData, rowSize * numRows);
		else
			for (u32 j = 0; j < numRows; j++)
			{
				ueMemCpy(dstData, srcData, rowSize);
				dstData += dst.m_rowPitch;
				srcData += src->m_rowPitch;
			}

		dstData += dst.m_slicePitch;
		srcData += src->m_slicePitch;
	}

	glTextureBuffer_Unlock(tb, face, level);

	return rowSize * numRows * areaDepth;
}

void glTextureBuffer_DestroyInPlace(glTextureBuffer* tb)
{
	if (tb->m_surfaceHandle)
	{
		const UINT refCount = tb->m_surfaceHandle->Release();
//		UE_ASSERT(refCount == 0);
		tb->m_surfaceHandle = NULL;
	}

	if (tb->m_textureHandle)
	{
		const UINT refCount = tb->m_textureHandle->Release();
		UE_ASSERT(refCount == 0);
		tb->m_textureHandle = NULL;
	}
}

UE_INLINE DWORD glTextureBuffer_ToD3DLockFlags(u32 lockFlags)
{
	DWORD flags = 0;
	if (lockFlags == glBufferLockFlags_Read) flags |= D3DLOCK_READONLY;
	if (lockFlags == glBufferLockFlags_Discard) flags |= D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK;
	return flags;
}

ueBool glTextureBuffer_Lock(glTextureBuffer* tb, glTextureData* result, u32 face, u32 level, u32 lockFlags, const glTextureArea* area)
{
	switch (tb->m_desc.m_type)
	{
		case glTexType_2D:
		case glTexType_Cube:
		{
			RECT d3dRect;
			if (area)
			{
				d3dRect.left = area->m_x;
				d3dRect.top = area->m_y;
				d3dRect.right = area->m_x + area->m_width;
				d3dRect.bottom = area->m_y + area->m_height;
			}

			D3DLOCKED_RECT d3dLockedRect;
			if (tb->m_desc.m_type == glTexType_2D ?
				FAILED(glTextureBuffer_GetHandle2D(tb)->LockRect(level, &d3dLockedRect, area ? &d3dRect : NULL, glTextureBuffer_ToD3DLockFlags(lockFlags))) :
				FAILED(glTextureBuffer_GetHandleCube(tb)->LockRect(glUtils_ToD3DCUBEMAP_FACES((glCubeFace) face), level, &d3dLockedRect, area ? &d3dRect : NULL, glTextureBuffer_ToD3DLockFlags(lockFlags))))
				return UE_FALSE;

			result->m_rowPitch = d3dLockedRect.Pitch;
			result->m_slicePitch = 0;
			result->m_data = d3dLockedRect.pBits;
			break;
		}
		case glTexType_3D:
		{
			D3DBOX d3dBox;
			if (area)
			{
				d3dBox.Left = area->m_x;
				d3dBox.Top = area->m_y;
				d3dBox.Front = area->m_z;
				d3dBox.Right = area->m_x + area->m_width;
				d3dBox.Bottom = area->m_y + area->m_height;
				d3dBox.Back = area->m_z + area->m_depth;
			}

			D3DLOCKED_BOX d3dLockedBox;
			if (FAILED(glTextureBuffer_GetHandle3D(tb)->LockBox(level, &d3dLockedBox, area ? &d3dBox : NULL, glTextureBuffer_ToD3DLockFlags(lockFlags))))
				return UE_FALSE;

			result->m_rowPitch = d3dLockedBox.RowPitch;
			result->m_slicePitch = d3dLockedBox.SlicePitch;
			result->m_data = d3dLockedBox.pBits;

			break;
		}
	}
	return UE_TRUE;
}

void glTextureBuffer_Unlock(glTextureBuffer* tb, u32 face, u32 level)
{
	switch (tb->m_desc.m_type)
	{
		case glTexType_2D:
			glTextureBuffer_GetHandle2D(tb)->UnlockRect(level);
			break;
		case glTexType_3D:
			glTextureBuffer_GetHandle3D(tb)->UnlockBox(level);
			break;
		case glTexType_Cube:
			glTextureBuffer_GetHandleCube(tb)->UnlockRect(glUtils_ToD3DCUBEMAP_FACES((glCubeFace) face), level);
			break;
		UE_INVALID_CASE(tb->m_desc.m_type);
	}
}

void glTextureBuffer_GenerateMips(glTextureBuffer* tb)
{
	UE_ASSERT(glTextureBuffer_IsTexture(tb) && tb->m_desc.m_enableManualMips);
	if (!tb->m_desc.m_enableAutoMips) // Otherwise it's generated automatically
		tb->m_textureHandle->GenerateMipSubLevels();
}

void glTextureBuffer_OnLostDevice(glTextureBuffer* tb)
{
	if (glTextureBuffer_IsTexture(tb) && !glTextureBuffer_IsRenderTarget(tb) && !(tb->m_desc.m_flags & glTextureBufferFlags_IsDynamic))
		return;

	glTextureBuffer_DestroyInPlace(tb);
}

void glTextureBuffer_OnResetDevice(glTextureBuffer* tb)
{
	if (glTextureBuffer_IsTexture(tb) && !glTextureBuffer_IsRenderTarget(tb) && !(tb->m_desc.m_flags & glTextureBufferFlags_IsDynamic))
		return;

	if (tb->m_isMainFrameBuffer)
		return;

	glTextureBuffer_CreateInPlace(tb);
}

#else // defined(GL_D3D9)
	UE_NO_EMPTY_FILE
#endif
#include "Base/ueBase.h"

#if defined(GL_D3D11)

#include "Graphics/glLib_Private.h"

UE_INLINE void glCtx_SetSampler(glCtx* ctx, glShaderType shaderType, u32 samplerIndex, glTextureBuffer* tb, const glSamplerParams* desc)
{
	glCtx::D3D11SamplerStateArray& stateArray = ctx->m_samplerStateArrays[shaderType];

	// Set texture

	if (ctx->m_samplers[shaderType][samplerIndex] != tb)
	{
		ctx->m_samplers[shaderType][samplerIndex] = tb;
		stateArray.m_minDirtyShaderView = ueMin(stateArray.m_minDirtyShaderView, samplerIndex);
		stateArray.m_maxDirtyShaderView = ueMax(stateArray.m_maxDirtyShaderView, samplerIndex);
	}

	// Set sampler state
	{
		// Construct D3D11 sampler state description

		D3D11_SAMPLER_DESC out;
		out.Filter = glUtils_ToD3DTEXTUREFILTERTYPE(desc->m_minFilter, desc->m_magFilter, desc->m_mipFilter, UE_FALSE);
		out.AddressU = glUtils_ToD3DTEXTUREADDRESS(desc->m_addressU);
		out.AddressV = glUtils_ToD3DTEXTUREADDRESS(desc->m_addressV);
		out.AddressW = glUtils_ToD3DTEXTUREADDRESS(desc->m_addressW);
		out.MipLODBias = 0;
		out.MaxAnisotropy = desc->m_maxAnisotropicFilteringLevel;
		out.BorderColor[0] = ueColor32::U8ToF32(desc->m_borderColor.r);
		out.BorderColor[1] = ueColor32::U8ToF32(desc->m_borderColor.g);
		out.BorderColor[2] = ueColor32::U8ToF32(desc->m_borderColor.b);
		out.BorderColor[3] = ueColor32::U8ToF32(desc->m_borderColor.a);
		out.MinLOD = (f32) desc->m_maxMipLevel;
		out.MaxLOD = D3D11_FLOAT32_MAX;

		// Check against currently set sampler state

		if (!ueMemCmp(&ctx->m_samplerStateDescs[shaderType][samplerIndex], &out, sizeof(out)))
			return;

		// Look up existing state or create new one

		ID3D11SamplerState* state = NULL;
		ID3D11SamplerState** statePtr = GLDEV->m_samplerStateCache.Find(out);
		if (statePtr)
			state = *statePtr;
		else
		{
			// Create new state

			UE_ASSERTWinCall(D3DEV->CreateSamplerState(&out, &state));
			GLDEV->m_samplerStateCache.Insert(out, state);
		}

		// Store state and state description
		
		ueMemCpy(&ctx->m_samplerStateDescs[shaderType][samplerIndex], &out, sizeof(CD3D11_SAMPLER_DESC));
		stateArray.m_states[samplerIndex] = state;
		stateArray.m_minDirtyState = ueMin(stateArray.m_minDirtyState, samplerIndex);
		stateArray.m_maxDirtyState = ueMax(stateArray.m_maxDirtyState, samplerIndex);
	}
}

// Context

void glCtx_SetStream(glCtx* ctx, u32 streamIndex, glVertexBuffer* vertexBuffer, glStreamFormat* format, u32 offset, u32 divider)
{
	if (ctx->m_vertexStreamFormats[streamIndex] != format)
	{
		ctx->m_vertexDeclDirty = UE_TRUE;
		ctx->m_vertexStreamFormats[streamIndex] = format;
	}

	if (ctx->m_vertexStreamInfos[streamIndex].m_VB != vertexBuffer ||
		ctx->m_vertexStreamInfos[streamIndex].m_offset != offset ||
		ctx->m_vertexStreamInfos[streamIndex].m_divider != divider)
	{
		ctx->m_vertexStreamInfos[streamIndex].m_VB = vertexBuffer;
		ctx->m_vertexStreamInfos[streamIndex].m_offset = offset;
		ctx->m_vertexStreamInfos[streamIndex].m_divider = divider;

		// TODO: Optimize by setting multiple objects at once

		if (vertexBuffer)
			ctx->m_handle->IASetVertexBuffers(streamIndex, 1, &vertexBuffer->m_handle, &format->m_stride, &offset);
	}
}

void glCtx_SetIndices(glCtx* ctx, glIndexBuffer* indexBuffer)
{
	if (ctx->m_indexBuffer == indexBuffer)
		return;

	ctx->m_indexBuffer = indexBuffer;

	if (indexBuffer)
	{
		const DXGI_FORMAT format = indexBuffer->m_desc.m_indexSize == 4 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		ctx->m_handle->IASetIndexBuffer(indexBuffer->m_handle, format, 0);
	}
}

void glCtx_SetProgram(glCtx* ctx, glProgram* program)
{
	if (ctx->m_program == program)
		return;

	ctx->m_program = program;

	if (!program)
	{
		for (u32 i = 0; i < glShaderType_MAX; i++)
			if (ctx->m_shaderInfos[i].m_shader)
			{
				ctx->m_shaderInfos[i].m_shader = NULL;
				ctx->m_shaderInfos[i].m_isDirty = UE_TRUE;
				if (i == glShaderType_Vertex)
					ctx->m_vertexDeclDirty = UE_TRUE;
			}
	}
	else
		for (u32 i = 0; i < glShaderType_MAX; i++)
			if (ctx->m_shaderInfos[i].m_shader != program->m_shaders[i])
			{
				ctx->m_shaderInfos[i].m_shader = program->m_shaders[i];
				ctx->m_shaderInfos[i].m_isDirty = UE_TRUE;
				if (i == glShaderType_Vertex)
					ctx->m_vertexDeclDirty = UE_TRUE;
			}
}

void glCtx_CommitProgram(glCtx* ctx)
{
	for (u32 i = 0; i < glShaderType_MAX; i++)
	{
		glCtx::ShaderInfo& shaderInfo = ctx->m_shaderInfos[i];
		glShader* shader = shaderInfo.m_shader;

		if (!shaderInfo.m_isDirty)
			continue;

		switch (i)
		{
			case glShaderType_Vertex:
				ctx->m_handle->VSSetShader(shader ? shader->GetVSHandle() : NULL, NULL, 0);
				break;
			case glShaderType_Fragment:
				ctx->m_handle->PSSetShader(shader ? shader->GetPSHandle() : NULL, NULL, 0);
				break;
			case glShaderType_Geometry:
				ctx->m_handle->GSSetShader(shader ? shader->GetGSHandle() : NULL, NULL, 0);
				break;
			UE_INVALID_CASE(i);
		}

		shaderInfo.m_isDirty = UE_FALSE;
	}
}

void glCtx_SetViewport(glCtx* ctx, const glViewportDesc& viewport)
{
	D3D11_VIEWPORT d3dViewport;
	d3dViewport.TopLeftX = (f32) viewport.m_left;
	d3dViewport.TopLeftY = (f32) viewport.m_top;
	d3dViewport.Width = (f32) viewport.m_width;
	d3dViewport.Height = (f32) viewport.m_height;
	d3dViewport.MinDepth = viewport.m_minZ;
	d3dViewport.MaxDepth = viewport.m_maxZ;
	ctx->m_handle->RSSetViewports(1, &d3dViewport);
}

void glCtx_SetClearColor(glCtx* ctx, f32 red, f32 green, f32 blue, f32 alpha)
{
	ctx->m_clearColorRed = red;
	ctx->m_clearColorGreen = green;
	ctx->m_clearColorBlue = blue;
	ctx->m_clearColorAlpha = alpha;
}

void glCtx_SetClearDepthStencil(glCtx* ctx, f32 depth, u32 stencil)
{
	ctx->m_clearDepth = depth;
	ctx->m_clearStencil = stencil;
}

void glCtx_Clear(glCtx* ctx, u32 clearFlags)
{
	UE_ASSERT_MSG(ctx->m_renderGroup, "Attempted to clear buffers while there's no render buffer group set.");
	UE_ASSERT_MSG(!(clearFlags & glClearFlag_Color) || ctx->m_renderGroup->m_colorRenderTargets[0], "Attempted to clear color while there's no color buffer bound.");
	UE_ASSERT_MSG(!(clearFlags & (glClearFlag_Depth | glClearFlag_Stencil)) || ctx->m_renderGroup->m_depthStencilRenderTarget, "Attempted to clear depth while there's no depth buffer bound.");

	if (clearFlags == 0) return;

	if ((clearFlags & (glClearFlag_Depth | glClearFlag_Stencil)) && ctx->m_renderGroup->m_desc.m_hasDepthStencil)
		ctx->m_handle->ClearDepthStencilView(
			ctx->m_renderGroup->m_depthStencilRenderTarget->m_depthStencilView,
			((clearFlags & glClearFlag_Depth) ? D3D11_CLEAR_DEPTH : 0) |
			((clearFlags & glClearFlag_Stencil) ? D3D11_CLEAR_STENCIL : 0),
			ctx->m_clearDepth,
			ctx->m_clearStencil);

	if (clearFlags & glClearFlag_Color)
		for (u32 i = 0; i < ctx->m_renderGroup->m_desc.m_numColorBuffers; i++)
			ctx->m_handle->ClearRenderTargetView(ctx->m_renderGroup->m_colorRenderTargets[i]->m_renderTargetView, &ctx->m_clearColorRed);
}

template <typename TYPE>
UE_INLINE ueBool glCtx_SetGenericParam(glCtx* ctx, TYPE& dst, TYPE src)
{
	if (dst == src)
		return UE_FALSE;

	dst = src;
	return UE_TRUE;
}

void glCtx_SetColorWrite(glCtx* ctx, u32 colorTargetIndex, u32 colorMask)
{
	ctx->m_isBlendStateDirty |= glCtx_SetGenericParam(
		ctx,
		ctx->m_blendStateDesc.RenderTarget[colorTargetIndex].RenderTargetWriteMask,
		(UINT8) (((colorMask & glColorMask_Red) ? D3D11_COLOR_WRITE_ENABLE_RED : 0) |
		((colorMask & glColorMask_Green) ? D3D11_COLOR_WRITE_ENABLE_GREEN : 0) |
		((colorMask & glColorMask_Blue) ? D3D11_COLOR_WRITE_ENABLE_BLUE : 0) |
		((colorMask & glColorMask_Alpha) ? D3D11_COLOR_WRITE_ENABLE_ALPHA : 0)));
}

void glCtx_SetAlphaTest(glCtx* ctx, ueBool enable)
{
	UE_ASSERT_MSG(!enable, "Alpha test on D3D11 is only supported from shader code");
}

void glCtx_SetAlphaFunc(glCtx* ctx, glCmpFunc alphaFunction, u32 referenceValue)
{
	UE_ASSERT(!"Alpha test on D3D11 is only supported from shader code");
}

void glCtx_SetBlending(glCtx* ctx, ueBool enable)
{
	ctx->m_isBlendStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_blendStateDesc.RenderTarget[0].BlendEnable, enable ? TRUE : FALSE);
}

void glCtx_SetBlendOp(glCtx* ctx, glBlendOp mode)
{
	ctx->m_isBlendStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_blendStateDesc.RenderTarget[0].BlendOp, glUtils_ToD3DBLENDOP(mode));
}

void glCtx_SetBlendOp(glCtx* ctx, glBlendOp colorOp, glBlendOp alphaOp)
{
	ctx->m_isBlendStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_blendStateDesc.RenderTarget[0].BlendOp, glUtils_ToD3DBLENDOP(colorOp));
	ctx->m_isBlendStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_blendStateDesc.RenderTarget[0].BlendOpAlpha, glUtils_ToD3DBLENDOP(alphaOp));
}

void glCtx_SetBlendFunc(glCtx* ctx, glBlendingFunc src, glBlendingFunc dst)
{
	ctx->m_isBlendStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_blendStateDesc.RenderTarget[0].SrcBlend, glUtils_ToD3DBLEND(src));
	ctx->m_isBlendStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_blendStateDesc.RenderTarget[0].DestBlend, glUtils_ToD3DBLEND(dst));
}

void glCtx_SetBlendFunc(glCtx* ctx, glBlendingFunc srcColor, glBlendingFunc dstColor, glBlendingFunc srcAlpha, glBlendingFunc dstAlpha)
{
	ctx->m_isBlendStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_blendStateDesc.RenderTarget[0].SrcBlend, glUtils_ToD3DBLEND(srcColor));
	ctx->m_isBlendStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_blendStateDesc.RenderTarget[0].DestBlend, glUtils_ToD3DBLEND(dstColor));
	ctx->m_isBlendStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_blendStateDesc.RenderTarget[0].SrcBlendAlpha, glUtils_ToD3DBLEND(srcAlpha));
	ctx->m_isBlendStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_blendStateDesc.RenderTarget[0].DestBlendAlpha, glUtils_ToD3DBLEND(dstAlpha));
}

void glCtx_SetCullMode(glCtx* ctx, glCullMode mode)
{
	D3D11_CULL_MODE d3dCull;
	switch (mode)
	{
		case glCullMode_CW: d3dCull = D3D11_CULL_FRONT; break;
		case glCullMode_CCW: d3dCull = D3D11_CULL_BACK; break;
		case glCullMode_None: d3dCull = D3D11_CULL_NONE; break;
		UE_INVALID_CASE(mode);
	}

	ctx->m_isRasterStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_rasterStateDesc.CullMode, d3dCull);
}

void glCtx_SetDepthTest(glCtx* ctx, ueBool enable)
{
	ctx->m_isDepthStencilStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_depthStencilStateDesc.DepthEnable, enable ? TRUE : FALSE);
}

void glCtx_SetDepthWrite(glCtx* ctx, ueBool enable)
{
	ctx->m_isDepthStencilStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_depthStencilStateDesc.DepthWriteMask, enable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO);
}

void glCtx_SetDepthFunc(glCtx* ctx, glCmpFunc depthFunction)
{
	ctx->m_isDepthStencilStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_depthStencilStateDesc.DepthFunc, glUtils_ToD3DCMPFUNC(depthFunction));
}

void glCtx_SetFillMode(glCtx* ctx, glFillMode mode)
{
	D3D11_FILL_MODE d3dFillMode;
	switch (mode)
	{
		case glFillMode_Points: d3dFillMode = D3D11_FILL_WIREFRAME; break;
		case glFillMode_Wire: d3dFillMode = D3D11_FILL_WIREFRAME; break;
		case glFillMode_Solid: d3dFillMode = D3D11_FILL_SOLID; break;
		UE_INVALID_CASE(mode);
	}

	ctx->m_isRasterStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_rasterStateDesc.FillMode, d3dFillMode);
}

void glCtx_SetScissorTest(glCtx* ctx, ueBool enable)
{
	ctx->m_isRasterStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_rasterStateDesc.ScissorEnable, enable ? TRUE : FALSE);
}

void glCtx_SetScissorRectangle(glCtx* ctx, u32 left, u32 top, u32 width, u32 height)
{
	D3D11_RECT d3dRect;
	d3dRect.left = left;
	d3dRect.top = top;
	d3dRect.right = left + width;
	d3dRect.bottom = top + height;
	ctx->m_handle->RSSetScissorRects(1, &d3dRect);
}

void glCtx_SetStencilTest(glCtx* ctx, ueBool enable)
{
	ctx->m_isDepthStencilStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_depthStencilStateDesc.StencilEnable, enable ? TRUE : FALSE);
}

void glCtx_SetStencilFunc(glCtx* ctx, glCmpFunc stencilFunc, u32 ref, u32 mask, u32 writeMask)
{
	if (ctx->m_stencilRef != ref)
	{
		ctx->m_stencilRef = ref;
		ctx->m_isDepthStencilStateDirty = UE_TRUE;
	}
	ctx->m_isDepthStencilStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_depthStencilStateDesc.FrontFace.StencilFunc, glUtils_ToD3DCMPFUNC(stencilFunc));
	ctx->m_isDepthStencilStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_depthStencilStateDesc.StencilReadMask, (UINT8) mask);
	ctx->m_isDepthStencilStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_depthStencilStateDesc.StencilWriteMask, (UINT8) writeMask);
}

void glCtx_SetStencilOp(glCtx* ctx, glStencilOperation stencilFail, glStencilOperation depthFail, glStencilOperation depthPass)
{
	ctx->m_isDepthStencilStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_depthStencilStateDesc.FrontFace.StencilFailOp, glUtils_ToD3DSTENCILOP(stencilFail));
	ctx->m_isDepthStencilStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_depthStencilStateDesc.FrontFace.StencilDepthFailOp, glUtils_ToD3DSTENCILOP(depthFail));
	ctx->m_isDepthStencilStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_depthStencilStateDesc.FrontFace.StencilPassOp, glUtils_ToD3DSTENCILOP(depthPass));
}

void glCtx_SetTwoSidedStencilMode(glCtx* ctx, ueBool enable)
{
	UE_ASSERT_MSG(enable, "Two sided stencil mode is always on under D3D11");
}

void glCtx_SetCCWStencilFunc(glCtx* ctx, glCmpFunc stencilFunc)
{
	ctx->m_isDepthStencilStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_depthStencilStateDesc.BackFace.StencilFunc, glUtils_ToD3DCMPFUNC(stencilFunc));
}

void glCtx_SetCCWStencilOp(glCtx* ctx, glStencilOperation stencilFail, glStencilOperation depthFail, glStencilOperation depthPass)
{
	ctx->m_isDepthStencilStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_depthStencilStateDesc.BackFace.StencilFailOp, glUtils_ToD3DSTENCILOP(stencilFail));
	ctx->m_isDepthStencilStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_depthStencilStateDesc.BackFace.StencilDepthFailOp, glUtils_ToD3DSTENCILOP(depthFail));
	ctx->m_isDepthStencilStateDirty |= glCtx_SetGenericParam(ctx, ctx->m_depthStencilStateDesc.BackFace.StencilPassOp, glUtils_ToD3DSTENCILOP(depthPass));
}

void glCtx_SetVertexDeclaration(glCtx* ctx, glVertexDeclaration* decl)
{
	if (decl == ctx->m_vertexDeclaration)
		return;

	ctx->m_vertexDeclaration = decl;
	ctx->m_vertexDeclDirty = UE_FALSE;

	ctx->m_handle->IASetInputLayout(decl ? decl->m_handle : NULL);
}

ueBool glCtx_PreDraw(glCtx* ctx)
{
	glCtx_CommitRenderStates(ctx);

	// Make sure vertex shader is bound

	if (!ctx->m_shaderInfos[glShaderType_Vertex].m_shader)
	{
		ueLogE("No vertex shader set.");
		return UE_FALSE;
	}

	// Check if valid fragment shader is bound	

	glShader* fragmentShader = ctx->m_shaderInfos[glShaderType_Fragment].m_shader;
	if (fragmentShader)
	{
		// Make sure fragment shader outputs same no. elements as no. render targets bound
		if (fragmentShader->m_numOutputs != ctx->m_renderGroup->m_desc.m_numColorBuffers)
		{
			ueLogE("Fragment shader output count doesn't match number of set render targets (%d != %d).", fragmentShader->m_numOutputs, ctx->m_renderGroup->m_desc.m_numColorBuffers);
			return UE_FALSE;
		}
	}

	// ... if there's no fragment shader bound, make sure no. render targets bound is 0 and depth-stencil buffer is set

	else if (ctx->m_renderGroup->m_desc.m_numColorBuffers > 0 || ctx->m_renderGroup->m_depthStencilRenderTarget)
	{
		ueLogE("No fragment shader is set (depth only pass?) but either color buffer is set or no depth stencil is set.");
		return UE_FALSE;
	}

	return UE_TRUE;
}

template <glShaderType shaderType, void (__stdcall ID3D11DeviceContext::*D3D11_SetSamplers)(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState *const *ppSamplers), void (__stdcall ID3D11DeviceContext::*D3D11_SetShaderResources)(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView *const *ppShaderResourceViews)>
void glCtx_CommitShaderViewsAndSamplerStates(glCtx* ctx)
{
	glCtx::D3D11SamplerStateArray& stateArray = ctx->m_samplerStateArrays[shaderType];

	if (stateArray.m_minDirtyState <= stateArray.m_maxDirtyState)
	{
		(ctx->m_handle->*D3D11_SetSamplers)(
			stateArray.m_minDirtyState,
			stateArray.m_maxDirtyState - stateArray.m_minDirtyState + 1,
			stateArray.m_states + stateArray.m_minDirtyState);

		stateArray.m_minDirtyState = U32_MAX;
		stateArray.m_maxDirtyState = 0;
	}

	if (stateArray.m_minDirtyShaderView <= stateArray.m_maxDirtyShaderView)
	{
		ID3D11ShaderResourceView* shaderViews[16];
		ueMemZeroS(shaderViews);

		for (u32 i = stateArray.m_minDirtyShaderView; i <= stateArray.m_maxDirtyShaderView; i++)
		{
			glTextureBuffer* tb = ctx->m_samplers[shaderType][i];
			shaderViews[i] = tb ? tb->m_shaderView : NULL;
		}

		(ctx->m_handle->*D3D11_SetShaderResources)(
			stateArray.m_minDirtyShaderView,
			stateArray.m_maxDirtyShaderView - stateArray.m_minDirtyShaderView + 1,
			shaderViews + stateArray.m_minDirtyShaderView);

		stateArray.m_minDirtyShaderView = U32_MAX;
		stateArray.m_maxDirtyShaderView = 0;
	}
}

void glCtx_CommitConstants(glCtx* ctx)
{
	for (s32 i = 0; i < glShaderType_MAX; i++)
	{
		glCtx::ShaderInfo& shaderInfo = ctx->m_shaderInfos[i];
		glShader* shader = shaderInfo.m_shader;

		if (!shader)
			continue;

		for (u32 j = 0; j < shader->m_numConstants; j++)
		{
			const glShaderConstantInstance& constantInstance = shader->m_constants[j];
			const glConstant* constant = constantInstance.m_constant;

			// Handle sampler separately

			if (glUtils_IsSampler((glConstantType) constant->m_desc.m_type))
			{
				glCtx::SamplerConstantInfo& samplerInfo = ctx->m_samplerShaderConstantsLocal[constant->m_sampler.m_index];
				glCtx_SetSampler(ctx, (glShaderType) i, constantInstance.m_offset, samplerInfo.m_textureBuffer, &samplerInfo.m_samplerParams);
				continue;
			}

			// Handle numeric data (float, int or bool)

			const u8* srcData = &ctx->m_numericShaderConstantsLocal[constant->m_numeric.m_cacheOffset];

#define GL_D3D11_OFFSET_MASK 0xFFFF
#define GL_D3D11_BUFFER_INDEX_SHIFT 16
			glShader::BufferData& dstBuffer = shader->m_bufferDatas[constantInstance.m_offset >> GL_D3D11_BUFFER_INDEX_SHIFT];
			const u32 dstBufferOffset = constantInstance.m_offset & GL_D3D11_OFFSET_MASK;
			u8* dstData = &dstBuffer.m_memory[dstBufferOffset];

			// If constant value is the same, skip it

			if (!ueMemCmp(dstData, srcData, constant->m_numeric.m_size))
				continue;

			ueMemCpy(dstData, srcData, constant->m_numeric.m_size);
			dstBuffer.m_minDirtyOffset = ueMin(dstBuffer.m_minDirtyOffset, dstBufferOffset);
			dstBuffer.m_maxDirtyOffset = ueMax(dstBuffer.m_maxDirtyOffset, dstBufferOffset + constant->m_numeric.m_size);
		}

		// Update buffers contents

		for (u32 j = 0; j < shader->m_numBuffers; j++)
		{
			glShader::BufferData& data = shader->m_bufferDatas[j];
			if (data.m_minDirtyOffset >= data.m_maxDirtyOffset)
				continue;

#if 0 // Unfortunately DX11 doesn't allow for partial D3D11_BIND_CONSTANT_BUFFER resource updates
			D3D11_BOX dstRegion;
			dstRegion.top = dstRegion.front = 0;
			dstRegion.bottom = dstRegion.back = 1;
			dstRegion.left = data.m_minDirtyOffset;
			dstRegion.right = data.m_maxDirtyOffset;

			ctx->m_handle->UpdateSubresource(shader->m_buffers[j], 0, &dstRegion, (u8*) data.m_memory + data.m_minDirtyOffset, 0, 0);
#else
			ctx->m_handle->UpdateSubresource(shader->m_buffers[j], 0, NULL, data.m_memory, 0, 0);
#endif
			
			data.m_minDirtyOffset = U32_MAX;
			data.m_maxDirtyOffset = 0;
		}

		// Set all buffers at once

		switch (i)
		{
		case glShaderType_Geometry:
			ctx->m_handle->GSSetConstantBuffers(0, shader->m_numBuffers, shader->m_buffers);
			break;
		case glShaderType_Vertex:
			ctx->m_handle->VSSetConstantBuffers(0, shader->m_numBuffers, shader->m_buffers);
			break;
		case glShaderType_Fragment:
			ctx->m_handle->PSSetConstantBuffers(0, shader->m_numBuffers, shader->m_buffers);
			break;
		}
	}

	// Commit shader views and sampler states in batches

	glCtx_CommitShaderViewsAndSamplerStates<glShaderType_Vertex, &ID3D11DeviceContext::VSSetSamplers, &ID3D11DeviceContext::VSSetShaderResources>(ctx);
	glCtx_CommitShaderViewsAndSamplerStates<glShaderType_Geometry, &ID3D11DeviceContext::GSSetSamplers, &ID3D11DeviceContext::GSSetShaderResources>(ctx);
	glCtx_CommitShaderViewsAndSamplerStates<glShaderType_Fragment, &ID3D11DeviceContext::PSSetSamplers, &ID3D11DeviceContext::PSSetShaderResources>(ctx);
}

void glCtx_CommitRenderStates(glCtx* ctx)
{
	// Commit depth-stencil state

	if (ctx->m_isDepthStencilStateDirty)
	{
		ID3D11DepthStencilState* state = NULL;
		ID3D11DepthStencilState** statePtr = GLDEV->m_depthStencilStateCache.Find(ctx->m_depthStencilStateDesc);
		if (statePtr)
			state = *statePtr;
		else
		{
			UE_ASSERTWinCall(D3DEV->CreateDepthStencilState(&ctx->m_depthStencilStateDesc, &state));
			GLDEV->m_depthStencilStateCache.Insert(ctx->m_depthStencilStateDesc, state);
		}

		ctx->m_handle->OMSetDepthStencilState(state, ctx->m_stencilRef);
		ctx->m_isDepthStencilStateDirty = UE_FALSE;
	}

	// Commit blend state

	if (ctx->m_isBlendStateDirty)
	{
		ID3D11BlendState* state = NULL;
		ID3D11BlendState** statePtr = GLDEV->m_blendStateCache.Find(ctx->m_blendStateDesc);
		if (statePtr)
			state = *statePtr;
		else
		{
			UE_ASSERTWinCall(D3DEV->CreateBlendState(&ctx->m_blendStateDesc, &state));
			GLDEV->m_blendStateCache.Insert(ctx->m_blendStateDesc, state);
		}

		ctx->m_handle->OMSetBlendState(state, ctx->m_blendFactor, 0xffffffff);
		ctx->m_isBlendStateDirty = UE_FALSE;
	}

	// Commit raster state

	if (ctx->m_isRasterStateDirty)
	{
		ID3D11RasterizerState* state = NULL;
		ID3D11RasterizerState** statePtr = GLDEV->m_rasterStateCache.Find(ctx->m_rasterStateDesc);
		if (statePtr)
			state = *statePtr;
		else
		{
			UE_ASSERTWinCall(D3DEV->CreateRasterizerState(&ctx->m_rasterStateDesc, &state));
			GLDEV->m_rasterStateCache.Insert(ctx->m_rasterStateDesc, state);
		}

		ctx->m_handle->RSSetState(state);
		ctx->m_isRasterStateDirty = UE_FALSE;
	}

	// Set vertex declaration

	if (ctx->m_vertexDeclDirty)
	{
		glShader* vs = ctx->m_shaderInfos[glShaderType_Vertex].m_shader;
		if (!vs)
			glCtx_SetVertexDeclaration(ctx, NULL);
		else
		{
			u32 numFormats = 0;
			for (u32 i = 0; i < GL_MAX_BOUND_VERTEX_STREAMS; i++)
				if (ctx->m_vertexStreamFormats[i])
					numFormats = i + 1;

			glVertexDeclaration* decl = glVertexShaderInput_GetVertexDeclaration(vs, numFormats, ctx->m_vertexStreamFormats);
			glCtx_SetVertexDeclaration(ctx, decl);
		}
	}

	// Set shaders

	glCtx_CommitProgram(ctx);

	// Commit shader constants

	glCtx_CommitConstants(ctx);
}

// Context

void glCtx_Init(glCtx* ctx)
{
	glCtx_Base_Init(ctx);

	// Render states

	new(&ctx->m_depthStencilStateDesc) CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
	ctx->m_stencilRef = 0;

	new(&ctx->m_rasterStateDesc) CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());

	new(&ctx->m_blendStateDesc) CD3D11_BLEND_DESC(CD3D11_DEFAULT());
	ueMemZeroS(ctx->m_blendFactor);

	// Samplers and sampler states

	for (u32 i = 0; i < glShaderType_MAX; i++)
	{
		ctx->m_numSamplers[i] = GLCAPS->m_maxSamplers[i];
		if (ctx->m_numSamplers[i] == 0)
			continue;

		ctx->m_samplers[i] = (glTextureBuffer**) GL_STACK_ALLOC->Alloc(sizeof(glTextureBuffer*) * ctx->m_numSamplers[i]);
		UE_ASSERT(ctx->m_samplers[i]);
		ueMemZero(ctx->m_samplers[i], ctx->m_numSamplers[i] * sizeof(glTextureBuffer*));

		ctx->m_samplerStateDescs[i] = (CD3D11_SAMPLER_DESC*) GL_STACK_ALLOC->Alloc(sizeof(CD3D11_SAMPLER_DESC) * ctx->m_numSamplers[i]);
		UE_ASSERT(ctx->m_samplerStateDescs[i]);
		for (u32 j = 0; j < ctx->m_numSamplers[i]; j++)
			new (&ctx->m_samplerStateDescs[i][j]) CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());

		glCtx::D3D11SamplerStateArray& stateArray = ctx->m_samplerStateArrays[i];

		stateArray.m_minDirtyShaderView = U32_MAX;
		stateArray.m_maxDirtyShaderView = 0;

		stateArray.m_minDirtyState = U32_MAX;
		stateArray.m_maxDirtyState = 0;

		stateArray.m_states = (ID3D11SamplerState**) GL_STACK_ALLOC->Alloc(sizeof(ID3D11SamplerState*) * ctx->m_numSamplers[i]);
		UE_ASSERT(stateArray.m_states);
		ueMemZero(stateArray.m_states, sizeof(ID3D11SamplerState*) * ctx->m_numSamplers[i]);
	}
}

void glCtx_Deinit(glCtx* ctx)
{
	for (s32 i = glShaderType_MAX - 1; i >= 0; i--)
		if (ctx->m_numSamplers[i] > 0)
		{
			GL_STACK_ALLOC->Free(ctx->m_samplerStateArrays[i].m_states);
			GL_STACK_ALLOC->Free(ctx->m_samplerStateDescs[i]);
			GL_STACK_ALLOC->Free(ctx->m_samplers[i]);
		}

	glCtx_Base_Deinit(ctx);

	ctx->m_handle->ClearState();
	ctx->m_handle->Flush();

	const UINT refCount = ctx->m_handle->Release();
	UE_ASSERT(refCount == 0);
}

void glCtx_Reset(glCtx* ctx, u32 flags)
{
	const glDeviceStartupParams* params = glDevice_GetStartupParams();

	if (flags & glResetFlags_Streams)
	{
		glCtx_SetVertexDeclaration(ctx, NULL);
		for (u32 i = 0; i < GL_MAX_BOUND_VERTEX_STREAMS; i++)
			glCtx_SetStream(ctx, i, NULL, NULL);
		glCtx_SetIndices(ctx, NULL);
	}

	if (flags & glResetFlags_RenderTargets)
		glCtx_SetRenderGroup(ctx, NULL);

	if (flags & glResetFlags_Shaders)
	{
		glCtx_SetProgram(ctx, NULL);
		glCtx_CommitProgram(ctx);
	}

#if 0 // FIXME
	if (flags)
	{
		ctx->m_handle->ClearState();

		new(&ctx->m_depthStencilStateDesc) CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
		ctx->m_stencilRef = 0;

		new(&ctx->m_rasterStateDesc) CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());

		new(&ctx->m_blendStateDesc) CD3D11_BLEND_DESC(CD3D11_DEFAULT());
		ueMemZeroS(ctx->m_blendFactor);

		for (u32 i = 0; i < glShaderType_MAX; i++)
			for (u32 j = 0; j < ctx->m_numSamplers[i]; j++)
				new (&ctx->m_samplerStateDescs[i][j]) CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
	}
#endif

	if (flags & glResetFlags_Samplers)
	{
		ID3D11ShaderResourceView* nullShaderViews[16];
		ueMemZeroS(nullShaderViews);

		ctx->m_handle->VSSetShaderResources(0, UE_ARRAY_SIZE(nullShaderViews), nullShaderViews);
		ctx->m_handle->GSSetShaderResources(0, UE_ARRAY_SIZE(nullShaderViews), nullShaderViews);
		ctx->m_handle->PSSetShaderResources(0, UE_ARRAY_SIZE(nullShaderViews), nullShaderViews);

		for (u32 i = 0; i < glShaderType_MAX; i++)
		{
			glCtx::D3D11SamplerStateArray& stateArray = ctx->m_samplerStateArrays[i];
			stateArray.m_minDirtyShaderView = U32_MAX;
			stateArray.m_maxDirtyShaderView = 0;

			ueMemZero(ctx->m_samplers[i], ctx->m_numSamplers[i] * sizeof(glTextureBuffer*));
		}

		for (u32 i = 0; i < params->m_maxGlobalSamplerConstants; i++)
			ctx->m_samplerShaderConstantsLocal[i].m_textureBuffer = NULL;
	}
}

void glCtx_SetRenderGroup(glCtx* ctx, glRenderGroup* group)
{
	// Unset all samplers to avoid possibility of them being bound as render targets too

	glCtx_Reset(ctx, glResetFlags_Samplers);

	// Set render targets

	if (group)
	{
		ctx->m_renderGroup = group;

		ID3D11RenderTargetView* renderTargetViews[GL_MAX_COLOR_RENDER_TARGETS];
		for (u32 i = 0; i < group->m_desc.m_numColorBuffers; i++)
			renderTargetViews[i] = group->m_colorRenderTargets[i]->m_renderTargetView;
		ctx->m_handle->OMSetRenderTargets(
			group->m_desc.m_numColorBuffers,
			renderTargetViews,
			group->m_desc.m_hasDepthStencil ? group->m_depthStencilRenderTarget->m_depthStencilView : NULL);

		glCtx_SetGenericParam(ctx, ctx->m_rasterStateDesc.MultisampleEnable, group->m_desc.m_MSAALevel > 1 ? TRUE : FALSE);
	}
	else
	{
		ctx->m_renderGroup = NULL;
		ctx->m_handle->OMSetRenderTargets(0, NULL, NULL);

		glCtx_SetGenericParam(ctx, ctx->m_rasterStateDesc.MultisampleEnable, FALSE);
	}
}

ueBool glCtx_Begin(glCtx* ctx)
{
	return UE_TRUE;
}

void glCtx_End(glCtx* ctx)
{
	// Release unnecessary references to resources

	glCtx_Reset(ctx, glResetFlags_Resources);
}

void glCtx_Present(glCtx* ctx, glTextureBuffer* tb)
{
	UE_ASSERT_MSG(tb == glDevice_GetBackBuffer(), "On D3D11 can only present using back buffer.");
	GLDEV->m_swapChain->Present(0, 0);
}

void glCtx_DrawIndexed(glCtx* ctx, glPrimitive primitive, u32 baseVertexIndex, u32 minVertexIndex, u32 numVerts, u32 startIndex, u32 numIndices)
{
	if (!glCtx_PreDraw(ctx)) return;
	ctx->m_handle->IASetPrimitiveTopology(glUtils_ToD3DPRIMITIVETYPE(primitive));
	ctx->m_handle->DrawIndexed(numIndices, startIndex, baseVertexIndex);
}

void glCtx_Draw(glCtx* ctx, glPrimitive primitive, u32 startVertex, u32 numVerts)
{
	if (!glCtx_PreDraw(ctx)) return;
	ctx->m_handle->IASetPrimitiveTopology(glUtils_ToD3DPRIMITIVETYPE(primitive));
	ctx->m_handle->Draw(numVerts, startVertex);
}

void glCtx_ResolveRenderTargets(glCtx* ctx)
{
	UE_ASSERT(ctx->m_renderGroup);

	glRenderGroup* group = ctx->m_renderGroup;

	if (group->m_desc.m_MSAALevel > 1)
	{
		if (group->m_desc.m_needDepthStencilContent)
		{
			glTextureBuffer* dstTexture = group->m_depthStencilTexture;

			DXGI_FORMAT d3dFormat;
			UE_ASSERT_FUNC( glUtils_ToD3DFORMAT(group->m_depthStencilTexture->m_desc.m_format, d3dFormat) );

			ctx->m_handle->ResolveSubresource(
				group->m_depthStencilTexture->m_textureHandle, 0,
				group->m_depthStencilRenderTarget->m_textureHandle, 0,
				d3dFormat);
		}

		if (group->m_desc.m_needColorContent)
			for (u32 i = 0; i < group->m_desc.m_numColorBuffers; i++)
			{
				DXGI_FORMAT d3dFormat;
				UE_ASSERT_FUNC( glUtils_ToD3DFORMAT(group->m_colorRenderTargets[i]->m_desc.m_format, d3dFormat) );

				ctx->m_handle->ResolveSubresource(
					group->m_colorTextures[i]->m_textureHandle, 0,
					group->m_colorRenderTargets[i]->m_textureHandle, 0,
					d3dFormat);
			}
	}

	// Generate mip tails if needed

	if (group->m_desc.m_hasDepthStencil && group->m_depthStencilRenderTarget->m_desc.m_enableAutoMips)
		ctx->m_handle->GenerateMips(group->m_depthStencilRenderTarget->m_shaderView);

	for (u32 i = 0; i < group->m_desc.m_numColorBuffers; i++)
	{
		glTextureBuffer* tb = group->m_colorRenderTargets[i];
		if (tb->m_desc.m_enableAutoMips)
			ctx->m_handle->GenerateMips(tb->m_shaderView);
	}
}

ueBool glCtx_CopyBuffer(glCtx* ctx, glTextureBuffer* src, glTextureBuffer* dst)
{
	UE_ASSERT((src->m_desc.m_flags & glTextureBufferFlags_IsTexture) && src->m_desc.m_type == glTexType_2D);
	UE_ASSERT((dst->m_desc.m_flags & glTextureBufferFlags_IsTexture) && dst->m_desc.m_type == glTexType_2D);
#if 0
	IDirect3DTexture9* sourceTexture = (IDirect3DTexture9*) src->m_textureHandle;
	IDirect3DSurface9* sourceSurface = NULL;
	HRESULT hr = sourceTexture->GetSurfaceLevel(0, &sourceSurface);
	if (FAILED(hr))
		return UE_FALSE;

	IDirect3DTexture9* destTexture = (IDirect3DTexture9*) dst->m_textureHandle;
	IDirect3DSurface9* destSurface = NULL;
	hr = destTexture->GetSurfaceLevel(0, &destSurface);
	if (FAILED(hr))
	{
		sourceSurface->Release();
		return UE_FALSE;
	}

	hr = ctx->m_handle->StretchRect(sourceSurface, NULL, destSurface, NULL, D3DTEXF_LINEAR);

	sourceSurface->Release();
	destSurface->Release();
	return SUCCEEDED(hr);
#else
	UE_ASSERT(!"Not yet implemented");
	return UE_FALSE;
#endif
}

#else // defined(GL_D3D11)
	UE_NO_EMPTY_FILE
#endif
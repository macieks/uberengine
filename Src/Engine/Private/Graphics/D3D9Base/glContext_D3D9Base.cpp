#include "Base/ueBase.h"

#if defined(GL_D3D9) || defined(GL_X360)

#include "Graphics/glLib_Private.h"

// D3D9Base Context

void glCtx_SetRenderState_Immediate(glCtx_D3D9Base* ctx, D3DRENDERSTATETYPE renderState, DWORD value)
{
	if (!ctx->m_renderStates[renderState].Set(value)) return;
#if !defined(UE_TOOL_X360)
	ctx->m_handle->SetRenderState(renderState, value);
#endif // !defined(UE_TOOL_X360)
}

void glCtx_SetSamplerState_Immediate(glCtx_D3D9Base* ctx, glShaderType shaderType, u32 samplerIndex, D3DSAMPLERSTATETYPE samplerState, DWORD value)
{
	UE_ASSERT(shaderType == glShaderType_Vertex || shaderType == glShaderType_Fragment);

	if (!ctx->m_samplerStates[shaderType][samplerIndex].m_states[samplerState].Set(value)) return;

#if !defined(UE_TOOL_X360)
	u32 deviceSamplerIndex = samplerIndex;
	if (shaderType == glShaderType_Vertex)
	{
		UE_ASSERT(samplerIndex < 4); // D3D9 limitation
		deviceSamplerIndex += D3DVERTEXTEXTURESAMPLER0;
	}

	ctx->m_handle->SetSamplerState(deviceSamplerIndex, samplerState, value);
#endif // !defined(UE_TOOL_X360)
}

void glCtx_SetSamplerState_Immediate(glCtx_D3D9Base* ctx, glShaderType shaderType, u32 samplerIndex, const glSamplerParams* desc)
{
	glCtx_SetSamplerState_Immediate(ctx, shaderType, samplerIndex, D3DSAMP_ADDRESSU, glUtils_ToD3DTEXTUREADDRESS(desc->m_addressU));
	glCtx_SetSamplerState_Immediate(ctx, shaderType, samplerIndex, D3DSAMP_ADDRESSV, glUtils_ToD3DTEXTUREADDRESS(desc->m_addressV));
	glCtx_SetSamplerState_Immediate(ctx, shaderType, samplerIndex, D3DSAMP_ADDRESSW, glUtils_ToD3DTEXTUREADDRESS(desc->m_addressW));
	glCtx_SetSamplerState_Immediate(ctx, shaderType, samplerIndex, D3DSAMP_MINFILTER, glUtils_ToD3DTEXTUREFILTERTYPE(desc->m_minFilter));
	glCtx_SetSamplerState_Immediate(ctx, shaderType, samplerIndex, D3DSAMP_MAGFILTER, glUtils_ToD3DTEXTUREFILTERTYPE(desc->m_magFilter));
	glCtx_SetSamplerState_Immediate(ctx, shaderType, samplerIndex, D3DSAMP_MIPFILTER, glUtils_ToD3DTEXTUREFILTERTYPE(desc->m_mipFilter));
	glCtx_SetSamplerState_Immediate(ctx, shaderType, samplerIndex, D3DSAMP_MAXANISOTROPY, desc->m_maxAnisotropicFilteringLevel);
	glCtx_SetSamplerState_Immediate(ctx, shaderType, samplerIndex, D3DSAMP_MAXMIPLEVEL, desc->m_maxMipLevel);
	glCtx_SetSamplerState_Immediate(ctx, shaderType, samplerIndex, D3DSAMP_BORDERCOLOR, desc->m_borderColor.AsRGBA());
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

		ctx->m_handle->SetStreamSource(streamIndex, vertexBuffer ? vertexBuffer->m_handle : NULL, offset, format ? format->m_stride : 0);
	}
}

void glCtx_SetIndices(glCtx* ctx, glIndexBuffer* indexBuffer)
{
	if (ctx->m_indexBuffer == indexBuffer)
		return;

	ctx->m_indexBuffer = indexBuffer;
	ctx->m_handle->SetIndices(indexBuffer ? indexBuffer->m_handle : NULL);
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
			case glShaderType_Vertex: ctx->m_handle->SetVertexShader( shader ? shader->GetVSHandle() : NULL ); break;
			case glShaderType_Fragment: ctx->m_handle->SetPixelShader( shader ? shader->GetPSHandle() : NULL ); break;
			case glShaderType_Geometry: UE_ASSERT(!shader); break;
			UE_INVALID_CASE(i);
		}

		shaderInfo.m_isDirty = UE_FALSE;
	}
}

void glCtx_SetViewport(glCtx* ctx, const glViewportDesc& viewport)
{
	D3DVIEWPORT9 d3dViewport;
	d3dViewport.X = viewport.m_left;
	d3dViewport.Y = viewport.m_top;
	d3dViewport.Width = viewport.m_width;
	d3dViewport.Height = viewport.m_height;
	d3dViewport.MinZ = viewport.m_minZ;
	d3dViewport.MaxZ = viewport.m_maxZ;
	ctx->m_handle->SetViewport(&d3dViewport);
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

	const DWORD d3dClearFlags =
		((clearFlags & glClearFlag_Color) ? D3DCLEAR_TARGET : 0) |
		((clearFlags & glClearFlag_Depth) ? D3DCLEAR_ZBUFFER : 0) |
		((clearFlags & glClearFlag_Stencil) ? D3DCLEAR_STENCIL : 0);
	const D3DCOLOR d3dClearColor = ueColor32::F(ctx->m_clearColorRed, ctx->m_clearColorGreen, ctx->m_clearColorBlue, ctx->m_clearColorAlpha).AsARGB();
	ctx->m_handle->Clear(0, NULL, d3dClearFlags, d3dClearColor, ctx->m_clearDepth, ctx->m_clearStencil);
}

void glCtx_SetColorWrite(glCtx* ctx, u32 colorTargetIndex, u32 colorMask)
{
	const DWORD d3dMask =
		((colorMask & glColorMask_Red) ? D3DCOLORWRITEENABLE_RED : 0) |
		((colorMask & glColorMask_Green) ? D3DCOLORWRITEENABLE_GREEN : 0) |
		((colorMask & glColorMask_Blue) ? D3DCOLORWRITEENABLE_BLUE : 0) |
		((colorMask & glColorMask_Alpha) ? D3DCOLORWRITEENABLE_ALPHA : 0);

	if (colorTargetIndex == 0)
		glCtx_SetRenderState_Immediate(ctx, D3DRS_COLORWRITEENABLE, d3dMask);
	else
	{
		UE_ASSERT( glDevice_GetCaps()->m_supportsIndependentColorWriteMasks );

		D3DRENDERSTATETYPE d3dColorWriteRS;
		switch (colorTargetIndex)
		{
			case 1: d3dColorWriteRS = D3DRS_COLORWRITEENABLE1; break;
			case 2: d3dColorWriteRS = D3DRS_COLORWRITEENABLE2; break;
			case 3: d3dColorWriteRS = D3DRS_COLORWRITEENABLE3; break;
			default:
				UE_ASSERT(0);
		}
		glCtx_SetRenderState_Immediate(ctx, d3dColorWriteRS, d3dMask);
	}
}

void glCtx_SetAlphaTest(glCtx* ctx, ueBool enable)
{
	glCtx_SetRenderState_Immediate(ctx, D3DRS_ALPHATESTENABLE, enable ? TRUE : FALSE);
}

void glCtx_SetAlphaFunc(glCtx* ctx, glCmpFunc alphaFunction, u32 referenceValue)
{
	UE_ASSERT(0 <= referenceValue && referenceValue <= 255);
	glCtx_SetRenderState_Immediate(ctx, D3DRS_ALPHAFUNC, glUtils_ToD3DCMPFUNC(alphaFunction));
	glCtx_SetRenderState_Immediate(ctx, D3DRS_ALPHAREF, referenceValue);
}

void glCtx_SetBlending(glCtx* ctx, ueBool enable)
{
	glCtx_SetRenderState_Immediate(ctx, D3DRS_ALPHABLENDENABLE, enable ? TRUE : FALSE);
}

void glCtx_SetBlendOp(glCtx* ctx, glBlendOp mode)
{
	glCtx_SetRenderState_Immediate(ctx, D3DRS_BLENDOP, glUtils_ToD3DBLENDOP(mode)); 
}

void glCtx_SetBlendOp(glCtx* ctx, glBlendOp colorOp, glBlendOp alphaOp)
{
	glCtx_SetRenderState_Immediate(ctx, D3DRS_BLENDOP, glUtils_ToD3DBLENDOP(colorOp)); 
	glCtx_SetRenderState_Immediate(ctx, D3DRS_BLENDOPALPHA, glUtils_ToD3DBLENDOP(alphaOp)); 
}

void glCtx_SetBlendFunc(glCtx* ctx, glBlendingFunc src, glBlendingFunc dest)
{
	glCtx_SetRenderState_Immediate(ctx, D3DRS_SRCBLEND, glUtils_ToD3DBLEND(src));
	glCtx_SetRenderState_Immediate(ctx, D3DRS_DESTBLEND, glUtils_ToD3DBLEND(dest));
}

void glCtx_SetBlendFunc(glCtx* ctx, glBlendingFunc srcColor, glBlendingFunc destColor, glBlendingFunc srcAlpha, glBlendingFunc destAlpha)
{
	glCtx_SetRenderState_Immediate(ctx, D3DRS_SRCBLEND, glUtils_ToD3DBLEND(srcColor));
	glCtx_SetRenderState_Immediate(ctx, D3DRS_DESTBLEND, glUtils_ToD3DBLEND(destColor));
	glCtx_SetRenderState_Immediate(ctx, D3DRS_SRCBLENDALPHA, glUtils_ToD3DBLEND(srcAlpha));
	glCtx_SetRenderState_Immediate(ctx, D3DRS_DESTBLENDALPHA, glUtils_ToD3DBLEND(destAlpha));
}

void glCtx_SetCullMode(glCtx* ctx, glCullMode mode)
{
	D3DCULL d3dCull;
	switch (mode)
	{
		case glCullMode_CW: d3dCull = D3DCULL_CW; break;
		case glCullMode_CCW: d3dCull = D3DCULL_CCW; break;
		case glCullMode_None: d3dCull = D3DCULL_NONE; break;
	}
	glCtx_SetRenderState_Immediate(ctx, D3DRS_CULLMODE, d3dCull);
}

void glCtx_SetDepthTest(glCtx* ctx, ueBool enable)
{
	glCtx_SetRenderState_Immediate(ctx, D3DRS_ZENABLE, enable ? D3DZB_TRUE : D3DZB_FALSE);
}

void glCtx_SetDepthWrite(glCtx* ctx, ueBool enable)
{
	glCtx_SetRenderState_Immediate(ctx, D3DRS_ZWRITEENABLE, enable ? TRUE : FALSE);
}

void glCtx_SetDepthFunc(glCtx* ctx, glCmpFunc depthFunction)
{
	glCtx_SetRenderState_Immediate(ctx, D3DRS_ZFUNC, glUtils_ToD3DCMPFUNC(depthFunction));
}

void glCtx_SetFillMode(glCtx* ctx, glFillMode mode)
{
	D3DFILLMODE d3dFilMode;
	switch (mode)
	{
		case glFillMode_Points: d3dFilMode = D3DFILL_POINT; break;
		case glFillMode_Wire: d3dFilMode = D3DFILL_WIREFRAME; break;
		case glFillMode_Solid: d3dFilMode = D3DFILL_SOLID; break;
	}
	glCtx_SetRenderState_Immediate(ctx, D3DRS_FILLMODE, d3dFilMode);
}

void glCtx_SetScissorTest(glCtx* ctx, ueBool enable)
{
	glCtx_SetRenderState_Immediate(ctx, D3DRS_SCISSORTESTENABLE, enable ? TRUE : FALSE);
}

void glCtx_SetScissorRectangle(glCtx* ctx, u32 left, u32 top, u32 width, u32 height)
{
	RECT d3dRect;
	d3dRect.left = left;
	d3dRect.top = top;
	d3dRect.right = left + width;
	d3dRect.bottom = top + height;
	ctx->m_handle->SetScissorRect(&d3dRect);
}

void glCtx_SetStencilTest(glCtx* ctx, ueBool enable)
{
	glCtx_SetRenderState_Immediate(ctx, D3DRS_STENCILENABLE, enable ? TRUE : FALSE);
}

void glCtx_SetStencilFunc(glCtx* ctx, glCmpFunc stencilFunc, u32 ref, u32 mask, u32 writeMask)
{
	glCtx_SetRenderState_Immediate(ctx, D3DRS_STENCILFUNC, glUtils_ToD3DCMPFUNC(stencilFunc));
	glCtx_SetRenderState_Immediate(ctx, D3DRS_STENCILREF, ref);
	glCtx_SetRenderState_Immediate(ctx, D3DRS_STENCILMASK, mask);
	glCtx_SetRenderState_Immediate(ctx, D3DRS_STENCILWRITEMASK, writeMask);
}

void glCtx_SetStencilOp(glCtx* ctx, glStencilOperation stencilFail, glStencilOperation depthFail, glStencilOperation depthPass)
{
	glCtx_SetRenderState_Immediate(ctx, D3DRS_STENCILFAIL, glUtils_ToD3DSTENCILOP(stencilFail));
	glCtx_SetRenderState_Immediate(ctx, D3DRS_STENCILZFAIL, glUtils_ToD3DSTENCILOP(depthFail));
	glCtx_SetRenderState_Immediate(ctx, D3DRS_STENCILPASS, glUtils_ToD3DSTENCILOP(depthPass));
}

void glCtx_SetTwoSidedStencilMode(glCtx* ctx, ueBool enable)
{
	glCtx_SetRenderState_Immediate(ctx, D3DRS_TWOSIDEDSTENCILMODE, enable ? TRUE : FALSE);
}

void glCtx_SetCCWStencilFunc(glCtx* ctx, glCmpFunc stencilFunc)
{
	glCtx_SetRenderState_Immediate(ctx, D3DRS_CCW_STENCILFUNC, glUtils_ToD3DCMPFUNC(stencilFunc));
}

void glCtx_SetCCWStencilOp(glCtx* ctx, glStencilOperation stencilFail, glStencilOperation depthFail, glStencilOperation depthPass)
{
	glCtx_SetRenderState_Immediate(ctx, D3DRS_CCW_STENCILFAIL, glUtils_ToD3DSTENCILOP(stencilFail));
	glCtx_SetRenderState_Immediate(ctx, D3DRS_CCW_STENCILZFAIL, glUtils_ToD3DSTENCILOP(depthFail));
	glCtx_SetRenderState_Immediate(ctx, D3DRS_CCW_STENCILPASS, glUtils_ToD3DSTENCILOP(depthPass));
}

void glCtx_SetVertexDeclaration(glCtx_D3D9Base* ctx, glVertexDeclaration* decl)
{
	if (decl == ctx->m_vertexDeclaration)
		return;

	ctx->m_vertexDeclaration = decl;
	ctx->m_vertexDeclDirty = UE_FALSE;

	if (!decl)
		ctx->m_handle->SetFVF(D3DFVF_POSITION_MASK); // This is to dereference current vertex declaration
	else
		ctx->m_handle->SetVertexDeclaration(decl->m_handle);
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

void glCtx_CommitRenderStates(glCtx* ctx)
{
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

			glVertexDeclaration* decl = glVertexShaderInput_GetVertexDeclaration(vs->m_input, numFormats, ctx->m_vertexStreamFormats);
			glCtx_SetVertexDeclaration(ctx, decl);
		}
	}

	// Set shaders

	glCtx_CommitProgram(ctx);

	// Set shaders and commit shader constants if necessary

	u32 usedSamplers[glShaderType_MAX] = {0};
	for (s32 i = 0; i < glShaderType_MAX; i++)
	{
		glCtx::ShaderInfo& shaderInfo = ctx->m_shaderInfos[i];
		glShader* shader = shaderInfo.m_shader;

		if (!shader)
			continue;

		enum BasicConstantType
		{
			BasicConstantType_Float = 0,
			BasicConstantType_Int,
			BasicConstantType_Bool,

			BasicConstantType_Count,

			BasicConstantType_Sampler
		};

		// Process all shader constants in batches (calculate register range and commit whole block at once)
		u32 minOffset[BasicConstantType_Count] = {1 << 30, 1 << 30, 1 << 30};
		u32 maxOffset[BasicConstantType_Count] = {0, 0, 0};
		for (u32 j = 0; j < shader->m_numConstants; j++)
		{
			const glShaderConstantInstance& constantInstance = shader->m_constants[j];
			const glConstant* constant = constantInstance.m_constant;

			// Determine "basic" constant type
			BasicConstantType basicConstantType;
			switch (constant->m_desc.m_type)
			{
				case glConstantType_Sampler1D:
				case glConstantType_Sampler2D:
				case glConstantType_Sampler3D:
				case glConstantType_SamplerCube:
					basicConstantType = BasicConstantType_Sampler;
					break;

				case glConstantType_Float:
				case glConstantType_Float2:
				case glConstantType_Float3:
				case glConstantType_Float4:
				case glConstantType_Float3x3:
				case glConstantType_Float4x3:
				case glConstantType_Float4x4:
					basicConstantType = BasicConstantType_Float;
					break;
				case glConstantType_Int:
				case glConstantType_Int2:
				case glConstantType_Int3:
				case glConstantType_Int4:
					basicConstantType = BasicConstantType_Int;
					break;
				case glConstantType_Bool:
				case glConstantType_Bool2:
				case glConstantType_Bool3:
				case glConstantType_Bool4:
					basicConstantType = BasicConstantType_Bool;
					break;
			}

			// Handle sampler separately
			if (basicConstantType == BasicConstantType_Sampler)
			{
				glCtx::SamplerConstantInfo& samplerInfo = ctx->m_samplerShaderConstantsLocal[constant->m_sampler.m_index];

				glCtx_SetTexture_Immediate(ctx, (glShaderType) i, constantInstance.m_offset, samplerInfo.m_textureBuffer);
				glCtx_SetSamplerState_Immediate(ctx, (glShaderType) i, constantInstance.m_offset, &samplerInfo.m_samplerParams);

				usedSamplers[i] |= 1 << constantInstance.m_offset;
				continue;
			}

			// Handle numeric data (float, int or bool)
			const u8* srcData = &ctx->m_numericShaderConstantsLocal[constant->m_numeric.m_cacheOffset];
			u8* dstData = &ctx->m_shaderInfos[i].m_data[constantInstance.m_offset];

			// If constant value is the same, skip it
			if (!ueMemCmp(dstData, srcData, constant->m_numeric.m_size))
				continue;

			ueMemCpy(dstData, srcData, constant->m_numeric.m_size);

			minOffset[basicConstantType] = ueMin(minOffset[basicConstantType], constantInstance.m_offset);
			maxOffset[basicConstantType] = ueMax(maxOffset[basicConstantType], constantInstance.m_offset + constant->m_numeric.m_size);
		}

		// Set whole constants blocks in one go

		for (u32 j = 0; j < BasicConstantType_Count; j++)
			if (minOffset[j] <= maxOffset[j])
			{
				const s32 startRegister = minOffset[j] >> 4;
				const u8* data = &ctx->m_shaderInfos[i].m_data[minOffset[j]];
				const s32 count = maxOffset[j] - minOffset[j];
#if !defined(UE_TOOL_X360)
				switch ((glShaderType) i)
				{
					case glShaderType_Fragment:
						switch ((BasicConstantType) j)
						{
							case BasicConstantType_Float: ctx->m_handle->SetPixelShaderConstantF(startRegister, (const FLOAT*) data, (count + 15) >> 4); break;
							case BasicConstantType_Int: ctx->m_handle->SetPixelShaderConstantI(startRegister, (const INT*) data, (count + 15) >> 4); break;
							case BasicConstantType_Bool:
							{
								static BOOL d3d9Bools[64];
								const u32 numBools = count / sizeof(ueBool);
								UE_ASSERT(numBools < UE_ARRAY_SIZE(d3d9Bools));
								for (u32 k = 0; k < numBools; k++)
									d3d9Bools[k] = ((const ueBool*)data)[k] ? TRUE : FALSE;
								ctx->m_handle->SetPixelShaderConstantB(startRegister, d3d9Bools, numBools);
								break;
							}
						}
						break;

					case glShaderType_Vertex:
						switch ((BasicConstantType) j)
						{
							case BasicConstantType_Float: ctx->m_handle->SetVertexShaderConstantF(startRegister, (const FLOAT*) data, (count + 15) >> 4); break;
							case BasicConstantType_Int: ctx->m_handle->SetVertexShaderConstantI(startRegister, (const INT*) data, (count + 15) >> 4); break;
							case BasicConstantType_Bool:
							{
								static BOOL d3d9Bools[64];
								const u32 numBools = count / sizeof(ueBool);
								UE_ASSERT(numBools < UE_ARRAY_SIZE(d3d9Bools));
								for (u32 k = 0; k < numBools; k++)
									d3d9Bools[k] = ((const ueBool*)data)[k] ? TRUE : FALSE;
								ctx->m_handle->SetVertexShaderConstantB(startRegister, d3d9Bools, numBools);
								break;
							}
						}
						break;
				}
#endif // !defined(UE_TOOL_X360)
			}
	}

	// Disable unused samplers

	for (u32 i = 0; i < glShaderType_MAX; i++)
		for (u32 j = 0; j < ctx->m_numSamplers[i]; j++)
			if (!(usedSamplers[i] & (1 << j)))
				glCtx_SetTexture_Immediate(ctx, (glShaderType) i, j, NULL);
}

// Context (internal lazy state management)

void glCtx_D3D9Base_Init(glCtx* ctx)
{
	glCtx_Base_Init(ctx);

	const glCaps* caps = glDevice_GetCaps();

	for (u32 i = 0; i < glShaderType_MAX; i++)
	{
		ctx->m_numSamplers[i] = GLCAPS->m_maxSamplers[i];
		if (ctx->m_numSamplers[i] > 0)
		{
			ctx->m_samplers[i] = (glTextureBuffer**) GL_STACK_ALLOC->Alloc(sizeof(glTextureBuffer*) * ctx->m_numSamplers[i]);
			UE_ASSERT(ctx->m_samplers[i]);
		}
	}

	const D3DRENDERSTATETYPE supportedRenderStates[] =
	{
		D3DRS_ZENABLE,
		D3DRS_FILLMODE,
		D3DRS_ZWRITEENABLE,
		D3DRS_ALPHATESTENABLE,
		D3DRS_LASTPIXEL,
		D3DRS_SRCBLEND,
		D3DRS_DESTBLEND,
		D3DRS_CULLMODE,
		D3DRS_ZFUNC,
		D3DRS_ALPHAREF,
		D3DRS_ALPHAFUNC,
		//	D3DRS_DITHERENABLE,
		D3DRS_ALPHABLENDENABLE,
		D3DRS_STENCILENABLE,
		D3DRS_STENCILFAIL,
		D3DRS_STENCILZFAIL,
		D3DRS_STENCILPASS,
		D3DRS_STENCILFUNC,
		D3DRS_STENCILREF,
		D3DRS_STENCILMASK,
		D3DRS_STENCILWRITEMASK,
		//	D3DRS_TEXTUREFACTOR,
		D3DRS_WRAP0,
		D3DRS_WRAP1,
		D3DRS_WRAP2,
		D3DRS_WRAP3,
		D3DRS_WRAP4,
		D3DRS_WRAP5,
		D3DRS_WRAP6,
		D3DRS_WRAP7,
		//	D3DRS_CLIPPING,
		//	D3DRS_CLIPPLANEENABLE,
		D3DRS_POINTSIZE,
		D3DRS_POINTSIZE_MIN,
		D3DRS_POINTSPRITEENABLE,
		D3DRS_MULTISAMPLEANTIALIAS,
		D3DRS_MULTISAMPLEMASK,
		//	D3DRS_PATCHEDGESTYLE,
		//	D3DRS_DEBUGMONITORTOKEN,
		D3DRS_POINTSIZE_MAX,
		D3DRS_COLORWRITEENABLE,
		D3DRS_BLENDOP,
		//	D3DRS_POSITIONDEGREE,
		//	D3DRS_NORMALDEGREE,
		D3DRS_SCISSORTESTENABLE,
		D3DRS_SLOPESCALEDEPTHBIAS,
		//	D3DRS_ANTIALIASEDLINEENABLE,
		D3DRS_MINTESSELLATIONLEVEL,
		D3DRS_MAXTESSELLATIONLEVEL,
		//	D3DRS_ADAPTIVETESS_X,
		//	D3DRS_ADAPTIVETESS_Y,
		//	D3DRS_ADAPTIVETESS_Z,
		//	D3DRS_ADAPTIVETESS_W,
		//	D3DRS_ENABLEADAPTIVETESSELLATION,
		D3DRS_TWOSIDEDSTENCILMODE,
		D3DRS_CCW_STENCILFAIL,
		D3DRS_CCW_STENCILZFAIL,
		D3DRS_CCW_STENCILPASS,
		D3DRS_CCW_STENCILFUNC,
		D3DRS_COLORWRITEENABLE1,
		D3DRS_COLORWRITEENABLE2,
		D3DRS_COLORWRITEENABLE3,
		D3DRS_BLENDFACTOR,
		//	D3DRS_SRGBWRITEENABLE,
		D3DRS_DEPTHBIAS,
		D3DRS_WRAP8,
		D3DRS_WRAP9,
		D3DRS_WRAP10,
		D3DRS_WRAP11,
		D3DRS_WRAP12,
		D3DRS_WRAP13,
		D3DRS_WRAP14,
		D3DRS_WRAP15,
		D3DRS_SEPARATEALPHABLENDENABLE,
		D3DRS_SRCBLENDALPHA,
		D3DRS_DESTBLENDALPHA,
		D3DRS_BLENDOPALPHA
	};

	u32 maxRenderStateIndex = 0;
	for (u32 i = 0; i < UE_ARRAY_SIZE(supportedRenderStates); i++)
		maxRenderStateIndex = ueMax(maxRenderStateIndex, (u32) supportedRenderStates[i]);

	ctx->m_maxRenderStates = maxRenderStateIndex + 1;
	ctx->m_renderStates = ueNewArray<glCtx::State>(GL_STACK_ALLOC, ctx->m_maxRenderStates);
	for (u32 i = 0; i < UE_ARRAY_SIZE(supportedRenderStates); i++)
		ctx->m_renderStates[ (u32) supportedRenderStates[i] ].m_isUsed = UE_TRUE;

	const D3DSAMPLERSTATETYPE supportedSamplerStates[] =
	{
		D3DSAMP_ADDRESSU,
		D3DSAMP_ADDRESSV,
		D3DSAMP_ADDRESSW,
		D3DSAMP_BORDERCOLOR,
		D3DSAMP_MAGFILTER,
		D3DSAMP_MINFILTER,
		D3DSAMP_MIPFILTER,
		D3DSAMP_MIPMAPLODBIAS,
		D3DSAMP_MAXMIPLEVEL,
		D3DSAMP_MAXANISOTROPY
		//  REGISTER_SAMPLER_RS(D3DSAMP_SRGBTEXTURE)
		//  REGISTER_SAMPLER_RS(D3DSAMP_ELEMENTINDEX)
		//  REGISTER_SAMPLER_RS(D3DSAMP_DMAPOFFSET)
	};

	u32 maxSamplerStateIndex = 0;
	for (u32 i = 0; i < UE_ARRAY_SIZE(supportedSamplerStates); i++)
		maxSamplerStateIndex = ueMax(maxSamplerStateIndex, (u32) supportedSamplerStates[i]);

	ctx->m_maxSamplerRenderStates = maxSamplerStateIndex + 1;
	for (u32 s = 0; s < glShaderType_MAX; s++)
		if (ctx->m_numSamplers[s] > 0)
		{
			ctx->m_samplerStates[s] = ueNewArray<glCtx::SamplerState>(GL_STACK_ALLOC, ctx->m_numSamplers[s]);
			for (u32 i = 0; i < ctx->m_numSamplers[s]; i++)
			{
				ctx->m_samplerStates[s][i].m_states = ueNewArray<glCtx::State>(GL_STACK_ALLOC, ctx->m_maxSamplerRenderStates);
				for (u32 j = 0; j < UE_ARRAY_SIZE(supportedSamplerStates); j++)
					ctx->m_samplerStates[s][i].m_states[ (u32) supportedSamplerStates[j] ].m_isUsed = UE_TRUE;
			}
		}
}

void glCtx_D3D9Base_Deinit(glCtx* ctx)
{
	for (s32 s = glShaderType_MAX - 1; s >= 0; s--)
		if (ctx->m_numSamplers[s] > 0)
		{
			for (s32 i = (s32) ctx->m_numSamplers[s] - 1; i >= 0; i--)
				GL_STACK_ALLOC->Free(ctx->m_samplerStates[s][i].m_states);
			GL_STACK_ALLOC->Free(ctx->m_samplerStates[s]);
		}

	GL_STACK_ALLOC->Free(ctx->m_renderStates);

	for (s32 i = glShaderType_MAX - 1; i >= 0; i--)
		if (ctx->m_numSamplers[i] > 0)
			GL_STACK_ALLOC->Free(ctx->m_samplers[i]);

	glCtx_Base_Deinit(ctx);

	const UINT refCount = ctx->m_handle->Release();
	UE_ASSERT(refCount == 0);
}

void glCtx_Reset(glCtx* ctx, u32 flags)
{
	glCtx_Base_Reset(ctx, flags);

	if (flags & glResetFlags_RenderStates)
	{
		// Get current render states to avoid redundant changes later

		for (u32 i = 0; i < ctx->m_maxRenderStates; i++)
			if (ctx->m_renderStates[i].m_isUsed)
			{
				HRESULT hr = ctx->m_handle->GetRenderState((D3DRENDERSTATETYPE) i, &ctx->m_renderStates[i].m_value);
				UE_ASSERT(SUCCEEDED(hr));
			}

		for (u32 s = 0; s < glShaderType_MAX; s++)
			for (u32 i = 0; i < ctx->m_numSamplers[s]; i++)
			{
				u32 deviceSamplerIndex = i;
				if (i == glShaderType_Vertex)
				{
					UE_ASSERT(i < 4); // D3D9 limitation
					deviceSamplerIndex += D3DVERTEXTEXTURESAMPLER0;
				}

				for (u32 j = 0; j < ctx->m_maxSamplerRenderStates; j++)
					if (ctx->m_samplerStates[s][i].m_states[j].m_isUsed)
					{
						HRESULT hr = ctx->m_handle->GetSamplerState(deviceSamplerIndex, (D3DSAMPLERSTATETYPE) j, &ctx->m_samplerStates[s][i].m_states[j].m_value);
						UE_ASSERT(SUCCEEDED(hr));
					}
			}
	}

	if (flags & glResetFlags_Streams)
		glCtx_SetVertexDeclaration(ctx, NULL);

	if (flags & glResetFlags_Shaders)
		glCtx_CommitProgram(ctx);
}

void glCtx_SetTexture_Immediate(glCtx* ctx, glShaderType shaderType, u32 samplerIndex, glTextureBuffer* tb)
{
	UE_ASSERT(shaderType == glShaderType_Vertex || shaderType == glShaderType_Fragment);

	u32 deviceSamplerIndex = samplerIndex;
	if (shaderType == glShaderType_Vertex)
	{
		UE_ASSERT(samplerIndex < 4); // D3D9 limitation
		deviceSamplerIndex += D3DVERTEXTEXTURESAMPLER0;
	}

	if (ctx->m_samplers[shaderType][samplerIndex] == tb)
		return;

	ctx->m_samplers[shaderType][samplerIndex] = tb;
	ctx->m_handle->SetTexture(deviceSamplerIndex, tb ? tb->m_textureHandle : NULL);
}

void glCtx_SetRenderGroup(glCtx* ctx, glRenderGroup* group)
{
	const u32 numRenderTargets = GLCAPS->m_maxColorRenderTargets;

	if (group)
	{
		ctx->m_renderGroup = group;
		ctx->m_handle->SetDepthStencilSurface(group->m_depthStencilRenderTarget ? group->m_depthStencilRenderTarget->m_surfaceHandle : NULL);
		for (u32 i = 0; i < numRenderTargets; i++)
		{
			glTextureBuffer* colorBuffer = i < group->m_desc.m_numColorBuffers ? group->m_colorRenderTargets[i] : NULL;
			ctx->m_handle->SetRenderTarget(i, colorBuffer ? colorBuffer->m_surfaceHandle : NULL);
		}
	}
	else
	{
		ctx->m_renderGroup = NULL;
		ctx->m_handle->SetDepthStencilSurface(glDevice_GetDepthStencilBuffer()->m_surfaceHandle);
		ctx->m_handle->SetRenderTarget(0, glDevice_GetBackBuffer()->m_surfaceHandle);
		for (u32 i = 1; i < numRenderTargets; i++)
			ctx->m_handle->SetRenderTarget(i, NULL);
	}
}

#else // defined(GL_D3D9) || defined(GL_X360)
	UE_NO_EMPTY_FILE
#endif
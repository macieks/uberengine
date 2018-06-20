#include "Base/ueBase.h"

#if defined(GL_OPENGL)

#include "Graphics/OpenGL/glLib_Private_OpenGL.h"

void glCtx_SetTexture_Immediate(glCtx* ctx, glShaderType shaderType, u32 samplerIndex, glTextureBuffer* tb, const glSamplerParams* params)
{
	GL(glActiveTextureARB(GL_TEXTURE0 + samplerIndex));

	// Get sampler type

	GLenum target;
	if (tb)
		switch (tb->m_desc.m_type)
		{
			case glTexType_2D: target = GL_TEXTURE_2D; break;
			case glTexType_3D: target = GL_TEXTURE_3D; break;
			case glTexType_Cube: target = GL_TEXTURE_CUBE_MAP; break;
			default: UE_ASSERT(0);
		}
	else
		target = GL_TEXTURE_2D;

	// Set the texture and sampler states

	if (tb)
	{
		const ueBool isSamplerDirty = ctx->m_samplers[samplerIndex] != tb;
		const ueBool isSamplerParamsDirty = tb && tb->m_samplerParams != *params;

		if (isSamplerDirty || isSamplerParamsDirty)
		{
			ctx->m_samplers[samplerIndex] = tb;

#ifndef GL_OPENGL_ES
			GL(glEnable(target));
#endif
			GL(glBindTexture(target, tb->m_textureHandle));

			//glUniform1i(0, 0);

			if (isSamplerParamsDirty)
			{
				tb->m_samplerParams = *params;

				GL(glTexParameteri(target, GL_TEXTURE_WRAP_S, glUtils_ToOpenGLTexAddr(params->m_addressU)));
				GL(glTexParameteri(target, GL_TEXTURE_WRAP_T, glUtils_ToOpenGLTexAddr(params->m_addressV)));
#ifndef GL_OPENGL_ES
				GL(glTexParameteri(target, GL_TEXTURE_WRAP_R, glUtils_ToOpenGLTexAddr(params->m_addressW)));
#endif

				GLenum minGLFilter, magGLFilter;
				glUtils_ToOpenGLTexFilter(params->m_minFilter, params->m_magFilter, params->m_mipFilter, tb->m_desc.m_numLevels, minGLFilter, magGLFilter);
				GL(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minGLFilter));
				GL(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magGLFilter));

				if (GLEW_EXT_texture_filter_anisotropic)
					GL(glTexParameteri(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, params->m_maxAnisotropicFilteringLevel));

#ifndef GL_OPENGL_ES
				GL(glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, params->m_maxMipLevel));

				const GLfloat border[4] = {params->m_borderColor.RedF(), params->m_borderColor.GreenF(), params->m_borderColor.BlueF(), params->m_borderColor.AlphaF()};
				GL(glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, border));
#endif
			}
		}
	}
	else if (!tb && ctx->m_samplers[samplerIndex])
	{
		ctx->m_samplers[samplerIndex] = NULL;

#ifndef GL_OPENGL_ES
		GL(glDisable(target));
#endif
		GL(glBindTexture(target, 0));
	}
}

// Context

void glCtx_SetVertexStreams(glCtx* ctx)
{
	if (!ctx->m_vertexDeclDirty || !ctx->m_program)
		return;

	const u32 numAttrs = ctx->m_program->m_numAttrs;
	const glProgram::Attr* attrs = ctx->m_program->m_attrs;

	for (u32 i = 0; i < GL_MAX_BOUND_VERTEX_STREAMS; i++)
	{
		glStreamFormat* sf = ctx->m_vertexStreamFormats[i];
		if (!sf)
			continue;

		// Bind buffer

		glCtx_Base::VertexStreamInfo& vsi = ctx->m_vertexStreamInfos[i];
		UE_ASSERT(vsi.m_VB);

		GL(glBindBufferARB(GL_ARRAY_BUFFER_ARB, vsi.m_VB->m_handle));

		// Set attribute offsets

		const u32 streamOffset = ctx->m_vertexStreamInfos[i].m_offset;
		const u32 stride = sf->m_stride;

		for (u32 j = 0; j < sf->m_numElements; j++)
		{
			const glVertexElement& elem = sf->m_elements[j];

			GLenum type;
			glUtils_ToOpenGLType((ueNumType) elem.m_type, type);

			const GLvoid* elemPtr = (const GLvoid*) (streamOffset + elem.m_offset);

			// Find matching attribute within program
			// FIXME: Improve efficiency by cleverer matching + add error checking for when shader input isn't satisfied

			const glProgram::Attr* attr = NULL;
			for (u32 k = 0; k < numAttrs; k++)
				if (attrs[k].m_semantic == elem.m_semantic && attrs[k].m_usageIndex == elem.m_usageIndex)
				{
					attr = &attrs[k];
					break;
				}
			if (!attr)
				continue;

			// Set data stream offset

			if (attr->m_isGeneric)
			{
				GL(glEnableVertexAttribArrayARB(attr->m_location));
				GL(glVertexAttribPointerARB(attr->m_location, elem.m_count, type, elem.m_isNormalized ? GL_TRUE : GL_FALSE, stride, elemPtr));
			}
#ifdef GL_OPENGL_ES
			else
				UE_ASSERT(!"OpenGL ES only supports generic vertex attributes");
#else
			else switch (elem.m_semantic)
			{
			case glSemantic_Position:
				UE_ASSERT(elem.m_usageIndex == 0);
				GL(glEnableClientState(GL_VERTEX_ARRAY));
				GL(glVertexPointer(elem.m_count, type, stride, elemPtr));
				break;
			case glSemantic_Normal:
				UE_ASSERT(elem.m_usageIndex == 0);
				UE_ASSERT(elem.m_type == ueNumType_S8 || elem.m_type == ueNumType_S16 || elem.m_type == ueNumType_S32 || elem.m_type == ueNumType_F32);
				GL(glEnableClientState(GL_NORMAL_ARRAY));
				GL(glNormalPointer(type, stride, elemPtr));
				break;
			case glSemantic_Color:
				switch (elem.m_usageIndex)
				{
				case 0:
					GL(glEnableClientState(GL_COLOR_ARRAY));
					GL(glColorPointer(elem.m_count, type, stride, elemPtr));
					break;
				case 1:
					UE_ASSERT(GLEW_EXT_secondary_color);
					GL(glEnableClientState(GL_SECONDARY_COLOR_ARRAY));
					GL(glSecondaryColorPointer(elem.m_count, type, stride, (GLvoid*) elemPtr));
					break;
				UE_INVALID_CASE(elem.m_usageIndex);
				}
				break;
			case glSemantic_TexCoord:
				UE_ASSERT(!elem.m_isNormalized);
				GL(glClientActiveTexture(GL_TEXTURE0 + elem.m_usageIndex));
				GL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
				GL(glTexCoordPointer(elem.m_count, type, stride, elemPtr));
				break;
			UE_INVALID_CASE(elem.m_semantic)
			}
#endif
		}
	}

	ctx->m_vertexDeclDirty = UE_FALSE;
}

void glCtx_SetStream(glCtx* ctx, u32 streamIndex, glVertexBuffer* vertexBuffer, glStreamFormat* format, u32 offset, u32 divider)
{
	if (ctx->m_vertexStreamFormats[streamIndex] != format)
	{
		ctx->m_vertexStreamFormats[streamIndex] = format;
		ctx->m_vertexDeclDirty = UE_TRUE;
	}

	if (ctx->m_vertexStreamInfos[streamIndex].m_VB != vertexBuffer ||
		ctx->m_vertexStreamInfos[streamIndex].m_offset != offset ||
		ctx->m_vertexStreamInfos[streamIndex].m_divider != divider)
	{
		ctx->m_vertexStreamInfos[streamIndex].m_VB = vertexBuffer;
		ctx->m_vertexStreamInfos[streamIndex].m_offset = offset;
		ctx->m_vertexStreamInfos[streamIndex].m_divider = divider;
		ctx->m_vertexDeclDirty = UE_TRUE;
	}
}

void glCtx_SetIndices(glCtx* ctx, glIndexBuffer* indexBuffer)
{
	if (ctx->m_indexBuffer == indexBuffer)
		return;

	ctx->m_indexBuffer = indexBuffer;
	GL(glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indexBuffer ? indexBuffer->m_handle : 0));
}

void glCtx_SetProgram(glCtx* ctx, glProgram* program)
{
	if (ctx->m_program == program)
		return;

	ctx->m_program = program;
	ctx->m_vertexDeclDirty = UE_TRUE;

	GL(glUseProgramObjectARB(program ? program->m_handle : 0));
}

void glCtx_SetViewport(glCtx* ctx, const glViewportDesc& viewport)
{
	glRenderGroup* renderGroup = glCtx_GetCurrentRenderGroup(ctx);
	const glRenderGroupDesc* renderGroupDesc = glRenderGroup_GetDesc(renderGroup);

	GL(glViewport(viewport.m_left, renderGroupDesc->m_height - viewport.m_height - viewport.m_top, viewport.m_width, viewport.m_height));
	GL(glDepthRange(viewport.m_minZ, viewport.m_maxZ));
}

void glCtx_SetClearColor(glCtx* ctx, f32 red, f32 green, f32 blue, f32 alpha)
{
	ctx->m_clearColorRed = red;
	ctx->m_clearColorGreen = green;
	ctx->m_clearColorBlue = blue;
	ctx->m_clearColorAlpha = alpha;

	GL(glClearColor(red, green, blue, alpha));
}

void glCtx_SetClearDepthStencil(glCtx* ctx, f32 depth, u32 stencil)
{
	ctx->m_clearDepth = depth;
	ctx->m_clearStencil = stencil;

	GL(glClearDepth(depth));
	GL(glClearStencil(stencil));
}

void glCtx_Clear(glCtx* ctx, u32 clearFlags)
{
	UE_ASSERT_MSG(!(clearFlags & glClearFlag_Color) || ctx->m_renderGroup->m_colorRenderTargets[0], "Attempted to clear color while there's no color buffer bound.");
//	UE_ASSERT_MSG(!(clearFlags & (glClearFlag_Depth | glClearFlag_Stencil)) || ctx->m_renderGroup->m_depthStencilRenderTarget, "Attempted to clear depth / stencil while there's no depth-stencil buffer bound.");

	if (!clearFlags) return;

	// Enable all write masks

	if (!ctx->m_depthMask)
		GL(glDepthMask(GL_TRUE));
	if (ctx->m_stencilMask != ~0U)
		GL(glStencilMask(~0));
	if (ctx->m_colorMask != glColorMask_All)
		GL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

	// Set scissor rectangle to viewport rectangle

	GLint viewport[4], scissor[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetIntegerv(GL_SCISSOR_BOX, scissor);
	if (viewport != scissor)
		glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);

	// Clear

	GL(glClear(
		((clearFlags & glClearFlag_Color) ? GL_COLOR_BUFFER_BIT : 0) |
		((clearFlags & glClearFlag_Depth) ? GL_DEPTH_BUFFER_BIT : 0) |
		((clearFlags & glClearFlag_Stencil) ? GL_STENCIL_BUFFER_BIT : 0)));

	// Revert scissor rectangle

	if (viewport != scissor)
		glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);

	// Revert write masks

	if (!ctx->m_depthMask)
		GL(glDepthMask(GL_FALSE));
	if (ctx->m_stencilMask != ~0U)
		GL(glStencilMask(ctx->m_stencilMask));
	if (ctx->m_colorMask != glColorMask_All)
		GL(glColorMask(
			(ctx->m_colorMask & glColorMask_Red) ? GL_TRUE : GL_FALSE,
			(ctx->m_colorMask & glColorMask_Green) ? GL_TRUE : GL_FALSE,
			(ctx->m_colorMask & glColorMask_Blue) ? GL_TRUE : GL_FALSE,
			(ctx->m_colorMask & glColorMask_Alpha) ? GL_TRUE : GL_FALSE));
}

void glCtx_SetColorWrite(glCtx* ctx, u32 colorTargetIndex, u32 colorMask)
{
	if (colorTargetIndex == 0)
	{
		GL(glColorMask(
			(colorMask & glColorMask_Red) ? GL_TRUE : GL_FALSE,
			(colorMask & glColorMask_Green) ? GL_TRUE : GL_FALSE,
			(colorMask & glColorMask_Blue) ? GL_TRUE : GL_FALSE,
			(colorMask & glColorMask_Alpha) ? GL_TRUE : GL_FALSE));
		ctx->m_colorMask = colorMask;
	}
	else
	{
// TODO
#if 0
		UE_ASSERT( glDevice_GetCaps()->m_flags & glCaps::Flags_SupportsIndependentColorWriteMasks );

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
#else
		UE_NOT_IMPLEMENTED();
#endif
	}
}

void glCtx_SetAlphaTest(glCtx* ctx, ueBool enable)
{
#ifndef GL_OPENGL_ES
	if (enable)
		GL(glEnable(GL_ALPHA_TEST));
	else
		GL(glDisable(GL_ALPHA_TEST));
#endif
}

void glCtx_SetAlphaFunc(glCtx* ctx, glCmpFunc alphaFunction, u32 referenceValue)
{
#ifndef GL_OPENGL_ES
	UE_ASSERT(referenceValue <= 255);
	GL(glAlphaFunc(glUtils_ToOpenGLCmpFunc(alphaFunction), (f32) referenceValue / 255.0f));
#endif
}

void glCtx_SetBlending(glCtx* ctx, ueBool enable)
{
	if (enable)
		GL(glEnable(GL_BLEND));
	else
		GL(glDisable(GL_BLEND));
}

void glCtx_SetBlendOp(glCtx* ctx, glBlendOp op)
{
	GL(glBlendEquation(glUtils_ToOpenGLBlendOp(op)));
}

void glCtx_SetBlendOp(glCtx* ctx, glBlendOp colorOp, glBlendOp alphaOp)
{
	GL(glBlendEquationSeparate(glUtils_ToOpenGLBlendOp(colorOp), glUtils_ToOpenGLBlendOp(alphaOp)));
}

void glCtx_SetBlendFunc(glCtx* ctx, glBlendingFunc src, glBlendingFunc dst)
{
	GL(glBlendFunc(glUtils_ToOpenGLBlendFunc(src), glUtils_ToOpenGLBlendFunc(dst)));
}

void glCtx_SetBlendFunc(glCtx* ctx, glBlendingFunc srcColor, glBlendingFunc dstColor, glBlendingFunc srcAlpha, glBlendingFunc dstAlpha)
{
	GL(glBlendFuncSeparate(
		glUtils_ToOpenGLBlendFunc(srcColor), glUtils_ToOpenGLBlendFunc(dstColor),
		glUtils_ToOpenGLBlendFunc(srcAlpha), glUtils_ToOpenGLBlendFunc(dstAlpha)));
}

void glCtx_SetCullMode(glCtx* ctx, glCullMode mode)
{
	ctx->m_cullMode = mode;

	// Apply inverted cull mode when rendering to texture

	if (glCtx_IsRenderingToTexture(ctx))
	{
		if (mode == glCullMode_CW)
			mode = glCullMode_CCW;
		else if (mode == glCullMode_CCW)
			mode = glCullMode_CW;
	}

	switch (mode)
	{
		case glCullMode_CW:
			GL(glEnable(GL_CULL_FACE));
			GL(glCullFace(GL_BACK));
			break;
		case glCullMode_CCW:
			GL(glEnable(GL_CULL_FACE));
			GL(glCullFace(GL_FRONT));
			break;
		case glCullMode_None:
			GL(glDisable(GL_CULL_FACE));
			break;
		UE_INVALID_CASE(mode);
	}
}

void glCtx_SetDepthTest(glCtx* ctx, ueBool enable)
{
	if (enable)
		GL(glEnable(GL_DEPTH_TEST));
	else
		GL(glDisable(GL_DEPTH_TEST));
}

void glCtx_SetDepthWrite(glCtx* ctx, ueBool enable)
{
	if (enable)
		GL(glDepthMask(GL_TRUE));
	else
		GL(glDepthMask(GL_FALSE));

	ctx->m_depthMask = enable;
}

void glCtx_SetDepthFunc(glCtx* ctx, glCmpFunc depthFunction)
{
	GL(glDepthFunc(glUtils_ToOpenGLCmpFunc(depthFunction)));
}

void glCtx_SetFillMode(glCtx* ctx, glFillMode mode)
{
#ifndef GL_OPENGL_ES // OpenGL ES doesn't support glPolygonMode

	GLenum polygonMode;
	switch (mode)
	{
		case glFillMode_Points: polygonMode = GL_POINT; break;
		case glFillMode_Wire: polygonMode = GL_LINE; break;
		case glFillMode_Solid: polygonMode = GL_FILL; break;
		UE_INVALID_CASE(mode);
	}
	GL(glPolygonMode(GL_FRONT_AND_BACK, polygonMode));

#endif
}

void glCtx_SetScissorTest(glCtx* ctx, ueBool enable)
{
	if (enable)
		GL(glEnable(GL_SCISSOR_TEST));
	else
		GL(glDisable(GL_SCISSOR_TEST));
}

void glCtx_SetScissorRectangle(glCtx* ctx, u32 left, u32 top, u32 width, u32 height)
{
	glRenderGroup* renderGroup = glCtx_GetCurrentRenderGroup(ctx);
	const glRenderGroupDesc* renderGroupDesc = glRenderGroup_GetDesc(renderGroup);
	GL(glScissor(left, renderGroupDesc->m_height - height - top, width, height));
}

void glCtx_SetStencilTest(glCtx* ctx, ueBool enable)
{
	if (enable)
		GL(glEnable(GL_STENCIL_TEST));
	else
		GL(glDisable(GL_STENCIL_TEST));
}

void glCtx_SetStencilFunc(glCtx* ctx, glCmpFunc stencilFunc, u32 ref, u32 mask, u32 writeMask)
{
	if (writeMask == 0xFF)
		writeMask = 0xFFFFFFFF;

#ifndef GL_OPENGL_ES
	if (GLEW_EXT_stencil_two_side)
		GL(glActiveStencilFaceEXT(GL_BACK));
#endif
	GL(glStencilFunc(glUtils_ToOpenGLCmpFunc(stencilFunc), ref, mask));
	GL(glStencilMask(writeMask));

	ctx->m_stencilMask = writeMask;
}

void glCtx_SetStencilOp(glCtx* ctx, glStencilOperation stencilFail, glStencilOperation depthFail, glStencilOperation depthPass)
{
#ifndef GL_OPENGL_ES
	if (GLEW_EXT_stencil_two_side)
		GL(glActiveStencilFaceEXT(GL_BACK));
#endif
	GL(glStencilOp(
		glUtils_ToOpenGLStencilOp(stencilFail),
		glUtils_ToOpenGLStencilOp(depthFail),
		glUtils_ToOpenGLStencilOp(depthPass)));
}

void glCtx_SetTwoSidedStencilMode(glCtx* ctx, ueBool enable)
{
#ifdef GL_OPENGL_ES
	UE_ASSERT(!"OpenGL ES doesn't support two sided stencil");
#else
	UE_ASSERT(GLEW_EXT_stencil_two_side);
	if (enable)
		GL(glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT));
	else
		GL(glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT));
#endif
}

void glCtx_SetCCWStencilFunc(glCtx* ctx, glCmpFunc stencilFunc)
{
#ifdef GL_OPENGL_ES
	UE_ASSERT(!"OpenGL ES doesn't support two sided stencil");
#else

	UE_ASSERT(GLEW_EXT_stencil_two_side);
	GL(glActiveStencilFaceEXT(GL_FRONT));
#if 0
	GL(glStencilFunc(glUtils_ToOpenGLCmpFunc(stencilFunc), ref, mask));
	GL(glStencilMask(writeMask));
#else
	UE_NOT_IMPLEMENTED();
#endif

#endif
}

void glCtx_SetCCWStencilOp(glCtx* ctx, glStencilOperation stencilFail, glStencilOperation depthFail, glStencilOperation depthPass)
{
#ifdef GL_OPENGL_ES
	UE_ASSERT(!"OpenGL ES doesn't support two sided stencil");
#else
	UE_ASSERT(GLEW_EXT_stencil_two_side);
	GL(glActiveStencilFaceEXT(GL_FRONT));
#endif
	GL(glStencilOp(
		glUtils_ToOpenGLStencilOp(stencilFail),
		glUtils_ToOpenGLStencilOp(depthFail),
		glUtils_ToOpenGLStencilOp(depthPass)));
}

ueBool glCtx_PreDraw(glCtx* ctx)
{
	glCtx_CommitRenderStates(ctx);

	// Check program

	if (!ctx->m_program)
	{
		ueLogE("No shader program set.");
		return UE_FALSE;
	}

	// Check vertex shader

	if (!ctx->m_program->m_shaders[glShaderType_Vertex])
	{
		ueLogE("No vertex shader set.");
		return UE_FALSE;
	}

	// Validate fragment shader against currently set FBO

	glShader* fragmentShader = ctx->m_program->m_shaders[glShaderType_Fragment];
	if (fragmentShader)
	{
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
	// Set vertex streams

	glCtx_SetVertexStreams(ctx);

	// Set uniform values

	u32 samplerIndex = 0;

	if (ctx->m_program)
		for (u32 i = 0; i < ctx->m_program->m_numUniforms; i++)
		{
			glProgram::Uniform& uniform = ctx->m_program->m_uniforms[i];
			const glConstant* constant = uniform.m_constant;

			// Handle sampler

			if (glUtils_IsSampler((glConstantType) constant->m_desc.m_type))
			{
				glCtx_Base::SamplerConstantInfo& info = ctx->m_samplerShaderConstantsLocal[constant->m_sampler.m_index];
				glCtx_SetTexture_Immediate(ctx, glShaderType_Fragment, samplerIndex, info.m_textureBuffer, &info.m_samplerParams);
				samplerIndex++;
				continue;
			}

			// Handle numeric uniform

			const u8* srcData = &ctx->m_numericShaderConstantsLocal[constant->m_numeric.m_cacheOffset];

			switch (constant->m_desc.m_type)
			{
				case glConstantType_Float:
					GL(glUniform1fv(uniform.m_location, constant->m_desc.m_count, (const GLfloat*) srcData));
					break;
				case glConstantType_Float2:
					GL(glUniform2fv(uniform.m_location, constant->m_desc.m_count, (const GLfloat*) srcData));
					break;
				case glConstantType_Float3:
					GL(glUniform3fv(uniform.m_location, constant->m_desc.m_count, (const GLfloat*) srcData));
					break;
				case glConstantType_Float4:
					GL(glUniform4fv(uniform.m_location, constant->m_desc.m_count, (const GLfloat*) srcData));
					break;
				case glConstantType_Float4x4:
				{
					// Transpose matrices for OpenGL (column major -> row major)

					ueMat44 transposed[128];
					const u32 count = constant->m_desc.m_count;
					UE_ASSERT(count <= UE_ARRAY_SIZE(transposed));
					for (u32 i = 0; i < count; i++)
					{
						ueMat44_Set44(transposed[i], (const f32*) srcData + i * 16);
						ueMat44_Transpose(transposed[i]);
					}

					// Submit each 4x4 matrix as 4 vec4s

					GL(glUniform4fv(uniform.m_location, count * 4, (const GLfloat*) transposed));
					break;
				}
				UE_INVALID_CASE(constant->m_desc.m_type);
			}
		}

	// Unbind unused samplers

	for (u32 i = samplerIndex; i < ctx->m_numSamplers; i++)
		glCtx_SetTexture_Immediate(ctx, glShaderType_Fragment, i, NULL, NULL);
}

// Context

void glCtx_Init(glCtx* ctx)
{
	glCtx_Base_Init(ctx);

	ctx->m_numSamplers = GLCAPS->m_maxSamplers[glShaderType_Fragment];
	ctx->m_samplers = ueNewArray<glTextureBuffer*>(GL_STACK_ALLOC, ctx->m_numSamplers);
	UE_ASSERT(ctx->m_samplers);

	glCtx_SetCullMode(ctx, glCullMode_CW);

	ctx->m_enableFBOTest = UE_FALSE;
}

void glCtx_Deinit(glCtx* ctx)
{
	GL_STACK_ALLOC->Free(ctx->m_samplers);

	glCtx_Base_Deinit(ctx);
}

void glCtx_Reset(glCtx* ctx, u32 flags)
{
	glCtx_Base_Reset(ctx, flags);

	if (flags & glResetFlags_RenderStates)
	{
		glCtx_SetDepthTest(ctx, UE_TRUE);
		glCtx_SetDepthWrite(ctx, UE_TRUE);
		glCtx_SetDepthFunc(ctx, glCmpFunc_Less);
		glCtx_SetColorWrite(ctx, 0, glColorMask_All);
		glCtx_SetFillMode(ctx, glFillMode_Solid);
		glCtx_SetCullMode(ctx, glCullMode_CW);
	}
}

ueBool glCtx_Begin(glCtx* ctx)
{
	return UE_TRUE;
}

void glCtx_End(glCtx* ctx)
{
	// Release any references to resources

	glCtx_Reset(ctx, glResetFlags_Resources);
}

void glCtx_Present(glCtx* ctx, glTextureBuffer* tb)
{
	UE_ASSERT_MSG(tb == glDevice_GetBackBuffer(), "Can only present using back buffer.");

	glDevice* dev = glDevice_Get();

#ifdef GL_OPENGL_ES
	eglSwapBuffers(dev->m_EGLDisplay, dev->m_EGLSurface);
#elif defined(UE_WIN32)
	if (WGLEW_EXT_swap_control)
		GL(wglSwapIntervalEXT(dev->m_params.m_displaySync != glDisplaySync_None ? 1 : 0));
	SwapBuffers(dev->m_hdc);
#elif defined(UE_LINUX)
	glXSwapBuffers(dev->m_display, dev->m_window);
	GL(glFinish());
#elif defined(UE_MAC)
	aglSwapBuffers(glContext);
	GL(glFinish());
#elif defined(UE_MARMALADE)
	IwGLSwapBuffers();
#endif
}

void glCtx_DrawIndexed(glCtx* ctx, glPrimitive primitive, u32 baseVertexIndex, u32 minVertexIndex, u32 numVerts, u32 startIndex, u32 numIndices)
{
	if (!glCtx_PreDraw(ctx)) return;
	UE_ASSERT(baseVertexIndex == 0); // FIXME: This can be fixed via rebinding VBOs
	const u32 indexSize = ctx->m_indexBuffer->m_desc.m_indexSize;
	GL(glDrawElements(glUtils_ToOpenGLPrimitive(primitive), numIndices, indexSize == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (const void*) (indexSize * startIndex)));
}

void glCtx_Draw(glCtx* ctx, glPrimitive primitive, u32 startVertex, u32 numVerts)
{
	if (!glCtx_PreDraw(ctx)) return;
	GL(glDrawArrays(glUtils_ToOpenGLPrimitive(primitive), startVertex, numVerts));
}

void glCtx_SetRenderGroup(glCtx* ctx, glRenderGroup* group)
{
	if (!group)
		group = glDevice_GetMainRenderGroup();

	ctx->m_renderGroup = group;

	// Update culling - render to texture requires inversed setting to render to main buffer

	glCtx_SetCullMode(ctx, ctx->m_cullMode);

	// Change viewport to cover full FBO

	glViewport(0, 0, group->m_desc.m_width, group->m_desc.m_height);

	// Toggle MSAA if necessary

#ifdef GL_OPENGL_ES
	// TODO
#else
	if (
#if defined(UE_WIN32)
     WGLEW_ARB_multisample &&
#endif
        GLEW_ARB_multisample &&
		group->m_desc.m_MSAALevel > 1)
	{
		GL(glEnable(GL_MULTISAMPLE_ARB));
	}
	else
	{
		GL(glDisable(GL_MULTISAMPLE_ARB));
	}
#endif

	// Switch to main frame buffer

	if (group->m_desc.m_isMainFrameBuffer)
	{
		if (GLEW_EXT_framebuffer_object)
		{
			GL(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
#ifndef GL_OPENGL_ES
			GL(glDrawBuffer(GL_BACK));
			GL(glReadBuffer(GL_BACK));
#endif
		}
		else
		{
			// TODO: pbuffer
		}
		return;
	}

	// Set non-main frame buffer

	GL(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ctx->m_fbo));

	const GLint mipLevel = 0; // TODO: Expose this

	// Attach / detach depth-stencil render target

	glTextureBuffer* depthStencilRenderTarget = group->m_depthStencilRenderTarget;

	if (depthStencilRenderTarget && glTextureBuffer_IsTexture(depthStencilRenderTarget))
	{
		GL(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depthStencilRenderTarget->m_textureHandle, mipLevel));
		GL(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, glUtils_IsStencilFormat(depthStencilRenderTarget->m_desc.m_format) ? depthStencilRenderTarget->m_textureHandle : 0, mipLevel));
	}
	else
	{
		GL(glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthStencilRenderTarget ? depthStencilRenderTarget->m_renderBufferHandle : 0));
		GL(glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, (depthStencilRenderTarget && glUtils_IsStencilFormat(depthStencilRenderTarget->m_desc.m_format)) ? depthStencilRenderTarget->m_renderBufferHandle : 0));
	}

	// Attach / detach all color render targets

	for (u32 i = 0; i < GL_MAX_COLOR_RENDER_TARGETS; i++)
	{
		glTextureBuffer* colorRenderTarget = i < group->m_desc.m_numColorBuffers ? group->m_colorRenderTargets[i] : NULL;

		if (colorRenderTarget && glTextureBuffer_IsTexture(colorRenderTarget))
			GL(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_2D, colorRenderTarget->m_textureHandle, mipLevel));
		else
			GL(glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_RENDERBUFFER_EXT, colorRenderTarget ? colorRenderTarget->m_renderBufferHandle : 0));
	}

#ifndef GL_OPENGL_ES

	// Set draw & read buffers

	if (group->m_desc.m_numColorBuffers == 0)
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}
	else
	{
		static const GLenum drawBuffers[] =
		{
			GL_COLOR_ATTACHMENT0_EXT,
			GL_COLOR_ATTACHMENT1_EXT,
			GL_COLOR_ATTACHMENT2_EXT,
			GL_COLOR_ATTACHMENT3_EXT,
			GL_COLOR_ATTACHMENT4_EXT,
			GL_COLOR_ATTACHMENT5_EXT,
			GL_COLOR_ATTACHMENT6_EXT,
			GL_COLOR_ATTACHMENT7_EXT,
			GL_COLOR_ATTACHMENT8_EXT,
			GL_COLOR_ATTACHMENT9_EXT,
			GL_COLOR_ATTACHMENT10_EXT,
			GL_COLOR_ATTACHMENT11_EXT,
			GL_COLOR_ATTACHMENT12_EXT,
			GL_COLOR_ATTACHMENT13_EXT,
			GL_COLOR_ATTACHMENT14_EXT,
			GL_COLOR_ATTACHMENT15_EXT
		};

		GL(glDrawBuffers(group->m_desc.m_numColorBuffers, drawBuffers));
		GL(glReadBuffer(GL_COLOR_ATTACHMENT0_EXT));
	}

#endif

	// Verify framebuffer setup is valid

	ctx->m_FBOTestResult = glUtils_CheckFramebufferStatus(ctx->m_enableFBOTest);
	if (!ctx->m_enableFBOTest &&	// Not a test mode
		!ctx->m_FBOTestResult)		// Test didn't pass
		glUtils_ReportUnsupportedFBO(group, "render");
}

void glCtx_ResolveRenderTargets(glCtx* ctx)
{
	UE_ASSERT(ctx->m_renderGroup);

	glRenderGroup* group = ctx->m_renderGroup;

	if (group->m_desc.m_isMainFrameBuffer)
		return;

	// Resolve MSAA to non-MSAA buffers

	if (group->m_desc.m_MSAALevel > 1)
	{
		// Construct destination non-MSAA FBO

		const u32 dstMipLevel = 0; // TODO: Expose this

		GL(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ctx->m_fbo2));

		GL(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, group->m_desc.m_needDepthStencilContent ? group->m_depthStencilTexture->m_textureHandle : 0, dstMipLevel));
		GL(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, group->m_desc.m_needDepthStencilContent ? group->m_depthStencilTexture->m_textureHandle : 0, dstMipLevel));
		for (u32 i = 0; i < GL_MAX_COLOR_RENDER_TARGETS; i++)
			GL(glFramebufferTexture2DEXT(
				GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_2D,
				(group->m_desc.m_needColorContent && i < group->m_desc.m_numColorBuffers) ? group->m_colorTextures[i]->m_textureHandle : 0,
				dstMipLevel));

		if (!glUtils_CheckFramebufferStatus(UE_FALSE))
			glUtils_ReportUnsupportedFBO(group, "resolve");

		if (GLEW_EXT_framebuffer_blit)
		{
			// Blit from MSAA FBO to non-MSAA FBO

			glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, ctx->m_fbo);
			glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, ctx->m_fbo2);

			glBlitFramebufferEXT(
				0, 0, group->m_desc.m_width, group->m_desc.m_height,
				0, 0, group->m_desc.m_width, group->m_desc.m_height,
				(group->m_desc.m_needColorContent ? GL_COLOR_BUFFER_BIT : 0) |
				(group->m_desc.m_needDepthStencilContent ? (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT) : 0),
				GL_NEAREST);

			glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
			glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
		}
		else
		{
			UE_ASSERT(0);
		}
	}

	// Store previous texture

	GLint oldTex;
	GL(glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTex));

	// Generate mip tail if needed

	for (u32 i = 0; i < group->m_desc.m_numColorBuffers; i++)
	{
		glTextureBuffer* tb = group->m_colorTextures[i];
		if (tb->m_desc.m_enableAutoMips)
		{
			GL(glEnable(GL_TEXTURE_2D));
			GL(glBindTexture(GL_TEXTURE_2D, tb->m_textureHandle));
			if (GLEW_VERSION_3_0)
				GL(glGenerateMipmap(GL_TEXTURE_2D));
			else if (GLEW_EXT_framebuffer_object)
				GL(glGenerateMipmapEXT(GL_TEXTURE_2D));
		}
	}

	// Restore previous texture

	GL(glBindTexture(GL_TEXTURE_2D, oldTex));
}

ueBool glCtx_CopyBuffer(glCtx* ctx, glTextureBuffer* src, glTextureBuffer* dst)
{
#if 0
	UE_ASSERT((src->m_desc.m_flags & glTextureBufferFlags_IsTexture) && src->m_desc.m_type == glTexType_2D);
	UE_ASSERT((dst->m_desc.m_flags & glTextureBufferFlags_IsTexture) && dst->m_desc.m_type == glTexType_2D);

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
	UE_NOT_IMPLEMENTED();
	return UE_FALSE;
#endif
}

ueBool glRenderGroup_CheckSupport(glRenderGroup* group)
{
	// Check cached result

	const ueBool* cachedFBOTestResult = GLDEV->m_fboSupportCache.Find(group->m_desc);
	if (cachedFBOTestResult)
		return *cachedFBOTestResult;

	// Enable FBO testing

	glCtx* ctx = glDevice_GetDefaultContext();
	ctx->m_enableFBOTest = UE_TRUE;

	// Test (with small render target)

	const u32 fullWidth = group->m_desc.m_width;
	const u32 fullHeight = group->m_desc.m_height;
	group->m_desc.m_width = 16;
	group->m_desc.m_height = 16;

	glRenderGroup_Begin(group, ctx, 0);
	glRenderGroup_EndDrawing(group);
	glRenderGroup_End(group); // FIXME: Destroy temporary render targets

	group->m_desc.m_width = fullWidth;
	group->m_desc.m_height = fullHeight;

	// Disable FBO test and check support

	ctx->m_enableFBOTest = UE_FALSE;
	const ueBool isSupported = ctx->m_FBOTestResult;

	// Cache the result (reset cache if full)

	if (GLDEV->m_fboSupportCache.Size() == GLDEV->m_fboSupportCache.Capacity())
		GLDEV->m_fboSupportCache.Clear();
	GLDEV->m_fboSupportCache.Insert(group->m_desc, isSupported);

	return isSupported;
}

#else // defined(GL_OPENGL)
	UE_NO_EMPTY_FILE
#endif
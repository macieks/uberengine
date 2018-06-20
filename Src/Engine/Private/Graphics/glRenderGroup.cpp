#include "Graphics/glLib_Private.h"

UE_INLINE ueBool glRenderGroup_RequiresSeparateOutputTextures(glRenderGroup* group)
{
#if defined(GL_X360)
	return UE_TRUE;
#else
	return group->m_desc.m_MSAALevel > 1;
#endif
}

ueBool glRenderGroup_GetClosestSupportedFormat(glRenderGroup* group, glRenderGroupDesc::BufferDesc& bufDesc, ueBool needContent)
{
	if (group->m_desc.m_isMainFrameBuffer)
		return UE_TRUE;

#if defined(GL_OPENGL)

	// Handle format fallbacks for OpenGL

	const glDevice* device = glDevice_Get();
	const glCaps* caps = glDevice_GetCaps();
	const glCaps::FormatInfo& info = caps->m_formatInfos[bufDesc.m_format];

	const ueBool needsRTOnly = glRenderGroup_RequiresSeparateOutputTextures(group);

	glBufferFormat rtOnlyFormat = bufDesc.m_format;
	if ((!needContent || needsRTOnly) && !info.m_supportedAsRTOnly)
	{
		const glOpenGLBufferFormat& mapping = device->m_bufferFormatMappings[bufDesc.m_format].m_rtFormat;
		if (mapping.m_fallback == glBufferFormat_Unknown)
			return UE_FALSE;
		rtOnlyFormat = mapping.m_fallback;
	}

	glBufferFormat rtAndTextureFormat = bufDesc.m_format;
	if (needContent && !info.m_supportedAsRTAndTexture)
	{
		const glOpenGLBufferFormat& mapping = device->m_bufferFormatMappings[bufDesc.m_format].m_rtTexFormat;
		if (mapping.m_fallback == glBufferFormat_Unknown)
			return UE_FALSE;
		rtAndTextureFormat = mapping.m_fallback;
	}

	if (needsRTOnly && rtOnlyFormat != rtAndTextureFormat)
		return UE_FALSE;

	bufDesc.m_format = needContent ? rtAndTextureFormat : rtOnlyFormat;

#else

	switch (bufDesc.m_format)
	{
	case glBufferFormat_Depth:
		bufDesc.m_format = glBufferFormat_D24S8;
		break;
	}

#endif

	return UE_TRUE;
}

glRenderGroup* glRenderGroup_Create(const glRenderGroupDesc* desc)
{
	glRenderGroup* group = new(GLDEV->m_renderBufferGroupsPool) glRenderGroup();
	UE_ASSERT(group);

	group->m_state = glRenderGroupState_Initial;
	group->m_currentContext = NULL;

	group->m_desc = *desc;

	group->m_depthStencilRenderTarget = NULL;
	ueMemZeroS(group->m_colorRenderTargets);
	group->m_depthStencilTexture = NULL;
	ueMemZeroS(group->m_colorTextures);

	// Determine compatible formats

	const u32 maxLevels = glUtils_CalcNumMipLevels(desc->m_width, desc->m_height, 0);

	if (group->m_desc.m_hasDepthStencil)
	{
		if (group->m_desc.m_depthStencilDesc.m_numLevels == 0)
			group->m_desc.m_depthStencilDesc.m_numLevels = maxLevels;

		UE_ASSERT_FUNC(glRenderGroup_GetClosestSupportedFormat(
			group,
			group->m_desc.m_depthStencilDesc,
			group->m_desc.m_needDepthStencilContent));
	}

	for (u32 i = 0; i < group->m_desc.m_numColorBuffers; i++)
	{
		if (group->m_desc.m_colorDesc[i].m_numLevels == 0)
			group->m_desc.m_colorDesc[i].m_numLevels = maxLevels;

		UE_ASSERT_FUNC(glRenderGroup_GetClosestSupportedFormat(
			group,
			group->m_desc.m_colorDesc[i],
			group->m_desc.m_needColorContent));
	}

	// Verify that size of a pixel for each color buffer is the same (MRT requirement)

	const u32 firstColorRTPixelSize = glUtils_GetFormatBytes(group->m_desc.m_colorDesc[0].m_format);
	for (u32 i = 1; i < desc->m_numColorBuffers; i++)
	{
		const u32 nonFirstRTPixelSize = glUtils_GetFormatBytes(group->m_desc.m_colorDesc[i].m_format);
		if (firstColorRTPixelSize != nonFirstRTPixelSize)
		{
			ueLogW("Failed to create render group, reason: color render target pixel sizes don't match (RT 0 (%s) pixel size is %u, RT %u (%s) pixel size is %u)",
				firstColorRTPixelSize, glBufferFormat_ToString(group->m_desc.m_colorDesc[0].m_format),
				i, nonFirstRTPixelSize, glBufferFormat_ToString(group->m_desc.m_colorDesc[i].m_format));
			GLDEV->m_renderBufferGroupsPool.Free(group);
			return NULL;
		}
	}

#if defined(GL_OPENGL)

	// Verify that the render group set up is correct

	if (!group->m_desc.m_isMainFrameBuffer && !glRenderGroup_CheckSupport(group))
	{
		GLDEV->m_renderBufferGroupsPool.Free(group);
		return NULL;
	}

#endif

#if defined(GL_X360)
	group->CalculateOptimalTiling();
#endif

	return group;
}

void glRenderGroup_Destroy(glRenderGroup* group)
{
	UE_ASSERT(group->m_state == glRenderGroupState_Initial);
	GLDEV->m_renderBufferGroupsPool.Free(group);
}

void glRenderGroup_SetDepthStencilRenderTarget(glRenderGroup* group, glTextureBuffer* tb)
{
	UE_ASSERT(group->m_state == glRenderGroupState_Initial);
	UE_ASSERT(!group->m_depthStencilRenderTarget);
	UE_ASSERT(group->m_desc.m_hasDepthStencil);
	UE_ASSERT(group->m_desc.m_MSAALevel == tb->m_desc.m_rt.m_MSAALevel);
	UE_ASSERT(group->m_desc.m_width == tb->m_desc.m_width);
	UE_ASSERT(group->m_desc.m_height == tb->m_desc.m_height);
	UE_ASSERT(group->m_desc.m_depthStencilDesc.m_format == tb->m_desc.m_format);
	UE_ASSERT(group->m_desc.m_depthStencilDesc.m_numLevels == tb->m_desc.m_numLevels);
	UE_ASSERT(glTextureBuffer_IsRenderTarget(tb));

	group->m_depthStencilRenderTarget = tb;
}

void glRenderGroup_SetColorRenderTarget(glRenderGroup* group, u32 index, glTextureBuffer* tb)
{
	UE_ASSERT(group->m_state == glRenderGroupState_Initial);
	UE_ASSERT(index < group->m_desc.m_numColorBuffers);
	UE_ASSERT(!group->m_colorRenderTargets[index]);
	UE_ASSERT(group->m_desc.m_MSAALevel == tb->m_desc.m_rt.m_MSAALevel);
	UE_ASSERT(group->m_desc.m_width == tb->m_desc.m_width);
	UE_ASSERT(group->m_desc.m_height == tb->m_desc.m_height);
	UE_ASSERT(group->m_desc.m_colorDesc[index].m_format == tb->m_desc.m_format);
	UE_ASSERT(group->m_desc.m_colorDesc[index].m_numLevels == tb->m_desc.m_numLevels);
	UE_ASSERT(glTextureBuffer_IsRenderTarget(tb));

	group->m_colorRenderTargets[index] = tb;
}

void glRenderGroup_GetBuffers(glRenderGroup* group, const glRenderGroupDesc::BufferDesc& desc, ueBool needContent, glTextureBuffer*& renderTarget, glTextureBuffer*& texture)
{
	// Get render target

	glTextureBufferDesc texDesc;
	texDesc.m_format = desc.m_format;
#if defined(GL_X360)
	texDesc.m_width = group->m_tileInfo.m_maxRectWidth;
	texDesc.m_height = group->m_tileInfo.m_maxRectHeight;
#else
	texDesc.m_width = group->m_desc.m_width;
	texDesc.m_height = group->m_desc.m_height;
#endif
	texDesc.m_numLevels = glRenderGroup_RequiresSeparateOutputTextures(group) ? 1 : desc.m_numLevels;
	texDesc.m_enableAutoMips = glRenderGroup_RequiresSeparateOutputTextures(group) ? UE_FALSE : desc.m_enableAutoMips;
	texDesc.m_rt.m_MSAALevel = group->m_desc.m_MSAALevel;
	texDesc.m_flags =
		glTextureBufferFlags_IsRenderTarget |
		((!glRenderGroup_RequiresSeparateOutputTextures(group) && needContent) ? glTextureBufferFlags_IsTexture : 0);

	if (!renderTarget)
	{
		renderTarget = glRenderBufferPool_GetTextureBuffer(texDesc, UE_TRUE, group);
		UE_ASSERT(renderTarget);
	}

	// Get texture

	if (needContent && !texture)
	{
		if (glRenderGroup_RequiresSeparateOutputTextures(group))
		{
			texDesc.m_rt.m_MSAALevel = 1;
			texDesc.m_flags =
				glTextureBufferFlags_IsTexture |
				glTextureBufferFlags_IsRenderTarget |
				glTextureBufferFlags_IsDynamic;
			texDesc.m_numLevels = desc.m_numLevels;
			texDesc.m_enableAutoMips = desc.m_enableAutoMips;

			texture = glRenderBufferPool_GetTextureBuffer(texDesc, UE_TRUE, group);
			UE_ASSERT(texture);
		}
		else
			texture = renderTarget;
	}
}

void glRenderGroup_Begin(glRenderGroup* group, glCtx* ctx, u32 clearFlags)
{
	UE_ASSERT(group->m_state == glRenderGroupState_Initial);

	// Get all buffers

	if (group->m_desc.m_hasDepthStencil)
		glRenderGroup_GetBuffers(
			group,
			group->m_desc.m_depthStencilDesc,
			group->m_desc.m_needDepthStencilContent,
			group->m_depthStencilRenderTarget,
			group->m_depthStencilTexture);

	for (u32 i = 0; i < group->m_desc.m_numColorBuffers; i++)
		glRenderGroup_GetBuffers(
			group,
			group->m_desc.m_colorDesc[i],
			group->m_desc.m_needColorContent,
			group->m_colorRenderTargets[i],
			group->m_colorTextures[i]);

#if defined(GL_X360)

	// Set up EDRAM offsets for each X360 render target

	if (group->m_depthStencilRenderTarget)
	{
		IDirect3DSurface9* surface = group->m_depthStencilBuffer->m_surfaceHandle;
		surface->DepthInfo.DepthBase = group->m_tileInfo.m_depthStencilEDRAMOffset;
		surface->HiControl.HiBaseAddr = group->m_tileInfo.m_hierarchicalZOffset;
	}
	for (u32 i = 0; i < group->m_desc.m_numColorBuffers; i++)
	{
		IDirect3DSurface9* surface = group->m_colorRenderTargets[i]->m_surfaceHandle;
		surface->ColorInfo.ColorBase = group->m_tileInfo.m_colorEDRAMOffsets[i];
		surface->HiControl.HiBaseAddr = group->m_tileInfo.m_hierarchicalZOffset;
	}

#endif // defined(GL_X360)

	// All buffers in place - set them on a device

	group->m_currentContext = ctx;
	glCtx_SetRenderGroup(ctx, group);

#if defined(GL_X360)
	glCtx_BeginTiling(ctx, &group->m_tileInfo, clearFlags);
#else
	glCtx_Clear(ctx, clearFlags);
#endif

	group->m_state = glRenderGroupState_Begun;
}

void glRenderGroup_EndDrawing(glRenderGroup* group)
{
	UE_ASSERT(group->m_state == glRenderGroupState_Begun);

	glCtx_ResolveRenderTargets(group->m_currentContext);
#if defined(GL_X360)
	glCtx_EndTiling(group->m_currentContext);
#endif

	group->m_state = glRenderGroupState_Resolved;
}

glTextureBuffer* glRenderGroup_AcquireDepthStencilOutput(glRenderGroup* group)
{
	UE_ASSERT(group->m_state == glRenderGroupState_Resolved);

	return (glRenderGroup_RequiresSeparateOutputTextures(group) && group->m_desc.m_needDepthStencilContent) ?
		glRenderGroup_AcquireDepthStencilTexture(group) :
		glRenderGroup_AcquireDepthStencilRenderTarget(group);
}

glTextureBuffer* glRenderGroup_AcquireColorOutput(glRenderGroup* group, u32 index)
{
	UE_ASSERT(group->m_state == glRenderGroupState_Resolved);

	return (glRenderGroup_RequiresSeparateOutputTextures(group) && group->m_desc.m_needColorContent) ?
		glRenderGroup_AcquireColorTexture(group, index) :
		glRenderGroup_AcquireColorRenderTarget(group, index);
}

glTextureBuffer* glRenderGroup_AcquireDepthStencilRenderTarget(glRenderGroup* group)
{
	UE_ASSERT(group->m_state == glRenderGroupState_Resolved);
	UE_ASSERT(group->m_depthStencilRenderTarget);

	if (group->m_depthStencilTexture == group->m_depthStencilRenderTarget)
		group->m_depthStencilTexture = NULL;

	glTextureBuffer* buffer = group->m_depthStencilRenderTarget;
	group->m_depthStencilRenderTarget = NULL;

	return buffer;
}

glTextureBuffer* glRenderGroup_AcquireColorRenderTarget(glRenderGroup* group, u32 index)
{
	UE_ASSERT(group->m_state == glRenderGroupState_Resolved);
	UE_ASSERT(group->m_colorRenderTargets[index]);

	if (group->m_colorTextures[index] == group->m_colorRenderTargets[index])
		group->m_colorTextures[index] = NULL;

	glTextureBuffer* buffer = group->m_colorRenderTargets[index];
	group->m_colorRenderTargets[index] = NULL;

	return buffer;
}

glTextureBuffer* glRenderGroup_AcquireDepthStencilTexture(glRenderGroup* group)
{
	UE_ASSERT(group->m_state == glRenderGroupState_Resolved);
	UE_ASSERT(group->m_depthStencilTexture);

	if (group->m_depthStencilTexture == group->m_depthStencilRenderTarget)
		group->m_depthStencilRenderTarget = NULL;

	glTextureBuffer* buffer = group->m_depthStencilTexture;
	group->m_depthStencilTexture = NULL;

	return buffer;
}

glTextureBuffer* glRenderGroup_AcquireColorTexture(glRenderGroup* group, u32 index)
{
	UE_ASSERT(group->m_state == glRenderGroupState_Resolved);
	UE_ASSERT(group->m_colorTextures[index]);

	if (group->m_colorTextures[index] == group->m_colorRenderTargets[index])
		group->m_colorRenderTargets[index] = NULL;

	glTextureBuffer* buffer = group->m_colorTextures[index];
	group->m_colorTextures[index] = NULL;

	return buffer;
}

void glRenderGroup_End(glRenderGroup* group)
{
	UE_ASSERT(group->m_state == glRenderGroupState_Resolved);

	// Release all render targets and textures

	if (group->m_depthStencilTexture && group->m_depthStencilTexture != group->m_depthStencilRenderTarget)
	{
		glRenderBufferPool_ReleaseTextureBuffer(group->m_depthStencilTexture);
		group->m_depthStencilTexture = NULL;
	}

	if (group->m_depthStencilRenderTarget)
	{
		glRenderBufferPool_ReleaseTextureBuffer(group->m_depthStencilRenderTarget);
		group->m_depthStencilRenderTarget = NULL;
	}

	for (u32 i = 0; i < group->m_desc.m_numColorBuffers; i++)
	{
		if (group->m_colorTextures[i])
		{
			if (group->m_colorTextures[i] != group->m_colorRenderTargets[i])
				glRenderBufferPool_ReleaseTextureBuffer(group->m_colorTextures[i]);
			group->m_colorTextures[i] = NULL;
		}

		if (group->m_colorRenderTargets[i])
		{
			glRenderBufferPool_ReleaseTextureBuffer(group->m_colorRenderTargets[i]);
			group->m_colorRenderTargets[i] = NULL;
		}
	}

	// Change state

	group->m_state = glRenderGroupState_Initial;
	group->m_currentContext = NULL;
}

const glRenderGroupDesc* glRenderGroup_GetDesc(glRenderGroup* group)
{
	return &group->m_desc;
}
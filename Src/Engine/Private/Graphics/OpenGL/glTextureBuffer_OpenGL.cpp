#include "Base/ueBase.h"

#if defined(GL_OPENGL)

#include "Graphics/glLib_Private.h"
#include "IO/ueBuffer.h"

UE_INLINE void glTextureBuffer_GetGLBindingAndTarget(glTextureBuffer* tb, GLenum& binding, GLenum& target)
{
	switch (tb->m_desc.m_type)
	{
		case glTexType_2D:
			binding = GL_TEXTURE_BINDING_2D;
			target = GL_TEXTURE_2D;
			break;
#if !defined(GL_OPENGL_ES)
		case glTexType_3D:
			binding = GL_TEXTURE_BINDING_3D;
			target = GL_TEXTURE_3D;
			break;
#endif
		case glTexType_Cube:
			binding = GL_TEXTURE_BINDING_CUBE_MAP;
			target = GL_TEXTURE_CUBE_MAP;
			break;
		UE_INVALID_CASE(tb->m_desc.m_type);
	}
}

void glTextureBuffer_CreateInPlace(glTextureBuffer* tb, const glTextureInitData* initData)
{
	UE_ASSERT(glTextureBuffer_IsRenderTarget(tb) || glTextureBuffer_IsTexture(tb));

	tb->m_textureHandle = 0;
	tb->m_renderBufferHandle = 0;
	ueMemZeroS(tb->m_pbos);
	tb->m_samplerParams.m_maxMipLevel = 555; // Set something that doesn't make sense, so it will be picked by the lazy sampler state setting function in the context update

	// Load / convert from source format?

	if (initData && initData->m_storage != glTexStorage_GPU)
	{
		UE_ASSERT(initData->m_dataArrayLength == 1);
		UE_NOT_IMPLEMENTED();
	}

	// Get OpenGL format

	glOpenGLBufferFormat format;
	UE_ASSERT_FUNC(glUtils_ToOpenGLFormat(tb->m_desc.m_format, tb->m_desc.m_flags, format));

	// Create render buffer

	if (glTextureBuffer_IsRenderTarget(tb) && !glTextureBuffer_IsTexture(tb))
	{

#if defined(UE_ENABLE_ASSERTION)

		// Verify MSAA support

		UE_ASSERT(tb->m_desc.m_rt.m_MSAALevel == 1 || GLEW_EXT_framebuffer_multisample);

		const glCaps* caps = glDevice_GetCaps();
		ueBool isMSAALevelSupported = tb->m_desc.m_rt.m_MSAALevel == 1;
		if (!isMSAALevelSupported)
			for (u32 i = 0; i < caps->m_numMSAALevels; i++)
				if (tb->m_desc.m_rt.m_MSAALevel == caps->m_MSAALevels[i])
				{
					isMSAALevelSupported = UE_TRUE;
					break;
				}
		UE_ASSERT(isMSAALevelSupported);

#endif // defined(UE_ENABLE_ASSERTION)

		UE_ASSERT(!tb->m_desc.m_enableAutoMips);
		UE_ASSERT(tb->m_desc.m_numLevels == 1);

		GLint oldBuf;
		GL(glGetIntegerv(GL_RENDERBUFFER_BINDING_EXT, &oldBuf));

		GL(glGenRenderbuffersEXT(1, &tb->m_renderBufferHandle));
		GL(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, tb->m_renderBufferHandle));
		if (tb->m_desc.m_rt.m_MSAALevel > 1)
			GL(glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, tb->m_desc.m_rt.m_MSAALevel, format.m_internalFormat, tb->m_desc.m_width, tb->m_desc.m_height));
		else
			GL(glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, format.m_internalFormat, tb->m_desc.m_width, tb->m_desc.m_height));

		GL(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, oldBuf));
	}

	// Create texture

	else
	{
		UE_ASSERT(!glTextureBuffer_IsRenderTarget(tb) || tb->m_desc.m_rt.m_MSAALevel == 1);

		GLenum binding, target;
		glTextureBuffer_GetGLBindingAndTarget(tb, binding, target);

		GLint oldTex;
		GL(glGetIntegerv(binding, &oldTex));

		GL(glGenTextures(1, &tb->m_textureHandle));
		GL(glBindTexture(target, tb->m_textureHandle));

#ifndef GL_OPENGL_ES
		GL(glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, tb->m_desc.m_numLevels - 1));
		if (tb->m_desc.m_enableAutoMips && !glTextureBuffer_IsRenderTarget(tb) &&
			!GLEW_VERSION_3_0 && !GLEW_EXT_framebuffer_object) // GL_GENERATE_MIPMAP deprecated since GL 3.0
			GL(glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE));
#endif

		u32 levelWidth = tb->m_desc.m_width;
		u32 levelHeight = tb->m_desc.m_height;
		u32 levelDepth = tb->m_desc.m_depth;

		const ueBool is4x4Compressed = glUtils_Is4x4CompressedFormat(tb->m_desc.m_format);

		for (u32 i = 0; i < tb->m_desc.m_numLevels; i++)
		{
			const u32 levelSize = glUtils_CalcSurfaceSize(tb->m_desc.m_type, levelWidth, levelHeight, levelDepth, tb->m_desc.m_format);

			switch (tb->m_desc.m_type)
			{
				case glTexType_2D:
				{
					const void* data = (initData && i < initData->m_dataArrayLength) ? initData->m_dataArray[i].m_data : NULL;
					if (is4x4Compressed)
						GL(glCompressedTexImage2D(GL_TEXTURE_2D, i, format.m_internalFormat, levelWidth, levelHeight, 0, levelSize, data));
					else
						GL(glTexImage2D(GL_TEXTURE_2D, i, format.m_internalFormat, levelWidth, levelHeight, 0, format.m_format, format.m_type, data));
					break;
				}
#if !defined(GL_OPENGL_ES)
				case glTexType_3D:
				{
					const void* data = (initData && i < initData->m_dataArrayLength) ? initData->m_dataArray[i].m_data : NULL;
					if (is4x4Compressed)
						GL(glCompressedTexImage3D(GL_TEXTURE_3D, i, format.m_internalFormat, levelWidth, levelHeight, levelDepth, 0, levelSize, data));
					else
						GL(glTexImage3D(GL_TEXTURE_3D, i, format.m_internalFormat, levelWidth, levelHeight, levelDepth, 0, format.m_format, format.m_type, data));
					break;
				}
#endif // !defined(GL_OPENGL_ES)
				case glTexType_Cube:
					for (u32 j = 0; j < 6; j++)
					{
						const u32 initDataIndex = i + j * tb->m_desc.m_numLevels;
						const void* data = (initData && initDataIndex < initData->m_dataArrayLength) ? initData->m_dataArray[initDataIndex].m_data : NULL;
						if (is4x4Compressed)
							GL(glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, i, format.m_internalFormat, levelWidth, levelHeight, 0, levelSize, data));
						else
							GL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, i, format.m_internalFormat, levelWidth, levelHeight, 0, format.m_format, format.m_type, data));
					}
					break;
			}

			if (levelWidth > 1) levelWidth /= 2;
			if (levelHeight > 1) levelHeight /= 2;
			if (levelDepth > 1) levelDepth /= 2;
		}

		GL(glBindTexture(target, oldTex));
	}
}

void glTextureBuffer_DestroyInPlace(glTextureBuffer* tb)
{
#if defined(UE_ENABLE_ASSERTION)
	if (!tb->m_isMainFrameBuffer)
		for (u32 i = 0; i < tb->m_desc.m_numLevels; i++)
			UE_ASSERT(!tb->m_pbos[i]);
#endif

	if (tb->m_textureHandle)
	{
		GL(glDeleteTextures(1, &tb->m_textureHandle));
		tb->m_textureHandle = 0;
	}
	else if (tb->m_renderBufferHandle)
	{
		GL(glDeleteRenderbuffersEXT(1, &tb->m_renderBufferHandle));
		tb->m_renderBufferHandle = 0;
	}
}

ueBool glTextureBuffer_Lock(glTextureBuffer* tb, glTextureData* result, u32 face, u32 level, u32 lockFlags, const glTextureArea* _area)
{
	UE_ASSERT(glTextureBuffer_IsTexture(tb));
	UE_ASSERT_MSGP(!tb->m_pbos[level], "Mip-level %u already locked", level);
	UE_ASSERT_MSG(!(lockFlags & glBufferLockFlags_Read), "No support (yet) for locking textures for reading");

	u32 levelWidth, levelHeight, levelDepth;
	glUtils_CalcMipLevelDimensions(tb->m_desc.m_width, tb->m_desc.m_height, tb->m_desc.m_depth, level, levelWidth, levelHeight, levelDepth);

	// Figure out area to lock

	glTextureArea area;
	if (_area)
		area = *_area;
	else
	{
		area.m_x = area.m_y = area.m_z = 0;
		area.m_width = levelWidth;
		area.m_height = levelHeight;
		area.m_depth = levelDepth;
	}

#if defined(UE_ENABLE_ASSERTION)

	// Can only copy whole rectangle / box if compressed (glCompressedTexSubImage2D crashing for me on ATI 5650)

	if (glUtils_Is4x4CompressedFormat(tb->m_desc.m_format))
		switch (tb->m_desc.m_type)
		{
		case glTexType_3D:
			UE_ASSERT(area.m_z == 0 && area.m_depth == levelDepth);
		case glTexType_2D:
		case glTexType_Cube:
			UE_ASSERT(area.m_x == 0 && area.m_y == 0 && area.m_width == levelWidth && area.m_height == levelHeight);
			break;
		}

#endif

	const u32 dataSize = glUtils_CalcSurfaceSize(tb->m_desc.m_type, area.m_width, area.m_height, area.m_depth, tb->m_desc.m_format);

	// Get and map PBO

	glPBO* pbo = tb->m_pbos[level] =
#if !defined(GL_OPENGL_ES)
		glPBO_Map(GL_PIXEL_UNPACK_BUFFER, GL_STREAM_DRAW_ARB, GL_WRITE_ONLY_ARB, dataSize);
#else
		glPBO_Map(GL_NONE, GL_NONE, GL_NONE, dataSize); // PBO emulation
#endif
	if (!pbo)
		return UE_FALSE;
	pbo->m_lockedArea = area;
	pbo->m_lockedSize = dataSize;

	// Set up result

	result->m_data = pbo->m_lockedData;
	u32 numRows;
	glUtils_CalcSurfaceRowParams(area.m_width, area.m_height, area.m_depth, tb->m_desc.m_format, result->m_rowPitch, numRows);
	result->m_slicePitch = result->m_rowPitch * numRows;

	return UE_TRUE;
}

void glTextureBuffer_Unlock(glTextureBuffer* tb, u32 face, u32 level)
{
	GLenum binding, target;
	glTextureBuffer_GetGLBindingAndTarget(tb, binding, target);

	GLint oldTex;
	GL(glGetIntegerv(binding, &oldTex));

	// Unmap and bind PBO (source)

	glPBO* pbo = tb->m_pbos[level];
	UE_ASSERT(pbo);
	glPBO_UnmapAndBind(pbo);

	// Bind texture (destination)

	GL(glBindTexture(target, tb->m_textureHandle));

	// Transfer from PBO to texture

	glOpenGLBufferFormat format;
	UE_ASSERT_FUNC(glUtils_ToOpenGLFormat(tb->m_desc.m_format, tb->m_desc.m_flags, format));

	switch (tb->m_desc.m_type)
	{
		case glTexType_2D:
			if (glUtils_Is4x4CompressedFormat(tb->m_desc.m_format))
				GL(glCompressedTexImage2D(GL_TEXTURE_2D, level, format.m_internalFormat, pbo->m_lockedArea.m_width, pbo->m_lockedArea.m_height, 0, pbo->m_lockedSize, pbo->m_texOpData));
			else
				GL(glTexSubImage2D(GL_TEXTURE_2D, level, pbo->m_lockedArea.m_x, pbo->m_lockedArea.m_y, pbo->m_lockedArea.m_width, pbo->m_lockedArea.m_height, format.m_format, format.m_type, pbo->m_texOpData));
			break;
#if !defined(GL_OPENGL_ES)
		case glTexType_3D:
			if (glUtils_Is4x4CompressedFormat(tb->m_desc.m_format))
				GL(glCompressedTexImage3D(GL_TEXTURE_3D, level, format.m_internalFormat, pbo->m_lockedArea.m_width, pbo->m_lockedArea.m_height, pbo->m_lockedArea.m_depth, 0, pbo->m_lockedSize, pbo->m_texOpData));
			else
				GL(glTexSubImage3D(GL_TEXTURE_3D, level, pbo->m_lockedArea.m_x, pbo->m_lockedArea.m_y, pbo->m_lockedArea.m_z, pbo->m_lockedArea.m_width, pbo->m_lockedArea.m_height, pbo->m_lockedArea.m_depth, format.m_format, format.m_type, pbo->m_texOpData));
			break;
#endif
		case glTexType_Cube:
			if (glUtils_Is4x4CompressedFormat(tb->m_desc.m_format))
				GL(glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, format.m_internalFormat, pbo->m_lockedArea.m_width, pbo->m_lockedArea.m_height, 0, pbo->m_lockedSize, pbo->m_texOpData));
			else
				GL(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, pbo->m_lockedArea.m_x, pbo->m_lockedArea.m_y, pbo->m_lockedArea.m_width, pbo->m_lockedArea.m_height, format.m_format, format.m_type, pbo->m_texOpData));
			break;
	}

	// Release PBO

	glPBO_Release(pbo);
	tb->m_pbos[level] = NULL;

#if !defined(GL_OPENGL_ES)

	// Update mip levels

	if (level == 0 && tb->m_desc.m_enableAutoMips)
	{
		GL(glEnable(target));
		if (GLEW_VERSION_3_0)
			GL(glGenerateMipmap(target));
		else if (GLEW_EXT_framebuffer_object)
			GL(glGenerateMipmapEXT(target));
	}

#endif

	// Restore previous texture

	GL(glBindTexture(target, oldTex));
}

u32 glTextureBuffer_Update(glTextureBuffer* tb, u32 face, u32 level, const glTextureArea* _area, const void* data)
{
	UE_ASSERT(glTextureBuffer_IsTexture(tb));
	UE_ASSERT_MSGP(!tb->m_pbos[level], "Can't upload. Mip-level %u is locked", level);

	// Store previous texture handle

	GLenum binding, target;
	glTextureBuffer_GetGLBindingAndTarget(tb, binding, target);

	GLint oldTex;
	GL(glGetIntegerv(binding, &oldTex));

	// Get level dimensions

	u32 levelWidth, levelHeight, levelDepth;
	glUtils_CalcMipLevelDimensions(tb->m_desc.m_width, tb->m_desc.m_height, tb->m_desc.m_depth, level, levelWidth, levelHeight, levelDepth);

	// Figure out area to copy to

	glTextureArea area;
	if (_area)
		area = *_area;
	else
	{
		area.m_x = area.m_y = area.m_z = 0;
		area.m_width = levelWidth;
		area.m_height = levelHeight;
		area.m_depth = levelDepth;
	}

#if defined(UE_ENABLE_ASSERTION)

	// Can only copy whole rectangle / box if compressed (glCompressedTexSubImage2D crashing for me on ATI 5650)

	if (glUtils_Is4x4CompressedFormat(tb->m_desc.m_format))
		switch (tb->m_desc.m_type)
		{
		case glTexType_3D:
			UE_ASSERT(area.m_z == 0 && area.m_depth == levelDepth);
		case glTexType_2D:
		case glTexType_Cube:
			UE_ASSERT(area.m_x == 0 && area.m_y == 0 && area.m_width == levelWidth && area.m_height == levelHeight);
			break;
		}
#endif

	u32 rowSize, numRows;
	glUtils_CalcSurfaceRowParams(area.m_width, area.m_height, area.m_depth, tb->m_desc.m_format, rowSize, numRows);
	const u32 dataSize = rowSize * numRows * levelDepth;

	// Bind texture (destination)

	GL(glBindTexture(target, tb->m_textureHandle));

	// Copy data

	glOpenGLBufferFormat format;
	UE_ASSERT_FUNC(glUtils_ToOpenGLFormat(tb->m_desc.m_format, tb->m_desc.m_flags, format));

	switch (tb->m_desc.m_type)
	{
		case glTexType_2D:
			if (glUtils_Is4x4CompressedFormat(tb->m_desc.m_format))
				GL(glCompressedTexImage2D(GL_TEXTURE_2D, level, format.m_internalFormat, area.m_width, area.m_height, 0, dataSize, data));
			else
				GL(glTexSubImage2D(GL_TEXTURE_2D, level, area.m_x, area.m_y, area.m_width, area.m_height, format.m_format, format.m_type, data));
			break;
#if !defined(GL_OPENGL_ES)
		case glTexType_3D:
			if (glUtils_Is4x4CompressedFormat(tb->m_desc.m_format))
				GL(glCompressedTexImage3D(GL_TEXTURE_3D, level, format.m_internalFormat, area.m_width, area.m_height, area.m_depth, 0, dataSize, data));
			else
				GL(glTexSubImage3D(GL_TEXTURE_3D, level, area.m_x, area.m_y, area.m_z, area.m_width, area.m_height, area.m_depth, format.m_format, format.m_type, data));
			break;
#endif
		case glTexType_Cube:
			if (glUtils_Is4x4CompressedFormat(tb->m_desc.m_format))
				GL(glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, format.m_internalFormat, area.m_width, area.m_height, 0, dataSize, data));
			else
				GL(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, area.m_x, area.m_y, area.m_width, area.m_height, format.m_format, format.m_type, data));
			break;
	}

#if !defined(GL_OPENGL_ES)

	// Update mip levels

	if (level == 0 && tb->m_desc.m_enableAutoMips)
	{
		GL(glEnable(target));
		if (GLEW_VERSION_3_0)
			GL(glGenerateMipmap(target));
		else if (GLEW_EXT_framebuffer_object)
			GL(glGenerateMipmapEXT(target));
	}

#endif

	// Restore previous texture

	GL(glBindTexture(target, oldTex));

	return dataSize;
}

void glTextureBuffer_GenerateMips(glTextureBuffer* tb)
{
	UE_ASSERT(glTextureBuffer_IsTexture(tb) && !tb->m_desc.m_enableAutoMips);

	if (GLEW_VERSION_3_0 || GLEW_EXT_framebuffer_object)
	{
		GLenum binding, target;
		glTextureBuffer_GetGLBindingAndTarget(tb, binding, target);

		GLint oldTex;
		GL(glGetIntegerv(binding, &oldTex));

		GL(glBindTexture(target, tb->m_textureHandle));
		GL(glEnable(target));
		if (GLEW_VERSION_3_0)
			GL(glGenerateMipmap(target));
		else
			GL(glGenerateMipmapEXT(target));

		GL(glBindTexture(target, oldTex));
	}
	else
	{
		// Hmm, gluBuild*DMipmaps is going to be extremely slow - need to download texture data to system memory
		// gluBuild2DMipmaps(GLU_TEXTURE_2D, , ...
		UE_NOT_IMPLEMENTED();
	}
}

#else // defined(GL_OPENGL)
	UE_NO_EMPTY_FILE
#endif
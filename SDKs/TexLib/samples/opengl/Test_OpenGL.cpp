#include <stdio.h>    /* for printf and NULL */
#include <stdlib.h>   /* for exit */
#include <math.h>     /* for sqrt, sin, and cos */
#include <assert.h>   /* for assert */
#include <windows.h>   /* for assert */

#include <GL/glew.h>
#include "TexLib.h"

void Tex::Create()
{
}

void* Tex::Lock(int face, int level)
{
	assert(!m_locked);

	int width = max(1, m_width >> level);
	int height = max(1, m_height >> level);

	int size = 0;
	if (IsDXT(m_format))
	{
		m_lockedRowPitch = ((width + 3) / 4) * (IsDXT1(m_format) ? 8 : 16);
		size = m_lockedRowPitch * ((height + 3) / 4);
	}
	else
	{
		m_lockedRowPitch = width * NumBytesPerPixel(m_format);
		size = m_lockedRowPitch * height;
	}

	m_locked = malloc(size);
	m_lockedSize = size;

	return m_locked;
}

void Tex::Unlock(int face, int level)
{
	assert(m_locked);

	int width = max(1, m_width >> level);
	int height = max(1, m_height >> level);

	if (IsDXT(m_format))
	{
		GLenum internalFormat;
		switch (m_format)
		{
		case TexLib_Format_DXT1:
			internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			break;
		case TexLib_Format_DXT5:
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
		default:
			assert(!"Unsupported DXT format");
		}

		glCompressedTexImage2D(GL_TEXTURE_2D, level, internalFormat, width, height, 0, m_lockedSize, m_locked);
		GLenum error = glGetError();
		assert(error == GL_NO_ERROR);
	}
	else
	{
		GLenum internalFormat, format, type;
		if (m_format == TexLib_Format_ARGB)
		{
			internalFormat = GL_RGBA8;
			format = GL_BGRA;
			type = GL_UNSIGNED_BYTE;
		}
		else if (m_format == TexLib_Format_RGB)
		{
			internalFormat = GL_RGB8;
			format = GL_BGR;
			type = GL_UNSIGNED_BYTE;
		}
		else
			assert(!"Unsupported format");
		glTexImage2D(GL_TEXTURE_2D, level, internalFormat, width, height, 0, format, type, m_locked);
		GLenum error = glGetError();
		assert(error == GL_NO_ERROR);
	}

	free(m_locked);
	m_locked = NULL;
}

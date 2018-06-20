#include "Base/ueBase.h"

#if defined(GL_OPENGL)

#include "Graphics/glLib_Private.h"
#include "IO/ueBuffer.h"

void glBuffer_CreateInPlace(glBuffer* buf, const void* initData)
{
	GLint oldBuf;
	GL(glGetIntegerv(buf->m_binding, &oldBuf));

	GL(glGenBuffers(1, &buf->m_handle));
	GL(glBindBuffer(buf->m_target, buf->m_handle));
	GL(glBufferData(buf->m_target, buf->m_size, initData, buf->m_usage));

	GL(glBindBuffer(buf->m_target, oldBuf));

	buf->m_isLocked = UE_FALSE;
}

void glBuffer_DestroyInPlace(glBuffer* buf)
{
	UE_ASSERT(buf->m_handle);
	UE_ASSERT(!buf->m_isLocked);
	GL(glDeleteBuffers(1, &buf->m_handle));
	buf->m_handle = 0;
}

void* glBuffer_Lock(glBuffer* buf, u32 lockFlags, u32 offset, u32 size)
{
	UE_ASSERT(!buf->m_isLocked);

	if (!size)
		size = buf->m_size;

	UE_ASSERT(offset + size <= buf->m_size);

	// Store previous buffer

	GLint oldBuf;
	GL(glGetIntegerv(buf->m_binding, &oldBuf));

	// Lock

	GL(glBindBuffer(buf->m_target, buf->m_handle));

	// Lock (use system memory for small sizes and buffer mapping for large ones)

	void* data = NULL;

#if !defined(GL_OPENGL_ES)
	if (!GLEW_ARB_map_buffer_range && size < buf->m_size)
#endif
	{
		buf->m_scratchPad = GL_FREQ_ALLOC->Alloc(size);
		if (!buf->m_scratchPad)
			return NULL;
		buf->m_scratchPadOffset = offset;
		buf->m_scratchPadSize = size;
		buf->m_lockFlags = lockFlags;

		if (lockFlags & glBufferLockFlags_Read)
#if !defined(GL_OPENGL_ES)
			GL(glGetBufferSubData(buf->m_target, buf->m_scratchPadOffset, buf->m_scratchPadSize, buf->m_scratchPad));
#else
			UE_ASSERT(0);
#endif

		data = buf->m_scratchPad;
	}
#if !defined(GL_OPENGL_ES)
	else
	{
		if (lockFlags & glBufferLockFlags_Discard)
			GL(glBufferData(buf->m_target, buf->m_size, NULL, buf->m_usage));

		// Take advantage of glMapBufferRange if available

		if (GLEW_ARB_map_buffer_range && size < buf->m_size)
		{
			const ueBool isRead = (lockFlags & glBufferLockFlags_Read);
			const GLbitfield access =
				((lockFlags & glBufferLockFlags_Write) ? (GL_MAP_WRITE_BIT | (isRead ? 0 : GL_MAP_INVALIDATE_RANGE_BIT)) : 0) |
				(isRead ? GL_MAP_READ_BIT : 0);
			 
			data = GLC(glMapBufferRange(buf->m_target, offset, size, access));
			if (!data)
				return NULL;
		}
		else
		{
			GLenum access;
			if (lockFlags & glBufferLockFlags_Write)
				access = (lockFlags & glBufferLockFlags_Read) ? GL_READ_WRITE : GL_WRITE_ONLY;
			else
				access = GL_READ_ONLY;

			void* mappedBuffer = GLC(glMapBuffer(buf->m_target, access));
			if (!mappedBuffer)
				return NULL;
			data = (u8*) mappedBuffer + offset;
		}

		buf->m_scratchPad = NULL;
	}
#endif

	buf->m_isLocked = UE_TRUE;

	// Restore previous buffer

	GL(glBindBuffer(buf->m_target, oldBuf));

	return data;
}

void glBuffer_Unlock(glBuffer* buf)
{
	UE_ASSERT(buf->m_isLocked);

	// Store previous buffer

	GLint oldBuf;
	GL(glGetIntegerv(buf->m_binding, &oldBuf));

	// Unlock

	GL(glBindBuffer(buf->m_target, buf->m_handle));

#if !defined(GL_OPENGL_ES)
	if (buf->m_scratchPad)
#endif
	{
		if (buf->m_lockFlags & glBufferLockFlags_Discard)
			GL(glBufferData(buf->m_target, buf->m_size, NULL, buf->m_usage));
		if (buf->m_lockFlags & glBufferLockFlags_Write)
			GL(glBufferSubData(buf->m_target, buf->m_scratchPadOffset, buf->m_scratchPadSize, buf->m_scratchPad));
		GL_FREQ_ALLOC->Free(buf->m_scratchPad);
	}
#if !defined(GL_OPENGL_ES)
	else
		GL(glUnmapBuffer(buf->m_target));
#endif

	buf->m_isLocked = UE_FALSE;

	// Restore previous buffer

	GL(glBindBuffer(buf->m_target, oldBuf));
}

// Vertex buffer

void glVertexBuffer_CreateInPlace(glVertexBuffer* vb, const void* initData)
{
	vb->m_size = vb->m_desc.m_size;
	vb->m_usage = (vb->m_desc.m_flags & glBufferFlags_Dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	vb->m_target = GL_ARRAY_BUFFER;
	vb->m_binding = GL_ARRAY_BUFFER_BINDING;

	glBuffer_CreateInPlace(vb, initData);
}

void glVertexBuffer_DestroyInPlace(glVertexBuffer* vb) { glBuffer_DestroyInPlace(vb); }
void* glVertexBuffer_Lock(glVertexBuffer* vb, u32 lockFlags, u32 offset, u32 size) { return glBuffer_Lock(vb, lockFlags, offset, size); }
void glVertexBuffer_Unlock(glVertexBuffer* vb) { glBuffer_Unlock(vb); }

// Index buffer

void glIndexBuffer_CreateInPlace(glIndexBuffer* buf, const void* initData)
{
	buf->m_size = buf->m_desc.m_indexSize * buf->m_desc.m_numIndices;
	buf->m_usage = (buf->m_desc.m_flags & glBufferFlags_Dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	buf->m_target = GL_ELEMENT_ARRAY_BUFFER;
	buf->m_binding = GL_ELEMENT_ARRAY_BUFFER_BINDING;

	glBuffer_CreateInPlace(buf, initData);
}

void glIndexBuffer_DestroyInPlace(glIndexBuffer* buf) { glBuffer_DestroyInPlace(buf); }
void* glIndexBuffer_Lock(glIndexBuffer* buf, u32 lockFlags, u32 offset, u32 size) { return glBuffer_Lock(buf, lockFlags, offset, size); }
void glIndexBuffer_Unlock(glIndexBuffer* buf) { glBuffer_Unlock(buf); }

#else // defined(GL_OPENGL)
	UE_NO_EMPTY_FILE
#endif
#ifndef GL_VERTEX_BUFFER_FACTOR_H
#define GL_VERTEX_BUFFER_FACTOR_H

#include "Graphics/glLib.h"

/**
 *	@addtogroup gl
 *	@{
 */

//! Dynamic vertex buffer wrapper; assures best performance CPU writable vertex buffer via either double-buffering or other platform specific methods
class glVertexBufferFactory
{
public:
	//! Locked vertex buffer chunk
	struct LockedChunk
	{
		glVertexBuffer* m_VB;	//!< Locked vertex buffer
		u32 m_size;				//!< Locked size
		u32 m_offset;			//!< Locked offset
		void* m_data;			//!< Pointer into locked data
	};

	glVertexBufferFactory();
	~glVertexBufferFactory();

	//! Initializes vertex buffer factory of given vertex buffer data size (in bytes)
	void Init(u32 size);
	//! Deinitializes vertex buffer factory
	void Deinit();

	//! Tells whether vertex buffer was initialized
	ueBool IsInitialized() const;

	//! Allocates vertex buffer chunk of given size
	ueBool Allocate(u32 size, LockedChunk* chunk);
	//! Frees / unlocks vertex buffer chunk; rendering using freed vertex buffer chunk should occur immediately
	void Unlock(LockedChunk* chunk);

protected:
	ueBool m_isDoubleBuffered;

	u32 m_size;
	glVertexBuffer* m_buffers[2];

	u32 m_frontBufferId;
	u32 m_currentOffset;

	ueBool m_isLocked;
};

// @}

#endif // GL_VERTEX_BUFFER_FACTOR_H

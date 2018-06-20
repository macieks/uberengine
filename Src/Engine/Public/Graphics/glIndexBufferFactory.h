#ifndef GL_INDEX_BUFFER_FACTOR_H
#define GL_INDEX_BUFFER_FACTOR_H

#include "Graphics/glLib.h"

/**
 *	@addtogroup gl
 *	@{
 */

//! Dynamic index buffer wrapper; assures best performance CPU writable index buffer via either double-buffering or other platform specific methods
class glIndexBufferFactory
{
public:
	//! Locked index buffer chunk
	struct LockedChunk
	{
		glIndexBuffer* m_IB;	//!< Index buffer
		u32 m_firstIndex;		//!< First index
		void* m_data;			//!< Pointer into index buffer data
	};

	glIndexBufferFactory();
	~glIndexBufferFactory();

	//! Initializes index buffer
	void Init(u32 numIndices, u32 indexSize);
	//! Deinitializes index buffer
	void Deinit();

	//! Tells whether index buffer factory was initialized
	ueBool IsInitialized() const;

	//! Allocates index buffer chunk containing numIndices indices
	ueBool Allocate(u32 numIndices, LockedChunk* chunk);
	//! Frees / unlocks index buffer chunk; rendering using freed index buffer chunk should occur immediately
	void Unlock(LockedChunk* chunk);

protected:
	ueBool m_isDoubleBuffered;

	u32 m_numIndices;
	u32 m_indexSize;
	glIndexBuffer* m_buffers[2];

	u32 m_frontBufferId;
	u32 m_currentOffset;

	ueBool m_isLocked;
};

// @}

#endif // GL_INDEX_BUFFER_FACTOR_H

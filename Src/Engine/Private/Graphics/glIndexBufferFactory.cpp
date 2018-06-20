#include "Graphics/glIndexBufferFactory.h"

glIndexBufferFactory::glIndexBufferFactory() :
	m_numIndices(0),
	m_indexSize(sizeof(u16)),
	m_frontBufferId(0),
	m_currentOffset(0),
	m_isLocked(UE_FALSE)
{
	m_buffers[0] = m_buffers[1] = NULL;
}

glIndexBufferFactory::~glIndexBufferFactory()
{
	UE_ASSERT(!IsInitialized());
}

void glIndexBufferFactory::Init(u32 numIndices, u32 indexSize)
{
	UE_ASSERT(!IsInitialized());

	m_numIndices = numIndices;
	m_indexSize = indexSize;

#if !defined(GL_X360) && !defined(GL_PS3)
	m_isDoubleBuffered = UE_FALSE; // Can use discard lock mode
#else
	m_isDoubleBuffered = UE_TRUE;
#endif

	// Create index buffer

	glIndexBufferDesc desc;
	desc.m_numIndices = m_numIndices;
	desc.m_indexSize = m_indexSize;
	desc.m_flags = glBufferFlags_WriteOnly | (m_isDoubleBuffered ? 0 : glBufferFlags_Dynamic);

	m_buffers[0] = glIndexBuffer_Create(&desc);
	UE_ASSERT(m_buffers[0]);

	// Create secondary index buffer

	if (m_isDoubleBuffered)
	{
		m_buffers[1] = glIndexBuffer_Create(&desc);
		UE_ASSERT(m_buffers[1]);
	}
}

void glIndexBufferFactory::Deinit()
{
	UE_ASSERT(IsInitialized());
	glIndexBuffer_Destroy(m_buffers[0]);
	if (m_isDoubleBuffered)
		glIndexBuffer_Destroy(m_buffers[1]);
	m_buffers[0] = m_buffers[1] = NULL;
}

ueBool glIndexBufferFactory::IsInitialized() const
{
	return m_buffers[0] != NULL;
}

ueBool glIndexBufferFactory::Allocate(u32 numIndices, LockedChunk* chunk)
{
	UE_ASSERT(IsInitialized());
	UE_ASSERT(!m_isLocked);

	// Request too big?

	if (numIndices > m_numIndices)
		return UE_FALSE;

	u32 lockFlags = 0;
	if (m_isDoubleBuffered)
	{
		lockFlags = glBufferLockFlags_Write;

		// Need to switch the buffers?

		if (m_currentOffset + numIndices > m_numIndices)
		{
			// TODO: Insert fence here
			m_currentOffset = 0;
			m_frontBufferId = 1 - m_frontBufferId;
			// TODO: Wait for previous fence here (if there is any)
		}
	}
	else
	{
		lockFlags = glBufferLockFlags_Write;

		// Need to flush the buffer?

		if (m_currentOffset + numIndices > m_numIndices)
		{
			lockFlags |= glBufferLockFlags_Discard;
			m_currentOffset = 0;
		}
		else
			// Reuse buffer space

			lockFlags |= glBufferLockFlags_NoOverwrite;
	}

	// Update offsets

	const u32 offset = m_currentOffset;
	m_currentOffset += numIndices * m_indexSize;

	// Lock the data

	void* data = glIndexBuffer_Lock(m_buffers[m_frontBufferId], lockFlags, offset, numIndices * m_indexSize);
	if (!data)
		return UE_FALSE;

	// Set up result

	chunk->m_firstIndex = offset / m_indexSize;
	chunk->m_data = data;
	chunk->m_IB = m_buffers[m_frontBufferId];

	m_isLocked = UE_TRUE;
	return UE_TRUE;
}

void glIndexBufferFactory::Unlock(LockedChunk* chunk)
{
	UE_ASSERT(IsInitialized());
	UE_ASSERT(m_isLocked);
	UE_ASSERT(chunk->m_IB == m_buffers[m_frontBufferId]);

	glIndexBuffer_Unlock(m_buffers[m_frontBufferId]);
	m_isLocked = UE_FALSE;
}
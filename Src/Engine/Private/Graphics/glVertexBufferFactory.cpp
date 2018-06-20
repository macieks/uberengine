#include "Graphics/glVertexBufferFactory.h"

glVertexBufferFactory::glVertexBufferFactory() :
	m_size(0),
	m_frontBufferId(0),
	m_currentOffset(0),
	m_isLocked(UE_FALSE)
{
	m_buffers[0] = m_buffers[1] = NULL;
}

glVertexBufferFactory::~glVertexBufferFactory()
{
	UE_ASSERT(!IsInitialized());
}

void glVertexBufferFactory::Init(u32 size)
{
	UE_ASSERT(!IsInitialized());

	m_size = size;

#if !defined(GL_X360) && !defined(GL_PS3)
	m_isDoubleBuffered = UE_FALSE; // Can use discard lock mode
#else
	m_isDoubleBuffered = UE_TRUE;
#endif

	// Create vertex buffer

	glVertexBufferDesc desc;
	desc.m_size = m_size;
	desc.m_flags = glBufferFlags_WriteOnly | (m_isDoubleBuffered ? 0 : glBufferFlags_Dynamic);

	m_buffers[0] = glVertexBuffer_Create(&desc);
	UE_ASSERT(m_buffers[0]);

	// Create secondary vertex buffer

	if (m_isDoubleBuffered)
	{
		m_buffers[1] = glVertexBuffer_Create(&desc);
		UE_ASSERT(m_buffers[1]);
	}
}

void glVertexBufferFactory::Deinit()
{
	UE_ASSERT(IsInitialized());
	glVertexBuffer_Destroy(m_buffers[0]);
	if (m_isDoubleBuffered)
		glVertexBuffer_Destroy(m_buffers[1]);
	m_buffers[0] = m_buffers[1] = NULL;
}

ueBool glVertexBufferFactory::IsInitialized() const
{
	return m_buffers[0] != NULL;
}

ueBool glVertexBufferFactory::Allocate(u32 size, LockedChunk* chunk)
{
	UE_ASSERT(IsInitialized());
	UE_ASSERT(!m_isLocked);

	// Request too big?

	if (size > m_size)
		return UE_FALSE;

	u32 lockFlags = 0;
	if (m_isDoubleBuffered)
	{
		lockFlags = glBufferLockFlags_Write;

		// Need to switch the buffers?

		if (m_currentOffset + size > m_size)
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

		if (m_currentOffset + size > m_size)
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
	m_currentOffset += size;

	// Lock the data

	void* data = glVertexBuffer_Lock(m_buffers[m_frontBufferId], lockFlags, offset, size);
	if (!data)
		return UE_FALSE;

	// Set up result

	chunk->m_size = size;
	chunk->m_offset = offset;
	chunk->m_data = data;
	chunk->m_VB = m_buffers[m_frontBufferId];

	m_isLocked = UE_TRUE;
	return UE_TRUE;
}

void glVertexBufferFactory::Unlock(LockedChunk* chunk)
{
	UE_ASSERT(IsInitialized());
	UE_ASSERT(m_isLocked);
	UE_ASSERT(chunk->m_VB == m_buffers[m_frontBufferId]);

	glVertexBuffer_Unlock(m_buffers[m_frontBufferId]);
	m_isLocked = UE_FALSE;
}
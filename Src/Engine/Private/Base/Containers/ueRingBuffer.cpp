#include "Base/Containers/ueRingBuffer.h"

ueRingBuffer::ueRingBuffer() :
	m_allocator(NULL),
	m_buffer(NULL),
	m_bufferSize(0),
	m_enableWrapping(UE_TRUE)
{}

ueRingBuffer::~ueRingBuffer()
{
	Deinit();
}

ueBool ueRingBuffer::Init(ueAllocator* allocator, u32 maxItems, ueSize bufferSize)
{
	const ueSize memorySize = CalcMemReq(maxItems, bufferSize);

	m_allocator = allocator;
	void* memory = m_allocator->Alloc(memorySize);
	if (!memory)
		return UE_FALSE;

	InitMem(memory, memorySize, maxItems, bufferSize);
	return UE_TRUE;
}

ueSize ueRingBuffer::CalcMemReq(u32 maxItems, ueSize bufferSize)
{
	return 2 * maxItems * sizeof(u32) + bufferSize;
}

void ueRingBuffer::InitMem(void* memory, ueSize memorySize, u32 maxItems, ueSize bufferSize)
{
	UE_ASSERT(!m_buffer);
	UE_ASSERT(memory);
	UE_ASSERT(memorySize == CalcMemReq(maxItems, bufferSize));

	m_buffer = memory;
	m_bufferSize = (u32) bufferSize;

	m_sizes = (u32*) ((u8*) m_buffer + m_bufferSize);
	m_offsets = m_sizes + maxItems;
	m_first = 0;
	m_count = 0;
	m_capacity = maxItems;

	m_bufferedDataSize = 0;

#if defined(UE_DEBUG)
	ueMemZero(m_sizes, sizeof(u32) * maxItems);
	ueMemZero(m_offsets, sizeof(u32) * maxItems);
#endif
}

void ueRingBuffer::Deinit()
{
	if (m_allocator)
	{
		if (m_buffer)
			m_allocator->Free(m_buffer);
		m_allocator = NULL;
	}

	m_buffer = NULL;
	m_bufferSize = 0;
}

void ueRingBuffer::Clear()
{
	m_first = 0;
	m_count = 0;
	m_bufferedDataSize = 0;
}

void ueRingBuffer::EnableWrapping(ueBool enable)
{
	UE_ASSERT(m_count == 0);
	m_enableWrapping = enable;
}

void* ueRingBuffer::PushBack(u32 dataSize, ueBool popIfNeeded)
{
	UE_ASSERT(!m_enableWrapping);
	UE_ASSERT(dataSize <= m_bufferSize);

	while (1)
	{
		if (m_count == 0)
		{
			m_first = 0;
			m_count = 1;
			m_offsets[0] = 0;
			m_sizes[0] = dataSize;
			m_bufferedDataSize = dataSize;
			return m_buffer;
		}
		else if (m_count < m_capacity)
		{
			const u32 lastIndex = (m_first + m_count - 1) % m_capacity;
			const u32 newIndex = (m_first + m_count) % m_capacity;

			const u32 startOffset = m_offsets[lastIndex] + m_sizes[lastIndex];
			const u32 endOffset = startOffset + dataSize;

			void* dst = NULL;
			if (endOffset <= m_bufferSize &&
				(m_offsets[m_first] < startOffset || endOffset <= m_offsets[m_first])) // Check overlap with first element
			{
				dst = (u8*) m_buffer + startOffset;
				m_offsets[newIndex] = startOffset;
			}
			else if (dataSize <= m_offsets[m_first])
			{
				dst = m_buffer;
				m_offsets[newIndex] = 0;
			}
			else if (!popIfNeeded)
				return NULL;

			m_sizes[newIndex] = dataSize;
			m_count++;
			m_bufferedDataSize += dataSize;
			return dst;
		}

		if (!popIfNeeded || !PopFront())
			break;
	}

	return NULL;
}

ueBool ueRingBuffer::PushBack(const void* data, u32 dataSize, ueBool popIfNeeded)
{
	UE_ASSERT(dataSize <= m_bufferSize);

	while (1)
	{
		if (m_count == 0)
		{
			m_first = 0;
			m_count = 1;
			m_offsets[0] = 0;
			m_sizes[0] = dataSize;
			ueMemCpy(m_buffer, data, dataSize);
			m_bufferedDataSize = dataSize;
			return UE_TRUE;
		}
		else if (m_count < m_capacity)
		{
			const u32 lastIndex = (m_first + m_count - 1) % m_capacity;
			const u32 newIndex = (m_first + m_count) % m_capacity;

			const u32 startOffset = m_offsets[lastIndex] + m_sizes[lastIndex];
			const u32 endOffset = startOffset + dataSize;

			if (endOffset <= m_bufferSize &&
				(m_offsets[m_first] < startOffset || endOffset <= m_offsets[m_first])) // Check overlap with first element
			{
				ueMemCpy((u8*) m_buffer + startOffset, data, dataSize);
				m_offsets[newIndex] = startOffset;
			}
			else if (m_enableWrapping)
			{
				const u32 part0Size = m_bufferSize - startOffset;
				ueMemCpy((u8*) m_buffer + startOffset, data, part0Size);

				const u32 part1Size = dataSize - part0Size;
				ueMemCpy(m_buffer, data, part1Size);

				m_offsets[newIndex] = startOffset;
			}
			else if (dataSize <= m_offsets[m_first])
			{
				ueMemCpy(m_buffer, data, dataSize);
				m_offsets[newIndex] = 0;
			}
			else if (!popIfNeeded)
				return UE_FALSE;

			m_sizes[newIndex] = dataSize;
			m_count++;
			m_bufferedDataSize += dataSize;

			UE_ASSERT(m_offsets[6] != 3452816845U);
			return UE_TRUE;
		}

		if (!popIfNeeded || !PopFront())
			break;
	}

	return UE_FALSE;
}

ueBool ueRingBuffer::PopFront(void* dst, u32& dstSize)
{
	if (m_count == 0)
		return UE_FALSE;

	const u32 size = m_sizes[m_first];
	UE_ASSERT(size <= dstSize);

	const u32 startOffset = m_offsets[m_first];
	const u32 endOffset = startOffset + size;

	if (startOffset < endOffset)
		ueMemCpy(dst, (u8*) m_buffer + startOffset, size);
	else
	{
		const u32 part0Size = m_bufferSize - startOffset;
		ueMemCpy(dst, (u8*) m_buffer + startOffset, part0Size);

		const u32 part1Size = size - part0Size;
		ueMemCpy((u8*) dst + part0Size, m_buffer, part1Size);
	}
	dstSize = size;

	m_bufferedDataSize -= size;

	m_first = (m_first + 1) % m_capacity;
	m_count--;
	return UE_TRUE;
}

void* ueRingBuffer::PopFront(u32& dstSize)
{
	UE_ASSERT(!m_enableWrapping);

	if (m_count == 0)
		return NULL;

	void* dst = NULL;
	const u32 size = m_sizes[m_first];
	UE_ASSERT(size <= dstSize);

	const u32 startOffset = m_offsets[m_first];
	const u32 endOffset = startOffset + size;
	UE_ASSERT(startOffset < endOffset);

	dst = (u8*) m_buffer + startOffset;
	dstSize = size;

	m_bufferedDataSize -= size;

	m_first = (m_first + 1) % m_capacity;
	m_count--;
	return dst;
}

ueBool ueRingBuffer::PopFront()
{
	if (m_count == 0)
		return UE_FALSE;

	m_bufferedDataSize -= m_sizes[m_first];
	m_first = (m_first + 1) % m_capacity;
	m_count--;
	return UE_TRUE;
}

ueBool ueRingBuffer::GetFrontSize(u32& dstSize) const
{
	if (m_count == 0)
		return UE_FALSE;
	dstSize = m_sizes[m_first];
	return UE_TRUE;
}

ueBool ueRingBuffer::GetFrontBuffer(void** dst) const
{
	UE_ASSERT(!m_enableWrapping);
	if (m_count == 0)
		return UE_FALSE;
	*dst = (u8*) m_buffer + m_offsets[m_first];
	return UE_TRUE;
}

void ueRingBuffer::GetFrontSize(u32 index, u32& dstSize) const
{
	UE_ASSERT(index < m_count);
	dstSize = m_sizes[(m_first + index) % m_capacity];
}

void ueRingBuffer::GetFrontData(u32 index, void** dst) const
{
	UE_ASSERT(!m_enableWrapping);
	UE_ASSERT(index < m_count);
	*dst = (u8*) m_buffer + m_offsets[(m_first + index) % m_capacity];

#if defined(UE_ENABLE_ASSERTION)
	const u32 offset = m_offsets[(m_first + index) % m_capacity];
	UE_ASSERT(offset <= 1000000); // FIXME !!!! Hits sometimes with UberSample->Movie
#endif
}

void ueRingBuffer::GetBackSize(u32 index, u32& dstSize) const
{
	UE_ASSERT(index < m_count);
	dstSize = m_sizes[(m_first + m_count - 1 - index) % m_capacity];
}

void ueRingBuffer::GetBackData(u32 index, void** dst) const
{
	UE_ASSERT(!m_enableWrapping);
	UE_ASSERT(index < m_count);
	*dst = (u8*) m_buffer + m_offsets[(m_first + m_count - 1 - index) % m_capacity];
}

u32 ueRingBuffer::GetMaxBytesThatCanBuffer() const
{
	if (m_count == 0)
		return m_bufferSize;
	if (m_enableWrapping)
		return m_bufferSize - m_bufferedDataSize;

	const u32 lastIndex = (m_first + m_count - 1) % m_capacity;
	const u32 startOffset = m_offsets[m_first];
	const u32 endOffset = m_offsets[lastIndex] + m_sizes[lastIndex];

	if (endOffset < startOffset)
		return startOffset - endOffset;

	return ueMax(m_bufferSize - endOffset, startOffset);
}
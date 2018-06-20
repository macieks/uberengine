#include "IO/ueBuffer.h"

ueBuffer::ueBuffer() :
	m_data(NULL),
	m_capacity(0),
	m_size(0),
	m_offset(0),
	m_mode(Mode_Read),
#if defined(IO_BUFFER_SUPPORTS_ENDIANESS_SWAP)
	m_endianess(UE_ENDIANESS),
#endif
	m_allocator(NULL)
{}

ueBuffer::ueBuffer(void* buffer, u32 size, Mode mode) :
	m_data(NULL),
	m_capacity(0),
	m_size(0),
	m_offset(0),
	m_mode(Mode_Read),
#if defined(IO_BUFFER_SUPPORTS_ENDIANESS_SWAP)
	m_endianess(UE_ENDIANESS),
#endif
	m_allocator(NULL)
{
	InitMem(buffer, size, mode);
}

ueBuffer::~ueBuffer()
{
	Clear(UE_TRUE);
}

void ueBuffer::Clear(ueBool freeMemory)
{
	if (m_allocator && freeMemory)
	{
		m_allocator->Free(m_data);
		m_data = NULL;
		m_capacity = 0;
	}

	m_size = 0;
	m_offset = 0;
}

void ueBuffer::InitMem(void* data, u32 size, Mode mode)
{
	Clear(UE_TRUE);

	m_data = (u8*) data;
	m_capacity = size;

	m_size = (mode == Mode_Write) ? 0 : size;
	m_offset = 0;

	m_mode = mode;

	m_allocator = NULL;

	UE_ASSERT(data || mode == Mode_Read);
}

void ueBuffer::Init(ueAllocator* allocator, Mode mode)
{
	Clear(UE_TRUE);

	m_allocator = allocator;
	m_mode = mode;
}

void ueBuffer::ResetForWriting()
{
	m_mode = Mode_Write;
	m_size = 0;
	m_offset = 0;
}

void ueBuffer::SetReading()
{
	m_mode = Mode_Read;
	m_offset = 0;
}

void ueBuffer::SeekTo(u32 offset)
{
	UE_ASSERT(offset <= m_size);
	m_offset = offset;
}

void ueBuffer::Seek(s32 offset)
{
	if (offset > 0)
	{
		if (m_mode == Mode_Write)
		{
			const u32 newSize = m_offset + offset;
			Reserve(newSize);

			m_offset = newSize;
			m_size = newSize;
		}
		else
		{
			UE_ASSERT(m_offset + offset <= m_size);
			m_offset += offset;
		}
	}
	else
	{
		UE_ASSERT(m_offset >= (u32) -offset);
		m_offset += offset;
	}
}

ueBool ueBuffer::ReadF32(f32* destValues, u32 destCount)
{
	for (u32 i = 0; i < destCount; i++)
		if (!ReadBytes(destValues + i, sizeof(f32)))
			return UE_FALSE;
	return UE_TRUE;
}

ueBool ueBuffer::ReadS32(s32* destValues, u32 destCount)
{
	for (u32 i = 0; i < destCount; i++)
		if (!ReadBytes(destValues + i, sizeof(s32)))
			return UE_FALSE;
	return UE_TRUE;
}

ueBool ueBuffer::ReadString(char* buffer, u32& bufferSize)
{
	u16 stringLength;

	// Just check string length

	if (!buffer)
	{
		const u32 offsetBefore = m_offset;
		if (!ReadAny<u16>(stringLength)) return UE_FALSE;
		m_offset = offsetBefore;
		bufferSize = (u32) stringLength;
		return UE_TRUE;
	}

	// Read string

	if (!ReadAny<u16>(stringLength)) return UE_FALSE;
	UE_ASSERT(stringLength < bufferSize);
	bufferSize = stringLength;
	if (!ReadBytes(buffer, stringLength)) return UE_FALSE;
	buffer[stringLength] = '\0';

	return UE_TRUE;
}

ueBool ueBuffer::ReadBytes(void* destData, u32 destSize)
{
	UE_ASSERT(m_mode == Mode_Read);

	if (m_offset + destSize > m_size)
		return UE_FALSE;

	ueMemCpy(destData, m_data + m_offset, destSize);
	m_offset += destSize;
	return UE_TRUE;
}

void ueBuffer::WriteF32(const f32* srcValues, u32 srcCount)
{
#if defined(IO_BUFFER_SUPPORTS_ENDIANESS_SWAP)
	for (u32 i = 0; i < srcCount; i++)
	{
		const f32 swapped = ueToEndianess(srcValues[i], m_endianess);
		WriteBytes(&swapped, sizeof(f32));
	}
#else
	WriteBytes(srcValues, sizeof(f32) * srcCount);
#endif
}

void ueBuffer::WriteS32(const s32* srcValues, u32 srcCount)
{
#if defined(IO_BUFFER_SUPPORTS_ENDIANESS_SWAP)
	for (u32 i = 0; i < srcCount; i++)
	{
		const s32 swapped = ueToEndianess(srcValues[i], m_endianess);
		WriteBytes(&swapped, sizeof(s32));
	}
#else
	WriteBytes(srcValues, sizeof(s32) * srcCount);
#endif
}

void ueBuffer::WriteString(const char* buffer)
{
	const u16 stringLength = (u16) ueStrLen(buffer);
	WriteAny<u16>(stringLength);
	WriteBytes(buffer, stringLength);
}

void ueBuffer::WriteBytes(const void* srcData, u32 srcSize, u32 offset)
{
	UE_ASSERT(m_mode == Mode_Write);

	const ueBool specificOffset = offset != 0xFFFFFFFF;
	if (!specificOffset)
		offset = m_offset;

	Reserve(m_offset + srcSize);
	UE_ASSERT(m_offset + srcSize <= m_capacity);

	ueMemCpy(m_data + offset, srcData, srcSize);

	if (!specificOffset)
	{
		m_offset += srcSize;
		m_size += srcSize;
	}
}

void ueBuffer::Reserve(u32 capacity)
{
	if (capacity <= m_capacity)
		return;

	UE_ASSERT(m_allocator);

	u32 newCapacity = m_capacity == 0 ? 1 : m_capacity;
	while (newCapacity < capacity)
		newCapacity *= 2;

	m_data = (u8*) m_allocator->Realloc(m_data, newCapacity);
	UE_ASSERT(m_data);

	m_capacity = newCapacity;
}

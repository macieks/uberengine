#include "IO/ueBitBuffer.h"

ueBitBuffer::ueBitBuffer() :
	m_data(NULL),
	m_capacity(0),
	m_size(0),
	m_offset(0),
	m_mode(Mode_Read),
#if defined(IO_BIT_BUFFER_SUPPORTS_ENDIANESS_SWAP)
	m_endianess(UE_ENDIANESS),
#endif
	m_allocator(NULL)
{}

ueBitBuffer::ueBitBuffer(void* buffer, u32 bufferSizeInBytes, Mode mode) :
	m_data(NULL),
	m_capacity(0),
	m_size(0),
	m_offset(0),
	m_mode(Mode_Read),
#if defined(IO_BIT_BUFFER_SUPPORTS_ENDIANESS_SWAP)
	m_endianess(UE_ENDIANESS),
#endif
	m_allocator(NULL)
{
	InitMem(buffer, bufferSizeInBytes, mode);
}

ueBitBuffer::~ueBitBuffer()
{
	Clear(UE_TRUE);
}

void ueBitBuffer::Clear(ueBool freeMemory)
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

void ueBitBuffer::InitMem(void* data, u32 dataSizeInBytes, Mode mode)
{
	Clear(UE_TRUE);

	m_data = (u8*) data;
	m_capacity = dataSizeInBytes << 3;

	m_size = (mode == Mode_Write) ? 0 : (dataSizeInBytes << 3);
	m_offset = 0;

	m_mode = mode;

	m_allocator = NULL;

	UE_ASSERT(data || mode == Mode_Read);
}

void ueBitBuffer::Init(ueAllocator* allocator, Mode mode)
{
	Clear(UE_TRUE);

	m_allocator = allocator;
	m_mode = mode;
}

void ueBitBuffer::ResetForWriting()
{
	m_mode = Mode_Write;
	m_size = 0;
	m_offset = 0;
}

void ueBitBuffer::SetReading()
{
	m_mode = Mode_Read;
	m_offset = 0;
}

void ueBitBuffer::SeekToBits(u32 offsetInBits)
{
	UE_ASSERT(offsetInBits <= m_size);
	m_offset = offsetInBits;
}

void ueBitBuffer::SeekByBytes(s32 offsetInBytes)
{
	SeekByBits(offsetInBytes << 3);
}

void ueBitBuffer::SeekByBits(s32 offsetInBits)
{
	if (offsetInBits > 0)
	{
		if (m_mode == Mode_Write)
		{
			const u32 newSize = m_offset + offsetInBits;
			ReserveBits(newSize);

			m_offset = newSize;
			m_size = newSize;
		}
		else
		{
			UE_ASSERT(m_offset + offsetInBits <= m_size);
			m_offset += offsetInBits;
		}
	}
	else
	{
		UE_ASSERT(m_offset >= (u32) -offsetInBits);
		m_offset += offsetInBits;
	}
}

ueBool ueBitBuffer::IsByteAligned() const
{
	return !(m_offset & 7);
}

ueBool ueBitBuffer::ReadF32(f32* destValues, u32 destCount)
{
	for (u32 i = 0; i < destCount; i++)
		if (!ReadBytes(destValues + i, sizeof(f32)))
			return UE_FALSE;
	return UE_TRUE;
}

ueBool ueBitBuffer::ReadF32Compressed(f32& destValue, f32 minValue, f32 maxValue, u32 bits)
{
	UE_ASSERT(minValue < maxValue);
	UE_ASSERT(1 <= bits && bits <= 32);

	// Read float bits
	u32 compressedFloat = 0;
	if (!ReadBits(&compressedFloat, bits))
		return UE_FALSE;
	compressedFloat = ueLittleEndian(compressedFloat);

	// Reconstruct float value
	const u32 maxCompressedTypes = (1 << bits) - 1;
	destValue = (f32) (((f64) compressedFloat / (f64) maxCompressedTypes) * (maxValue - minValue) + minValue);

	return UE_TRUE;
}

ueBool ueBitBuffer::ReadS32(s32* destValues, u32 destCount)
{
	for (u32 i = 0; i < destCount; i++)
		if (!ReadBytes(destValues + i, sizeof(s32)))
			return UE_FALSE;
	return UE_TRUE;
}

ueBool ueBitBuffer::ReadS32StepCompressed(s32& dstValue, u32 pow2Step)
{
	UE_ASSERT(pow2Step == 2 || pow2Step == 4 || pow2Step == 8 || pow2Step == 16);

	dstValue = 0;

	ueBool isNonNegative;
	if (!ReadBit(isNonNegative))
		return UE_FALSE;

	for (u32 i = 0; i < 32; i += pow2Step)
	{
		ueBool hasAnyBits;
		if (!ReadBit(hasAnyBits))
			return UE_FALSE;
		if (hasAnyBits)
		{
			u32 bits;
			if (!ReadBits(bits, pow2Step))
				return UE_FALSE;
			dstValue |= bits << i;
		}
	}

	if (!isNonNegative)
		dstValue = -dstValue;
	return UE_TRUE;
}

ueBool ueBitBuffer::ReadS32RangeCompressed(s32& destValue, s32 minValue, s32 maxValue)
{
	UE_ASSERT(minValue <= maxValue);

	// Determine number of bits necessary
	const u32 range = maxValue - minValue + 1;
	u32 bits = 0;
	for (u32 i = 0; i < 32; i++)
		if (range & (1 << i))
			bits = i;

	// Read in shifted unsigned value
	u32 shiftedValue = 0;
	if (!ReadBits(&shiftedValue, bits))
		return UE_FALSE;

	// Reconstruct signed value
	shiftedValue = ueLittleEndian(shiftedValue);
	destValue = shiftedValue - minValue;

	// Verify reconstructed value is in desired range
	return minValue <= destValue && destValue <= maxValue;
}

ueBool ueBitBuffer::ReadBit(ueBool& destValue)
{
	u8 value = 0;
	if (!ReadBits(&value, 1))
		return UE_FALSE;
	UE_ASSERT(value == 0 || value == 1);
	destValue = value ? UE_TRUE : UE_FALSE;
	return true;
}

ueBool ueBitBuffer::ReadString(char* buffer, u32& bufferSizeInBytes)
{
	u16 stringLength;

	// Just check string length
	if (!buffer)
	{
		const u32 offsetBefore = m_offset;
		if (!ReadAny<u16>(stringLength)) return UE_FALSE;
		m_offset = offsetBefore;
		bufferSizeInBytes = (u32) stringLength;
		return UE_TRUE;
	}

	// Read string
	if (!ReadAny<u16>(stringLength)) return UE_FALSE;
	UE_ASSERT(stringLength < bufferSizeInBytes);
	bufferSizeInBytes = stringLength;
	if (!ReadBytes(buffer, stringLength)) return UE_FALSE;
	buffer[stringLength] = '\0';
	return UE_TRUE;
}

ueBool ueBitBuffer::ReadBytes(void* destData, u32 destSizeInBytes)
{
	return ReadBits(destData, destSizeInBytes << 3);
}

ueBool ueBitBuffer::ReadBits(u32& destValue, u32 numBits)
{
	UE_ASSERT(numBits <= 32);
	return ReadBits(&destValue, numBits);
}

ueBool ueBitBuffer::ReadBits(void* destData, u32 destSizeInBits)
{
	UE_ASSERT(m_mode == Mode_Read);

	if (m_offset + destSizeInBits > m_size)
		return UE_FALSE;

	// Copy bit by bit if not aligned
	if ((m_offset & 7) || (destSizeInBits & 7))
	{
		for (u32 i = 0; i < destSizeInBits; i++)
		{
			const u32 srcIndex = m_offset + i;
			const u32 srcByteIndex = srcIndex >> 3;
			const u32 srcBitIndex = srcIndex & 7;
			const u8 srcByte = *((u8*) m_data + srcByteIndex) & (1 << srcBitIndex);

			const u32 destByteIndex = i >> 3;
			const u32 destBitIndex = i & 7;
			u8& destBit = *((u8*) destData + destByteIndex);
			if (srcByte)
				destBit |= (1 << destBitIndex);
			else
				destBit &= ~(1 << destBitIndex);
		}
	}

	// Use memcpy if aligned
	else
		ueMemCpy(destData, m_data + (m_offset >> 3), destSizeInBits >> 3);
	m_offset += destSizeInBits;
	return UE_TRUE;
}

void ueBitBuffer::WriteF32(const f32* srcValues, u32 srcCount)
{
#if defined(IO_BIT_BUFFER_SUPPORTS_ENDIANESS_SWAP)
	for (u32 i = 0; i < srcCount; i++)
	{
		const f32 swapped = ueToEndianess(srcValues[i], m_endianess);
		WriteBytes(&swapped, sizeof(f32));
	}
#else
	WriteBytes(srcValues, sizeof(f32) * srcCount);
#endif
}

void ueBitBuffer::WriteF32Compressed(f32 srcValue, f32 minValue, f32 maxValue, u32 bits)
{
	UE_ASSERT(minValue <= srcValue && srcValue <= maxValue);
	UE_ASSERT(minValue < maxValue);
	UE_ASSERT(bits <= 32);

	// Compress float into given number of bits
	const f64 normalizedSrcValue = (srcValue - minValue) / (maxValue - minValue);
	const u32 maxCompressedTypes = (1 << bits) - 1;
	const u32 compressedFloat = (u32) ueClamp((s64) (normalizedSrcValue * (f64) maxCompressedTypes), (s64) 0, (s64) maxCompressedTypes);
	const u32 compressedFloatLE = ueLittleEndian(compressedFloat);

	WriteBits(&compressedFloatLE, bits);
}

void ueBitBuffer::WriteS32(const s32* srcValues, u32 srcCount)
{
#if defined(IO_BIT_BUFFER_SUPPORTS_ENDIANESS_SWAP)
	for (u32 i = 0; i < srcCount; i++)
	{
		const s32 swapped = ueToEndianess(srcValues[i], m_endianess);
		WriteBytes(&swapped, sizeof(s32));
	}
#else
	WriteBytes(srcValues, sizeof(s32) * srcCount);
#endif
}

void ueBitBuffer::WriteS32StepCompressed(s32 srcValue, u32 pow2Step)
{
	UE_ASSERT(pow2Step == 2 || pow2Step == 4 || pow2Step == 8 || pow2Step == 16);

	WriteBit(srcValue >= 0);
	if (srcValue < 0)
		srcValue = -srcValue;

	const u32 mask = (1 << pow2Step) - 1;
	for (u32 i = 0; i < 32; i += pow2Step)
	{
		const u32 bits = (srcValue & (mask << i)) >> i;
		WriteBit(bits != 0);
		if (bits)
			WriteBits(bits, pow2Step);
	}
}

void ueBitBuffer::WriteS32RangeCompressed(s32 srcValue, s32 minValue, s32 maxValue)
{
	UE_ASSERT(minValue <= srcValue && srcValue <= maxValue);
	UE_ASSERT(minValue <= maxValue);

	// Determine number of bits needed
	const u32 range = maxValue - minValue + 1;
	u32 bits = 0;
	for (u32 i = 0; i < 32; i++)
		if (range & (1 << i))
			bits = i;

	// Shift & write the value
	const u32 shiftedValue = (u32) (srcValue + minValue);
	const u32 shiftedValueLE = ueLittleEndian(shiftedValue);
	WriteBits(&shiftedValueLE, bits);
}

void ueBitBuffer::WriteBit(ueBool srcValue)
{
	const u8 value = srcValue ? 1 : 0;
	WriteBits(&value, 1);
}

void ueBitBuffer::WriteString(const char* buffer)
{
	const u16 stringLength = (u16) strlen(buffer);
	WriteAny<u16>(stringLength);
	WriteBytes(buffer, stringLength);
}

void ueBitBuffer::WriteBytes(const void* srcData, u32 srcSizeInBytes, u32 offset)
{
	return WriteBits(srcData, srcSizeInBytes << 3, offset);
}

void ueBitBuffer::WriteBits(u32 srcValue, u32 numBits)
{
	UE_ASSERT(numBits <= 32);
	WriteBits(&srcValue, numBits);
}

void ueBitBuffer::WriteBits(const void* srcData, u32 srcSizeInBits, u32 offset)
{
	UE_ASSERT(m_mode == Mode_Write);

	const ueBool specificOffset = offset != 0xFFFFFFFF;
	if (!specificOffset)
		offset = m_offset;

	ReserveBits(m_offset + srcSizeInBits);
	UE_ASSERT(m_offset + srcSizeInBits <= m_capacity);

	// Copy bit by bit if not aligned
	if ((offset & 7) || (srcSizeInBits & 7))
	{
		for (u32 i = 0; i < srcSizeInBits; i++)
		{
			const u32 srcByteIndex = i >> 3;
			const u32 srcBitIndex = i & 7;
			const u8 srcByte = *((u8*) srcData + srcByteIndex) & (1 << srcBitIndex);

			const u32 destIndex = offset + i;
			const u32 destByteIndex = destIndex >> 3;
			const u32 destBitIndex = destIndex & 7;
			u8& destBit = *((u8*) m_data + destByteIndex);
			if (srcByte)
				destBit |= (1 << destBitIndex);
			else
				destBit &= ~(1 << destBitIndex);
		}
	}
	// Use memcpy if aligned
	else
		ueMemCpy(m_data + (offset >> 3), srcData, srcSizeInBits >> 3);

	if (!specificOffset)
	{
		m_offset += srcSizeInBits;
		m_size += srcSizeInBits;
	}
}

void ueBitBuffer::ReserveBits(u32 capacityInBits)
{
	if (capacityInBits <= m_capacity)
		return;

	UE_ASSERT(m_allocator);

	const u32 minCapacityInBytes = (capacityInBits + 7) >> 3;
	u32 capacityInBytes = m_capacity == 0 ? 1 : m_capacity;
	while (capacityInBytes < minCapacityInBytes)
		capacityInBytes *= 2;

	m_data = (u8*) m_allocator->Realloc(m_data, capacityInBytes);
	UE_ASSERT(m_data);

	m_capacity = capacityInBytes << 3;
}

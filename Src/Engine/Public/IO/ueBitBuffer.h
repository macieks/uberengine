#ifndef IO_BIT_BUFFER_H
#define IO_BIT_BUFFER_H

#include "Base/ueBase.h"

/**
 *	@addtogroup io
 *	@{
 */

//! Bit buffer
class ueBitBuffer
{
public:
	//! Buffer mode
	enum Mode
	{
		Mode_Read,		//!< Reading from buffer
		Mode_Write		//!< Writing to buffer
	};

	//! Creates empty buffer
	ueBitBuffer();
	//! Creates buffer with given data; on destruction data will not be freed
	ueBitBuffer(void* buffer, u32 bufferSizeInBytes, Mode mode);
	//! Destroys buffer; only frees buffer memory if it was initialized using allocator
	~ueBitBuffer();

#if defined(IO_BIT_BUFFER_SUPPORTS_ENDIANESS_SWAP)
	void SetTargetEndianess(u32 endianess = UE_ENDIANESS) { m_endianess = endianess; }
#endif

	//! Initializes buffer with given data; on destruction data will not be freed
	void InitMem(void* buffer, u32 bufferSizeInBytes, Mode mode);
	//! Initializes buffer using allocator; on destruction data will be freed
	void Init(ueAllocator* allocator, Mode mode);

	//! Clears buffer
	void Clear(ueBool freeMemory = UE_FALSE);

	//! Switches buffer into reading mode
	void SetReading();
	//! Resets buffer for writing
	void ResetForWriting();

	//! Seeks to given offset (in bits)
	void SeekToBits(u32 offsetInBits);
	//! Seeks by given offset (in bits)
	void SeekByBits(s32 offsetInBits);
	//! Seeks by given offset (in bytes)
	void SeekByBytes(s32 offsetInBytes);

	//! Tells whether buffer is byte aligned (it's possible to be misaligned if bitwise writes were done)
	ueBool IsByteAligned() const;

	// Reading

	//! Reads float array
	ueBool ReadF32(f32* destValues, u32 destCount = 1);
	//! Reads compressed float
	ueBool ReadF32Compressed(f32& destValue, f32 minValue, f32 maxValue, u32 numBits);
	//! Reads 32-bit integer array
	ueBool ReadS32(s32* destValues, u32 destCount = 1);
	//! Reads step-compressed 32-bit integer
	ueBool ReadS32StepCompressed(s32& destValue, u32 pow2Step = 4);
	//! Reads range-compressed 32-bit integer
	ueBool ReadS32RangeCompressed(s32& destValue, s32 minValue, s32 maxValue);
	//! Reads single bit into boolean parameter
	ueBool ReadBit(ueBool& destValue);
	//! Reads requested number of bits (up to 32) into integer parameter
	ueBool ReadBits(u32& destValue, u32 numBits);
	//! Reads string into given buffer
	ueBool ReadString(char* buffer, u32& bufferSizeInBytes);
	//! Reads bytes
	ueBool ReadBytes(void* destData, u32 destSizeInBytes);
	//! Reads bits
	ueBool ReadBits(void* destData, u32 destSizeInBits);

	//! Reads in object of any type with a single read operation
	template <class TYPE>
	UE_INLINE ueBool ReadAny(TYPE& destData) { return ReadBytes(&destData, sizeof(TYPE)); }
	//! Reads array of objects of any type with a single read operation
	template <class TYPE>
	UE_INLINE ueBool ReadArray(TYPE* elems, u32 numElems) { return ReadBytes(elems, sizeof(TYPE) * numElems); }

	// Writing

	//! Writes float
	void WriteF32(const f32* srcValues, u32 srcCount = 1);
	//! Writes compressed float
	void WriteF32Compressed(f32 srcValue, f32 minValue, f32 maxValue, u32 numBits);
	//! Writes 32-bit integer
	void WriteS32(const s32* srcValues, u32 srcCount = 1);
	//! Writes step-compressed 32-bit integer
	void WriteS32StepCompressed(s32 srcValue, u32 pow2Step = 4);
	//! Writes range-compressed 32-bit integer
	void WriteS32RangeCompressed(s32 srcValue, s32 minValue, s32 maxValue);
	//! Writes bit
	void WriteBit(ueBool srcValue);
	//! Writes bits
	void WriteBits(u32 srcValue, u32 numBits);
	//! Writes string
	void WriteString(const char* buffer);
	//! Writes bytes
	void WriteBytes(const void* srcData, u32 srcSizeInBytes, u32 offset = 0xFFFFFFFF);
	//! Writes bits
	void WriteBits(const void* srcData, u32 srcSizeInBits, u32 offset = 0xFFFFFFFF);

	//! Writes number (optionally swapping endianess - if needed and only when IO_BIT_BUFFER_SUPPORTS_ENDIANESS_SWAP is defined)
	template <class NUMBER_TYPE>
	UE_INLINE void WriteNumber(NUMBER_TYPE srcNumber, u32 offset = 0xFFFFFFFF)
#if defined(IO_BIT_BUFFER_SUPPORTS_ENDIANESS_SWAP)
	{ WriteBytes(TempPtr(ueToEndianess(srcNumber, m_endianess)), sizeof(NUMBER_TYPE), offset); }
#else
	{ WriteBytes(&srcNumber, sizeof(NUMBER_TYPE), offset); }
#endif
	//! Writes object of any type
	template <class TYPE>
	UE_INLINE void WriteAny(TYPE srcData, u32 offset = 0xFFFFFFFF) { WriteBytes(&srcData, sizeof(TYPE), offset); }
	//! Writes array of objects of any type
	template <class TYPE>
	UE_INLINE void WriteArray(TYPE* elems, u32 numElems) { WriteBytes(elems, sizeof(TYPE) * numElems); }

	// Accessors

	//! Gets buffer data
	UE_INLINE u8* GetData() const { return m_data; }
	//! Gets current buffer size in bits
	UE_INLINE u32 GetSizeInBits() const { return m_size; }
	//! Gets current buffer size in bytes
	UE_INLINE u32 GetSizeInBytes() const { return (m_size + 7) >> 3; }
	//! Gets buffer capacity in bytes
	UE_INLINE u32 GetCapacityInBytes() const { return m_capacity >> 3; }
	//! Gets current buffer offset in bits
	UE_INLINE u32 GetOffsetInBits() const { return m_offset; }
	//! Gets current buffer offset in bytes
	UE_INLINE u32 GetOffsetInBytes() const { UE_ASSERT_MSG(!(m_offset & 7), "Current offset is not byte aligned; use GetOffsetInBits() when unsure of alignment."); return (m_offset + 7) >> 3; }

	//! Gets pointer into currently read or written (depending on buffer mode) data
	UE_INLINE void* GetCurrData() const { UE_ASSERT_MSG(!(m_offset & 7), "Current offset is not byte aligned; use GetOffsetInBits() when unsure of alignment."); return (u8*) m_data + ((m_offset + 7) >> 3); }

private:
	void ReserveBits(u32 capacityInBits);

	u8* m_data;
	u32 m_capacity;
	u32 m_size;
	u32 m_offset;
	Mode m_mode;
#if defined(IO_BIT_BUFFER_SUPPORTS_ENDIANESS_SWAP)
	u32 m_endianess;
#endif
	ueAllocator* m_allocator; //!< Optional allocator
};

//! Declares bit buffer (ueBitBuffer) of given size on the stack
#define IO_BIT_BUFFER_DECL(name, size, mode) \
	u8 name_##data[size]; \
	ueBitBuffer name(name_##data, size, mode);

// @}

#endif // IO_BIT_BUFFER_H

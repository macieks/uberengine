#ifndef IO_BUFFER_H
#define IO_BUFFER_H

#include "Base/ueBase.h"

/**
 *	@addtogroup io
 *	@{
 */

//! Byte buffer
class ueBuffer
{
public:
	//! Buffer mode
	enum Mode
	{
		Mode_Read,		//!< Reading from buffer
		Mode_Write		//!< Writing to buffer
	};

	//! Creates empty buffer
	ueBuffer();
	//! Creates buffer with given data; on destruction data will not be freed
	ueBuffer(void* buffer, u32 size, Mode mode);
	//! Destroys buffer; only frees buffer memory if it was initialized using allocator
	~ueBuffer();

#if defined(IO_BUFFER_SUPPORTS_ENDIANESS_SWAP)
	void SetTargetEndianess(u32 endianess = UE_ENDIANESS) { m_endianess = endianess; }
#endif // defined(IO_BIT_BUFFER_SUPPORTS_ENDIANESS_SWAP)

	//! Initializes buffer with given data; on destruction data will not be freed
	void InitMem(void* buffer, u32 size, Mode mode);
	//! Initializes buffer using allocator; on destruction data will be freed
	void Init(ueAllocator* allocator, Mode mode);

	//! Clears buffer
	void Clear(ueBool freeMemory = UE_FALSE);

	//! Switches buffer into reading mode
	void SetReading();
	//! Resets buffer for writing
	void ResetForWriting();

	//! Seeks to given offset
	void SeekTo(u32 offset);
	//! Seeks by given offset
	void Seek(s32 offset);

	// Reading

	//! Reads float array
	ueBool ReadF32(f32* destValues, u32 destCount = 1);
	//! Reads 32-bit integer array
	ueBool ReadS32(s32* destValues, u32 destCount = 1);
	//! Reads string into given buffer
	ueBool ReadString(char* buffer, u32& bufferSize);
	//! Reads bytes
	ueBool ReadBytes(void* destData, u32 destSize);

	//! Reads in object of any type with a single read operation
	template <class TYPE>
	UE_INLINE ueBool ReadAny(TYPE& destData) { return ReadBytes(&destData, sizeof(TYPE)); }
	//! Reads array of objects of any type with a single read operation
	template <class TYPE>
	UE_INLINE ueBool ReadArray(TYPE* elems, u32 numElems) { return ReadBytes(elems, sizeof(TYPE) * numElems); }

	// Writing

	//! Writes float
	void WriteF32(const f32* srcValues, u32 srcCount = 1);
	//! Writes 32-bit integer
	void WriteS32(const s32* srcValues, u32 srcCount = 1);
	//! Writes string
	void WriteString(const char* buffer);
	//! Writes bytes
	void WriteBytes(const void* srcData, u32 srcSize, u32 offset = 0xFFFFFFFF);

	//! Writes number (optionally swapping endianess - if needed and only when IO_BIT_BUFFER_SUPPORTS_ENDIANESS_SWAP is defined)
	template <class NUMBER_TYPE>
	UE_INLINE void WriteNumber(NUMBER_TYPE srcNumber, u32 offset = 0xFFFFFFFF)
#if defined(IO_BUFFER_SUPPORTS_ENDIANESS_SWAP)
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
	//! Gets current buffer size in bytes
	UE_INLINE u32 GetSize() const { return m_size; }
	//! Gets buffer capacity in bytes
	UE_INLINE u32 GetCapacity() const { return m_capacity; }
	//! Gets current buffer offset in bytes
	UE_INLINE u32 GetOffset() const { return m_offset; }

	//! Gets pointer into currently read or written (depending on buffer mode) data
	UE_INLINE void* GetCurrData() const { return (u8*) m_data + m_offset; }

private:
	void Reserve(u32 capacity);

	u8* m_data;		//!< Data buffer
	u32 m_capacity;	//!< Data buffer capacity
	u32 m_size;		//!< Data size
	u32 m_offset;	//!< Current read/write offset
	Mode m_mode;	//!< Current mode
#if defined(IO_BUFFER_SUPPORTS_ENDIANESS_SWAP)
	u32 m_endianess;	//!< Optional: endianess
#endif
	ueAllocator* m_allocator; //!< Optional allocator
};

//! Declares buffer (ueBuffer) of given size on the stack
#define IO_BUFFER_DECL(name, size, mode) \
	u8 name_##data[size]; \
	ueBuffer name(name_##data, size, mode);

// @}

#endif // IO_BUFFER_H

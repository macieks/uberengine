#ifndef UE_RING_BUFFER_H
#define UE_RING_BUFFER_H

#include "Base/ueBase.h"

//! Binary data FIFO queue (aka ring buffer)
class ueRingBuffer
{
public:
	ueRingBuffer();
	~ueRingBuffer();

	//! Initializes ring buffer using given allocator
	ueBool Init(ueAllocator* allocator, u32 maxItems, ueSize bufferSize);
	//! Calculates memory required to initialize ring buffer
	static ueSize CalcMemReq(u32 maxItems, ueSize bufferSize);
	//! Initializes ring buffer using given memory (which won't be freed upon deinitialization)
	void InitMem(void* memory, ueSize memorySize, u32 maxItems, ueSize bufferSize);
	//! Deinitializes ring buffer
	void Deinit();

	//! Removes all elements
	void Clear();

	//! Gets number of queued elements
	u32 Count() const { return m_count; }
	//! Gets total buffer size
	u32 GetBufferedDataSize() const { return m_bufferedDataSize; }
	//! Gets max. buffer size that can be buffered
	u32 GetMaxBytesThatCanBuffer() const;

	//! Enables wrapping of individual elements
	void EnableWrapping(ueBool enable);

	//! Pushes new element; returns true on success, UE_FALSE otherwise
	ueBool PushBack(const void* data, u32 size, ueBool popIfNeeded = UE_FALSE);
	//! Pushes new element; returns pointer to pushed data (to be used by user) on success, NULL otherwise
	void* PushBack(u32 size, ueBool popIfNeeded = UE_FALSE);

	//! Pops an element; returns true on success, UE_FALSE otherwise
	ueBool PopFront(void* data, u32& size);
	//! Pops an element; returns pointer to data on success, NULL otherwise
	void* PopFront(u32& size);
	//! Pops an element without getting its data; returns true on success, UE_FALSE otherwise
	ueBool PopFront();

	//! Gets top element (the one to be popped next) size; does not pop element
	ueBool GetFrontSize(u32& dstSize) const;
	//! Gets pointer to front (the one to be popped next) element; does not pop element; asserts in wrapping mode
	ueBool GetFrontBuffer(void** dst) const;

	//! Gets size of the buffer at given index (counting from front); does not pop element
	void GetFrontSize(u32 index, u32& dstSize) const;
	//! Gets pointer to the data of the buffer at given index (counting from front); does not pop element; asserts in wrapping mode
	void GetFrontData(u32 index, void** dst) const;

	//! Gets size of the buffer at given index (counting from back)
	void GetBackSize(u32 index, u32& dstSize) const;
	//! Gets pointer to the data of the buffer at given index (counting from back)
	void GetBackData(u32 index, void** dst) const;

private:
	ueAllocator* m_allocator;
	void* m_buffer;
	u32 m_bufferSize;

	u32* m_sizes;
	u32* m_offsets;
	u32 m_first;
	u32 m_count;
	u32 m_capacity;

	u32 m_bufferedDataSize;

	ueBool m_enableWrapping;
};

#endif // UE_RING_BUFFER_H

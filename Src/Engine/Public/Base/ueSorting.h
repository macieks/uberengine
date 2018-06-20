#ifndef UE_SORTING_H
#define UE_SORTING_H

/**
 *	@addtogroup ue
 *	@{
 */

#include "Base/ueCmp.h"

//! Default sorting function
#define ueSort ueHeapSort

//! Heap-sort based sorting function
template <class TYPE, class CMP_PRED>
void ueHeapSort(TYPE* elements, s32 size, CMP_PRED& comparator = ueCmpPredicate<TYPE>() )
{
	if (size <= 1)
		return;

	TYPE temp;

	// Build the heap

	const s32 lequalPowerOf2 = ueLEPow2(size);
	for (s32 i = lequalPowerOf2 - 2; i >= 0; --i)
	{
		// Pull down the element

		s32 index = i;
		while (index < size)
		{
			const s32 rightIndex = (index + 1) << 1;
			const s32 leftIndex = rightIndex - 1;

			s32 minIndex = index;
			if (leftIndex < size)
			{
				if (comparator(elements[minIndex], elements[leftIndex]) < 0)
					minIndex = leftIndex;
				if (rightIndex < size && comparator(elements[minIndex], elements[rightIndex]) < 0)
					minIndex = rightIndex;
			}

			if (index == minIndex)
				break;

			temp = elements[index];
			elements[index] = elements[minIndex];
			elements[minIndex] = temp;

			index = minIndex;
		}
	}

	// Extract min/max element and put it at the end of the array

	for (s32 i = 0; i < size; ++i)
	{
		temp = elements[0];

		s32 sizeLeft = size - i - 1;
		elements[0] = elements[sizeLeft];
		elements[sizeLeft] = temp;

		// Pull down the element

		s32 index = 0;
		while (index < sizeLeft)
		{
			const s32 rightIndex = (index + 1) << 1;
			const s32 leftIndex = rightIndex - 1;

			s32 minIndex = index;
			if (leftIndex < sizeLeft)
			{
				if (comparator(elements[minIndex], elements[leftIndex]) < 0)
					minIndex = leftIndex;
				if (rightIndex < sizeLeft && comparator(elements[minIndex], elements[rightIndex]) < 0)
					minIndex = rightIndex;
			}

			if (index == minIndex)
				break;

			temp = elements[index];
			elements[index] = elements[minIndex];
			elements[minIndex] = temp;

			index = minIndex;
		}
	}
}

// @}

#endif // UE_SORTING_H
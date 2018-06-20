#ifndef UE_TYPE_INFO_H
#define UE_TYPE_INFO_H

/**
 *	@addtogroup ue
 *	@{
 */

template<typename TYPE> struct TTypeInfoBasic
{
	UE_INLINE static void Construct(TYPE* element) {}
	UE_INLINE static void CopyConstruct(TYPE* element, const TYPE* src) { *element = *src; }
	UE_INLINE static void Destruct(TYPE* element) {}
	UE_INLINE static void ConstructArray(TYPE* elements, u32 count) {}
	UE_INLINE static void DestructArray(TYPE* elements, u32 count) {}
	UE_INLINE static void Move(TYPE* dst, const TYPE* src, u32 count) { ueMemMove(dst, src, count * sizeof(TYPE)); }
};

template<typename TYPE> struct TTypeInfoComplex
{
	UE_INLINE static void Construct(TYPE* element) { new(element) TYPE(); }
	UE_INLINE static void CopyConstruct(TYPE* element, const TYPE* src) { new(element) TYPE(src); }
	UE_INLINE static void Destruct(TYPE* element) { element->~TYPE(); }
	UE_INLINE static TYPE* ConstructArray(void* _elements, u32 count)
	{
		TYPE* elements = (TYPE*) _elements;
		for (u32 i = 0; i < count; i++)
			new(elements + i) TYPE();
		return elements;
	}

	UE_INLINE static void DestructArray(TYPE* elements, u32 count)
	{
		for (u32 i = 0; i < count; i++)
			(elements + i)->~TYPE();
	}
	UE_INLINE static void Move(TYPE* dst, const TYPE* src, u32 count)
	{
		UE_ASSERT(count > 0);

		if (dst < src)
			for (u32 i = 0; i < count; i++)
				dst[i] = src[i];
		else
			for (s32 i = (s32) count - 1; i >= 0; i--)
				dst[i] = src[i];
	}
};

// Complex types

template<typename TYPE> struct TTypeInfo : TTypeInfoComplex<TYPE> {};

// Simple types

template<typename TYPE> struct TTypeInfo<TYPE*> : TTypeInfoBasic<TYPE*> {};

template <> struct TTypeInfo<ueBool>	: TTypeInfoBasic<ueBool> {};
template <> struct TTypeInfo<s8>		: TTypeInfoBasic<s8> {};
template <> struct TTypeInfo<s16>		: TTypeInfoBasic<s16> {};
template <> struct TTypeInfo<s32>		: TTypeInfoBasic<s32> {};
template <> struct TTypeInfo<s64>		: TTypeInfoBasic<s64> {};
template <> struct TTypeInfo<u8>		: TTypeInfoBasic<u8> {};
template <> struct TTypeInfo<u16>		: TTypeInfoBasic<u16> {};
template <> struct TTypeInfo<u32>		: TTypeInfoBasic<u32> {};
template <> struct TTypeInfo<u64>		: TTypeInfoBasic<u64> {};
template <> struct TTypeInfo<f32>		: TTypeInfoBasic<f32> {};
template <> struct TTypeInfo<f64>		: TTypeInfoBasic<f64> {};

// @}

#endif // UE_TYPE_INFO_H

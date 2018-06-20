#ifndef UE_NEW_DELETE_H
#define UE_NEW_DELETE_H

#include "Base/ueTypeInfo.h"

// Include in-place operators

#include <new>

#if defined(UE_NO_GLOBAL_NEW)

// Forbidden new / delete operators

UE_INLINE void* ueForbidGlobalNewDelete() { UE_ASSERT(!"Global new and delete operators are forbidden. Use new and delete that take ueAllocator* or do not include this file."); return NULL; }

UE_INLINE void* operator new(size_t size) { return ueForbidGlobalNewDelete(); }
UE_INLINE void* operator new[](size_t size) { return ueForbidGlobalNewDelete(); }

UE_INLINE void operator delete[](void* memory) { ueForbidGlobalNewDelete(); }
UE_INLINE void operator delete(void* memory) { ueForbidGlobalNewDelete(); }

#endif

// ueAllocator based new / delete operators

UE_INLINE void* operator new(size_t size, ueAllocator* allocator) { return allocator->Alloc(size); }
UE_INLINE void* operator new[](size_t size, ueAllocator* allocator) { return allocator->Alloc(size); }
UE_INLINE void operator delete(void* memory, ueAllocator* allocator) { UE_ASSERT(!"Global delete operator unsupported. Use ueDelete instead."); }
UE_INLINE void operator delete[](void* memory, ueAllocator* allocator) { UE_ASSERT(!"Global delete[] operator unsupported. Use ueDeleteArray instead."); }

//! Creates an object using given allocator
template <class TYPE>
TYPE* ueNew(ueAllocator* allocator, ueSize alignment = UE_DEFAULT_ALIGNMENT)
{
	TYPE* ptr = (TYPE*) allocator->Alloc(sizeof(TYPE), alignment);
	if (!ptr) return NULL;
	TTypeInfo<TYPE>::Construct(ptr);
	return ptr;
}

//! Creates an array of objects using given allocator
template <class TYPE>
TYPE* ueNewArray(ueAllocator* allocator, u32 count, ueSize alignment = UE_DEFAULT_ALIGNMENT)
{
	TYPE* ptr = (TYPE*) allocator->Alloc(sizeof(TYPE) * count, alignment);
	if (!ptr) return NULL;
	TTypeInfo<TYPE>::ConstructArray(ptr, count);
	return ptr;
}

//! Deletes object using given allocator
template <class TYPE>
void ueDelete(TYPE* ptr, ueAllocator* allocator)
{
	TTypeInfo<TYPE>::Destruct(ptr);
	allocator->Free(ptr);
}

//! Deletes array of objects using given allocator
template <class TYPE>
void ueDeleteArray(TYPE* ptr, ueAllocator* allocator, u32 count)
{
	TTypeInfo<TYPE>::DestructArray(ptr, count);
	allocator->Free(ptr);
}

#endif // UE_NEW_DELETE_H

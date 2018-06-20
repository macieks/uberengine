#ifndef UE_BASIC_UTILS_H
#define UE_BASIC_UTILS_H

/**
 *	@addtogroup ue
 *	@{
 */

class ueAllocator;

// Misc

//! Max path length
#define UE_MAX_PATH						512
//! Path type
typedef char uePath[UE_MAX_PATH];

//! Gets 4-byte symbol value
#define UE_4BYTE_SYMBOL(v0, v1, v2, v3)			((u32)((((v0)&0xff)<<24)|(((v1)&0xff)<<16)|(((v2)&0xff)<<8)|((v3)&0xff)))

#if UE_ENDIANESS == UE_ENDIANESS_LITTLE
	#define UE_LE_4BYTE_SYMBOL(v0, v1, v2, v3)		UE_4BYTE_SYMBOL(v0, v1, v2, v3)
	#define UE_BE_4BYTE_SYMBOL(v0, v1, v2, v3)		UE_4BYTE_SYMBOL(v3, v2, v1, v0)
#else
	#define UE_LE_4BYTE_SYMBOL(v0, v1, v2, v3)		UE_4BYTE_SYMBOL(v3, v2, v1, v0)
	#define UE_BE_4BYTE_SYMBOL(v0, v1, v2, v3)		UE_4BYTE_SYMBOL(v0, v1, v2, v3)
#endif

//! Gets 4-byte symbol as string
UE_INLINE const char* UE_4BYTE_SYMBOL_STR(u32 value) { static char str[5] = {0}; *(u32*) &str = value; return str; }

//! Gets fixed size array
#define UE_ARRAY_SIZE(a)					(sizeof(a) / sizeof((a)[0]))
//! Gets offset of member within structure
#define UE_OFFSET_OF(s, m)					((u32) ((ueSize) &((s*) (0x1))->m - (ueSize) ((s*) (0x1))))
//! Invalid switch case handler
#define UE_INVALID_CASE(value)				default: UE_ASSERT_MSGP(0, "Invalid switch case (value = %d)", (s32) value);

//! Performs regular expression check
ueBool ueRegexMatches(const char* name, const char* pattern);
//! Performs wildcard expression check
ueBool ueWildcardMatches(const char* name, const char* pattern);

//! Checks if character is digit
UE_INLINE ueBool ueIsDigit(char c) { return '0' <= c && c <= '9'; }

//! Generates unique compilation time name (based on current source code line)
#define UE_GEN_NAME(prefix) _UE_GEN_NAME0(prefix, __LINE__)
#define _UE_GEN_NAME0(prefix, uniqueName) _UE_GEN_NAME1(prefix, uniqueName)
#define _UE_GEN_NAME1(prefix, uniqueName) prefix##uniqueName

//! Gets whether given character is digit
UE_INLINE ueBool ueIsDigit(u32 value) { return '0' <= value && value <= '9'; }

// Memory

//! Copies bytes (doesn't handle overlapping cases; @see ueMemMove)
void ueMemCpy(void* dst, const void* src, ueSize size);
//! Sets bytes to given value
void ueMemSet(void* dst, u32 value, ueSize size);
//! Zeroes bytes
void ueMemZero(void* dst, ueSize size);
//! Zeroes bytes
#define ueMemZeroS(dst) ueMemZero(&dst, sizeof(dst))
//! Compares bytes
s32 ueMemCmp(const void* a, const void* b, ueSize size);
//! Moves bytes (handles overlapping cases)
void ueMemMove(void* dst, const void* src, ueSize size);
//! Allocates new memory block and copies given block of memory into it
void* ueMemDup(const void* src, ueSize size, ueAllocator* allocator);

//! General purpose scoped object destructor
template <class TYPE>
class ueScopedDestructor
{
private:
	ueAllocator* m_allocator;
	TYPE* m_object;
public:
	UE_INLINE ueScopedDestructor() : m_allocator(NULL), m_object(NULL) {}
	UE_INLINE ueScopedDestructor(ueAllocator* allocator, TYPE* object) : m_allocator(allocator), m_object(object) {}
	UE_INLINE ~ueScopedDestructor() { if (m_object) m_allocator->Free(m_object); }
	UE_INLINE void Set(ueAllocator* allocator, TYPE* object) { m_allocator = allocator; m_object = object; }
	UE_INLINE void Destroy() { if (m_object) { m_allocator->Free(m_object); m_allocator = NULL; m_object = NULL; } }
};

struct ueEnumEntry
{
	const u32 m_value;
	const char* m_name;
};

u32 ueEnumGetValue(const ueEnumEntry* entries, const char* name);
const char* ueEnumGetName(const ueEnumEntry* entries, u32 value);

#define UE_ENUM_BEGIN(enumType) \
	const ueEnumEntry enumType##Entries[] = \
	{

#define UE_ENUM_VALUE(enumType, value) \
		{enumType##_##value, #value},

#define UE_ENUM_END(enumType) \
		{0, NULL} \
	}; \
	UE_INLINE enumType enumType##_##FromString(const char* name) { return (enumType) ueEnumGetValue(enumType##Entries, name); } \
	UE_INLINE const char* enumType##_##ToString(enumType value) { return ueEnumGetName(enumType##Entries, (u32) value); }

// Returns pointer to temp variable (this is to overcome GCC's "warning: taking address of temporary")
template <typename TYPE>
const TYPE* TempPtr(const TYPE& value) { return &value; };

// Returns pointer to temp variable (this is to overcome GCC's "warning: taking address of temporary")
template <typename TYPE>
TYPE* TempCPtr(const TYPE& value) { return const_cast<TYPE*>(&value); };

//! Simple handle
template <class TYPE> class ueSimpleHandle
{
public:
	//! Initializes handle
	UE_INLINE ueSimpleHandle() : m_object(NULL) {}
	//! Gets handled object
	UE_INLINE TYPE* operator * () const { return m_object; }
	//! Accesses handles object
	UE_INLINE TYPE* operator -> () const { UE_ASSERT(m_object); return m_object; }
	//! Is valid
	UE_INLINE ueBool IsValid() const { return m_object != NULL; }

	UE_INLINE void _SetHandle(TYPE* object) { m_object = object; }
protected:
	TYPE* m_object;
};

//! Gets engine version
void		ueGetEngineVersion(u32& majorVer, u32& minorVer);
//! Logs selected information on the machine to logs (e.g. machine specs, OS)
void		ueLogSystemInfo();

// Array

template <typename TYPE>
TYPE* ueArray_Find(TYPE* array, u32 count, const TYPE& value)
{
	for (u32 i = 0; i < count; i++)
		if (array[i] == value)
			return &array[i];
	return NULL;
}

template <typename TYPE>
ueBool ueArray_PushBack(TYPE* array, u32 capacity, u32& count, const TYPE& value)
{
	if (capacity == count)
		return UE_FALSE;

	array[count++] = value;
	return UE_TRUE;
}

template <typename TYPE>
ueBool ueArray_PushBackUnique(TYPE* array, u32 capacity, u32& count, const TYPE& value)
{
	return ueArray_Find(array, count, value) ?
		UE_FALSE :
		ueArray_PushBack(array, capacity, count, value);
}

// @}

#endif // UE_BASIC_UTILS_H

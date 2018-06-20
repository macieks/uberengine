#ifndef UE_TYPE_H
#define UE_TYPE_H

#include "Base/Containers/ueList.h"
#include "Base/Containers/ueVector.h"
#include "Base/ueResource.h"

class ueObject;
class ueType;
class ueVectorType;
class uePtrType;
struct ueClassAttribute;

//! General RTTI types
enum ueTypeEnum
{
	ueTypeEnum_Bool = 0,		//!< Boolean (ueBool)
	ueTypeEnum_S32,				//!< 32-bit integer (s32)
	ueTypeEnum_F32,				//!< 32-bit float (f32)
	ueTypeEnum_String,			//!< String (ueString)
	ueTypeEnum_Enum,			//!< Enumerator
	ueTypeEnum_Flag,			//!< Bit flag
	ueTypeEnum_Class,			//!< Class
	ueTypeEnum_Struct,			//!< Structure
	ueTypeEnum_ResourceHandle,	//!< Handle to resource
	ueTypeEnum_Ptr,				//!< Pointer to any type
	ueTypeEnum_Vector,			//!< ueVector container

	ueTypeEnum_MAX
};

//! RTTI failure reason
struct ueRTTIFailureReason
{
	char m_buffer[128];
	UE_INLINE ueRTTIFailureReason() { m_buffer[0] = 0; }
};

//! RTTI attribute node type
enum ueAttributeNodeType
{
	ueAttributeNodeType_Attribute = 0,
	ueAttributeNodeType_VectorElement,

	ueAttributeNodeType_MAX
};

//! RTTI attribute node
struct ueAttributeNode
{
	ueAttributeNodeType m_nodeType;
	ueType* m_type;
	union
	{
		ueClassAttribute* m_classAttribute;
		u32 m_vectorIndex;
	};

	void* m_ptr;
	ueAttributeNode* m_child;
};

//! RTTI attribute information
struct ueAttributeInfo
{
	ueObject* m_object;
	ueAttributeNode* m_root;
	ueAttributeNode* m_leaf;
};

//! Base RTTI type object
class ueType
{
public:
	ueType(ueTypeEnum e);
	virtual ~ueType();

	//! Gets type enumeration
	UE_INLINE ueTypeEnum GetEnum() const { return m_enum; }

	//! Gets type name
	virtual const char* GetName() const { return NULL; }
	//! Converts value to string
	virtual ueBool ToString(ueAttributeInfo* info, char* dst, u32 dstSize, const void* src) { dst[0] = 0; return UE_FALSE; }
	//! Converts value from string
	virtual ueBool FromString(ueAttributeInfo* info, void* dst, const char* src, ueRTTIFailureReason& reason) { return UE_FALSE; }

private:
	ueTypeEnum m_enum;

	ueVectorType* m_vectorType;
	uePtrType* m_ptrType;

public:
	ueVectorType* GetVectorType() const { return m_vectorType; }
	uePtrType* GetPtrType() const { return m_ptrType; }
	void SetVectorType(ueVectorType* t) { m_vectorType = t; }
	void SetPtrType(uePtrType* t) { m_ptrType = t; }

	friend class ueTypeManager;
};

template <typename TYPE> struct ueTypeGetter
{
	UE_INLINE static ueType* GetType();
};

// Primitive types

#define UE_DEFINE_PRIMITIVE_TYPE(type, name) \
	class ue##name##Type : public ueType \
	{ \
	public: \
		virtual const char* GetName() const { return #name; } \
		virtual ueBool ToString(ueAttributeInfo* info, char* dst, u32 dstSize, const void* src); \
		virtual ueBool FromString(ueAttributeInfo* info, void* dst, const char* src, ueRTTIFailureReason& reason); \
		static ueType* GetInstance() { return s_instance; } \
		static void Init(); \
		static void Deinit(); \
	private: \
		ue##name##Type() : ueType(ueTypeEnum_##name) {} \
		static ue##name##Type* s_instance; \
	}; \
	template <> class ueTypeGetter<type> \
	{ \
	public: \
		UE_INLINE static ueType* GetType() { return ue##name##Type::GetInstance(); } \
	};

#define UE_DECLARE_PRIMITIVE_TYPE(name) \
	void ue##name##Type::Init() \
	{ \
		s_instance = new(ueTypeManager_GetAllocator()) ue##name##Type(); \
	} \
	void ue##name##Type::Deinit() \
	{ \
		ueDelete(s_instance, ueTypeManager_GetAllocator()); \
	}

UE_DEFINE_PRIMITIVE_TYPE(ueBool, Bool)
UE_DEFINE_PRIMITIVE_TYPE(s32, S32)
UE_DEFINE_PRIMITIVE_TYPE(f32, F32)

// Pointer type

class uePtrType : public ueType
{
public:
	uePtrType(ueType* pointedType) : ueType(ueTypeEnum_Ptr), m_pointedType(pointedType) {}
	virtual const char* GetName() const { return "Ptr"; }
	virtual ueBool ToString(ueAttributeInfo* info, char* dst, u32 dstSize, const void* src);
	virtual ueBool FromString(ueAttributeInfo* info, void* dst, const char* src, ueRTTIFailureReason& reason);

private:
	ueType* m_pointedType;
};

// Vector type

class ueVectorType : public ueType
{
public:
	ueVectorType(ueType* elementType) : ueType(ueTypeEnum_Vector), m_elementType(elementType) {}
	virtual const char* GetName() const { return "Vector"; }

private:
	ueType* m_elementType;
};

// Resource handle type

class ueResourceHandleType : public ueType
{
public:
	ueResourceHandleType(u32 resourceTypeId) : ueType(ueTypeEnum_ResourceHandle), m_resourceTypeId(resourceTypeId) {}
	virtual const char* GetName() const { return "ResourceHandle"; }
	virtual ueBool ToString(ueAttributeInfo* info, char* dst, u32 dstSize, const void* src);
	virtual ueBool FromString(ueAttributeInfo* info, void* dst, const char* src, ueRTTIFailureReason& reason);

private:
	u32 m_resourceTypeId;

public:
	struct CmpPred
	{
		UE_INLINE s32 operator () (const ueResourceHandleType* a, const ueResourceHandleType* b) const
		{
			return a->m_resourceTypeId - b->m_resourceTypeId;
		}
	};

	friend class ueTypeManager;
};

// Class

struct ueClassAttribute;

typedef void* (*ueCreateInstanceFunc)(void* memory);
typedef ueBool (*ueAttrToStringFunc)(ueAttributeInfo* info, char* dst, u32 dstSize, const void* srcObject);
typedef ueBool (*ueAttrFromStringFunc)(ueAttributeInfo* info, void* dstObject, const char* src, ueRTTIFailureReason& reason);

enum ueAttrFlags
{
	ueAttrFlags_ReadOnly	= UE_POW2(0),
	ueAttrFlags_Virtual		= UE_POW2(1), // Set / get handled by callbacks

	ueAttrFlags_MAX
};

struct ueClassAttribute : ueList<ueClassAttribute>::Node
{
	ueType* m_type;
	u32 m_offset;
	const char* m_name;
	u32 m_flags;

	ueAttrToStringFunc m_toStringFunc;
	ueAttrFromStringFunc m_fromStringFunc;

	ueBool m_hasRange;
	union
	{
		struct
		{
			s32 m_min;
			s32 m_max;
			s32 m_step;
		} m_s32;
		struct
		{
			f32 m_min;
			f32 m_max;
			f32 m_step;
		} m_f32;
	};
};

//! Class representation
class ueClass : public ueType
{
public:
	//! Class constructor; do not use directly
	ueClass(const char* name, ueClass* super, ueCreateInstanceFunc createFunc, ueAllocator* allocator, u32 instanceSize, u32 version);
	~ueClass();

	//! Creates instance of an object within given memory; if memory is NULL, uses class allocator
	ueObject* CreateInstance(void* memory = NULL);

	//! Gets super class
	UE_INLINE ueClass* GetSuper() const { return m_super; }
	//! Checks if given class is subclass of this class
	ueBool IsSubClassOf(ueClass* cls) const;

	ueClassAttribute* AddAttribute(ueType* type, u32 offset, const char* name, u32 flags);
	ueClassAttribute* AddVirtualAttribute(ueType* type, const char* name, ueAttrToStringFunc toStringFunc, ueAttrFromStringFunc fromStringFunc, u32 flags);
	const ueClassAttribute* FindAttribute(const char* name) const;
	u32 GetNumAttributes(ueBool includeSuper = UE_TRUE) const;
	const ueList<ueClassAttribute>& GetAttributes() const { return m_attrs; }

	//! Gets class allocator
	UE_INLINE ueAllocator* GetAllocator() const { return m_allocator; }

	virtual const char* GetName() const { return m_name; }
	virtual ueBool ToString(ueAttributeInfo* info, char* dst, u32 dstSize, const void* src);
	virtual ueBool FromString(ueAttributeInfo* info, void* dst, const char* src, ueRTTIFailureReason& reason);

private:
	const char* m_name;
	ueClass* m_super;
	ueAllocator* m_allocator;
	ueList<ueClassAttribute> m_attrs;
	u32 m_instanceSize;
	u32 m_version;
	ueCreateInstanceFunc m_createInstanceFunc;

public:
	struct CmpPred
	{
		UE_INLINE s32 operator () (const ueClass* a, const ueClass* b) const
		{
			return ueStrCmp(a->m_name, b->m_name);
		}
	};

	friend class ueTypeManager;
};

// Class registration helper

typedef ueClass* (*ueClassInitFunc)(ueAllocator* allocator);
typedef void (*ueClassDeinitFunc)();

#define UE_CLASS_REGISTRY(className) {className::RTTI_Init, className::RTTI_Deinit}

struct ueClassRegistry
{
	ueClassInitFunc m_initFunc;
	ueClassDeinitFunc m_deinitFunc;
};

// Get type template specializations

template <typename TYPE> struct ueTypeGetter< ueVector<TYPE> >
{
	UE_INLINE static ueType* GetType() { return ueTypeManager_GetVectorType(ueTypeGetter<TYPE>::GetType()); }
};

template <typename TYPE> struct ueTypeGetter< ueResourceHandle<TYPE> >
{
	UE_INLINE static ueType* GetType() { return ueTypeManager_GetResourceHandleType(ueResourceType<TYPE>::ID); }
};

template <typename TYPE> struct ueTypeGetter<TYPE*>
{
	UE_INLINE static ueType* GetType() { return ueTypeManager_GetPtrType(ueTypeGetter<TYPE>::GetType()); }
};

template <typename TYPE>
UE_INLINE ueType* ueTypeGetter<TYPE>::GetType() { return TYPE::StaticGetClass(); }

#define UE_DEFINE_EXTERNAL_STRUCT(type, classVar) \
	extern ueClass* classVar; \
	template <> UE_INLINE ueType* ueTypeGetter<type>::GetType() { return classVar; }

#include "Base/ueMath.h"

UE_DEFINE_EXTERNAL_STRUCT(ueVec2, s_vec2Class);
UE_DEFINE_EXTERNAL_STRUCT(ueVec3, s_vec3Class);
UE_DEFINE_EXTERNAL_STRUCT(ueVec4, s_vec4Class);
UE_DEFINE_EXTERNAL_STRUCT(ueQuat, s_quatClass);

template <typename TYPE>
UE_INLINE ueType* rtGetType(const TYPE&)
{
	return ueTypeGetter<TYPE>::GetType();
}

// Type manager

void				ueTypeManager_Startup(ueAllocator* allocator, u32 maxClasses = 64);
void				ueTypeManager_Shutdown();
ueAllocator*		ueTypeManager_GetAllocator();

void				ueTypeManager_RegisterMathTypes();

ueClass*			ueTypeManager_CreateClass(const char* name, ueClass* super, ueCreateInstanceFunc createFunc, ueAllocator* allocator, u32 instanceSize, u32 version);
void				ueTypeManager_DestroyClass(ueClass* cls);

ueClass*			ueTypeManager_GetClass(const char* name);
uePtrType*			ueTypeManager_GetPtrType(ueType* pointedType);
ueVectorType*		ueTypeManager_GetVectorType(ueType* elementType);
ueResourceHandleType* ueTypeManager_GetResourceHandleType(u32 typeId);

#endif // UE_TYPE_H
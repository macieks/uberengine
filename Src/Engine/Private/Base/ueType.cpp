#include "Base/ueType.h"
#include "Base/ueObject.h"
#include "Base/Containers/ueHashSet.h"
#include "Base/Containers/ueGenericPool.h"

// Type

ueType::ueType(ueTypeEnum e) :
	m_enum(e),
	m_vectorType(NULL),
	m_ptrType(NULL)
{}

ueType::~ueType()
{
	if (m_vectorType)
		ueDelete(m_vectorType, ueTypeManager_GetAllocator());
	if (m_ptrType)
		ueDelete(m_ptrType, ueTypeManager_GetAllocator());
}

// Primitive types

ueBool ueBoolType::ToString(ueAttributeInfo* info, char* dst, u32 dstSize, const void* src)
{
	return ueStrFromBool(dst, dstSize, *(const ueBool*) src);
}

ueBool ueBoolType::FromString(ueAttributeInfo* info, void* dst, const char* src, ueRTTIFailureReason& reason)
{
	if (ueStrToBool(src, *(ueBool*)dst))
	{
		info->m_object->OnAttributeChanged(info, dst);
		return UE_TRUE;
	}
	return UE_FALSE;
}

ueBool ueS32Type::ToString(ueAttributeInfo* info, char* dst, u32 dstSize, const void* src)
{
	return ueStrFromS32(dst, dstSize, *(const s32*) src);
}

ueBool ueS32Type::FromString(ueAttributeInfo* info, void* dst, const char* src, ueRTTIFailureReason& reason)
{
	if (!ueStrToS32(src, *(s32*)dst))
	{
		info->m_object->OnAttributeChanged(info, dst);
		return UE_TRUE;
	}
	return UE_FALSE;
}

ueBool ueF32Type::ToString(ueAttributeInfo* info, char* dst, u32 dstSize, const void* src)
{
	return ueStrFromF32(dst, dstSize, *(const f32*) src);
}

ueBool ueF32Type::FromString(ueAttributeInfo* info, void* dst, const char* src, ueRTTIFailureReason& reason)
{
	if (!ueStrToF32(src, *(f32*)dst))
	{
		info->m_object->OnAttributeChanged(info, dst);
		return UE_TRUE;
	}
	return UE_FALSE;
}

// Pointer type

ueBool uePtrType::ToString(ueAttributeInfo* info, char* dst, u32 dstSize, const void* src)
{
	if (m_pointedType->GetEnum() == ueTypeEnum_Class)
	{
		const ueObject* object = *(const ueObject**) src;
		ueStrFormat(dst, dstSize, "%s", object ? object->GetClass()->GetName() : "<none>");
	}
	else
		ueStrCpy(dst, dstSize, m_pointedType->GetName());

	return UE_TRUE;
}

ueBool uePtrType::FromString(ueAttributeInfo* info, void* dst, const char* src, ueRTTIFailureReason& reason)
{
	if (m_pointedType->GetEnum() == ueTypeEnum_Class)
	{
		ueObject*& object = *(ueObject**&) dst;

		if (object)
		{
			if (!ueStrCmp(object->GetClass()->GetName(), src))
				return UE_TRUE;
			object->Destroy();
			object = NULL;
		}

		ueClass* cls = ueTypeManager_GetClass(src);
		if (!cls)
			return UE_FALSE;

		object = cls->CreateInstance();
		return object != NULL;
	}

	return UE_FALSE;
}

// Resource handle type

ueBool ueResourceHandleType::ToString(ueAttributeInfo* info, char* dst, u32 dstSize, const void* src)
{
	const ueResource* resource = *(const ueResource**) src;
	if (!resource)
		dst[0] = 0;
	else
		ueStrCpy(dst, dstSize, ueResource_GetName(resource));
	return UE_TRUE;
}

ueBool ueResourceHandleType::FromString(ueAttributeInfo* info, void* dst, const char* src, ueRTTIFailureReason& reason)
{
	ueResource*& resource = *(ueResource**) dst;

	if (resource)
	{
		if (!ueStrCmp(src, ueResource_GetName(resource)))
			return UE_TRUE;
		ueResourceMgr_Release(resource);
	}

	resource = ueResourceMgr_Get(m_resourceTypeId, src);
	if (!ueResource_IsReady(resource))
	{
		ueStrFormatS(reason.m_buffer, "Failed to find/load resource '%s'", src);
		return UE_FALSE;
	}

	return UE_TRUE;
}

// Class

ueClass::ueClass(const char* name, ueClass* super, ueCreateInstanceFunc createFunc, ueAllocator* allocator, u32 instanceSize, u32 version) :
	ueType(ueTypeEnum_Class),
	m_name(name),
	m_super(super),
	m_createInstanceFunc(createFunc),
	m_allocator(allocator),
	m_instanceSize(instanceSize),
	m_version(version)
{}

ueClass::~ueClass()
{
	while (ueClassAttribute* attr = m_attrs.PopFront())
		ueDelete(attr, ueTypeManager_GetAllocator());
}

ueObject* ueClass::CreateInstance(void* memory)
{
	if (!memory)
	{
		memory = m_allocator->Alloc(m_instanceSize);
		if (!memory)
			return UE_FALSE;
	}

	m_createInstanceFunc(memory);

	ueObject* obj = (ueObject*) memory;
	obj->m_class = this;

	return obj;
}

ueBool ueClass::IsSubClassOf(ueClass* cls) const
{
	const ueClass* thisCls = this;
	while (thisCls)
	{
		if (thisCls == cls)
			return UE_TRUE;
		thisCls = thisCls->m_super;
	}
	return UE_FALSE;
}

ueClassAttribute* ueClass::AddAttribute(ueType* type, u32 offset, const char* name, u32 flags)
{
	UE_ASSERT(type);
	UE_ASSERT_MSG(!FindAttribute(name), "Attribute '%s' already exists in class '%s'", name, m_name);

	ueClassAttribute* attr = new(ueTypeManager_GetAllocator()) ueClassAttribute;
	UE_ASSERT(attr);
	attr->m_type = type;
	attr->m_offset = offset;
	attr->m_name = name;
	attr->m_flags = flags;

	m_attrs.PushBack(attr);

	return attr;
}

ueClassAttribute* ueClass::AddVirtualAttribute(ueType* type, const char* name, ueAttrToStringFunc toStringFunc, ueAttrFromStringFunc fromStringFunc, u32 flags)
{
	UE_ASSERT(type);
	UE_ASSERT_MSG(!FindAttribute(name), "Attribute '%s' already exists in class '%s'", name, m_name);

	ueClassAttribute* attr = new(ueTypeManager_GetAllocator()) ueClassAttribute;
	UE_ASSERT(attr);
	attr->m_type = type;
	attr->m_offset = 0;
	attr->m_name = name;
	attr->m_flags = flags | ueAttrFlags_Virtual;
	attr->m_toStringFunc = toStringFunc;
	attr->m_fromStringFunc = fromStringFunc;

	m_attrs.PushBack(attr);

	return attr;
}

const ueClassAttribute* ueClass::FindAttribute(const char* name) const
{
	for (const ueClass* cls = this; cls; cls = cls->m_super)
		for (const ueClassAttribute* attr = cls->m_attrs.Front(); attr; attr = attr->Next())
			if (!ueStrCmp(attr->m_name, name))
				return attr;
	return NULL;
}

u32 ueClass::GetNumAttributes(ueBool includeSuper) const
{
	u32 numAttrs = m_attrs.Length();;
	if (includeSuper)
		for (const ueClass* cls = m_super; cls; cls = cls->m_super)
			numAttrs += cls->m_attrs.Length();
	return numAttrs;
}

UE_DECLARE_PRIMITIVE_TYPE(Bool);
UE_DECLARE_PRIMITIVE_TYPE(S32);
UE_DECLARE_PRIMITIVE_TYPE(F32);

// Type manager

struct ueTypeManagerData
{
	ueAllocator* m_allocator;

	ueHashSet<ueClass*, ueCalcHashPredicate<ueClass*>, ueClass::CmpPred> m_classMap;
	ueGenericPool m_classes;

	ueHashSet<ueResourceHandleType*, ueCalcHashPredicate<ueResourceHandleType*>, ueResourceHandleType::CmpPred> m_resourceHandleTypeMap;
	ueGenericPool m_resourceHandleTypes;
};

static ueTypeManagerData* s_data = NULL;

void ueTypeManager_Startup(ueAllocator* allocator, u32 maxClasses)
{
	UE_ASSERT(!s_data);
	s_data = new(allocator) ueTypeManagerData();
	UE_ASSERT(s_data);
	s_data->m_allocator = allocator;

	s_data->m_classes.Init(allocator, sizeof(ueClass), maxClasses);
	s_data->m_classMap.Init(allocator, maxClasses);

	s_data->m_resourceHandleTypes.Init(allocator, sizeof(ueResourceHandleType), ueResourceMgr_GetMaxResourceTypes());
	s_data->m_resourceHandleTypeMap.Init(allocator, ueResourceMgr_GetMaxResourceTypes());

	ueBoolType::Init();
	ueS32Type::Init();
	ueF32Type::Init();

	ueObject::RTTI_Init();
}

void ueTypeManager_Shutdown()
{
	UE_ASSERT(s_data);

	ueBoolType::Deinit();
	ueS32Type::Deinit();
	ueF32Type::Deinit();

	ueObject::RTTI_Deinit();

	s_data->m_classMap.Deinit();
	s_data->m_classes.Deinit();

	s_data->m_resourceHandleTypeMap.Deinit();
	s_data->m_resourceHandleTypes.Deinit();

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

ueAllocator* ueTypeManager_GetAllocator()
{
	return s_data->m_allocator;
}

ueClass* ueTypeManager_CreateClass(const char* name, ueClass* super, ueCreateInstanceFunc createFunc, ueAllocator* allocator, u32 instanceSize, u32 version)
{
	UE_ASSERT(!ueTypeManager_GetClass(name));
	UE_ASSERT(createFunc);

	ueClass* cls = new(s_data->m_classes) ueClass(name, super, createFunc, allocator, instanceSize, version);
	UE_ASSERT(cls);
	s_data->m_classMap.Insert(cls);

	return cls;
}

void ueTypeManager_DestroyClass(ueClass* cls)
{
	s_data->m_classMap.Remove(cls);
	ueDelete(cls, s_data->m_classes);
}

ueClass* ueTypeManager_GetClass(const char* name)
{
	ueClass temp(name, NULL, NULL, NULL, 0, 0);
	ueClass** result = s_data->m_classMap.Find(&temp);
	return result ? *result : NULL;
}

uePtrType* ueTypeManager_GetPtrType(ueType* pointedType)
{
	if (!pointedType->GetPtrType())
		pointedType->SetPtrType( new(s_data->m_allocator) uePtrType(pointedType) );
	return pointedType->GetPtrType();
}

ueVectorType* ueTypeManager_GetVectorType(ueType* pointedType)
{
	if (!pointedType->GetVectorType())
		pointedType->SetVectorType( new(s_data->m_allocator) ueVectorType(pointedType) );
	return pointedType->GetVectorType();
}

ueResourceHandleType* ueTypeManager_GetResourceHandleType(u32 typeId)
{
	ueResourceHandleType temp(typeId);
	ueResourceHandleType** result = s_data->m_resourceHandleTypeMap.Find(&temp);
	return result ? *result : NULL;
}

// Math

ueClass* s_vec2Class = NULL;
ueClass* s_vec3Class = NULL;
ueClass* s_vec4Class = NULL;
ueClass* s_quatClass = NULL;

void ueTypeManager_RegisterMathTypes()
{
	// ueVec2

	s_vec2Class = ueTypeManager_CreateClass("ueVec2", NULL, NULL, ueTypeManager_GetAllocator(), sizeof(ueVec2), 0);
	s_vec2Class->AddAttribute(ueF32Type::GetInstance(), sizeof(f32) * 0, "X", 0);
	s_vec2Class->AddAttribute(ueF32Type::GetInstance(), sizeof(f32) * 1, "Y", 0);

	// ueVec3

	s_vec3Class = ueTypeManager_CreateClass("ueVec3", NULL, NULL, ueTypeManager_GetAllocator(), sizeof(ueVec3), 0);
	s_vec3Class->AddAttribute(ueF32Type::GetInstance(), sizeof(f32) * 0, "X", 0);
	s_vec3Class->AddAttribute(ueF32Type::GetInstance(), sizeof(f32) * 1, "Y", 0);
	s_vec3Class->AddAttribute(ueF32Type::GetInstance(), sizeof(f32) * 2, "Z", 0);

	// ueVec4

	s_vec4Class = ueTypeManager_CreateClass("ueVec4", NULL, NULL, ueTypeManager_GetAllocator(), sizeof(ueVec4), 0);
	s_vec4Class->AddAttribute(ueF32Type::GetInstance(), sizeof(f32) * 0, "X", 0);
	s_vec4Class->AddAttribute(ueF32Type::GetInstance(), sizeof(f32) * 1, "Y", 0);
	s_vec4Class->AddAttribute(ueF32Type::GetInstance(), sizeof(f32) * 2, "Z", 0);
	s_vec4Class->AddAttribute(ueF32Type::GetInstance(), sizeof(f32) * 3, "W", 0);

	// ueQuaternion

	s_quatClass = ueTypeManager_CreateClass("ueQuat", NULL, NULL, ueTypeManager_GetAllocator(), sizeof(ueQuat), 0);
}
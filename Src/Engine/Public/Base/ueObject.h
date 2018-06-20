#ifndef UE_OBJECT_H
#define UE_OBJECT_H

#include "Base/ueSerializer.h"

class ueClass;

/**
 *	Base object class with RTTI support.
 */
class ueObject
{
public:
	ueObject() : m_class(NULL) {}
	virtual ~ueObject() {}
	//! Destroys the object and frees its memory using class' allocator; do not use destructor directly
	virtual void Destroy();
	//! Serializes the object using given serializer
	virtual void Serialize(ueSerializer& s);
	//! Notifies object of its attribute being changed
	virtual void OnAttributeChanged(ueAttributeInfo* info, const void* ptr);
	//! Gets object class
	UE_INLINE ueClass* GetClass() const { return m_class; }
	//! Gets whether object is an instance of given class
	ueBool IsInstanceOf(ueClass* cls) const;
	//! Static/class method used to get class object
	static ueClass* StaticGetClass() { return s_staticClass_ueObject; }
	static void RTTI_Init() { s_staticClass_ueObject = ueTypeManager_CreateClass("ueObject", NULL, NULL, NULL, sizeof(ueObject), 0); }
	static void RTTI_Deinit() { ueTypeManager_DestroyClass(s_staticClass_ueObject); s_staticClass_ueObject = NULL; }
private:
	ueClass* m_class;
	static ueClass* s_staticClass_ueObject;
	friend class ueClass;
};

#define UE_DECLARE_CLASS(thisClass, superClass, version) \
	public: \
		typedef superClass Super; \
		typedef thisClass ThisClass; \
		static ueClass* RTTI_Init(ueAllocator* allocator = NULL) \
		{ \
			s_staticClass_##thisClass = ueTypeManager_CreateClass(#thisClass, superClass::StaticGetClass(), thisClass::RTTI_CreateInstance, allocator, sizeof(thisClass), version); \
			RTTI_InitAttributes(); \
			return s_staticClass_##thisClass; \
		} \
		static void RTTI_Deinit() \
		{ \
			ueTypeManager_DestroyClass(s_staticClass_##thisClass); \
			s_staticClass_##thisClass = NULL; \
		} \
		static void* RTTI_CreateInstance(void* memory) { return new(memory) thisClass(); } \
		static void RTTI_InitAttributes(); \
		static ueClass* StaticGetClass() { UE_ASSERT_MSG(s_staticClass_##thisClass, "Class RTTI hasn't been initialized"); return s_staticClass_##thisClass; } \
	private: \
		static ueClass* s_staticClass_##thisClass;

#define RT_DECLARE_STRUCT(thisClass, version) \
	public: \
		typedef thisClass ThisClass; \
		static void* RTTI_CreateInstance(void* memory) { return new(memory) thisClass(); } \
		static ueClass* RTTI_Init(ueAllocator* allocator = NULL) \
		{ \
			s_staticClass_##thisClass = ueTypeManager_CreateClass(#thisClass, NULL, thisClass::RTTI_CreateInstance, allocator, sizeof(thisClass), version); \
			RTTI_InitAttributes(); \
			return s_staticClass_##thisClass; \
		} \
		static void RTTI_Deinit() \
		{ \
			ueTypeManager_DestroyClass(s_staticClass_##thisClass); \
			s_staticClass_##thisClass = NULL; \
		} \
		static void RTTI_InitAttributes(); \
		static ueClass* StaticGetClass() { return s_staticClass_##thisClass; } \
	private: \
		static ueClass* s_staticClass_##thisClass;

//! Initializes class attribute
#define UE_INIT_ATTR(member, name) \
	StaticGetClass()->AddAttribute(rtGetType(((ThisClass*) 0x1)->member), UE_OFFSET_OF(ThisClass, member), name, 0);
//! Initializes virtual (accessed via get/set functions) class attribute
#define UE_INIT_VIRTUAL_ATTR(type, name, setFunc, getFunc) \
	StaticGetClass()->AddVirtualAttribute(type, name, setFunc, getFunc, 0);

#endif // UE_OBJECT_H
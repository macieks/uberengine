#include "Base/ueObject.h"
#include "Base/ueType.h"

void ueObject::Destroy()
{
	ueDelete(this, m_class->GetAllocator());
}

void ueObject::Serialize(ueSerializer& s)
{
}

ueBool ueObject::IsInstanceOf(ueClass* cls) const
{
	return m_class->IsSubClassOf(cls);
}
#ifndef UE_SERIALIZER_H
#define UE_SERIALIZER_H

#include "Base/ueType.h"

//! Serializer interface
class ueSerializer
{
public:
	enum Mode
	{
		Mode_IsReading	= UE_POW2(0),
		Mode_IsWriting	= UE_POW2(1),
		Mode_IsSavegame = UE_POW2(2),

		Mode_MAX
	};

	ueSerializer(Mode mode) : m_mode(mode) {}
	virtual ~ueSerializer() {}

	UE_INLINE ueBool IsReading() const { return (m_mode & Mode_IsReading) != 0; }
	UE_INLINE ueBool IsWriting() const { return (m_mode & Mode_IsWriting) != 0; }
	UE_INLINE ueBool IsSavegame() const { return (m_mode & Mode_IsSavegame) != 0; }

	template <typename NUMBER_TYPE>
	UE_INLINE ueSerializer& operator << (NUMBER_TYPE& value)
	{
		Serialize(&value, sizeof(value));
		return *this;
	}

	template <class CLASS_TYPE>
	UE_INLINE ueSerializer& operator << (CLASS_TYPE*& object)
	{
		Serialize((ueObject*&) object);
		return *this;
	}

	virtual void Serialize(void* data, u32 size) = 0;
	virtual void Serialize(ueObject*& object) = 0;

private:
	Mode m_mode;
};

// Serialization helpers

template <typename TYPE>
ueSerializer& operator << (ueSerializer& s, ueList<TYPE>& list)
{
	u32 count;
	if (s.IsReading())
	{
		s << count;
		for (u32 i = 0; i < count; i++)
		{
			TYPE* elem = NULL;
			s << elem;
			list.PushBack(elem);
		}
	}
	else
	{
		count = list.Length();
		s << count;
		TYPE* elem = list.Front();
		while (elem)
		{
			s << elem;
			elem = elem->Next();
		}
	}
	return s;
}

#endif // UE_SERIALIZER_H
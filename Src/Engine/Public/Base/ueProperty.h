#ifndef UE_PROPERTY_H
#define UE_PROPERTY_H

#include "Base/ueBase.h"

/**
 *	@addtogroup ue
 *	@{
 */

//! Available ueValue types
enum ueValueType
{
	ueValueType_Bool = 0,		//!< Boolean (ueBool)
	ueValueType_S32,			//!< 32-bit signed integer (s32)
	ueValueType_S64,			//!< 64-bit signed integer (s64)
	ueValueType_F32,			//!< 32-bit float (f32)
	ueValueType_F64,			//!< 64-bit float (f64)
	ueValueType_DateTime,		//!< Date and time (@see ueDateTime)
	ueValueType_String,			//!< UTF-8 string
	ueValueType_Binary,			//!< Binary data

	ueValueType_MAX
};

//! Arbitrary general purpose value
struct ueValue
{
	ueValueType m_type;		//!< Value type

	//! Binary data
    struct Binary
    {
        void* m_data;
        u32 m_size;
    };

	union
	{
		ueBool m_bool;
		s32 m_s32;
		s64 m_s64;
		f32 m_f32;
		f64 m_f64;
		ueDateTime m_dateTime;
		char* m_string;			//!< UTF-8, zero-terminated string
		Binary m_binary;
	};

	//! Gets additional property size in bytes (e.g. string or binary data might have non-zero size additional data)
	UE_INLINE u32 GetExtraSize() const;
	UE_INLINE ueBool operator == (const ueValue& b) const;
	UE_INLINE ueBool operator != (const ueValue& b) const;
};

UE_INLINE u32 ueValue::GetExtraSize() const
{
	switch (m_type)
	{
		case ueValueType_String: return ueStrLen(m_string) + 1;
		case ueValueType_Binary: return m_binary.m_size;
		default: return 0;
	}
}

UE_INLINE ueBool ueValue::operator == (const ueValue& b) const
{
	if (m_type != b.m_type)
		return UE_FALSE;

	switch (m_type)
	{
		case ueValueType_Bool: return m_bool == b.m_bool;
		case ueValueType_S32: return m_s32 == b.m_s32;
		case ueValueType_S64: return m_s64 == b.m_s64;
		case ueValueType_F32: return m_f32 == b.m_f32;
		case ueValueType_F64: return m_f64 == b.m_f64;
		case ueValueType_DateTime: return m_dateTime == b.m_dateTime;
		case ueValueType_String: return !ueStrCmp(m_string, b.m_string);
		case ueValueType_Binary: return m_binary.m_size == b.m_binary.m_size && !ueMemCmp(m_binary.m_data, b.m_binary.m_data, m_binary.m_size);
	}

	return UE_FALSE;
}

UE_INLINE ueBool ueValue::operator != (const ueValue& b) const
{
	return !(*this == b);
}

//! Arbitrary general purpose property
struct ueProperty
{
	u32 m_id;			//!< Property id / name
	ueValue m_value;	//!< Property value

	UE_INLINE ueBool operator == (const ueProperty& b) const;
	UE_INLINE ueBool operator != (const ueProperty& b) const;
};

UE_INLINE ueBool ueProperty::operator == (const ueProperty& b) const
{
	return m_id == b.m_id && m_value == b.m_value;
}

UE_INLINE ueBool ueProperty::operator != (const ueProperty& b) const
{
	return m_id != b.m_id || m_value != b.m_value;
}

// @}

#endif // UE_PROPERTY_H
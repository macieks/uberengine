#include "Net/ENet/mpENet_Private.h"

// mpPropertySet_ENet

void mpPropertySet_ENet::Reset()
{
	m_count = 0;
	m_dataSize = 0;
}

void mpPropertySet_ENet::Add(const ueProperty* src)
{
	UE_ASSERT(m_count < MP_ENET_MAX_SESSION_PROPS);
	UE_ASSERT(m_dataSize + src->m_value.GetExtraSize() <= MP_ENET_MAX_SESSION_PROPS_DATA_SIZE);
	UE_ASSERT(src->m_value.m_type < ueValueType_MAX);

	ueProperty* dst = &m_props[m_count];
	*dst = *src;

	switch (src->m_value.m_type)
	{
		case ueValueType_String:
		{
			const u32 stringSize = ueStrLen(src->m_value.m_string) + 1;
			ueMemCpy(&m_data[m_dataSize], src->m_value.m_string, stringSize);
			m_dataSize += stringSize;
			break;
		}
		case ueValueType_Binary:
			ueMemCpy(&m_data[m_dataSize], src->m_value.m_binary.m_data, src->m_value.m_binary.m_size);
			m_dataSize += src->m_value.m_binary.m_size;
			break;
	}

	m_count++;
}

void mpPropertySet_ENet::Remove(const ueProperty* prop)
{
	UE_NOT_IMPLEMENTED();
}

ueBool mpPropertySet_ENet::Set(const ueProperty* prop)
{
	for (u32 i = 0; i < m_count; i++)
		if (m_props[i].m_id == prop->m_id)
		{
			UE_ASSERT(m_props[i].m_value.m_type == prop->m_value.m_type);

			switch (prop->m_value.m_type)
			{
				case ueValueType_Binary:
					UE_ASSERT(m_props[i].m_value.m_binary.m_size == prop->m_value.m_binary.m_size);
					ueMemCpy(m_props[i].m_value.m_binary.m_data, prop->m_value.m_binary.m_data, prop->m_value.m_binary.m_size);
					break;
				case ueValueType_String:
					UE_NOT_IMPLEMENTED();
					break;
				default:
					m_props[i].m_value = prop->m_value;
					break;
			}
			
			return UE_TRUE;
		}

	return UE_FALSE;
}

ueBool mpPropertySet_ENet::Get(ueProperty* prop)
{
	for (u32 i = 0; i < m_count; i++)
		if (m_props[i].m_id == prop->m_id)
		{
			*prop = m_props[i];
			return UE_TRUE;
		}

	return UE_FALSE;
}

void mpPropertySet_ENet::CopyFrom(u32 count, const ueProperty* src)
{
	Reset();
	for (u32 i = 0; i < count; i++)
		Add(&src[i]);
}

void mpPropertySet_ENet::CopyFrom(const mpPropertySet_ENet* src, mpCallbacks* callbacks)
{
	// Store previous props

	mpPropertySet_ENet old;
	if (callbacks)
		old.CopyFrom(src);

	// Copy

	*this = *src;

	u32 dataOffset = 0;
	for (u32 i = 0; i < m_count; i++)
	{
		ueProperty* prop = &m_props[i];

		switch (prop->m_value.m_type)
		{
		case ueValueType_String:
			prop->m_value.m_string = (char*) &m_data[dataOffset];
			dataOffset += ueStrLen(prop->m_value.m_string) + 1;
			break;
		case ueValueType_Binary:
			prop->m_value.m_binary.m_data = &m_data[dataOffset];
			dataOffset += prop->m_value.m_binary.m_size;
			break;
		}
	}

	// Invoke callbacks

	if (!callbacks)
		return;

	for (u32 i = 0; i < m_count; i++)
	{
		ueProperty& newProperty = m_props[i];

		ueBool found = UE_FALSE;
		for (u32 j = 0; j < old.m_count; j++)
			if (m_props[i].m_id == old.m_props[j].m_id)
			{
				ueProperty& oldProperty = old.m_props[j];
				if (oldProperty != newProperty)
					callbacks->OnPropertyChange(&oldProperty, &newProperty);
				found = UE_TRUE;
				break;
			}

		if (!found)
			callbacks->OnPropertyChange(NULL, &newProperty);
	}

	for (u32 i = 0; i < old.m_count; i++)
	{
		ueProperty& oldProperty = old.m_props[i];

		ueBool found = UE_FALSE;
		for (u32 j = 0; j < m_count; j++)
			if (m_props[j].m_id == old.m_props[i].m_id)
			{
				found = UE_TRUE;
				break;
			}

		if (!found)
			callbacks->OnPropertyChange(&oldProperty, NULL);
	}
}

ueBool mpPropertySet_ENet::Read(ueBitBuffer& input)
{
	u8 count = 0;
	if (!input.ReadAny<u8>(count)) return UE_FALSE;
	if (count > MP_ENET_MAX_SESSION_PROPS) return UE_FALSE;

	u16 dataSize = 0;
	if (!input.ReadAny<u16>(dataSize)) return UE_FALSE;
	if (dataSize > MP_ENET_MAX_SESSION_PROPS_DATA_SIZE) return UE_FALSE;

	m_count = count;
	m_dataSize = 0;
	for (u32 i = 0; i < m_count; i++)
	{
		ueProperty* prop = &m_props[i];

		if (!input.ReadAny<u32>(prop->m_id)) return UE_FALSE;

		u8 type = 0;
		if (!input.ReadAny<u8>(type)) return UE_FALSE;
		if (type >= ueValueType_MAX) return UE_FALSE;
		prop->m_value.m_type = (ueValueType) type;

		switch (type)
		{
			case ueValueType_S32:
				if (!input.ReadAny<s32>(prop->m_value.m_s32)) return UE_FALSE;
				break;
			case ueValueType_F32:
				if (!input.ReadAny<f32>(prop->m_value.m_f32)) return UE_FALSE;
				break;
			case ueValueType_S64:
				if (!input.ReadAny<s64>(prop->m_value.m_s64)) return UE_FALSE;
				break;
			case ueValueType_F64:
				if (!input.ReadAny<f64>(prop->m_value.m_f64)) return UE_FALSE;
				break;
			case ueValueType_DateTime:
				if (!input.ReadAny<ueDateTime>(prop->m_value.m_dateTime)) return UE_FALSE;
				break;
			case ueValueType_String:
			{
				u16 length = 0;
				if (!input.ReadAny<u16>(length)) return UE_FALSE;
				if (m_dataSize + length >= dataSize) return UE_FALSE;
				
				prop->m_value.m_string = (char*) &m_data[m_dataSize];
				if (!input.ReadBytes(prop->m_value.m_string, length)) return UE_FALSE;
				prop->m_value.m_string[length] = 0;

				m_dataSize += length + 1;
				break;
			}
			case ueValueType_Binary:
			{
				u16 size = 0;
				if (!input.ReadAny<u16>(size)) return UE_FALSE;
				if (m_dataSize + size >= dataSize) return UE_FALSE;
				
				prop->m_value.m_binary.m_size = size;
				prop->m_value.m_binary.m_data = &m_data[m_dataSize];
				if (!input.ReadBytes(prop->m_value.m_binary.m_data, prop->m_value.m_binary.m_size)) return UE_FALSE;

				m_dataSize += prop->m_value.m_binary.m_size;
				break;
			}
		}
	}

	if (m_dataSize != dataSize) return UE_FALSE;

	return UE_TRUE;
}

void mpPropertySet_ENet::Write(ueBitBuffer& output)
{
	UE_ASSERT(m_count <= MP_ENET_MAX_SESSION_PROPS);
	UE_ASSERT(m_dataSize <= MP_ENET_MAX_SESSION_PROPS_DATA_SIZE);

	output.WriteAny<u8>(m_count);
	output.WriteAny<u16>(m_dataSize);

	for (u32 i = 0; i < m_count; i++)
	{
		ueProperty* prop = &m_props[i];

		output.WriteAny<u32>(prop->m_id);
		output.WriteAny<u8>(prop->m_value.m_type);

		switch (prop->m_value.m_type)
		{
			case ueValueType_S32:
				output.WriteAny<s32>(prop->m_value.m_s32);
				break;
			case ueValueType_F32:
				output.WriteAny<f32>(prop->m_value.m_f32);
				break;
			case ueValueType_S64:
				output.WriteAny<s64>(prop->m_value.m_s64);
				break;
			case ueValueType_F64:
				output.WriteAny<f64>(prop->m_value.m_f64);
				break;
			case ueValueType_DateTime:
				output.WriteAny<ueDateTime>(prop->m_value.m_dateTime);
				break;
			case ueValueType_String:
			{
				const u32 length = ueStrLen(prop->m_value.m_string);
				output.WriteAny<u16>(length);
				output.WriteBytes(prop->m_value.m_string, length);
				break;
			}
			case ueValueType_Binary:
				output.WriteAny<u16>(prop->m_value.m_binary.m_size);
				output.WriteBytes(prop->m_value.m_binary.m_data, prop->m_value.m_binary.m_size);
				break;
		}
	}
}

ueBool mpPropertySet_ENet::Match(const mpPropertySet_ENet* other)
{
	for (u32 i = 0; i < m_count; i++)
	{
		const ueProperty& src = m_props[i];

		ueBool found = UE_FALSE;
		for (u32 j = 0; j < other->m_count; j++)
		{
			const ueProperty& dst = other->m_props[j];
			if (src == dst)
			{
				found = UE_TRUE;
				break;
			}
		}

		if (!found)
			return UE_FALSE;
	}

	return UE_TRUE;
}

// mpLib_ENet

void mpLib_ENet::SetProperty(ueProperty* prop)
{
	UE_ASSERT(IsHost());
	m_props.Set(prop);
}

ueBool mpLib_ENet::GetProperty(ueProperty* prop)
{
	UE_ASSERT(IsHost() || (m_isClient && m_state != mpState_Joining));
	return m_props.Get(prop);
}

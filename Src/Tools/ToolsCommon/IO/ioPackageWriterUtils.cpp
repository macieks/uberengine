#include "IO/ioPackageWriter.h"
#include "IO/ioPackageWriterUtils.h"
#include "Utils/utValueInTime.h"

void WriteVec2(ioSegmentWriter* sw, const ueVec2& v)
{
	sw->WriteAlignVec();
	sw->WriteNumber<f32>(v[0]);
	sw->WriteNumber<f32>(v[1]);
	if (sw->GetConfig().m_alignVecsTo16Bytes)
	{
		sw->WriteNumber<f32>(0.0f);
		sw->WriteNumber<f32>(0.0f);
	}
}

void WriteVec3(ioSegmentWriter* sw, const ueVec3& v)
{
	sw->WriteAlignVec();
	sw->WriteNumber<f32>(v[0]);
	sw->WriteNumber<f32>(v[1]);
	sw->WriteNumber<f32>(v[2]);
	if (sw->GetConfig().m_alignVecsTo16Bytes)
		sw->WriteNumber<f32>(0.0f);
}

void WriteVec4(ioSegmentWriter* sw, const ueVec4& v)
{
	sw->WriteAlignVec();
	sw->WriteNumber<f32>(v[0]);
	sw->WriteNumber<f32>(v[1]);
	sw->WriteNumber<f32>(v[2]);
	sw->WriteNumber<f32>(v[3]);
}

void WriteQuat(ioSegmentWriter* sw, const ueQuat& v)
{
	sw->WriteAlignVec();
	sw->WriteNumber<f32>(v[0]);
	sw->WriteNumber<f32>(v[1]);
	sw->WriteNumber<f32>(v[2]);
	sw->WriteNumber<f32>(v[3]);
}

void WriteMat44(ioSegmentWriter* sw, const ueMat44& m)
{
	for (u32 i = 0; i < 4; i++)
		WriteVec4(sw, m.GetColumn(i));
}

void WriteBox(ioSegmentWriter* sw, const ueBox& box)
{
	WriteVec3(sw, box.m_min);
	WriteVec3(sw, box.m_max);
}

void WriteRectI(ioSegmentWriter* sw, const ueRectI& rect)
{
	sw->WriteNumber<s32>(rect.m_left);
	sw->WriteNumber<s32>(rect.m_top);
	sw->WriteNumber<s32>(rect.m_width);
	sw->WriteNumber<s32>(rect.m_height);
}

void WriteEmptyList(ioSegmentWriter* sw)
{
	sw->WriteNullPtr();		// m_head::m_next
	sw->WriteNullPtr();		// m_head::m_prev
	sw->WriteNullPtr();		// m_last
	sw->WriteNumber<u32>(0);// m_size
}

void WriteVec2Stride(ioSegmentWriter* sw)
{
	const u32 vector2Size = sizeof(f32) * (sw->GetConfig().m_alignVecsTo16Bytes ? 4 : 2);
	sw->WriteNumber<u32>(vector2Size);
}

void WriteVec3Stride(ioSegmentWriter* sw)
{
	const u32 vector3Size = sizeof(f32) * (sw->GetConfig().m_alignVecsTo16Bytes ? 4 : 3);
	sw->WriteNumber<u32>(vector3Size);
}

void WriteValueInTime(ioSegmentWriter* sw, utValueInTime* value)
{
	sw->WriteNumber<u32>(value->m_numComponents);
	sw->WriteNumber<u32>(value->m_type);
	switch (value->m_type)
	{
		case utValueInTimeType_Constant:
		{
			utValueInTime_Constant* _value = (utValueInTime_Constant*) value;
			sw->WriteAndBeginPtr();
			for (u32 i = 0; i < value->m_numComponents; i++)
				sw->WriteNumber<f32>(_value->m_value[i]);
			break;
		}
		case utValueInTimeType_ConstantRange:
		{
			utValueInTime_ConstantRange* _value = (utValueInTime_ConstantRange*) value;
			ioPtr minPtr = sw->WritePtr();
			ioPtr maxPtr = sw->WritePtr();
			sw->BeginPtr(minPtr);
			for (u32 i = 0; i < value->m_numComponents; i++)
				sw->WriteNumber<f32>(_value->m_minValue[i]);
			sw->BeginPtr(maxPtr);
			for (u32 i = 0; i < value->m_numComponents; i++)
				sw->WriteNumber<f32>(_value->m_maxValue[i]);
			break;
		}
		case utValueInTimeType_TimeLine:
		{
			utValueInTime_TimeLine* _value = (utValueInTime_TimeLine*) value;
			sw->WriteNumber<u32>(_value->m_numSamples);
			sw->WriteNumber<f32>(_value->m_sampleTime);
			const u32 numSamplesTotal = _value->m_numSamples * value->m_numComponents;
			sw->WriteAndBeginPtr();
			for (u32 i = 0; i < numSamplesTotal; i++)
				sw->WriteNumber<f32>(_value->m_values[i]);
			break;
		}
		case utValueInTimeType_TimeLineRange:
		{
			utValueInTime_TimeLineRange* _value = (utValueInTime_TimeLineRange*) value;
			sw->WriteNumber<u32>(_value->m_numSamples);
			sw->WriteNumber<f32>(_value->m_sampleTime);
			const u32 numSamplesTotal = _value->m_numSamples * value->m_numComponents;
			ioPtr minPtr = sw->WritePtr();
			ioPtr maxPtr = sw->WritePtr();
			sw->BeginPtr(minPtr);
			for (u32 i = 0; i < numSamplesTotal; i++)
				sw->WriteNumber<f32>(_value->m_minValues[i]);
			sw->BeginPtr(maxPtr);
			for (u32 i = 0; i < numSamplesTotal; i++)
				sw->WriteNumber<f32>(_value->m_maxValues[i]);
			break;
		}
	}
}

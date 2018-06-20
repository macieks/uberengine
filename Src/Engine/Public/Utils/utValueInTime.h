#ifndef UT_VALUE_IN_TIME_H
#define UT_VALUE_IN_TIME_H

#include "Base/ueBase.h"

/**
 *	@addtogroup ut
 *	@{
 */

//! Type of the value (optionally changing in time)
enum utValueInTimeType
{
	utValueInTimeType_Constant = 0,		//!< Single constant value (one per component)
	utValueInTimeType_ConstantRange,	//!< Min/max constant range (one per component)
	utValueInTimeType_TimeLine,			//!< Value changing in time (one per component)
	utValueInTimeType_TimeLineRange,	//!< Min/max range changing in time (one per component)

	utValueInTimeType_MAX
};

struct utValueInTime
{
	u32 m_numComponents;			//!< Number of components (e.g. 4 for RGBA, 3 for XYZ)
	utValueInTimeType m_type : 32;	//!< Value type

	UE_INLINE utValueInTime(utValueInTimeType type) : m_type(type) {}
};

//! Constant value
struct utValueInTime_Constant : utValueInTime
{
	f32* m_value;	//!< Array of values (one per component)

	UE_INLINE utValueInTime_Constant() : utValueInTime(utValueInTimeType_Constant) {}
};

//! Constant range of values
struct utValueInTime_ConstantRange : utValueInTime
{
	f32* m_minValue;	//!< Array of min values (one per component)
	f32* m_maxValue;	//!< Array of max values (one per component)

	UE_INLINE utValueInTime_ConstantRange() : utValueInTime(utValueInTimeType_ConstantRange) {}

	//! Gets value for given seed (values in 0..1 range; one per component)
	UE_INLINE void Val(f32* seed, f32* dst) const
	{
		for (u32 i = 0; i < m_numComponents; i++)
		{
			UE_ASSERT(0.0f <= seed[i] && seed[i] <= 1.0f);
			dst[i] = ueLerp(m_minValue[i], m_maxValue[i], seed[i]);
		}
	}
};

//! Value changing in time
struct utValueInTime_TimeLine : utValueInTime
{
	u32 m_numSamples;	//!< Number of samples
	f32 m_sampleTime;	//!< Time between each sample
	f32* m_values;		//!< Array of values (one per component)

	UE_INLINE utValueInTime_TimeLine() : utValueInTime(utValueInTimeType_TimeLine) {}
	//! Gets value at given time (value in 0..1 range)
	UE_INLINE void Val(f32 time, f32* dst) const
	{
		u32 a, b;
		f32 scale;
		ueCalcTimelineSampleParams(m_numSamples, time, a, b, scale);

		a *= m_numComponents;
		b *= m_numComponents;

		for (u32 i = 0; i < m_numComponents; i++)
			dst[i] = ueLerp(m_values[a + i], m_values[b + i], scale);
	}
};

//! Range changing in time
struct utValueInTime_TimeLineRange : utValueInTime
{
	u32 m_numSamples;	//!< Number of samples
	f32 m_sampleTime;	//!< Time between each sample
	f32* m_minValues;	//!< Array of min values (one per component)
	f32* m_maxValues;	//!< Array of max values (one per component)

	UE_INLINE utValueInTime_TimeLineRange() : utValueInTime(utValueInTimeType_TimeLineRange) {}
	//! Gets value at given time (value in 0..1 range) for given seed (values in 0..1 range; one per component)
	UE_INLINE void Val(f32 time, const f32* seed, f32* dst) const
	{
		u32 a, b;
		f32 scale;
		ueCalcTimelineSampleParams(m_numSamples, time, a, b, scale);

		a *= m_numComponents;
		b *= m_numComponents;

		for (u32 i = 0; i < m_numComponents; i++)
		{
			UE_ASSERT(0.0f <= seed[i] && seed[i] <= 1.0f);
			dst[i] = ueLerp(
				ueLerp(m_minValues[a + i], m_minValues[b + i], scale),
				ueLerp(m_maxValues[a + i], m_maxValues[b + i], scale),
				seed[i]);
		}
	}
};

#if defined(UE_TOOLS)

	struct ioXmlNode;

	//! Loads and creates value from Xml node
	utValueInTime* utValueInTime_CreateFromXmlNode(ueAllocator* allocator, ioXmlNode* node);

#endif

// @}

#endif // UT_VALUE_IN_TIME_H
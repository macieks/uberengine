#include "Sample.h"

struct ueSampleMgrData
{
	u32 m_numSamples;
	ueSampleCreateFunc m_samples[32];
	const char* m_sampleNames[32];

	ueSampleMgrData() : m_numSamples(0) {}
};

static ueSampleMgrData s_data;

ueSampleInitializer::ueSampleInitializer(ueSampleCreateFunc func, const char* name)
{
	ueSampleMgr_Register(func, name);
}

void ueSampleMgr_Register(ueSampleCreateFunc sample, const char* name)
{
	UE_ASSERT(s_data.m_numSamples < UE_ARRAY_SIZE(s_data.m_samples));
	s_data.m_samples[s_data.m_numSamples] = sample;
	s_data.m_sampleNames[s_data.m_numSamples] = name;
	s_data.m_numSamples++;
}

void ueSampleMgr_Unregister(ueSampleCreateFunc sample)
{
	for (u32 i = 0; i < s_data.m_numSamples; i++)
		if (s_data.m_samples[i] == sample)
		{
			s_data.m_samples[i] = s_data.m_samples[--s_data.m_numSamples];
			break;
		}
	UE_ASSERT(0);
}

ueSampleCreateFunc ueSampleMgr_GetFirst()
{
	UE_ASSERT(s_data.m_numSamples > 0);
	return s_data.m_samples[0];
}

const char* ueSampleMgr_GetName(ueSampleCreateFunc func)
{
	for (u32 i = 0; i < s_data.m_numSamples; i++)
		if (s_data.m_samples[i] == func)
			return s_data.m_sampleNames[i];
	return NULL;
}

ueSampleCreateFunc ueSampleMgr_GetByName(const char* name)
{
	for (u32 i = 0; i < s_data.m_numSamples; i++)
		if (!ueStrCmp(s_data.m_sampleNames[i], name))
			return s_data.m_samples[i];
	UE_ASSERT_MSGP(0, "Sample '%s' not found", name);
	return NULL;
}

ueSampleCreateFunc ueSampleMgr_GetNext(ueSampleCreateFunc sample)
{
	for (u32 i = 0; i < s_data.m_numSamples; i++)
		if (s_data.m_samples[i] == sample)
			return s_data.m_samples[(i + 1) % s_data.m_numSamples];
	UE_ASSERT(0);
	return NULL;
}

ueSampleCreateFunc ueSampleMgr_GetPrev(ueSampleCreateFunc sample)
{
	for (u32 i = 0; i < s_data.m_numSamples; i++)
		if (s_data.m_samples[i] == sample)
			return s_data.m_samples[(i + s_data.m_numSamples - 1) % s_data.m_numSamples];
	UE_ASSERT(0);
	return NULL;
}
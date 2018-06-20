#include "Base/Containers/ueUniqueIdGenerator.h"

void ueUniqueIdGenerator::Reset()
{
	for (u32 i = 0; i < MAX_IDS; i++)
	{
		m_slots[i].m_lastUsed = ueClock_GetCurrent();
		m_slots[i].m_isUsed = UE_FALSE;
	}
}

u8 ueUniqueIdGenerator::GenerateId()
{
	u32 bestIndex = U32_MAX;
	ueTime bestTicks = 0;

	for (u32 i = 0; i < MAX_IDS; i++)
		if (!m_slots[i].m_isUsed && (bestIndex == U32_MAX || m_slots[i].m_lastUsed < bestTicks))
		{
			bestIndex = i;
			bestTicks = m_slots[i].m_lastUsed;
		}
	UE_ASSERT(bestIndex != U32_MAX);

	m_slots[bestIndex].m_isUsed = UE_TRUE;
	return (u8) bestIndex;
}

void ueUniqueIdGenerator::ReleaseId(u8 id)
{
	UE_ASSERT(m_slots[id].m_isUsed);
	m_slots[id].m_lastUsed = ueClock_GetCurrent();
	m_slots[id].m_isUsed = UE_FALSE;
}

void ueUniqueIdGenerator::MarkIdAsUsed(u8 id)
{
	UE_ASSERT(!m_slots[id].m_isUsed);
	m_slots[id].m_isUsed = UE_TRUE;
}

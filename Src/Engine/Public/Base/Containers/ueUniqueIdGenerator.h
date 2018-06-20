#ifndef UE_UNIQUE_ID_GENERATOR_H
#define UE_UNIQUE_ID_GENERATOR_H

#include "Base/ueBase.h"

//! Unique id generator that minimizes frequency of reusing values
class ueUniqueIdGenerator
{
public:
	//! Max. number of ids this generator can manage at the same time
	static const u32 MAX_IDS = 256;

	//! Clears set of ids
	void Reset();

	//! Returns new unique id
	u8 GenerateId();
	//! Releases id
	void ReleaseId(u8 id);

	//! Marks id as sused
	void MarkIdAsUsed(u8 id);

private:
	struct Slot
	{
		ueTime m_lastUsed;
		u8 m_isUsed;
	};

	Slot m_slots[MAX_IDS];
};

#endif // UE_UNIQUE_ID_GENERATOR_H

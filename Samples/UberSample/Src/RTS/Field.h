#pragma once

#include "Point.h"

class Entity;

struct Field
{
	enum Flags
	{
		Flags_Passable	= 1 << 0,

		// Path finding

		Flags_IsOpen	= 1 << 1,
		Flags_IsClosed	= 1 << 2,
	};

	Point m_pos;
	u32 m_flags;
	Entity* m_entity;

	// Path finding

	f32 g, f;
	Field* m_parent;

	UE_INLINE void ResetPathFinding()
	{
		m_flags &= ~(Flags_IsOpen | Flags_IsClosed);
		m_parent = NULL;
	}

	UE_INLINE ueBool IsEmpty() const
	{
		return (m_flags & Flags_Passable) && !m_entity;
	}
};

#pragma once

class Entity
{
public:
	enum Type
	{
		Type_Soldier = 0,

		Type_MAX
	};

	Entity(Type type) : m_type(type) {}

protected:
	Type m_type;
};
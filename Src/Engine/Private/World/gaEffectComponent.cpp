#include "World/gaEffectComponent.h"

void gaEffectComponent::RTTI_InitAttributes()
{
	UE_INIT_ATTR(m_effectType, "Effect");
}

void gaEffectComponent::Serialize(ueSerializer& s)
{
	Super::Serialize(s);

	s << m_effectType;

	if (s.IsReading() && m_effectType.IsReady())
		m_handle.Init(*m_effectType);
}

gaEffectComponent::~gaEffectComponent()
{
}

void gaEffectComponent::SetEffectTypeByName(const char* name)
{
	m_effectType.SetByName(name);
	m_handle.Init(*m_effectType);
}

void gaEffectComponent::Update(f32 dt)
{
	m_handle.Update(dt);
}

void gaEffectComponent::SubmitRenderPackets(gxFrame* frame)
{
	//m_handle.SubmitRenderPackets(frame);
}
#include "World/gaActor.h"
#include "World/gaComponent.h"

void gaActor::RTTI_InitAttributes()
{
	UE_INIT_ATTR(m_pos, "Position");
	UE_INIT_ATTR(m_scale, "Scale");
	UE_INIT_ATTR(m_rot, "Rotation");
}

void gaActor::Update(f32 dt)
{
	GA_FOREACH_COMPONENT(c)
		c->Update(dt);
}

void gaActor::SubmitRenderPackets(gxFrame* frame)
{
	GA_FOREACH_COMPONENT(c)
		c->SubmitRenderPackets(frame);
}

void gaActor::Serialize(ueSerializer& s)
{
	s << m_components << m_pos << m_scale << m_rot;
}

void gaActor::AddComponent(gaComponent* component)
{
	UE_ASSERT(!component->m_actor);
	m_components.PushBack(component);
	component->m_actor = this;
}

void gaActor::RemoveComponent(gaComponent* component)
{
	UE_ASSERT(component->m_actor == this);
	component->m_actor = NULL;
	m_components.Remove(component);
}

void gaActor::DestroyComponent(gaComponent* component)
{
	RemoveComponent(component);
	component->Destroy();
}
#include "World/gaLightComponent.h"

void gaLightComponent::RTTI_InitAttributes()
{
	UE_INIT_ATTR(m_desc, "Description");

	UE_INIT_ATTR(m_isStatic, "Is static");
	UE_INIT_ATTR(m_castsDynamicShadows, "Casts dynamic shadows");
	UE_INIT_ATTR(m_castsStaticShadows, "Casts static shadows");

	UE_INIT_ATTR(m_radius, "Radius");
	UE_INIT_ATTR(m_color, "Color");

	UE_INIT_ATTR(m_projectedSize, "Projected texture size");
	UE_INIT_ATTR(m_projectedTexture, "Projected texture");
}

gaLightComponent::~gaLightComponent()
{
}

void gaLightComponent::Update(f32 dt)
{
}

void gaLightComponent::SubmitRenderPackets(gxFrame* frame)
{
	if (rdDebugSpherePacket* packet = frame->AddPacket<rdDebugSpherePacket>(gxPass_Debug))
	{
		packet->m_sphere.m_radius = m_radius;
		packet->m_sphere.m_center = m_pos;
	}
}
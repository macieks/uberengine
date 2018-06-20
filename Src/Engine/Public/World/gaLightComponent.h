#ifndef GA_LIGHT_COMPONENT_H
#define GA_LIGHT_COMPONENT_H

#include "GraphicsExt/gxMaterialConfig_Common.h"
#include "GraphicsExt/gxTexture.h"
#include "World/gaComponent.h"

class gaLightComponent : public gaComponent
{
	UE_DECLARE_CLASS(gaLightComponent, gaComponent, 0);

public:
	gaLightComponent() {}
	~gaLightComponent();
	virtual void Update(f32 dt);
	virtual void SubmitRenderPackets(gxFrame* frame);
	virtual void Serialize(ueSerializer& s);

private:
	gxLightDesc		m_desc;

	ueBool			m_isStatic;
	ueBool			m_castsDynamicShadows;
	ueBool			m_castsStaticShadows;

	f32				m_radius;
	ueVec3			m_color;
	//ueFunc*		m_colorFunction; // Optional

	ueVec3			m_projectedSize;
	ueResourceHandle<gxTexture> m_projectedTexture;
};

#endif // GA_LIGHT_COMPONENT_H
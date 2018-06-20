#ifndef GA_EFFECT_COMPONENT_H
#define GA_EFFECT_COMPONENT_H

#include "GraphicsExt/gxEffect.h"
#include "World/gaComponent.h"

class gaEffectComponent : public gaComponent
{
	UE_DECLARE_CLASS(gaEffectComponent, gaComponent, 0);

public:
	gaEffectComponent();
	~gaEffectComponent();

	void SetEffectTypeByName(const char* name);

	virtual void Update(f32 dt);
	virtual void SubmitRenderPackets(gxFrame* frame);
	virtual void Serialize(ueSerializer& s);

private:
	gxEffectHandle m_handle;

	ueResourceHandle<gxEffectType> m_effectType;
};

#endif // GA_EFFECT_COMPONENT_H
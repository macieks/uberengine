#ifndef GA_MODEL_COMPONENT_H
#define GA_MODEL_COMPONENT_H

#include "GraphicsExt/gxMaterialConfig_Common.h"
#include "GraphicsExt/gxModel.h"
#include "World/gaComponent.h"

class gaModelComponent : public gaComponent
{
	UE_DECLARE_CLASS(gaModelComponent, gaComponent, 0);

public:
	gaModelComponent() : m_modelInstance(NULL) {}
	~gaModelComponent();

	void SetModelByName(const char* name);
	void SetModel(gxModel* model);

	virtual void Update(f32 dt);
	virtual void SubmitRenderPackets(gxFrame* frame);
	virtual void Serialize(ueSerializer& s);

private:
	gxModelInstance* m_modelInstance;

	ueResourceHandle<gxModel> m_model;

	gxLightMapDesc m_lightmapDesc;

	ueBool m_isStatic;
	ueBool m_castsDynamicShadows;
	ueBool m_castsStaticShadows;
	ueBool m_receivesDynamicShadows;
	ueBool m_receivesStaticShadows;

	ueBool m_interactsWithFog;
};

#endif // GA_MODEL_COMPONENT_H
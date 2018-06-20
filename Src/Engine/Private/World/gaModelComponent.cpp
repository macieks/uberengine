#include "World/gaModelComponent.h"
#include "GraphicsExt/gxModelInstance.h"

void gaModelComponent::RTTI_InitAttributes()
{
	UE_INIT_ATTR(m_model, "Model");
	UE_INIT_ATTR(m_isStatic, "Is static");
	UE_INIT_ATTR(m_castsDynamicShadows, "Casts dynamic shadows");
	UE_INIT_ATTR(m_castsStaticShadows, "Casts static shadows");
	UE_INIT_ATTR(m_receivesDynamicShadows, "Receives dynamic shadows");
	UE_INIT_ATTR(m_receivesStaticShadows, "Receives static shadows");
}

void gaModelComponent::Serialize(ueSerializer& s)
{
	Super::Serialize(s);

	s << m_model;

	if (s.IsReading() && m_model.IsReady())
		m_modelInstance = gxModel_CreateInstance(*m_model);
}

gaModelComponent::~gaModelComponent()
{
	SetModel(NULL);
}

void gaModelComponent::SetModel(gxModel* model)
{
	m_model = model;

	// Recreate model instance

	if (m_modelInstance)
	{
		gxModelInstance_Destroy(m_modelInstance);
		m_modelInstance = NULL;
	}
	if (m_model.IsReady())
		m_modelInstance = gxModel_CreateInstance(model);
}

void gaModelComponent::SetModelByName(const char* name)
{
}

void gaModelComponent::Update(f32 dt)
{
	if (m_modelInstance)
		gxModelInstance_Update(m_modelInstance, dt);
}

void gaModelComponent::SubmitRenderPackets(gxFrame* frame)
{
	if (!m_modelInstance)
		return;

	gxModelInstance_SubmitRenderPackets(m_modelInstance, frame);
}
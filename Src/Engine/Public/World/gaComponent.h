#ifndef GA_COMPONENT_H
#define GA_COMPONENT_H

#include "Base/ueObject.h"
#include "Base/ueMath.h"
#include "Base/Containers/ueList.h"

struct gxFrame;
class gaActor;

struct ueTransform
{
	ueVec3 m_pos;
	ueVec3 m_scale;
	ueQuat m_rot;
};

class gaComponent : public ueObject, public ueList<gaComponent>::Node
{
	UE_DECLARE_CLASS(gaComponent, ueObject, 0);

public:
	gaComponent();
	~gaComponent();
	virtual void Update(f32 dt) {}
	virtual void SubmitRenderPackets(gxFrame* frame) {}

private:
	gaActor* m_actor;

	ueTransform m_localTransform;
	ueTransform m_worldTransform;

	friend class gaActor;
};

#endif // GA_COMPONENT_H
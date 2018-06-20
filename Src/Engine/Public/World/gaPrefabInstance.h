#ifndef GA_PREFAB_INSTANCE_H
#define GA_PREFAB_INSTANCE_H

#include "World/gaActor.h"
#include "World/gaPrefab.h"

class gaPrefabInstance : public gaActor
{
	UE_DECLARE_CLASS(gaPrefabInstance, gaActor, 0);

public:
	virtual void Destroy();
	virtual void Update(f32 dt);
	virtual void SubmitRenderPackets(gxFrame* frame);
	virtual void Serialize(ueSerializer& s);

private:
	ueVector<gaActor*> m_actors;

	ueResourceHandle<gaPrefab> m_prefab;
};

#endif // GA_PREFAB_INSTANCE_H
#ifndef GA_ACTOR_H
#define GA_ACTOR_H

#include "World/gaComponent.h"

class gaPrefabInstance;

class gaActor : public ueObject
{
	UE_DECLARE_CLASS(gaActor, ueObject, 0);

public:
	template <class COMPONENT_TYPE> COMPONENT_TYPE* CreateComponent();
	void DestroyComponent(gaComponent* component);
	template <class COMPONENT_TYPE> COMPONENT_TYPE* FindComponent();

	UE_INLINE void SetPosition(const ueVec3& pos) { m_transform.m_pos = pos; }
	UE_INLINE void SetScale(const ueVec3& scale) { m_transform.m_scale = scale; }
	UE_INLINE void SetScale(f32 scale) { m_transform.m_scale.Set(scale, scale, scale); }
	UE_INLINE void SetRotation(const ueQuat& rot) { m_transform.m_rot = rot; }

	virtual void Destroy();
	virtual void Update(f32 dt);
	virtual void SubmitRenderPackets(gxFrame* frame);
	virtual void Serialize(ueSerializer& s);

private:
	void AddComponent(gaComponent* component);
	void RemoveComponent(gaComponent* component);

	ueList<gaComponent> m_components;

	ueTransform m_worldTransform;

	gaPrefabInstance* m_parentPrefabInstance;

	friend class gaComponent;
};

template <class COMPONENT_TYPE>
COMPONENT_TYPE* gaActor::CreateComponent()
{
	COMPONENT_TYPE* component = COMPONENT_TYPE::StaticGetClass()->CreateInstance();
	UE_ASSERT(component);
	UE_ASSERT_MSG(component->InstanceOf(gaComponent::StaticGetClass()), "Attempted to create object that isn't component using gaActor::CreateComponent()");
	AddComponent(component);
	return component;
}

template <class COMPONENT_TYPE>
COMPONENT_TYPE* gaActor::FindComponent()
{
	ueClass* cls = COMPONENT_TYPE::StaticGetClass();
	COMPONENT_TYPE* component = m_components.Front();
	while (component)
	{
		if (component->InstanceOf(cls))
			return component;
		component = component->Next();
	}
	return NULL;
}

#define GA_FOREACH_COMPONENT(component) \
	for (gaComponent* component = m_components.Front(); component; component = component->Next())

#endif // GA_ACTOR_H
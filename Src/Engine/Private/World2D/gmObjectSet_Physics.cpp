#include "gmObjectSet.h"
#include "gmObject.h"
#include "gmObjectDef.h"
#include "GameObjectSystem/gmRenderer.h"
#include "GL/glLib.h"
#include "GX/gxDebugWindow.h"
#include "GX/gxSprite.h"
#include "Physics/Box2D/b2SoftBody.h"
#include "Physics/Box2D/phBox2D.h"
#include "Input/inSys.h"
#include "Audio/auLib.h"
#include "IO/ioFile.h"
#include <Box2D.h>

// gmContactListener

void gmContactListener::Add(const b2ContactPoint* point)
{
	if (m_objectSet->m_isDebugDrawing || m_objectSet->m_numContacts == m_objectSet->m_maxContacts)
		return;
	m_objectSet->m_contacts[m_objectSet->m_numContacts++] = *point;
}

void gmContactListener::Persist(const b2ContactPoint* point)
{
	if (m_objectSet->m_isDebugDrawing || m_objectSet->m_numContacts == m_objectSet->m_maxContacts)
		return;
	m_objectSet->m_contacts[m_objectSet->m_numContacts++] = *point;
}

void gmContactListener::Remove(const b2ContactPoint* point)
{
}

void gmContactListener::Result(const b2ContactResult* point)
{
	m_objectSet->OnContactPointSolved(point);
}

// gmObjectSet

void gmObjectSet::CreateWorld()
{
	UE_ASSERT(!m_world);

	// Determine bounding rectangle

	b2AABB aabb;
	aabb.lowerBound.Set(m_aabb.m_left, m_aabb.m_top);
	aabb.lowerBound *= gmObjectSys_GetToPhysScale();
	aabb.upperBound.Set(m_aabb.m_right, m_aabb.m_bottom);
	aabb.upperBound *= gmObjectSys_GetToPhysScale();

	// Create world

	m_world = new(gmObjectSys_GetAllocator()) b2World(aabb, b2Vec2(0.0f, m_gravity * gmObjectSys_GetToPhysScale()), false);
	UE_ASSERT(m_world);

	m_contactListener.m_objectSet = this;
	m_world->SetContactListener(&m_contactListener);

	// Init contact points buffer

	m_maxContacts = 2048;
	m_numContacts = 0;
	m_contacts = (b2ContactPoint*) gmObjectSys_GetAllocator()->Alloc(sizeof(b2ContactPoint) * m_maxContacts);
	UE_ASSERT(m_contacts);
}

void gmObjectSet::DestroyWorld()
{
	ueDelete(m_world, gmObjectSys_GetAllocator());
	m_world = NULL;

	gmObjectSys_GetAllocator()->Free(m_contacts);
	m_contacts = NULL;
}

void gmObjectSet::UpdatePhysics(f32 dt)
{
	ueTimer physicsTimer;

	// Step the world

	m_numContacts = 0;
	dt = ueMin(dt, 1.0f / 20.0f);
	m_world->Step(dt, 5);

	// Process collisions

	ProcessCollisions();

	m_physicsTime = physicsTimer.GetSecs();
}

void gmObjectSet::ProcessCollisions()
{
	for (u32 i = 0; i < m_numContacts; i++)
	{
		b2ContactPoint* contact = &m_contacts[i];

		gmElement* element1 = (gmElement*) contact->shape1->GetBody()->GetUserData();
		gmElement* element2 = (gmElement*) contact->shape2->GetBody()->GetUserData();

		// Handle object 1 -> object 2 interaction

		if ((element1 && element1->m_object->m_toDelete) || (element2 && element2->m_object->m_toDelete))
			continue;

		if (element1)
			element1->SendEvent(gmEventType_OnCollide, contact);

		// Handle object 2 -> object 1 interaction

		if ((element1 && element1->m_object->m_toDelete) || (element2 && element2->m_object->m_toDelete))
			continue;

		if (element2)
			element2->SendEvent(gmEventType_OnCollide, contact);
	}
}

void gmObjectSet::OnContactPointSolved(const b2ContactResult* point)
{
	gmElement* element1 = (gmElement*) point->shape1->GetBody()->GetUserData();
	gmElement* element2 = (gmElement*) point->shape2->GetBody()->GetUserData();

	// Handle object 1 -> object 2 interaction

	if ((element1 && element1->m_object->m_toDelete) || (element2 && element2->m_object->m_toDelete))
		return;

	if (element2)
		element2->SendEvent(gmEventType_OnCollide2, const_cast<b2ContactResult*>(point));
}
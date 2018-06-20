#include "Base/ueBase.h"

#if defined(PH_BULLET)

#include "Physics/Bullet/phLib_Bullet_Private.h"

phBody* phBody_Create(phScene* scene, phBodyDesc* desc)
{
	const ueBool isDynamic = desc->m_mass != 0;

	// Determine initial transformation
	btTransform btInitialTransform;
	phBulletConverter::ToBulletTransform(*desc->m_transform, btInitialTransform);

	// Handle dynamic object
	btVector3 localInertia(0, 0, 0);
	btDefaultMotionState* motionState = NULL;
	if (isDynamic)
	{
		((btCollisionShape*) desc->m_shape)->calculateLocalInertia(desc->m_mass, localInertia);
		motionState = PH_NEW_P(s_data.m_allocator, btDefaultMotionState, btInitialTransform);
	}

	// Construct the body
	btRigidBody::btRigidBodyConstructionInfo initInfo(desc->m_mass, motionState, (btCollisionShape*) desc->m_shape, localInertia);
	btRigidBody* body = new btRigidBody(initInfo);

	// Handle static object's transformation
	if (!motionState)
		body->setWorldTransform(btInitialTransform);

	// Add an object to world
	body->setUserPointer2(scene);
	scene->m_world->addRigidBody(body);

	return (phBody*) body;
}

void phBody_Destroy(phBody* _body)
{
	btRigidBody* body = (btRigidBody*) _body;
	phScene* scene = (phScene*) body->getUserPointer2();
	scene->m_world->removeRigidBody(body);
	btMotionState* ms = body->getMotionState();
	if (ms)
		PH_DELETE(s_data.m_allocator, ms, btMotionState);
	delete body;
}

void phBody_SetUserData(phBody* body, void* userData) { ((btRigidBody*) body)->setUserPointer(userData); }
void* phBody_GetUserData(phBody* body) { return ((btRigidBody*) body)->getUserPointer(); }

f32 phBody_GetSpeed(phBody* body)
{
	ueVec3 vel;
	phBody_GetLinearVelocity(body, vel);
	return ueVec3_Len(vel);
}

ueBool phBody_IsDynamic(phBody* body) { return ((btRigidBody*) body)->getInvMass() != 0.0f; }

void phBody_GetTransform(phBody* body, ueMat44& transform)
{
	const btTransform& srcTransform = ((btRigidBody*) body)->getWorldTransform();
	phBulletConverter::FromBulletTransform(transform, srcTransform);
}

void phBody_SetTransform(phBody* body, const ueMat44& transform)
{
	btTransform dstTransform;
	phBulletConverter::ToBulletTransform(transform, dstTransform);
	((btRigidBody*) body)->setWorldTransform(dstTransform);
}

void phBody_GetLinearVelocity(phBody* body, ueVec3& out)
{
	const btVector3& velocity = ((btRigidBody*) body)->getLinearVelocity();
	out = phBulletConverter::FromBulletVec3(velocity);
}

void phBody_SetLinearVelocity(phBody* body, const ueVec3& velocity)
{
	((btRigidBody*) body)->setLinearVelocity( phBulletConverter::ToBulletVec3(velocity) );
}

f32 phBody_GetMass(phBody* body)
{
	const btScalar invMass = ((btRigidBody*) body)->getInvMass();
	return invMass == 0 ? 0 : (1.0f / invMass);
}

void phBody_SetMass(phBody* body, f32 mass)
{
	const ueBool isDynamic = mass != 0.0f;
	UE_ASSERT(isDynamic == phBody_IsDynamic(body));

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		((btRigidBody*) body)->getCollisionShape()->calculateLocalInertia(mass, localInertia);
	((btRigidBody*) body)->setMassProps(mass, localInertia);
}

u32 phBody_GetCollisionFlags(phBody* body)
{
	// TODO
	return 0;
}

void phBody_SetCollisionFlags(phBody* body, u32 flags)
{
	// TODO
}

void phBody_ApplyTorque(phBody* body, const ueVec3& torque)
{
	((btRigidBody*) body)->applyTorque(phBulletConverter::ToBulletVec3(torque));
}

void phBody_ApplyImpulse(phBody* body, const ueVec3& impulse)
{
	((btRigidBody*) body)->applyCentralImpulse(phBulletConverter::ToBulletVec3(impulse));
}

#else // defined(PH_BULLET)
	UE_NO_EMPTY_FILE
#endif
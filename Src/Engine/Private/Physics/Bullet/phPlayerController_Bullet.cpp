#include "Base/ueBase.h"

#if defined(PH_BULLET)

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btMultiSphereShape.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "BulletCollision/CollisionDispatch/btCollisionWorld.h"
#include "LinearMath/btDefaultMotionState.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"

#include "Physics/Bullet/phLib_Bullet_Private.h"

phPlayerController*	phPlayerController_Create(phPlayerControllerDesc* desc)
{
	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(phBulletConverter::ToBulletVec3(desc->m_initialPos));

	btPairCachingGhostObject* ghostObject = new btPairCachingGhostObject();
	ghostObject->setUserPointer(desc->m_scene);
	ghostObject->setWorldTransform(startTransform);

	btConvexShape* capsule = new btCapsuleShape(desc->m_radius, desc->m_height);
	ghostObject->setCollisionShape(capsule);
	ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

	btKinematicCharacterController* kcc = PH_NEW_P3(s_data.m_allocator, btKinematicCharacterController, ghostObject, capsule, btScalar(desc->m_stepHeight));
	UE_ASSERT(kcc);

	desc->m_scene->m_world->addCollisionObject(ghostObject, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
	desc->m_scene->m_world->addCharacter(kcc);

	return (phPlayerController*) kcc;
}

void phPlayerController_Destroy(phPlayerController* pc)
{
	btKinematicCharacterController* kcc = (btKinematicCharacterController*) pc;

	btGhostObject* ghostObject = kcc->getGhostObject();
	btCollisionShape* shape = ghostObject->getCollisionShape();

	phScene* scene = (phScene*) ghostObject->getUserPointer();

	scene->m_world->removeCharacter(kcc);
	scene->m_world->removeCollisionObject(ghostObject);
	PH_DELETE(s_data.m_allocator, kcc, btKinematicCharacterController);
	delete shape;
	delete ghostObject;
}

void phPlayerController_Move(phPlayerController* pc, ueVec3 dir, f32 dt)
{
	btKinematicCharacterController* kcc = (btKinematicCharacterController*) pc;
	kcc->setWalkDirection(phBulletConverter::ToBulletVec3(dir) * dt);
}

void phPlayerController_SetPosition(phPlayerController* pc, const ueVec3& pos)
{
	btKinematicCharacterController* kcc = (btKinematicCharacterController*) pc;

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(phBulletConverter::ToBulletVec3(pos));

	kcc->getGhostObject()->setWorldTransform(transform);
}

void phPlayerController_GetPosition(phPlayerController* pc, ueVec3& pos)
{
	btKinematicCharacterController* kcc = (btKinematicCharacterController*) pc;
	const btTransform& transform = kcc->getGhostObject()->getWorldTransform();
	pos = phBulletConverter::FromBulletVec3(transform.getOrigin());
}

void phPlayerController_Jump(phPlayerController* pc, f32 v)
{
	btKinematicCharacterController* kcc = (btKinematicCharacterController*) pc;

	// Current bullet version doesn't support jumps
	kcc->jump();
}

void phPlayerController_UpdateAll()
{
	// Do nothing; controllers get updated during scene update
}

#else // defined(PH_BULLET)
	UE_NO_EMPTY_FILE
#endif
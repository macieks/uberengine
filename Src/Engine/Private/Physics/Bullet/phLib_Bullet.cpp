#include "Base/ueBase.h"

#if defined(PH_BULLET)

#include "Physics/Bullet/phLib_Bullet_Private.h"
#include "IO/ueBuffer.h"

phLibData s_data;

/*-------------- Bullet allocation functions ----------------*/

void* BulletAlignedAllocFunc(size_t size, int alignment)
{
	UE_ASSERT(s_data);
	return !size ? NULL : s_data.m_allocator->Alloc(size, alignment);
}

void BulletAlignedFreeFunc(void* memblock)
{
	UE_ASSERT(s_data);
	UE_ASSERT(memblock);
	return s_data.m_allocator->Free(memblock);
}

void* BulletAllocFunc(size_t size)
{
	UE_ASSERT(s_data);
	return !size ? NULL : s_data.m_allocator->Alloc(size);
}

void BulletFreeFunc(void* memblock)
{
	UE_ASSERT(s_data);
	return s_data.m_allocator->Free(memblock);
}

/*-------------- Bullet collision callback ----------------*/

void BulletNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo)
{
	btCollisionObject* colObj0 = (btCollisionObject*)collisionPair.m_pProxy0->m_clientObject;
	btCollisionObject* colObj1 = (btCollisionObject*)collisionPair.m_pProxy1->m_clientObject;

	if (dispatcher.needsCollision(colObj0,colObj1))
	{
		//dispatcher will keep algorithms persistent in the collision pair
		if (!collisionPair.m_algorithm)
		{
			collisionPair.m_algorithm = dispatcher.findAlgorithm(colObj0,colObj1);
		}

		if (collisionPair.m_algorithm)
		{
			btManifoldResult contactPointResult(colObj0,colObj1);

			if (dispatchInfo.m_dispatchFunc == btDispatcherInfo::DISPATCH_DISCRETE)
			{
				//discrete collision detection query
				collisionPair.m_algorithm->processCollision(colObj0,colObj1,dispatchInfo,&contactPointResult);
			}
			else
			{
				//continuous collision detection query, time of impact (toi)
				btScalar toi = collisionPair.m_algorithm->calculateTimeOfImpact(colObj0,colObj1,dispatchInfo,&contactPointResult);
				if (dispatchInfo.m_timeOfImpact > toi)
					dispatchInfo.m_timeOfImpact = toi;
			}
		}
	}
}

/*-------------- Bullet based Physics Engine ----------------*/

void phLib_Startup(phStartupParams* params)
{
	UE_ASSERT(!s_data.m_allocator);
	s_data.m_allocator = params->m_allocator;

	// Register custom memory allocator
	btAlignedAllocSetCustomAligned(BulletAlignedAllocFunc, BulletAlignedFreeFunc);
	btAlignedAllocSetCustom(BulletAllocFunc, BulletFreeFunc);

	// Collision configuration contains default setup for memory, collision setup
	s_data.m_collisionConfiguration = PH_NEW(params->m_allocator, btSoftBodyRigidBodyCollisionConfiguration);
	UE_ASSERT(s_data.m_collisionConfiguration);

	// Use the default collision dispatcher. For parallel processing you can use a different dispatcher (see Extras/BulletMultiThreaded)
	s_data.m_dispatcher = PH_NEW_P(params->m_allocator, btCollisionDispatcher, s_data.m_collisionConfiguration);
	UE_ASSERT(s_data.m_dispatcher);
	s_data.m_dispatcher->setNearCallback(BulletNearCallback);

	s_data.m_broadphase = PH_NEW(params->m_allocator, btDbvtBroadphase);
	//s_data.m_broadphase = PH_NEW_P2(params->m_allocator, btAxisSweep3, btVector3(-1000,-1000,-1000), btVector3(1000,1000,1000));
	UE_ASSERT(s_data.m_broadphase);

	// The default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	s_data.m_solver = PH_NEW(params->m_allocator, btSequentialImpulseConstraintSolver);
	UE_ASSERT(s_data.m_solver);

	s_data.m_ghostPairCallback = PH_NEW(s_data.m_allocator, btGhostPairCallback);
	s_data.m_broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(s_data.m_ghostPairCallback);
}

void phLib_Shutdown()
{
	UE_ASSERT(s_data);
	PH_DELETE(s_data.m_allocator, s_data.m_ghostPairCallback, btGhostPairCallback);
	PH_DELETE(s_data.m_allocator, s_data.m_solver, btConstraintSolver);
	PH_DELETE(s_data.m_allocator, s_data.m_broadphase, btBroadphaseInterface);
	PH_DELETE(s_data.m_allocator, s_data.m_dispatcher, btCollisionDispatcher);
	PH_DELETE(s_data.m_allocator, s_data.m_collisionConfiguration, btCollisionConfiguration);
	s_data.m_allocator = NULL;
}

u32 phLib_GetSymbol()
{
	return UE_BE_4BYTE_SYMBOL('b','l','l','t');
}

#else // defined(PH_BULLET)
	UE_NO_EMPTY_FILE
#endif
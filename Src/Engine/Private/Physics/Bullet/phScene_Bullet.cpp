#include "Base/ueBase.h"

#if defined(PH_BULLET)

#include "Physics/Bullet/phLib_Bullet_Private.h"

extern phDebugDraw* s_defaultDebugDraw;
static phBulletDebugDraw s_bulletDebugDraw;

phScene* phScene_Create(const phSceneDesc* desc)
{
	UE_ASSERT(s_data);

	// Create our scene

	phScene* scene = PH_NEW(s_data.m_allocator, phScene);
	UE_ASSERT(scene);
	scene->m_state = phScene::State_Idle;
	scene->m_desc = *desc;
	scene->m_world = NULL;
	scene->m_supportsSoftBodies = desc->m_supportSoftBodies;

	// Create bullet world

	if (desc->m_discreteWorld)
	{
		if (desc->m_supportSoftBodies)
		{
			btSoftRigidDynamicsWorld* world = PH_NEW_P4(s_data.m_allocator, btSoftRigidDynamicsWorld, s_data.m_dispatcher, s_data.m_broadphase, s_data.m_solver, s_data.m_collisionConfiguration);
			scene->m_world = world;
//			world->setDrawFlags(~0);

			scene->m_softBodyWorldInfo.air_density		=	(btScalar) 1.2f;
			scene->m_softBodyWorldInfo.water_density	=	0;
			scene->m_softBodyWorldInfo.water_offset		=	0;
			scene->m_softBodyWorldInfo.water_normal		=	btVector3(0, 0, 0);
			scene->m_softBodyWorldInfo.m_gravity.setValue(0, -9.81f, 0);
			scene->m_softBodyWorldInfo.m_sparsesdf.Initialize();
			scene->m_softBodyWorldInfo.m_broadphase = s_data.m_broadphase;
			scene->m_softBodyWorldInfo.m_dispatcher = s_data.m_dispatcher;
		}
		else
			scene->m_world = PH_NEW_P4(s_data.m_allocator, btDiscreteDynamicsWorld, s_data.m_dispatcher, s_data.m_broadphase, s_data.m_solver, s_data.m_collisionConfiguration);
	}
	else
	{
		UE_ASSERT(!desc->m_supportSoftBodies);
		scene->m_world = PH_NEW_P4(s_data.m_allocator, btContinuousDynamicsWorld, s_data.m_dispatcher, s_data.m_broadphase, s_data.m_solver, s_data.m_collisionConfiguration);
	}
	UE_ASSERT(scene->m_world);

	// Set default gravity

	scene->m_world->setGravity(btVector3(0, -9.81f, 0));

	return scene;
}

void phScene_Destroy(phScene* scene)
{
	UE_ASSERT(s_data);

	const u32 numRemainingObjects = scene->m_world->getNumCollisionObjects();
	UE_ASSERT_MSGP(numRemainingObjects == 0, "Not all bodies have been destroyed before destroying physics scene - this will cause memory leaks (%d remaining objects).", numRemainingObjects);
	PH_DELETE(s_data.m_allocator, scene->m_world, btDynamicsWorld);
	PH_DELETE(s_data.m_allocator, scene, phScene);
}

void phScene_BeginUpdate(phScene* scene, f32 deltaTime)
{
	scene->m_state = phScene::State_Update;
	scene->m_world->stepSimulation(deltaTime, 4);
	if (scene->m_supportsSoftBodies)
		scene->m_softBodyWorldInfo.m_sparsesdf.GarbageCollect();
	scene->m_state = phScene::State_Idle;
}

void phScene_EndUpdate(phScene* scene)
{

}

void phScene_ProcessCollisions(phScene* scene, phCollisionCallback callback)
{

}

class BulletCallback
{
protected:
	phShapeIntersectionResultSet* m_resultSet;
	const phIntersectionTestDesc* m_testDesc;
	f32 m_bulletHitFraction;

public:
	BulletCallback(const phIntersectionTestDesc* testDesc, phShapeIntersectionResultSet* resultSet) :
		m_resultSet(resultSet),
		m_testDesc(testDesc),
		m_bulletHitFraction(1.0f)
	{}

protected:
	phShapeIntersectionResult* FindSlot(f32 fraction)
	{
		if (!m_testDesc->m_findClosestIntersections)
		{
			if (m_resultSet->m_numResults == m_testDesc->m_maxNumResults)
				return NULL;

			phShapeIntersectionResult* result = &m_resultSet->m_results[m_resultSet->m_numResults++];
			result->m_fraction = fraction;
			if (m_resultSet->m_numResults == m_testDesc->m_maxNumResults)
				m_bulletHitFraction = 0.0f; // Don't accept any more results
			return result;
		}

		if (m_resultSet->m_numResults > 0)
		{
			if (m_resultSet->m_numResults < m_testDesc->m_maxNumResults)
			{
				phShapeIntersectionResult* result = &m_resultSet->m_results[m_resultSet->m_numResults++];
				result->m_fraction = fraction;
				return result;
			}

			if (m_bulletHitFraction > fraction)
			{
				u32 furthestIndex = 0;
				for (u32 i = 1; i < m_resultSet->m_numResults; i++)
					if (m_resultSet->m_results[i].m_fraction > m_resultSet->m_results[furthestIndex].m_fraction)
						furthestIndex = i;

				m_bulletHitFraction = ueMin(m_resultSet->m_results[furthestIndex].m_fraction, fraction);

				phShapeIntersectionResult* result = &m_resultSet->m_results[furthestIndex];
				result->m_fraction = fraction;
				return result;
			}
			return NULL;
		}

		m_bulletHitFraction = fraction;

		phShapeIntersectionResult* result = &m_resultSet->m_results[m_resultSet->m_numResults++];
		result->m_fraction = fraction;
		return result;
	}
};

class BulletRayCallback : public BulletCallback
{
public:
	BulletRayCallback(const phRayIntersectionTestDesc* testDesc, phShapeIntersectionResultSet* resultSet) :
		BulletCallback(testDesc, resultSet)
	{}

	~BulletRayCallback()
	{
		const phRayIntersectionTestDesc* rayTestDesc = static_cast<const phRayIntersectionTestDesc*>(m_testDesc);
		ueVec3 rayVec;
		ueVec3_SubV(rayVec, rayTestDesc->m_end, rayTestDesc->m_start);
		const f32 rayLength = ueVec3_Len(rayVec);
		for (u32 i = 0; i < m_resultSet->m_numResults; i++)
		{
			phShapeIntersectionResult* result = &m_resultSet->m_results[i];
			result->m_distance = rayLength * result->m_fraction;
			ueVec3_Mul(result->m_position, rayVec, result->m_fraction);
			ueVec3_AddIV(result->m_position, rayTestDesc->m_start);
		}
	}

protected:
	btScalar AddResult(
		btCollisionObject* object,
		u32 triangleIndex,
		const ueVec3& hitNormal,
		f32 hitFraction)
	{
		phShapeIntersectionResult* result = FindSlot(hitFraction);
		if (!result)
			return m_bulletHitFraction;

		result->m_body = object->getInternalType() == btCollisionObject::CO_RIGID_BODY ? (phBody*) object : NULL;
		result->m_normal = hitNormal;
		result->m_triangleIndex = triangleIndex;

		return m_bulletHitFraction;
	}
};

class BulletWorldRayCallback : public btCollisionWorld::RayResultCallback, public BulletRayCallback
{
public:
	BulletWorldRayCallback(const phRayIntersectionTestDesc* testDesc, phShapeIntersectionResultSet* resultSet) :
		BulletRayCallback(testDesc, resultSet)
	{}

	btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, ueBool normalInWorldSpace)
	{
		return AddResult(
			rayResult.m_collisionObject,
			rayResult.m_localShapeInfo ? rayResult.m_localShapeInfo->m_triangleIndex : U32_MAX,
			phBulletConverter::FromBulletVec3(normalInWorldSpace ? rayResult.m_hitNormalLocal : rayResult.m_collisionObject->getWorldTransform().getBasis() * rayResult.m_hitNormalLocal),
			rayResult.m_hitFraction);
	}
};

class BulletConvexCallback : public btCollisionWorld::ConvexResultCallback, public BulletCallback
{
public:
	BulletConvexCallback(const phShapeIntersectionTestDesc* testDesc, phShapeIntersectionResultSet* resultSet) :
		BulletCallback(testDesc, resultSet)
	{}

	~BulletConvexCallback()
	{
		const phShapeIntersectionTestDesc* shapeTestDesc = static_cast<const phShapeIntersectionTestDesc*>(m_testDesc);

		ueVec3 sweepStart, sweepEnd, sweepVec;
		ueMat44_GetTranslation(sweepStart, *shapeTestDesc->m_transformEnd);
		ueMat44_GetTranslation(sweepEnd, *shapeTestDesc->m_transformStart);
		ueVec3_SubV(sweepVec, sweepEnd, sweepStart);

		const f32 sweepLength = ueVec3_Len(sweepVec);
		for (u32 i = 0; i < m_resultSet->m_numResults; i++)
		{
			phShapeIntersectionResult* result = &m_resultSet->m_results[i];
			result->m_distance = sweepLength * result->m_fraction;
		}
	}

	btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, ueBool normalInWorldSpace)
	{
		phShapeIntersectionResult* result = FindSlot(convexResult.m_hitFraction);
		if (!result)
			return m_bulletHitFraction;

		btCollisionObject* object = convexResult.m_hitCollisionObject;

		result->m_body = object->getInternalType() == btCollisionObject::CO_RIGID_BODY ? (phBody*) object : NULL;
		result->m_position = phBulletConverter::FromBulletVec3(convexResult.m_hitPointLocal);
		result->m_normal = phBulletConverter::FromBulletVec3(normalInWorldSpace ? convexResult.m_hitNormalLocal : convexResult.m_hitCollisionObject->getWorldTransform().getBasis() * convexResult.m_hitNormalLocal);
		result->m_triangleIndex = convexResult.m_localShapeInfo ? convexResult.m_localShapeInfo->m_triangleIndex / 3 : U32_MAX;

		return m_bulletHitFraction;
	}
};

class BulletTriangleRayCallback : public btTriangleRaycastCallback, public BulletRayCallback
{
public:
	BulletTriangleRayCallback(const phRayIntersectionTestDesc* testDesc, phShapeIntersectionResultSet* resultSet) :
		btTriangleRaycastCallback(phBulletConverter::ToBulletVec3(testDesc->m_start), phBulletConverter::ToBulletVec3(testDesc->m_end)),
		BulletRayCallback(testDesc, resultSet)
	{}

	btScalar reportHit(const btVector3& hitNormalLocal, btScalar hitFraction, int partId, int triangleIndex)
	{
		return AddResult(
			NULL,
			triangleIndex,
			phBulletConverter::FromBulletVec3(hitNormalLocal),
			hitFraction);
	}
};

ueBool phScene_IntersectRay(phScene* scene, const phRayIntersectionTestDesc* testDesc, phShapeIntersectionResultSet* result)
{
	BulletWorldRayCallback resultCallback(testDesc, result);
	scene->m_world->rayTest(phBulletConverter::ToBulletVec3(testDesc->m_start), phBulletConverter::ToBulletVec3(testDesc->m_end), resultCallback);
	return result->m_numResults > 0;
}

ueBool phShape_IntersectRay(phShape* _shape, const phRayIntersectionTestDesc* testDesc, phShapeIntersectionResultSet* result)
{
	btCollisionShape* shape = (btCollisionShape*) _shape;
	BulletTriangleRayCallback resultCallback(testDesc, result);
	switch (shape->getShapeType())
	{
		case TRIANGLE_MESH_SHAPE_PROXYTYPE:
		{
			btBvhTriangleMeshShape* meshShape = (btBvhTriangleMeshShape*) shape;
			meshShape->performRaycast(&resultCallback, phBulletConverter::ToBulletVec3(testDesc->m_start), phBulletConverter::ToBulletVec3(testDesc->m_end));
			break;
		}
		default:
			UE_NOT_IMPLEMENTED();
			return UE_FALSE;
	}
	return result->m_numResults > 0;
}

ueBool phScene_IntersectShape(phScene* scene, const phShapeIntersectionTestDesc* testDesc, phShapeIntersectionResultSet* result)
{
	btCollisionShape* shape = (btCollisionShape*) testDesc->m_shape;
	switch (shape->getShapeType())
	{
		case CONVEX_SHAPE_PROXYTYPE:
		{
			// Set up sweep 'from' and 'to' matrices
			btTransform from, to;
			phBulletConverter::ToBulletTransform(*testDesc->m_transformStart, from);
			phBulletConverter::ToBulletTransform(*testDesc->m_transformEnd, to);

			// Perform the actual test
			BulletConvexCallback resultCallback(testDesc, result);
			scene->m_world->convexSweepTest((btConvexShape*) shape, from, to, resultCallback);
			return result->m_numResults > 0;
		}

		default:
			UE_ASSERT_MSGP(0, "Intersection with shape type not supported (type = %d).", (s32) shape->getShapeType());
			return UE_FALSE;
	}
}


#if 0
ueBool phCollisionMesh_IntersectRay(const phMeshDesc* meshDesc, const phRayIntersectionTestDesc* testDesc, phShapeIntersectionResultSet* result)
{
	UE_ASSERT(meshDesc->m_posStride == sizeof(f32) * 3);

	UE_ASSERT(meshDesc->m_indexSize == sizeof(u16));
	const u16* indices = (const u16*) meshDesc->m_indexData;

	BulletTriangleRayCallback callback(testDesc, result);
	for (u32 i = 0; i < meshDesc->m_numIndices; i += 3)
	{
		btVector3 pos[3];
		for (u32 j = 0; j < 3; j++)
		{
			const f32* xyz = meshDesc->m_pos + indices[i + j] * 3;
			pos[j].setValue(xyz[0], xyz[1], xyz[2]);
		}

		callback.processTriangle(pos, 0, i);
	}
	return result->m_numResults > 0;
}
#endif

void phScene_SetGravity(phScene* scene, const ueVec3& gravity)
{
	scene->m_world->setGravity( phBulletConverter::ToBulletVec3(gravity) );
}

void phScene_GetGravity(ueVec3& out, phScene* scene)
{
	out = phBulletConverter::FromBulletVec3(scene->m_world->getGravity() );
}

void phScene_DebugDraw(phScene* scene, phDebugDraw* debugDraw)
{
	s_bulletDebugDraw.m_debugDraw = debugDraw ? debugDraw : s_defaultDebugDraw;

#if 1
	scene->m_world->setDebugDrawer(&s_bulletDebugDraw);
	scene->m_world->debugDrawWorld();
	scene->m_world->setDebugDrawer(NULL);
#else
	phMeshDesc* d = g_mi->m_desc;
	for (u32 i = 0; i < d->m_numIndices; i += 3)
	{
		ueVec3 v[3];
		for (u32 j = 0; j < 3; j++)
		{
			const u32 indexIndex = i + j;
			const u32 index = (d->m_indexSize == 2) ? ((u16*) d->m_indexData)[indexIndex] : ((u32*) d->m_indexData)[indexIndex];

			v[j].Set(
				d->m_pos[index * 3],
				d->m_pos[index * 3 + 1],
				d->m_pos[index * 3 + 2]);
		}

		s_defaultDebugDraw->DrawLine(v[0], v[1], ueColor32::White);
		s_defaultDebugDraw->DrawLine(v[1], v[2], ueColor32::White);
		s_defaultDebugDraw->DrawLine(v[2], v[0], ueColor32::White);
	}
#endif
}

#else // defined(PH_BULLET)
	UE_NO_EMPTY_FILE
#endif
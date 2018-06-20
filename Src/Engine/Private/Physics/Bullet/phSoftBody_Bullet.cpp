#include "Base/ueBase.h"

#if defined(PH_BULLET)

#include "Physics/Bullet/phLib_Bullet_Private.h"
#include "Physics/phTetraBuilder.h"

phSoftBody*	phSoftBody_Create(phScene* scene, phSoftBodyDesc* desc)
{
	UE_ASSERT(scene->m_supportsSoftBodies);

	UE_ASSERT(((btCollisionShape*) desc->m_shape)->getShapeType() == SOFT_MESH_SHAPE_PROXYTYPE);
	phShape_SoftMesh* softMeshShape = (phShape_SoftMesh*) desc->m_shape;

	// Build body geometry

	btSoftBody*	body = PH_NEW_P4(s_data.m_allocator, btSoftBody, &scene->m_softBodyWorldInfo, softMeshShape->m_numVerts, softMeshShape->m_verts, NULL);
	UE_ASSERT(body);

	for (u32 i = 0; i < softMeshShape->m_numLinks; i++)
	{
		const phTetraLink& link = softMeshShape->m_links[i];
		body->appendLink(link.a, link.b);
	}

	const u32 numIndices = softMeshShape->m_numOuterFaces * 3;
	for (u32 i = 0; i < numIndices; i += 3)
	{
		const u16* faceIds = softMeshShape->m_outerFaces + i;
		body->appendFace(faceIds[0], faceIds[1], faceIds[2]);
//		body->appendLink(faceIds[0], faceIds[1]);
//		body->appendLink(faceIds[1], faceIds[2]);
//		body->appendLink(faceIds[2], faceIds[0]);
	}

	// Set up body parameters

	body->generateBendingConstraints(2);
	body->m_cfg.piterations = 2;
	body->m_cfg.collisions |= btSoftBody::fCollision::VF_SS;
	body->randomizeConstraints();
	body->setTotalMass(10, true);

	btTransform m;
	phBulletConverter::ToBulletTransform(*desc->m_transform, m);
	body->transform(m);

	// Add body to world

	body->setUserPointer2(scene);
	((btSoftRigidDynamicsWorld*) scene->m_world)->addSoftBody(body);

	return (phSoftBody*) body;
}

void phSoftBody_Destroy(phSoftBody* _body)
{
	btSoftBody* body = (btSoftBody*) _body;
	phScene* scene = (phScene*) body->getUserPointer2();
	((btSoftRigidDynamicsWorld*) scene->m_world)->removeSoftBody(body);
	PH_DELETE(s_data.m_allocator, body, btSoftBody);
}

void phSoftBody_ApplyImpulseAt(phSoftBody* _body, const ueVec3& pos)
{
	btSoftBody* body = (btSoftBody*) _body;
	//body->addForce();
	UE_NOT_IMPLEMENTED();
}

void phSoftBody_SetUserData(phSoftBody* body, void* userData) { ((btSoftBody*) body)->setUserPointer(userData); }
void* phSoftBody_GetUserData(phSoftBody* body) { return ((btSoftBody*) body)->getUserPointer(); }

#else // defined(PH_BULLET)
	UE_NO_EMPTY_FILE
#endif
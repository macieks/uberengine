#ifndef PH_LIB_BULLET_SHARED_H
#define PH_LIB_BULLET_SHARED_H

#include "btBulletDynamicsCommon.h"
#include "LinearMath/btIDebugDraw.h"

#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"

#include "Physics/phStructs_Shared.h"

struct phCollisionMeshInterface : public btStridingMeshInterface
{
	phMeshDesc* m_desc;

	// Implementation of the bullet's btStridingMeshInterface interface

	void getLockedVertexIndexBase(unsigned char **vertexbase, int& numverts, PHY_ScalarType& type, int& stride,unsigned char **indexbase,int & indexstride, int& numfaces, PHY_ScalarType& indicestype, int subpart = 0) {}
	void getLockedReadOnlyVertexIndexBase(const unsigned char **vertexbase, int& numverts, PHY_ScalarType& type, int& stride,const unsigned char **indexbase, int & indexstride, int& numfaces, PHY_ScalarType& indicestype, int subpart = 0) const
	{
		*vertexbase = (const unsigned char*) m_desc->m_pos;
		numverts = m_desc->m_numVerts;
		type = PHY_FLOAT;
		stride = m_desc->m_posStride;

		*indexbase = (const unsigned char*) m_desc->m_indexData;
		numfaces = m_desc->m_numIndices / 3;
		indexstride = m_desc->m_indexSize * 3;
		indicestype = m_desc->m_indexSize == 2 ? PHY_SHORT : PHY_INTEGER;
	}

	void unLockVertexBase(int subpart) {}
	void unLockReadOnlyVertexBase(int subpart) const {}

	int getNumSubParts() const { return 1; }

	void preallocateVertices(int numverts) {}
	void preallocateIndices(int numindices) {}
};

#endif // PH_LIB_BULLET_SHARED_H
#ifndef PH_LIB_BULLET_PRIVATE_H
#define PH_LIB_BULLET_PRIVATE_H

#include "Physics/Bullet/phLib_Bullet_Shared.h"
#include "Physics/phLib.h"
#include "Physics/Bullet/phLib_Bullet.h"

struct phTetraLink;

#define PH_NEW(allocator, type, ...) new((allocator)->Alloc(sizeof(type))) type()
#define PH_NEW_P(allocator, type, param0) new((allocator)->Alloc(sizeof(type))) type(param0)
#define PH_NEW_P2(allocator, type, param0, param1) new((allocator)->Alloc(sizeof(type))) type(param0, param1)
#define PH_NEW_P3(allocator, type, param0, param1, param2) new((allocator)->Alloc(sizeof(type))) type(param0, param1, param2)
#define PH_NEW_P4(allocator, type, param0, param1, param2, param3) new((allocator)->Alloc(sizeof(type))) type(param0, param1, param2, param3)
#define PH_DELETE(allocator, object, type) { object->~type(); (allocator)->Free(object); }

struct phLibData
{
	ueAllocator* m_allocator;

	btCollisionConfiguration* m_collisionConfiguration;
	btCollisionDispatcher* m_dispatcher;
	btBroadphaseInterface* m_broadphase;
	btConstraintSolver* m_solver;

	btGhostPairCallback* m_ghostPairCallback;

	phLibData() : m_allocator(NULL) {}
};

extern phLibData s_data;

struct phMeshShapeDesc_Serialized : phMeshShapeDesc
{
	u32 m_bvhSize;
	btOptimizedBvh* m_bvh;
};

struct phScene
{
	enum State
	{
		State_Idle = 0,
		State_Update
	};

	phSceneDesc m_desc;
	State m_state;

	btDynamicsWorld* m_world;

	ueBool m_supportsSoftBodies;
	btSoftBodyWorldInfo m_softBodyWorldInfo;
};

struct btCollisionShape_Adapter : public btCollisionShape
{
	virtual void				getAabb(const btTransform& t,btVector3& aabbMin,btVector3& aabbMax) const {}
	virtual void				setLocalScaling(const btVector3& scaling) {}
	virtual const btVector3&	getLocalScaling() const { static btVector3 s(1, 1, 1); return s; }
	virtual void				calculateLocalInertia(btScalar mass,btVector3& inertia) const {}
	virtual const char*			getName()const { return NULL; }
	virtual void				setMargin(btScalar margin) {}
	virtual btScalar			getMargin() const { return 0.0f; }
};

#define SOFT_MESH_SHAPE_PROXYTYPE 1001

struct phShape_SoftMesh : public btCollisionShape_Adapter
{
	u32 m_numVerts;
	btVector3* m_verts;
	u32 m_numLinks;
	phTetraLink* m_links;
	u32 m_numOuterFaces;
	u16* m_outerFaces;

	UE_INLINE phShape_SoftMesh() { m_shapeType = SOFT_MESH_SHAPE_PROXYTYPE; }
	BT_DECLARE_ALIGNED_ALLOCATOR();
};

struct phSoftBody
{
	//NxSoftBody* m_body;
	void* m_userData;

	u32 m_maxVerts;
	u32 m_numVerts;
	f32* m_pos;

	u32 m_maxIndices;
	u32 m_numIndices;
	u32* m_indices;
};

class phBulletConverter
{
public:
	static btVector3 ToBulletVec3(const ueVec3& v);
	static ueVec3 FromBulletVec3(const btVector3& v);

	static btQuaternion ToBulletQuaternion(const ueQuat& q);
	static ueQuat FromBulletQuaternion(const btQuaternion& q);

	static void ToBulletTransform(const ueMat44& src, btTransform& dst);
	static void FromBulletTransform(ueMat44& dst, const btTransform& src);
};

class phBulletDebugDraw : public btIDebugDraw
{
public:
	int m_debugMode;
	phDebugDraw* m_debugDraw;

	phBulletDebugDraw() : m_debugMode(DBG_DrawWireframe) {}

	// Implementation of the btIDebugDraw interface

	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
	void draw3dText(const btVector3& location, const char* textString);

	void reportErrorWarning(const char* warningString);

	void setDebugMode(int debugMode);
	int getDebugMode() const;
};

#endif // PH_LIB_BULLET_PRIVATE_H
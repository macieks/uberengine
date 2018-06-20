#ifndef PH_LIB_PHYSX_PRIVATE_H
#define PH_LIB_PHYSX_PRIVATE_H

#include "NxCharacter.h"
#include "NxCooking.h"
#include "NxCapsuleController.h"
#include "NxControllerManager.h"
#include "NxStream.h"
#include "NxPhysics.h"

#include "Physics/phLib.h"
#include "Physics/PhysX/phLib_PhysX.h"
#include "Physics/phJump.h"

// phLibData
// -------------------------

struct phLibData
{
	ueAllocator* m_allocator;

	NxPhysicsSDK* m_sdk;
	NxControllerManager* m_controllerManager;

	u8* m_cookingBuffer;
	u32 m_cookingBufferSize;

	ueBool m_supportDebugVisualization;

	phLibData() : m_allocator(NULL) {}
};

// phScene
// -------------------------

struct phScene
{
	NxScene* m_scene;
};

// phSoftBody
// -------------------------

struct phSoftBody
{
	NxSoftBody* m_body;
	void* m_userData;

	u32 m_maxVerts;
	u32 m_numVerts;
	f32* m_pos;

	u32 m_maxIndices;
	u32 m_numIndices;
	u32* m_indices;
};

// phPlayerController
// -------------------------

struct phPlayerController
{
	NxController* m_controller;
	phJump m_jump;
	void* m_userData;
};

// phShape
// -------------------------

struct phShape
{
	ueBool m_ownsMemory;
	phShapeDesc* m_desc;

	UE_INLINE phShape(phShapeDesc* desc) : m_ownsMemory(UE_TRUE), m_desc(desc) {}
};

struct phShape_Convex : phShape
{
	NxConvexMesh* m_convexMesh;

	UE_INLINE phShape_Convex(phConvexShapeDesc* desc) : phShape(desc) {}
};

struct phShape_Mesh : phShape
{
	NxTriangleMesh* m_triangleMesh;

	UE_INLINE phShape_Mesh(phMeshShapeDesc* desc) : phShape(desc) {}
};

struct phShape_SoftMesh : phShape
{
	NxSoftBodyMesh* m_softMesh;

	UE_INLINE phShape_SoftMesh(phSoftMeshShapeDesc* desc) : phShape(desc) {}
};

struct phShape_Compound : phShape
{
	phShape** m_shapes; // An array of phShape*; the size is determined by the m_desc (of type phCompoundShapeDesc*)

	UE_INLINE phShape_Compound(phCompoundShapeDesc* desc) : phShape(desc) {}
};

// phPhysXConverter
// -------------------------

class phPhysXConverter
{
public:
	UE_INLINE static void FromPhysXVec3(ueVec3& result, const NxVec3& v)
	{
		result.Set(&v.x);
	}

	UE_INLINE static void FromPhysXMat44(ueMat44& result, const NxMat34& m)
	{
		result.Set(
			m.M(0, 0), m.M(0, 1), m.M(0, 2), 0,
			m.M(1, 0), m.M(1, 1), m.M(1, 2), 0,
			m.M(2, 0), m.M(2, 1), m.M(2, 2), 0,
			m.t[0], m.t[1], m.t[2], 1);
	}

	UE_INLINE static void ToPhysXQuat(ueQuat& result, const NxQuat& q)
	{
		result.Set(q.x, q.y, q.z, q.w);
	}

	/*----------- Conversions to PhysX types ------------*/

	UE_INLINE static void ToPhysXVec3(NxVec3& dst, const ueVec3& src)
	{
		src.Store3(&dst.x);
	}

	UE_INLINE static void ToPhysXMat44(NxMat34& dst, const ueMat44& src)
	{
		dst.M = NxMat33(
			NxVec3(src(0, 0), src(1, 0), src(2, 0)),
			NxVec3(src(0, 1), src(1, 1), src(2, 1)),
			NxVec3(src(0, 2), src(1, 2), src(2, 2)));

		src.GetTranslation().Store3(&dst.t.x);
	}

	UE_INLINE static void ToPhysXQuat(NxQuat& dst, const ueQuat& src)
	{
		src.Store4(&dst.x);
	}
};

enum phPhysXCollisionGroup
{
	phPhysXCollisionGroup_NonCollidable = 0,
	phPhysXCollisionGroup_Static,
	phPhysXCollisionGroup_Dynamic,

	phPhysXCollisionGroup_MAX
};

#define phPhysXCollidableMask	(1 << phPhysXCollisionGroup_Dynamic) | (1 << phPhysXCollisionGroup_Static)

ueBool phShape_CookTriMesh(NxStream& out, phMeshDesc* desc);
ueBool phShape_CookConvexMesh(NxStream& out, phMeshDesc* desc);
ueBool phShape_CookSoftBodyMesh(NxStream& out, phSoftMeshDesc* desc);

extern phLibData* g_phLibData;

#endif // PH_LIB_PHYSX_PRIVATE_H
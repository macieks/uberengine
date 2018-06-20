#ifndef PH_STRUCTS_H
#define PH_STRUCTS_H

#include "Physics/phStructs_Shared.h"

/**
 *	@addtogroup ph
 *	@{
 */

struct phShapeDesc;
/**
 *	@struct phBody
 *	@brief Physics body in the scene
 */
struct phBody;
/**
 *	@struct phShape
 *	@brief Reusable physics shape
 */
struct phShape;
/**
 *	@struct phScene
 *	@brief Physics scene containing physics bodies
 */
struct phScene;
/**
 *	@struct phScene
 *	@brief Player controller object
 */
struct phPlayerController;
/**
 *	@struct phScene
 *	@brief Soft body constructed using links or tetras
 */
struct phSoftBody;

struct phStartupParams_Base
{
	ueAllocator* m_allocator;	//!< Allocator to be used for all physics memory allocations

	phStartupParams_Base() :
		m_allocator(NULL)
	{}
};

struct phSceneInit_Base
{
	phSceneInit_Base() {}
};

//! Base intersection description
struct phIntersectionTestDesc
{
	u32 m_collisionFlags;				//!< Collisions flags determining geometry to collide against

	u32 m_maxNumResults;				//!< Max. desired number of results
	ueBool m_findClosestIntersections;	//!< Whether to return closest results

	u32 m_numInclBodies;				//!< Number of bodies to only collide against; if set to 0 collide against all bodies
	phBody** m_inclBodies;				//!< An array of bodies to only collide against; if set to 0 collide against all bodies

	phIntersectionTestDesc() :
		m_collisionFlags(~0),
		m_maxNumResults(1),
		m_findClosestIntersections(UE_TRUE),
		m_numInclBodies(0),
		m_inclBodies(NULL)
	{}
};

//!	Description of the intersection test between shape and scene
struct phShapeIntersectionTestDesc : phIntersectionTestDesc
{
	phShape* m_shape;			//!< Shape to be intersected

	ueMat44* m_transformStart;	//!< Optional sweep start transformation (when not set identity is assumed)
	ueMat44* m_transformEnd;	//!< Optional sweep end transformation (if not set m_transformationStart is assumed)

	phShapeIntersectionTestDesc() :
		m_shape(NULL),
		m_transformStart(&ueMat44::Identity),
		m_transformEnd(&ueMat44::Identity)
	{}
};

//!	Description of the intersection test between ray and scene
struct phRayIntersectionTestDesc : phIntersectionTestDesc
{
	ueVec3 m_start;	//!< Ray start
	ueVec3 m_end;	//!< Ray end

	phRayIntersectionTestDesc() :
		m_start(ueVec3::Zero),
		m_end(ueVec3::Zero)
	{}
};

//!	Result of a single intersection.
struct phShapeIntersectionResult
{
	ueVec3 m_position;	//!< Intersection position
	ueVec3 m_normal;	//!< Normal vector on the body at intersection
	phBody* m_body;		//!< Pointer to body
	f32 m_distance;		//!< Distance at which intersection happened
	f32 m_fraction;		//!< Distance fraction at which intersection happened
	u32 m_triangleIndex;//!< Optional triangle info (valid if not U32_MAX)
};

//!	Set of intersection results.
struct phShapeIntersectionResultSet
{
	u32 m_numResults;					//!< Number of found results
	phShapeIntersectionResult* m_results;	//!< Set of found intersections

	phShapeIntersectionResultSet() : m_numResults(0), m_results(NULL) {}
};

//! Collision event.
struct phCollisionEventDesc
{
	ueVec3 m_position;		//!< Position at which collision happened
	ueVec3 m_velocity[2];	//!< Velocities of the 2 colliding bodies
	ueVec3 m_normals[2];	//!< Normal vectors at bodies 0 and 1 respectively while colliding
	phBody* m_body[2];		//!< Bodies involved in collision
};

//! Player controller description
struct phPlayerControllerDesc
{
	ueVec3 m_initialPos;	//!< Starting position (player controller has no orientation!)
	phScene* m_scene;		//!< Scene to add player controller to
	f32 m_height;			//!< Height of the player controller's capsule
	f32 m_radius;			//!< Radius of the player controller's capsule
	f32 m_stepHeight;		//!< Height of step
	f32 m_maxSlope;			//!< Max. walking slope angle

	phPlayerControllerDesc() :
		m_scene(NULL),
		m_height(1.0f),
		m_radius(0.25f),
		m_stepHeight(0.25f),
		m_maxSlope(UE_PI * 0.25f),
		m_initialPos(ueVec3::Zero)
	{}
};

// Shape descriptions

//! Base shape description
struct phShapeDesc
{
	phShapeType m_type;			//!< Shape type

	phShapeDesc(phShapeType type) : m_type(type) {}
};

//! Sphere shape description
struct phSphereShapeDesc : public phShapeDesc
{
	f32 m_radius;	//!< Sphere radius

	phSphereShapeDesc() : phShapeDesc(phShapeType_Sphere), m_radius(1) {}
};

//! Box shape description
struct phBoxShapeDesc : public phShapeDesc
{
	ueVec3 m_size;	//!< Box dimensions

	phBoxShapeDesc() :
		phShapeDesc(phShapeType_Box),
			m_size(ueVec3::Zero)
	{}
};

//! Cylinder shape description
struct phCylinderShapeDesc : public phShapeDesc
{
	f32 m_radius;		//!< Cylinder radius
	f32 m_height;		//!< Cylinder height

	phCylinderShapeDesc() : phShapeDesc(phShapeType_Cylinder), m_radius(1), m_height(1) {}
};

//! Capsule shape description
struct phCapsuleShapeDesc : public phShapeDesc
{
	f32 m_radius;		//!< Capsule radius
	f32 m_height;		//!< Capsule height

	phCapsuleShapeDesc() : phShapeDesc(phShapeType_Capsule), m_radius(1), m_height(1) {}
};

//! Convex shape description
struct phConvexShapeDesc : public phShapeDesc
{
	phMeshDesc* m_meshDesc;		//!< Convex mesh description
	ueVec3 m_scale;				//!< Scale (depending on physics engine may need to be uniform)s

	phConvexShapeDesc() :
		phShapeDesc(phShapeType_Convex),
		m_meshDesc(NULL),
		m_scale(ueVec3::One)
	{}
};

//! Mesh shape description
struct phMeshShapeDesc : public phShapeDesc
{
	phMeshDesc* m_meshDesc;		//!< Mesh description
	ueVec3 m_scale;				//!< Scale (depending on physics engine may need to be uniform)

	phMeshShapeDesc() :
		phShapeDesc(phShapeType_Mesh),
		m_meshDesc(NULL),
		m_scale(ueVec3::One)
	{}
};

//! Mesh shape description
struct phSoftMeshShapeDesc : public phShapeDesc
{
	phSoftMeshDesc* m_meshDesc;	//!< Mesh description
	ueVec3 m_scale;				//!< Scale (depending on physics engine may need to be uniform)

	phSoftMeshShapeDesc() :
		phShapeDesc(phShapeType_SoftMesh),
		m_meshDesc(NULL),
		m_scale(ueVec3::One)
	{}
};

//! Shape consisting of multiple (child) shapes
struct phCompoundShapeDesc : public phShapeDesc
{
	//! Child shape
	struct SubShapeDesc
	{
		ueVec3 m_translation;		//!< Translation wrt. parent
		ueVec3 m_scale;				//!< Scale wrt. parent
		ueQuat m_rotation;			//!< Rotation wrt. parent
		phShapeDesc* m_shapeDesc;	//!< Shape

		UE_INLINE SubShapeDesc() :
			m_shapeDesc(NULL),
			m_translation(ueVec3::Zero),
			m_scale(ueVec3::One),
			m_rotation(ueQuat::Identity)
		{}

		UE_INLINE ueBool HasIdentityLocalTransform() const
		{
			return
				m_translation == ueVec3::Zero &&
				m_scale == ueVec3::One &&
				m_rotation == ueQuat::Identity;
		}
	};

	u32 m_numShapes;			//!< Number of child shapes
	SubShapeDesc* m_shapes;		//!< An array of hild shapes

	phCompoundShapeDesc() :
		phShapeDesc(phShapeType_Compound),
		m_numShapes(0),
		m_shapes(NULL)
	{}
};

//! Body description
struct phBodyDesc
{
	f32 m_mass;				//!< Body mass; 0 indicates static object
	phShape* m_shape;		//!< Body shape
	const ueMat44* m_transform; //!< Body initial transformation

	phBodyDesc() :
		m_mass(0),
		m_shape(NULL),
		m_transform(&ueMat44::Identity)
	{}
};

//! Soft body description
struct phSoftBodyDesc
{
	phShape* m_shape;			//!< Soft mesh shape
	const ueMat44* m_transform; //!< Body initial transformation

	f32 m_volumeStiffness;		//!< Volume stiffness factor
	f32 m_stretchingStiffness;	//!< Volume stretching stiffness factor

	u32 m_maxVerts;				//!< Max. verts to be received for that body
	u32 m_maxIndices;			//!< Max. indices to be received for that body

	phSoftBodyDesc() :
		m_shape(NULL),
		m_transform(&ueMat44::Identity),
		m_volumeStiffness(1.0f),
		m_stretchingStiffness(1.0f),
		m_maxVerts(0),
		m_maxIndices(0)
	{}
};

// @}

#endif // PH_STRUCTS_H
#ifndef PH_STRUCTS_SHARED_H
#define PH_STRUCTS_SHARED_H

/**
 *	@addtogroup ph
 *	@{
 */

//! Available shape types
enum phShapeType
{
	phShapeType_Sphere = 0,		//!< Sphere
	phShapeType_Box,			//!< Box
	phShapeType_Cylinder,		//!< Vertical cylinder
	phShapeType_Capsule,		//!< Vertical capsule
	phShapeType_Convex,			//!< Convex mesh
	phShapeType_Mesh,			//!< Arbitrary (concave) mesh
	phShapeType_SoftMesh,		//!< Soft mesh
	phShapeType_Compound,		//!< Compound shape consistsing of multiple shapes

	phShapeType_MAX
};

//! Collision mesh description
struct phMeshDesc
{
	ueBool m_isConvex;		//!< Indicates whether the collision mesh is convex (used just as a hint)

	u32 m_numIndices;		//!< Number of indices
	const void* m_indexData;//!< Indices of the triangle list
	u32 m_indexSize;		//!< Index size in bytes: 2 (short) or 4 (integer)

	u32 m_numVerts;			//!< Number of vertices

	const f32* m_pos;		//!< 3-component positions
	u32 m_posStride;		//!< Stride of the position data

	const f32* m_normals;	//!< Optional 3-component normals
	u32 m_normalStride;		//!< Stride of the normal data

	phMeshDesc() :
		m_numIndices(0),
		m_indexData(NULL),
		m_numVerts(0),
		m_pos(NULL),
		m_posStride(0),
		m_normals(NULL),
		m_normalStride(0)
	{}
};

//! Soft mesh description
struct phSoftMeshDesc
{
	ueBool m_isTetra;		//!< Indicates whether description contains tetra-mesh (otherwise it's considered triangle mesh)

	u32 m_numIndices;		//!< Number of indices
	const void* m_indexData;//!< Indices (4 per tetra, 3 per triangle)
	u32 m_indexSize;		//!< Size of the single index in bytes (2 or 4)

	u32 m_numVerts;			//!< Number of vertices

	const f32* m_pos;		//!< Positions of the vertices
	u32 m_posStride;		//!< Stride of the float-32 XYZ position values

	// Tetra only

	const f32* m_mass;		//!< Masses of individual tetras
	u32 m_massStride;		//!< Stride of the float-32 mass values of tetras

	const u32* m_perVertexTearableFlags; //!< 1 bit per vertex; 1 - vertex is tearable; 0 - vertex isn't tearable

	// Tri-mesh only

	u32 m_subdivisionLevel;	//!< Triangle mesh subdivision level

	phSoftMeshDesc() :
		m_isTetra(UE_FALSE),
		m_numIndices(0),
		m_indexData(NULL),
		m_indexSize(0),
		m_numVerts(0),
		m_pos(NULL),
		m_posStride(0),
		m_mass(NULL),
		m_massStride(0),
		m_perVertexTearableFlags(NULL),
		m_subdivisionLevel(10)
	{}
};

// @}

#endif // PH_STRUCTS_SHARED_H
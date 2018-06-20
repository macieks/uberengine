#ifndef PH_TETRA_BUILDER_H
#define PH_TETRA_BUILDER_H

#include "Base/ueBase.h"

/**
 *	@addtogroup ph
 *	@{
 */

//! Tetra-mesh edge
struct phTetraLink
{
	u16 a;	//!< Vertex a index
	u16 b;	//!< Vertex b index
};

//! Tetra-mesh or tri-mesh description
struct phTetraMesh
{
	ueBool m_isTetra;		//!< Indicates whether this struct represents tetra mesh (if not, it's just tri-mesh)

	u32 m_numVerts;			//!< Number of verts
	f32* m_pos;				//!< Vertex positions (3 floats per vertex)

	u32 m_numIndices;		//!< Number of indices
	u32* m_indices;			//!< Indices (3 per trimesh or 4 per tetra-mesh)

	u32 m_numLinks;			//!< Number of links (tetra-mesh only)
	phTetraLink* m_links;	//!< Tetra mesh links

	u32 m_numOuterFaces;	//!< Number of outer faces
	u16* m_outerFaces;		//!< Outer faces

	phTetraMesh() :
		m_isTetra(UE_FALSE),
		m_numVerts(0),
		m_pos(NULL),
		m_numIndices(0),
		m_indices(NULL),
		m_numLinks(0),
		m_links(NULL),
		m_numOuterFaces(0),
		m_outerFaces(NULL)
	{}
};

//! Links between tri-mesh vertex and tetra-mesh
struct phTetraVertexLink
{
	u32 m_tetraIndex;			//!< Tetra index
	f32 m_barycentricCoords[3];	//!< Barycentric coordinates within given tetra
};

//! Starts up tetra mesh builder
void phTetraBuilder_Startup();
//! Shuts down tetra mesh builder
void phTetraBuilder_Shutdown();

//! Builds tetra-mesh from tri-mesh; when tetraMesh is not needed anymore it shall be released using phTetraBuilder_ReleaseMesh
ueBool phTetraBuilder_BuildTetraMesh(phTetraMesh& tetraMesh, const phTetraMesh& triMesh, u32 subdivisionLevel);
//! Releases memory used by mesh
void phTetraBuilder_ReleaseMesh(phTetraMesh& mesh);

//! Builds mapping between tri-mesh and tetra-mesh in a form of links (one for each tri-mesh vertex)
void phTetraBuilder_BuildTetraVertexLinks(phTetraVertexLink* links, const phTetraMesh& tetraMesh, const phTetraMesh& triMesh);

//! Builds tetra links (unique edges) for a given tetra-mesh
void phTetraBuilder_BuildTetraLinks(phTetraMesh& tetraMesh);
//! Builds set of outer tetra-mesh faces (faces that are outside the mesh)
void phTetraBuilder_BuildTetraOuterFaces(phTetraMesh& tetraMesh);

// @}

#endif // PH_TETRA_BUILDER_H
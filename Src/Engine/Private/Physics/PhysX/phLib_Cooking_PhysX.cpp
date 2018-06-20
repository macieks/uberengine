#include "Base/ueBase.h"

#if defined(UE_TOOLS) || defined(PH_PHYSX)

#include "Base/Containers/ueHashMap.h"
#include "IO/ueBuffer.h"
#include "Physics/phStructs_Shared.h"
#include "Physics/phTetraBuilder.h"

#include "NxCooking.h"
#include "NxCapsuleController.h"
#include "NxControllerManager.h"
#include "NxStream.h"
#include "NxPhysics.h"

ueBool phShape_CookTriMesh(NxStream& out, phMeshDesc* desc)
{
	NxTriangleMeshDesc meshDesc;
	meshDesc.numVertices				= desc->m_numVerts;
	meshDesc.numTriangles				= desc->m_numIndices / 3;
	meshDesc.pointStrideBytes			= desc->m_posStride;
	meshDesc.triangleStrideBytes		= 3 * desc->m_indexSize;
	meshDesc.points						= desc->m_pos;
	meshDesc.triangles					= desc->m_indexData;
	meshDesc.flags						= desc->m_indexSize == sizeof(u16) ? NX_MF_16_BIT_INDICES : 0;

	return NxCookTriangleMesh(meshDesc, out);
}

ueBool phShape_CookConvexMesh(NxStream& out, phMeshDesc* desc)
{
	NxConvexMeshDesc meshDesc;
	meshDesc.numVertices				= desc->m_numVerts;
	meshDesc.numTriangles				= desc->m_numIndices / 3;
	meshDesc.pointStrideBytes			= desc->m_posStride;
	meshDesc.triangleStrideBytes		= 3 * desc->m_indexSize;
	meshDesc.points						= desc->m_pos;
	meshDesc.triangles					= desc->m_indexData;
	meshDesc.flags						= desc->m_indexSize == sizeof(u16) ? NX_MF_16_BIT_INDICES : 0;

	return NxCookConvexMesh(meshDesc, out);
}

ueBool phShape_CookSoftBodyMesh(NxStream& out, phSoftMeshDesc* desc)
{
	NxSoftBodyMeshDesc nxDesc;
	phTetraMesh tetraMesh;

	// If we're given ready tetra mesh, then just set it up

	if (desc->m_isTetra)
	{
		nxDesc.numTetrahedra			= desc->m_numIndices / 4;
		nxDesc.tetrahedra				= desc->m_indexData;
		nxDesc.numVertices				= desc->m_numVerts;
		nxDesc.vertices					= const_cast<f32*>(desc->m_pos);
		nxDesc.flags					|= desc->m_indexSize == sizeof(u16) ? NX_SOFTBODY_MESH_16_BIT_INDICES : 0;
		nxDesc.tetrahedronStrideBytes	= 4 * desc->m_indexSize;
		nxDesc.vertexStrideBytes		= 3 * sizeof(f32);
	}

	// Otherwise generate tetra mesh (out of tri-mesh)

	else
	{
		phTetraMesh triMesh;
		triMesh.m_isTetra = UE_FALSE;
		UE_ASSERT(desc->m_indexSize == sizeof(u32));
		triMesh.m_indices = (u32*) desc->m_indexData;
		triMesh.m_numIndices = desc->m_numIndices;
		triMesh.m_numVerts = desc->m_numVerts;
		UE_ASSERT(desc->m_posStride == sizeof(f32) * 3);
		triMesh.m_pos = const_cast<f32*>(desc->m_pos);

		if (!phTetraBuilder_BuildTetraMesh(tetraMesh, triMesh, desc->m_subdivisionLevel))
		{
			ueLogE("Failed to build tetra mesh.");
			return UE_FALSE;
		}

		UE_ASSERT(tetraMesh.m_isTetra);

		nxDesc.numTetrahedra			= tetraMesh.m_numIndices / 4;
		nxDesc.tetrahedra				= tetraMesh.m_indices;
		nxDesc.numVertices				= tetraMesh.m_numVerts;
		nxDesc.vertices					= tetraMesh.m_pos;
		nxDesc.tetrahedronStrideBytes	= 4 * sizeof(tetraMesh.m_indices[0]);
		nxDesc.vertexStrideBytes		= 3 * sizeof(tetraMesh.m_pos[0]);
	}

	// Cook soft body mesh

	const bool result = NxCookSoftBodyMesh(nxDesc, out);

	// Release temp tetra mesh

	if (!desc->m_isTetra)
		phTetraBuilder_ReleaseMesh(tetraMesh);

	return result ? UE_TRUE : UE_FALSE;
}

#else // defined(UE_TOOLS) || defined(PH_PHYSX)
	UE_NO_EMPTY_FILE
#endif
#include "IO/ioPackageWriter.h"
#include "ModelCompiler_Common/uePhysicsCompiler.h"
#include "ModelCompiler_Common/ueToolCollisionGeometry.h"

ueToolShape& ueToolCollisionGeometry::GetAddShapeForBoneIndex(u32 boneIndex)
{
	for (u32 j = 0; j < m_shapes.size(); j++)
		if (m_shapes[j].m_nodeIndex == boneIndex)
			return m_shapes[j];
	ueToolShape& shape = vector_push(m_shapes);
	shape.m_nodeIndex = boneIndex;
	return shape;
}

void ueToolCollisionGeometry::Build(const ueToolModel::LOD* lod)
{
	m_shapes.clear();

	// Process meshes

	for (u32 i = 0; i < lod->m_meshes.size(); i++)
	{
		const ueToolModel::Mesh* mesh = &lod->m_meshes[i];
		if (!mesh->IsCollision())
			continue;

		const ueBool isConvex = (mesh->m_flags & ueToolModel::Mesh::Flags_IsConvex) != 0;

		// Get shape

		ueToolShape& shape = GetAddShapeForBoneIndex(mesh->m_nodeIndex);

		// Get mesh

		ueToolCollisionMesh* collMesh = NULL;
		if (isConvex)
		{
			collMesh = &vector_push(shape.m_meshes);
			collMesh->m_isConvex = true;
		}
		else
		{
			for (u32 j = 0; j < shape.m_meshes.size(); j++)
				if (!shape.m_meshes[j].m_isConvex)
				{
					collMesh = &shape.m_meshes[j];
					break;
				}
			if (!collMesh)
			{
				collMesh = &vector_push(shape.m_meshes);
				collMesh->m_isConvex = false;
			}
		}

		// Add source mesh's data to collision mesh

		const u32 startVertexIndex = (u32) collMesh->m_pos.size();
		for (u32 j = 0; j < mesh->m_verts.size(); j++)
			collMesh->m_pos.push_back(mesh->m_verts[j].m_pos);
		for (u32 j = 0; j < mesh->m_indices.size(); j++)
			collMesh->m_indices.push_back(mesh->m_indices[j] + startVertexIndex);
	}

	// Process primitives

	for (u32 i = 0; i < lod->m_prims.size(); i++)
	{
		const ueToolModel::Primitive& prim = lod->m_prims[i];
		if (prim.IsCollision())
			GetAddShapeForBoneIndex(prim.m_nodeIndex).m_prims.push_back(prim);
	}
}

void* s_physicsCompilerDLL = NULL;
uePhysicsCompilerFuncs s_physicsCompilerFuncs;

uePhysicsCompilerFuncs* uePhysicsCompiler_Load(const char* name)
{
	s_physicsCompilerDLL = dll_load(name);
    if (!s_physicsCompilerDLL)
		return NULL;

	s_physicsCompilerFuncs.m_writeCollisionGeometryFunc = (ueWriteCollisionGeometryFunc) dll_get_func(s_physicsCompilerDLL, "WriteCollisionGeometry");
	if (!s_physicsCompilerFuncs.m_writeCollisionGeometryFunc)
	{
		dll_unload(s_physicsCompilerDLL);
		return NULL;
	}

	return &s_physicsCompilerFuncs;
}

void uePhysicsCompiler_Unload()
{
	UE_ASSERT(s_physicsCompilerDLL);
	dll_unload(s_physicsCompilerDLL);
	s_physicsCompilerDLL = NULL;
}

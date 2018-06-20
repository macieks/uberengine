#pragma once

#include "ModelCompiler_Common/ueToolModel.h"

struct ueToolCollisionMesh
{
	ueMat44 m_localTransform;
	ueBool m_isConvex;
	std::vector<ueVec3> m_pos;
	std::vector<u32> m_indices;
};

struct ueToolShape
{
	u32 m_nodeIndex;
	ueMat44 m_localTransform;
	std::vector<ueToolCollisionMesh> m_meshes;
	std::vector<ueToolModel::Primitive> m_prims;
};

struct ueToolCollisionGeometry
{
	std::vector<ueToolShape> m_shapes;

	void Build(const ueToolModel::LOD* lod);
	ueToolShape& GetAddShapeForBoneIndex(u32 boneIndex);
};
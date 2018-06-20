#include "ModelCompiler_Common/ueToolModel.h"
#include "IO/ioFile.h"

// Assimp headers

#include "assimp.h"
#include "aiAssert.h"
#include "aiFileIO.h"
#include "aiPostProcess.h"
#include "aiScene.h"

static aiScene* s_scene = NULL;
static ueToolModel* s_dstModel = NULL;

void ProcessNode(u32 lodIndex, u32 parentIndex, aiNode* srcNode, std::map<u32, aiNode*>& meshIndexToNode)
{
	std::vector<ueToolModel::Node>& bones = s_dstModel->m_lods[lodIndex].m_nodes;

	const u32 index = (u32) bones.size();
	ueToolModel::Node& dstBone = vector_push(bones);

	dstBone.m_name = srcNode->mName.data;
	dstBone.m_parentIndex = parentIndex;

	dstBone.m_localTransform.Set((const f32*) &srcNode->mTransformation);
	dstBone.m_localTransform.Transpose();

	for (u32 i = 0; i < srcNode->mNumChildren; i++)
		ProcessNode(lodIndex, index, srcNode->mChildren[i], meshIndexToNode);

	for (u32 i = 0; i < srcNode->mNumMeshes; i++)
		meshIndexToNode[srcNode->mMeshes[i]] = srcNode;
}

ueToolModel::Mesh* ProcessMesh(aiMesh* srcMesh, u32 dstMaterialIndex, const ueMat44& nodeInvGlobalTransform)
{
	ueToolModel::LOD& dstLOD = s_dstModel->m_lods[0];

	// Determine vertex format

	u32 vertexFlags = 0;
	if (srcMesh->mNormals) vertexFlags |= ueToolModel::Vertex::Flags_HasNormal;
	if (srcMesh->mTangents) vertexFlags |= ueToolModel::Vertex::Flags_HasTangent;
	if (srcMesh->mBitangents) vertexFlags |= ueToolModel::Vertex::Flags_HasBinormal;
	for (u32 j = 0; j < ueToolModel::Vertex::MAX_TEX_STREAMS; j++)
		if (srcMesh->mTextureCoords[j] && (!srcMesh->mNumUVComponents[j] || srcMesh->mNumUVComponents[j] >= 2))
			vertexFlags |= (ueToolModel::Vertex::Flags_HasTex0 << j);

	// Create single mesh with single primitive

	ueToolModel::Mesh& dstMesh = vector_push(dstLOD.m_meshes);
	dstMesh.m_flags |= ueToolModel::Mesh::Flags_IsDrawable;
	dstMesh.m_materialIndex = dstMaterialIndex;

	// Process vertices

	for (u32 i = 0; i < srcMesh->mNumVertices; i++)
	{
		ueToolModel::Vertex& dstVertex = vector_push(dstMesh.m_verts);
		dstVertex.m_flags = vertexFlags;

		ueMemCpy(&dstVertex.m_pos, &srcMesh->mVertices[i], sizeof(f32) * 3);
		if (vertexFlags & ueToolModel::Vertex::Flags_HasNormal)
			ueMemCpy(&dstVertex.m_normal, &srcMesh->mNormals[i], sizeof(f32) * 3);
		if (vertexFlags & ueToolModel::Vertex::Flags_HasTangent)
			ueMemCpy(&dstVertex.m_tangent, &srcMesh->mTangents[i], sizeof(f32) * 3);
		if (vertexFlags & ueToolModel::Vertex::Flags_HasBinormal)
			ueMemCpy(&dstVertex.m_binormal, &srcMesh->mBitangents[i], sizeof(f32) * 3);
		for (u32 j = 0; j < ueToolModel::Vertex::MAX_TEX_STREAMS; j++)
			if (vertexFlags & (ueToolModel::Vertex::Flags_HasTex0 << j))
				ueMemCpy(&dstVertex.m_tex[j], &srcMesh->mTextureCoords[j][i], sizeof(f32) * 2);
	}

	for (u32 i = 0; i < srcMesh->mNumBones; i++)
	{
		aiBone* srcBone = srcMesh->mBones[i];
		UE_ASSERT(srcBone);

		const u32 dstBoneIndex = dstLOD.GetNodeIndex(srcBone->mName.data);
		UE_ASSERT(dstBoneIndex != U32_MAX);

		dstLOD.m_nodes[dstBoneIndex].m_invBindPose.Set((const f32*) &srcBone->mOffsetMatrix);
		dstLOD.m_nodes[dstBoneIndex].m_invBindPose.Transpose();
		dstLOD.m_nodes[dstBoneIndex].m_hasInvBindPose = true;

		for (u32 j = 0; j < srcBone->mNumWeights; j++)
		{
			const aiVertexWeight& srcVertexWeight = srcBone->mWeights[j];
			ueToolModel::Vertex& dstVertex = dstMesh.m_verts[srcVertexWeight.mVertexId];
			dstVertex.AddWeight(dstBoneIndex, srcVertexWeight.mWeight);
		}
	}

	// Transform verts from mesh space to global space

	for (u32 i = 0; i < srcMesh->mNumVertices; i++)
	{
		ueToolModel::Vertex& dstVertex = dstMesh.m_verts[i];
		if (dstVertex.m_numBones > 0)
			dstVertex.Transform(nodeInvGlobalTransform);
	}

	// Process indices

	for (u32 i = 0; i < srcMesh->mNumFaces; i++)
	{
		const aiFace& srcFace = srcMesh->mFaces[i];

		// Triangulate polygon (assuming convex polygon)

		const u32 numTris = srcFace.mNumIndices - 2;
		for (u32 j = 0; j < numTris; j++)
		{
			dstMesh.m_indices.push_back( srcFace.mIndices[j] );
			dstMesh.m_indices.push_back( srcFace.mIndices[j + 1] );
			dstMesh.m_indices.push_back( srcFace.mIndices[srcFace.mNumIndices - 1] );
		}
	}

	return &dstMesh;
}

void ProcessTexture(ueToolModel::Material& dstMaterial, ueToolModel::Material::SamplerType samplerType, aiMaterial* srcMaterial, aiTextureType srcTextureType)
{
	ueToolModel::Sampler& dstSampler = dstMaterial.m_samplers[samplerType];
	if (dstSampler.HasValidPath())
		return;

	aiTextureOp srcTextureOp;
	aiTextureMapping srcTextureMapping;
	aiTextureMapMode srcTextureMapMode[2];
	u32 srcUVIndex;
	f32 srcBlend;
	aiString srcPath;
	if (AI_SUCCESS != aiGetMaterialTexture(srcMaterial, srcTextureType, 0, &srcPath, &srcTextureMapping, &srcUVIndex, &srcBlend, &srcTextureOp, srcTextureMapMode))
		return;

	s_dstModel->DetermineSamplerTexture(srcPath.data, dstSampler);
}

u32 ProcessMaterial(aiMaterial* srcMaterial)
{
	ueToolModel::LOD& dstLOD = s_dstModel->m_lods[0];

	ueToolModel::Material dstMaterial;

	ProcessTexture(dstMaterial, ueToolModel::Material::SamplerType_Color, srcMaterial, aiTextureType_DIFFUSE);
	ProcessTexture(dstMaterial, ueToolModel::Material::SamplerType_Color, srcMaterial, aiTextureType_EMISSIVE);
	ProcessTexture(dstMaterial, ueToolModel::Material::SamplerType_Color, srcMaterial, aiTextureType_AMBIENT);
	ProcessTexture(dstMaterial, ueToolModel::Material::SamplerType_Normal, srcMaterial, aiTextureType_NORMALS);
	ProcessTexture(dstMaterial, ueToolModel::Material::SamplerType_Height, srcMaterial, aiTextureType_HEIGHT);
	ProcessTexture(dstMaterial, ueToolModel::Material::SamplerType_Specular, srcMaterial, aiTextureType_SPECULAR);

	aiString srcMaterialName;
	if (aiGetMaterialString(srcMaterial, AI_MATKEY_NAME, &srcMaterialName) == AI_SUCCESS)
		dstMaterial.m_name = srcMaterialName.data;

	int srcIsTwoSided;
	if (aiGetMaterialInteger(srcMaterial, AI_MATKEY_TWOSIDED, &srcIsTwoSided) == AI_SUCCESS)
		dstMaterial.m_flags |= ueToolModel::Material::Flags_TwoSided;

	aiColor4D color;
	if (aiGetMaterialColor(srcMaterial, AI_MATKEY_COLOR_AMBIENT, &color) == AI_SUCCESS)
		dstMaterial.m_ambient.Set(color.r, color.g, color.b);
	if (aiGetMaterialColor(srcMaterial, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS)
		dstMaterial.m_diffuse.Set(color.r, color.g, color.b);
	if (aiGetMaterialColor(srcMaterial, AI_MATKEY_COLOR_SPECULAR, &color) == AI_SUCCESS)
		dstMaterial.m_specular.Set(color.r, color.g, color.b);

	aiGetMaterialFloat(srcMaterial, AI_MATKEY_SHININESS, &dstMaterial.m_specularPower);
	aiGetMaterialFloat(srcMaterial, AI_MATKEY_OPACITY, &dstMaterial.m_opacity);

	return dstLOD.GetAddMaterial(dstMaterial);
}

void ConvertModel()
{
	ueLogI("Converting Assimp Scene to tool format...");

	// Add lod 0

	vector_push(s_dstModel->m_lods);

	// Process all nodes and build mapping from src mesh index to src node

	std::map<u32, aiNode*> meshIndexToNode;
	ProcessNode(0, U32_MAX, s_scene->mRootNode, meshIndexToNode);

	// Process all materials

	std::map<u32, u32> srcToDestMaterialIndex;
	for (u32 i = 0; i < s_scene->mNumMaterials; i++)
	{
		aiMaterial* material = s_scene->mMaterials[i];
		const u32 dstMaterialIndex = ProcessMaterial(material);
		if (dstMaterialIndex != U32_MAX)
			srcToDestMaterialIndex[i] = dstMaterialIndex;
	}

	// Process meshes

	std::vector<ueMat44> globalTransforms;
	s_dstModel->CalcGlobalTransforms(&s_dstModel->m_lods[0], globalTransforms);

	for (u32 i = 0; i < s_scene->mNumMeshes; i++)
	{
		aiMesh* srcMesh = s_scene->mMeshes[i];
		u32* dstMaterialIndex = map_find(srcToDestMaterialIndex, srcMesh->mMaterialIndex);

		// Skip meshes with no material

		if (!dstMaterialIndex)
			continue;

		// Get corresponding node

		aiNode** srcMeshOwnerNode = map_find(meshIndexToNode, i);

		// Determine global inversed node transform

		u32 boneIndex = U32_MAX;
		const char* nodeName = NULL;
		ueMat44 nodeInvGlobalTransform = ueMat44::Identity;

		if (srcMeshOwnerNode)
		{
			nodeName = (*srcMeshOwnerNode)->mName.data;
			boneIndex = s_dstModel->m_lods[0].GetNodeIndex(nodeName);
#if 0
			nodeInvGlobalTransform = globalTransforms[boneIndex];
			nodeInvGlobalTransform.Invert();
#endif
		}

		// Process mesh

		ueToolModel::Mesh* dstMesh = ProcessMesh(srcMesh, *dstMaterialIndex, nodeInvGlobalTransform);
		dstMesh->m_nodeIndex = boneIndex;
		dstMesh->m_name = nodeName;
	}
}

bool ImportFromFile(ueToolModel& model, const ueToolModel::ImportSettings& settings, const ueAssetParams& params)
{
	s_dstModel = &model;

	// Load Assimp scene

	ueLogI("Loading Assimp Scene from '%s'...", model.m_sourcePath.c_str());
	s_scene = (aiScene*) aiImportFile(
		model.m_sourcePath.c_str(),
//		aiProcess_CalcTangentSpace		| // calculate tangents and bitangents
//		aiProcess_JoinIdenticalVertices | // join identical vertices
		aiProcess_Triangulate			| // triangulate n-polygons
//		aiProcess_GenSmoothNormals		| // generate smooth normal vectors if not existing
		aiProcess_ConvertToLeftHanded	| // convert everything to D3D left handed space
		aiProcess_LimitBoneWeights		| // limit per-vertex influences to max 4
		aiProcess_ValidateDataStructure
//		aiProcess_ImproveCacheLocality |
//		aiProcess_RemoveRedundantMaterials// validate the output data structure
		);
	if (!s_scene)
	{
		ueLogE("Failed to load source model file (path = '%s').", model.m_sourcePath.c_str());
		return false;
	}

	// Convert

	ConvertModel();

	// Release Assimp scene

	aiReleaseImport(s_scene);
	s_scene = NULL;

	return true;
}

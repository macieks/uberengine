#include "ModelCompiler_Common/ueToolModel.h"
#include "IO/ioFile.h"

// Collada headers

#include "FCollada.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDSceneNode.h"
#include "FCDocument/FCDGeometry.h"
#include "FCDocument/FCDGeometrySource.h"
#include "FCDocument/FCDGeometryMesh.h"
#include "FCDocument/FCDGeometryPolygons.h"
#include "FCDocument/FCDGeometryPolygonsTools.h"
#include "FCDocument/FCDTexture.h"
#include "FCDocument/FCDMaterial.h"
#include "FCDocument/FCDMaterialInstance.h"
#include "FCDocument/FCDGeometryPolygonsInput.h"
#include "FCDocument/FCDEffectParameter.h"
#include "FCDocument/FCDEffectParameterSurface.h"
#include "FCDocument/FCDEffect.h"
#include "FCDocument/FCDEffectStandard.h"
#include "FCDocument/FCDEffectProfile.h"
#include "FCDocument/FCDEffectParameterSampler.h"
#include "FCDocument/FCDImage.h"
#include "FCDocument/FCDController.h"
#include "FCDocument/FCDControllerInstance.h"
#include "FCDocument/FCDSkinController.h"
#include "FCDocument/FCDocumentTools.h"
#include "FCDocument/FCDEffectTools.h"
#include "FCDocument/FCDLibrary.h"

// Types

struct ColladaMeshContext
{
	FCDSceneNode* m_node;
	FCDGeometryInstance* m_geometryInstance;
	FCDSkinController* m_controller;

	ColladaMeshContext() :
		m_node(NULL),
		m_geometryInstance(NULL),
		m_controller(NULL)
	{}
};

// Forward declarations

void ProcessColladaSceneNode_BuildBones(FCDSceneNode* sceneNode, u32 parentIndex);
void ProcessColladaSceneNode_BuildMeshes(FCDSceneNode* sceneNode);
void ProcessColladaSkinController(FCDSkinController* skinController);
void ProcessColladaMesh(FCDGeometryMesh* srcMesh, ColladaMeshContext& meshContext);
void ProcessColladaPolygonSet(const char* name, FCDGeometryMesh* srcMesh, u32 srcPolygonIndex, ColladaMeshContext& meshContext);
void ProcessColladaMaterial(ueToolModel::Material& dstMaterial, FCDGeometryPolygons* polySet, ColladaMeshContext& meshContext);
void ProcessColladaTextureBucket(FCDEffectStandard* effectStandard, u32 bucket, ueToolModel::Sampler& dstTexture);
void ProcessColladaImage(const char* srcPath, ueToolModel::Material& dstMaterial);

glTexAddr FromColladaTextureAddressMode(u32 mode);
glTexFilter FromColladaTextureFilterFunction(u32 function);

// Data

static FCDocument* s_document = NULL;
static ueToolModel* s_dstModel = NULL;
static ueToolModel::LOD* s_dstLOD = NULL;

// Implementation

void ConvertModel()
{
	// Create initial LOD

	s_dstLOD = &vector_push(s_dstModel->m_lods);

	// Extract bones from nodes

	FCDVisualSceneNodeLibrary* sceneLibrary = s_document->GetVisualSceneLibrary();
	ueLogI("Processing Collada nodes (%u)...", (u32) sceneLibrary->GetEntityCount());
	for (u32 i = 0; i < sceneLibrary->GetEntityCount(); i++)
		ProcessColladaSceneNode_BuildBones(sceneLibrary->GetEntity(i), U32_MAX);

	// Extract geometries from nodes

	ueLogI("Processing Collada meshes...");
	for (u32 i = 0; i < sceneLibrary->GetEntityCount(); i++)
		ProcessColladaSceneNode_BuildMeshes(sceneLibrary->GetEntity(i));

	// If no meshes found, process geometry library as if it was geometry instances

	if (s_dstLOD->m_meshes.size() == 0)
	{
		FCDGeometryLibrary* geometryLibrary = s_document->GetGeometryLibrary();
		ueLogI("Processing Collada geometry libraries (%u)...", geometryLibrary->GetEntityCount());
		for (u32 i = 0; i < geometryLibrary->GetEntityCount(); i++)
		{
			FCDGeometry* geometry = geometryLibrary->GetEntity(i);
			FCDGeometryMesh* srcMesh = geometry->GetMesh();
			if (srcMesh)
				ProcessColladaMesh(srcMesh, ColladaMeshContext());
		}
	}
}

void ProcessColladaSceneNode_BuildMeshes(FCDSceneNode* node)
{
	// Process geometries

	for (u32 i = 0; i < node->GetInstanceCount(); i++)
	{
		FCDEntityInstance* instance = node->GetInstance(i);
		FCDEntityInstance::Type instanceType = instance->GetType();

		ColladaMeshContext context;
		context.m_node = node;

		FCDGeometry* geometry = NULL;
		switch (instance->GetEntityType())
		{
			case FCDEntity::CONTROLLER:
			{
				FCDController* controller = (FCDController*) instance->GetEntity();

				geometry = controller->GetBaseGeometry();
				context.m_controller = controller->GetSkinController();
				context.m_geometryInstance = (FCDGeometryInstance*) instance;
				break;
			}
			case FCDEntity::GEOMETRY:
			{
				geometry = (FCDGeometry*) instance->GetEntity();
				break;
			}
			default:
				continue;
		}

		if (geometry)
		{
			FCDGeometryMesh* mesh = geometry->GetMesh();
			if (mesh)
				ProcessColladaMesh(mesh, context);
		}
	}

	// Process child nodes

	for (u32 i = 0; i < node->GetChildrenCount(); i++)
		ProcessColladaSceneNode_BuildMeshes(node->GetChild(i));
}

void ProcessColladaSceneNode_BuildBones(FCDSceneNode* srcNode, u32 parentIndex)
{
	const char* nodeName = srcNode->GetName().c_str();

	u32 dstNodeIndex = s_dstLOD->GetNodeIndex(nodeName);
	if (dstNodeIndex == U32_MAX)
	{
		dstNodeIndex = (u32) s_dstLOD->m_nodes.size();

		ueToolModel::Node& dstNode = vector_push(s_dstLOD->m_nodes);
		dstNode.m_name = nodeName;
		dstNode.m_parentIndex = parentIndex;
		dstNode.m_localTransform.Set((const f32*) srcNode->CalculateLocalTransform());
	}

	for (u32 i = 0; i < srcNode->GetChildrenCount(); i++)
		ProcessColladaSceneNode_BuildBones(srcNode->GetChild(i), dstNodeIndex);
}

void ProcessColladaMesh(FCDGeometryMesh* srcMesh, ColladaMeshContext& meshContext)
{
	// Extract mesh name

	const char* meshName = NULL;
	if (meshContext.m_geometryInstance)
		meshName = meshContext.m_geometryInstance->GetName().c_str();
	if ((!meshName || !meshName[0]) && meshContext.m_node)
		meshName = meshContext.m_node->GetName().c_str();

	// Process skin controller

	ueLogI("Processing Collada mesh (name: '%s' polys: %u)...", meshName, (u32) srcMesh->GetPolygonsCount());

	if (meshContext.m_controller)
		ProcessColladaSkinController(meshContext.m_controller);

	// Process all polygons

	for (u32 i = 0; i < srcMesh->GetPolygonsCount(); i++)
		ProcessColladaPolygonSet(meshName, srcMesh, i, meshContext);
}

void ProcessColladaSkinController(FCDSkinController* controller)
{
	// Reduce bone influences so can fit them in 4 vertex components

	const u32 maxInfluenceCount = ueMin((u32) 4, ueToolModel::Vertex::MAX_BONES_PER_VERTEX);
	const f32 minimumWeight = 0.01f;
	controller->ReduceInfluences(maxInfluenceCount, minimumWeight);

	// Grab info on all bones used

	for (u32 i = 0; i < controller->GetJointCount(); i++)
	{
		FCDSkinControllerJoint* joint = controller->GetJoint(i);
		const u32 boneIndex = s_dstLOD->GetNodeIndex(joint->GetId());
		UE_ASSERT(boneIndex != U32_MAX); // Node should have been created when traversed all scene nodes

		s_dstLOD->m_nodes[boneIndex].m_hasInvBindPose = true;
		s_dstLOD->m_nodes[boneIndex].m_invBindPose.Set((const f32*) joint->GetBindPoseInverse());
	}
}

struct UniqueVertex
{
	u32 m_posIndex;
	u32 m_normalIndex;
	u32 m_tangentIndex;
	u32 m_binormalIndex;
	u32 m_colorIndex;
	u32 m_texCoordIndex[ueToolModel::Vertex::MAX_TEX_STREAMS];

	u32 m_numBones;
	u32 m_boneIndices[ueToolModel::Vertex::MAX_TEX_STREAMS];
	f32 m_boneWeights[ueToolModel::Vertex::MAX_TEX_STREAMS];

	UniqueVertex()
	{
		m_posIndex = U32_MAX;
		m_normalIndex = U32_MAX;
		m_tangentIndex = U32_MAX;
		m_binormalIndex = U32_MAX;
		m_colorIndex = U32_MAX;
		for (u32 i = 0; i < ueToolModel::Vertex::MAX_TEX_STREAMS; i++)
			m_texCoordIndex[i] = U32_MAX;
		m_numBones = 0;
		for (u32 i = 0; i < ueToolModel::Vertex::MAX_BONES_PER_VERTEX; i++)
		{
			m_boneIndices[i] = U32_MAX;
			m_boneWeights[i] = 0.0f;
		}
	}
};

UE_INLINE bool operator < (const UniqueVertex& a, const UniqueVertex& b)
{
	return memcmp(&a, &b, sizeof(UniqueVertex)) < 0;
}

void ProcessColladaPolygonSet(const char* name, FCDGeometryMesh* srcMesh, u32 srcPolygonIndex, ColladaMeshContext& meshContext)
{
	// Turn each polygon set into separate mesh (they'll be merged later anyway)

	ueToolModel::Mesh& dstMesh = vector_push(s_dstLOD->m_meshes);
	dstMesh.m_flags |= ueToolModel::Mesh::Flags_IsDrawable;
	dstMesh.m_name = name;

	FCDGeometryPolygons* polySet = srcMesh->GetPolygons(srcPolygonIndex);
	FCDGeometryPolygonsTools::Triangulate(polySet);

	// If skinned, transform all verts into bind pose

	FMMatrix44 transform(FMMatrix44::Identity);
	bool isSkinned = false;

	if (meshContext.m_controller)
	{
		transform = meshContext.m_controller->GetBindShapeTransform();
		isSkinned = true;
	}

	// Determine relevant inputs

	FCDGeometryPolygonsInput* srcPosInput = polySet->FindInput(FUDaeGeometryInput::POSITION);
	if (!srcPosInput)
		return; // Skip polygon set if it doesn't have position stream
	FCDGeometryPolygonsInput* srcColorInput = polySet->FindInput(FUDaeGeometryInput::COLOR);
	FCDGeometryPolygonsInput* srcNormalInput = polySet->FindInput(FUDaeGeometryInput::NORMAL);
	FCDGeometryPolygonsInput* srcBinormalInput = polySet->FindInput(FUDaeGeometryInput::TEXBINORMAL);
	FCDGeometryPolygonsInput* srcTangentInput = polySet->FindInput(FUDaeGeometryInput::TEXTANGENT);

	FCDGeometryPolygonsInputList srcTexCoordInputList;
	polySet->FindInputs(FUDaeGeometryInput::TEXCOORD, srcTexCoordInputList);
	const u32 numTexInputs = ueMin((u32) ueToolModel::Vertex::MAX_TEX_STREAMS, (u32) srcTexCoordInputList.size());

	// Determine vertex format
	u32 vertexFlags = 0;
	if (srcColorInput) vertexFlags |= ueToolModel::Vertex::Flags_HasColor;
	if (srcNormalInput) vertexFlags |= ueToolModel::Vertex::Flags_HasNormal;
	if (srcTangentInput) vertexFlags |= ueToolModel::Vertex::Flags_HasTangent;
	if (srcBinormalInput) vertexFlags |= ueToolModel::Vertex::Flags_HasBinormal;
	if (numTexInputs >= 1) vertexFlags |= ueToolModel::Vertex::Flags_HasTex0;
	if (numTexInputs >= 2) vertexFlags |= ueToolModel::Vertex::Flags_HasTex1;
	if (numTexInputs >= 3) vertexFlags |= ueToolModel::Vertex::Flags_HasTex2;
	if (numTexInputs >= 4) vertexFlags |= ueToolModel::Vertex::Flags_HasTex3;

	// Build vertices and indices

	std::map<UniqueVertex, u32> uniqueVertexToIndex;
	for (u32 i = 0; i < polySet->GetFaceCount(); i++)
	{
		const u32 numFaceVerts = polySet->GetFaceVertexCounts()[i];
		if (numFaceVerts != 3)
			continue;

		for (u32 j = 0; j < 3; j++)
		{
			const u32 offset = (u32) polySet->GetFaceVertexOffset(i) + j;

			// Build unique vertex description

			UniqueVertex uniqueVertex;
			uniqueVertex.m_posIndex = srcPosInput->GetIndices()[offset];
			if (srcColorInput)
				uniqueVertex.m_colorIndex = srcColorInput->GetIndices()[offset];
			if (srcNormalInput)
				uniqueVertex.m_normalIndex = srcNormalInput->GetIndices()[offset];
			if (srcTangentInput)
				uniqueVertex.m_tangentIndex = srcTangentInput->GetIndices()[offset];
			if (srcBinormalInput)
				uniqueVertex.m_binormalIndex = srcBinormalInput->GetIndices()[offset];
			for (u32 k = 0; k < numTexInputs; k++)
				uniqueVertex.m_texCoordIndex[k] = srcTexCoordInputList[k]->GetIndices()[offset];

			if (isSkinned)
			{
				FCDSkinControllerVertex* skinControllerVertex = meshContext.m_controller->GetVertexInfluence(uniqueVertex.m_posIndex);
				UE_ASSERT( skinControllerVertex->GetPairCount() <= ueToolModel::Vertex::MAX_BONES_PER_VERTEX );
				uniqueVertex.m_numBones = (u32) skinControllerVertex->GetPairCount();
				for (u32 influence = 0; influence < skinControllerVertex->GetPairCount(); influence++)
				{
					FCDJointWeightPair* jointWeightPair = skinControllerVertex->GetPair(influence);
					FCDSkinControllerJoint* joint = meshContext.m_controller->GetJoint(jointWeightPair->jointIndex);

					uniqueVertex.m_boneIndices[influence] = s_dstLOD->GetNodeIndex(joint->GetId().c_str());
					UE_ASSERT_MSG(uniqueVertex.m_boneIndices[influence] != U32_MAX, "Node '%s' doesn't exist but is referenced by vertex.", joint->GetId().c_str());
					uniqueVertex.m_boneWeights[influence] = jointWeightPair->weight;
				}
			}

			// Determine if such vertex already exists - if so, reuse it; otherwise create new vertex

			u32* existingIndex = map_find(uniqueVertexToIndex, uniqueVertex);
			if (existingIndex)
				dstMesh.m_indices.push_back(*existingIndex);
			else
			{
				ueToolModel::Vertex dstVertex;
				dstVertex.m_flags = vertexFlags;

				const u32 posStartIndex = uniqueVertex.m_posIndex * srcPosInput->GetSource()->GetStride();
				const FMVector3 pos = transform.TransformCoordinate( FMVector3(
					srcPosInput->GetSource()->GetData()[posStartIndex],
					srcPosInput->GetSource()->GetData()[posStartIndex + 1],
					srcPosInput->GetSource()->GetData()[posStartIndex + 2]) );
				dstVertex.m_pos.Set(pos.x, pos.y, pos.z);

				if (srcNormalInput)
				{
					const u32 normalStartIndex = uniqueVertex.m_normalIndex * srcNormalInput->GetSource()->GetStride();
					const FMVector3 normal = transform.TransformVector( FMVector3(
						srcNormalInput->GetSource()->GetData()[normalStartIndex],
						srcNormalInput->GetSource()->GetData()[normalStartIndex + 1],
						srcNormalInput->GetSource()->GetData()[normalStartIndex + 2]) );
					dstVertex.m_normal.Set(&normal.x);
					dstVertex.m_normal.Normalize();
				}

				if (srcTangentInput)
				{
					const u32 tangentStartIndex = uniqueVertex.m_tangentIndex * srcTangentInput->GetSource()->GetStride();
					const FMVector3 tangent = transform.TransformVector( FMVector3(
						srcTangentInput->GetSource()->GetData()[tangentStartIndex],
						srcTangentInput->GetSource()->GetData()[tangentStartIndex + 1],
						srcTangentInput->GetSource()->GetData()[tangentStartIndex + 2]) );
					dstVertex.m_tangent.Set(&tangent.x);
					dstVertex.m_tangent.Normalize();
				}

				if (srcBinormalInput)
				{
					const u32 binormalStartIndex = uniqueVertex.m_binormalIndex * srcBinormalInput->GetSource()->GetStride();
					const FMVector3 binormal = transform.TransformVector( FMVector3(
						srcBinormalInput->GetSource()->GetData()[binormalStartIndex],
						srcBinormalInput->GetSource()->GetData()[binormalStartIndex + 1],
						srcBinormalInput->GetSource()->GetData()[binormalStartIndex + 2]) );
					dstVertex.m_binormal.Set(&binormal.x);
					dstVertex.m_binormal.Normalize();
				}

				if (srcColorInput)
				{
					const u32 colorStartIndex = uniqueVertex.m_colorIndex * srcColorInput->GetSource()->GetStride();
					dstVertex.m_color = ueColor32::F(
						srcColorInput->GetSource()->GetData()[colorStartIndex],
						srcColorInput->GetSource()->GetData()[colorStartIndex + 1],
						srcColorInput->GetSource()->GetData()[colorStartIndex + 2],
						1.0f);
				}

				for (u32 k = 0; k < numTexInputs; k++)
				{
					const u32 texCoordStartIndex = uniqueVertex.m_texCoordIndex[k] * srcTexCoordInputList[k]->GetSource()->GetStride();
					dstVertex.m_tex[k].Set(
						srcTexCoordInputList[k]->GetSource()->GetData()[texCoordStartIndex],
						1.0f - srcTexCoordInputList[k]->GetSource()->GetData()[texCoordStartIndex + 1]);
				}

				dstVertex.m_numBones = uniqueVertex.m_numBones;
				for (u32 k = 0; k < ueToolModel::Vertex::MAX_BONES_PER_VERTEX; k++)
				{
					dstVertex.m_boneWeights[k] = uniqueVertex.m_boneWeights[k];
					dstVertex.m_boneIndices[k] = uniqueVertex.m_boneIndices[k];
				}

				const u32 newVertexIndex = (u32) dstMesh.m_verts.size();
				dstMesh.m_verts.push_back(dstVertex);
				uniqueVertexToIndex[uniqueVertex] = newVertexIndex;

				dstMesh.m_indices.push_back(newVertexIndex);
			}
		}
	}

	// Determine the material

	ueToolModel::Material dstMaterial;
	ProcessColladaMaterial(dstMaterial, polySet, meshContext);

	dstMesh.m_materialIndex = s_dstLOD->GetAddMaterial(dstMaterial);
}

void ProcessColladaMaterial(ueToolModel::Material& dstMaterial, FCDGeometryPolygons* polySet, ColladaMeshContext& meshContext)
{
	dstMaterial.m_name = "<none>";

	if (meshContext.m_geometryInstance)
	{
		FCDMaterialInstance* matInstance = meshContext.m_geometryInstance->FindMaterialInstance( polySet->GetMaterialSemantic() );
		if (!matInstance)
			return;

		FCDMaterial* material = matInstance->GetMaterial();
		if (!material)
			return;

		dstMaterial.m_name = polySet->GetMaterialSemantic().c_str();

		FCDEffect* effect = material->GetEffect();
		if (!effect || effect->GetProfileCount() == 0)
			return;

		FCDEffectProfile* effectProfile = effect->GetProfile(0);

		// Attempt to retrieve as much as possible texture info using clean way

		if (effectProfile->GetObjectType() == FCDEffectStandard::GetClassType())
		{
			FCDEffectStandard* effectStandard = static_cast<FCDEffectStandard*>(effectProfile);
			ProcessColladaTextureBucket(effectStandard, FUDaeTextureChannel::DIFFUSE, dstMaterial.m_samplers[ueToolModel::Material::SamplerType_Color]);
			ProcessColladaTextureBucket(effectStandard, FUDaeTextureChannel::AMBIENT, dstMaterial.m_samplers[ueToolModel::Material::SamplerType_Color]);
			ProcessColladaTextureBucket(effectStandard, FUDaeTextureChannel::EMISSION, dstMaterial.m_samplers[ueToolModel::Material::SamplerType_Color]);
			ProcessColladaTextureBucket(effectStandard, FUDaeTextureChannel::BUMP, dstMaterial.m_samplers[ueToolModel::Material::SamplerType_Normal]);
			ProcessColladaTextureBucket(effectStandard, FUDaeTextureChannel::SPECULAR, dstMaterial.m_samplers[ueToolModel::Material::SamplerType_Specular]);
			ProcessColladaTextureBucket(effectStandard, FUDaeTextureChannel::DISPLACEMENT, dstMaterial.m_samplers[ueToolModel::Material::SamplerType_Height]);
		}

		// Process all effect and material params and attempt to determine remaining texture info

		std::vector<FCDEffectParameter*> effectParams;
		for (u32 i = 0; i < effectProfile->GetEffectParameterCount(); i++)
			effectParams.push_back( effectProfile->GetEffectParameter(i) );
		for (u32 i = 0; i < material->GetEffectParameterCount(); i++)
			effectParams.push_back( material->GetEffectParameter(i) );

		for (u32 i = 0; i < effectParams.size(); i++)
		{
			FCDEffectParameter* param = effectParams[i];
			if (param->GetType() != FCDEffectParameter::SURFACE)
				continue;

			FCDEffectParameterSurface* surface = (FCDEffectParameterSurface*) param;
			for (u32 j = 0; j < surface->GetImageCount(); j++)
			{
				FCDImage* image = surface->GetImage(j);
				const char* imageFileName = image->GetFilename().c_str();

				ProcessColladaImage(imageFileName, dstMaterial);
			}
		}
	}
}

void ProcessColladaTextureBucket(FCDEffectStandard* effectStandard, u32 bucket, ueToolModel::Sampler& dstSampler)
{
	// Skip if found before

	if (dstSampler.HasValidPath())
		return;

	// Skip if there's no source textures

	if (effectStandard->GetTextureCount(bucket) == 0)
		return;

	FCDTexture* texture = *effectStandard->GetTextureBucket(bucket);

	// Extract texture info

	FCDImage* image = texture->GetImage();
	const char* imageFileName = image->GetFilename().c_str();
	s_dstModel->DetermineSamplerTexture(imageFileName, dstSampler);

	// Extract sampler info

	if (texture->HasSampler())
	{
		FCDEffectParameterSampler* sampler = texture->GetSampler();

		dstSampler.m_desc.m_addressU = FromColladaTextureAddressMode(sampler->GetWrapS());
		dstSampler.m_desc.m_addressV = FromColladaTextureAddressMode(sampler->GetWrapT());
		dstSampler.m_desc.m_addressW = FromColladaTextureAddressMode(sampler->GetWrapP());

		dstSampler.m_desc.m_minFilter = FromColladaTextureFilterFunction(sampler->GetMinFilter());
		dstSampler.m_desc.m_magFilter = FromColladaTextureFilterFunction(sampler->GetMagFilter());
		dstSampler.m_desc.m_mipFilter = FromColladaTextureFilterFunction(sampler->GetMipFilter());
	}
}

glTexAddr FromColladaTextureAddressMode(u32 mode)
{
	switch (mode)
	{
		case FUDaeTextureWrapMode::WRAP: return glTexAddr_Wrap;
		case FUDaeTextureWrapMode::MIRROR: return glTexAddr_Mirror;
		case FUDaeTextureWrapMode::CLAMP: return glTexAddr_Clamp;
		case FUDaeTextureWrapMode::BORDER: return glTexAddr_ClampToBorder;
	}
	return glTexAddr_Wrap;
}

glTexFilter FromColladaTextureFilterFunction(u32 function)
{
	switch (function)
	{
		case FUDaeTextureFilterFunction::NEAREST: return glTexFilter_Nearest;
		case FUDaeTextureFilterFunction::LINEAR: return glTexFilter_Linear;
		case FUDaeTextureFilterFunction::NEAREST_MIPMAP_NEAREST: return glTexFilter_Nearest;
		case FUDaeTextureFilterFunction::LINEAR_MIPMAP_NEAREST: return glTexFilter_Linear;
		case FUDaeTextureFilterFunction::NEAREST_MIPMAP_LINEAR: return glTexFilter_Linear;
		case FUDaeTextureFilterFunction::LINEAR_MIPMAP_LINEAR: return glTexFilter_Linear;
	}
	return glTexFilter_Linear;
}

void ProcessColladaImage(const char* srcPath, ueToolModel::Material& dstMaterial)
{
	// Handle special modes

	if (ueStrStr(srcPath, "_2SIDED", UE_FALSE))
		dstMaterial.m_flags |= ueToolModel::Material::Flags_TwoSided;

	// Handle textures

	if (ueStrStr(srcPath, "NM_", UE_FALSE) ||
		ueStrStr(srcPath, "NORMAL", UE_FALSE) ||
		ueStrStr(srcPath, "BMP", UE_FALSE))
		if (!dstMaterial.m_samplers[ueToolModel::Material::SamplerType_Normal].HasValidPath() && s_dstModel->DetermineSamplerTexture(srcPath, dstMaterial.m_samplers[ueToolModel::Material::SamplerType_Normal]))
			return;

	if (ueStrStr(srcPath, "SPECMAP", UE_FALSE) ||
		ueStrStr(srcPath, "SPECULAR", UE_FALSE) ||
		ueStrStr(srcPath, "SPEC_", UE_FALSE) ||
		ueStrStr(srcPath, "_SPEC", UE_FALSE))
		if (!dstMaterial.m_samplers[ueToolModel::Material::SamplerType_Specular].HasValidPath() && s_dstModel->DetermineSamplerTexture(srcPath, dstMaterial.m_samplers[ueToolModel::Material::SamplerType_Specular]))
			return;

	if (!dstMaterial.m_samplers[ueToolModel::Material::SamplerType_Color].HasValidPath())
		s_dstModel->DetermineSamplerTexture(srcPath, dstMaterial.m_samplers[ueToolModel::Material::SamplerType_Color]);
}

// Main import function

bool ImportFromFile(ueToolModel& model, const ueToolModel::ImportSettings& settings, const ueAssetParams& params)
{
	s_dstModel = &model;

	// Load Collada scene

	FCollada::Initialize();
	s_document = FCollada::NewTopDocument();
	ueLogI("Loading Collada document from '%s'...", model.m_sourcePath.c_str());
	if (!FCollada::LoadDocumentFromFile(s_document, model.m_sourcePath.c_str()))
	{
		ueLogE("Failed to load source model file (path = '%s').", model.m_sourcePath.c_str());

		s_document->Release();
		s_document = NULL;

		FCollada::Release();
		return false;
	}

	// Make Y axis up axis

	FCDocumentTools::StandardizeUpAxisAndLength(s_document, FMVector3(0, 1, 0));

	// Convert

	ConvertModel();

	// Release Assimp scene

	s_document->Release();
	s_document = NULL;
	FCollada::Release();

	return true;
}

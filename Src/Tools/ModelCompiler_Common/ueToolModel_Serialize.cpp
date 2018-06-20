#include "ModelCompiler_Common/ueToolModel.h"
#include "Math/ueNumericalTypes.h"
#include "IO/ioPackageWriter.h"
#include "IO/ioPackageWriterUtils.h"
#include "GX/gxTexture_Shared.h"
#include "ModelCompiler_Common/uePhysicsCompiler.h"
#include "ModelCompiler_Common/ueToolCollisionGeometry.h"

#include <set>

static ueToolModel::ImportSettings* s_settings = NULL;

struct VertexFormat
{
	u32 m_stride;
	std::vector<glVertexElement> m_elements;

	UE_INLINE bool operator == (const VertexFormat& other) const
	{
		return m_stride == other.m_stride &&
			m_elements.size() == other.m_elements.size() &&
			!memcmp(&m_elements[0], &other.m_elements[0], m_elements.size() * sizeof(glVertexElement));
	}
};

struct MeshBones
{
	std::map<u32, u32> m_mapping; // Global to mesh node indices
	std::vector<u32> m_indices;
};

struct LODData
{
	// LOD

	std::vector<u8> m_VB;

	u32 m_indexSize;
	std::vector<u8> m_IB;

	// Vertex formats

	std::vector<VertexFormat> m_VFs;

	// Meshes

	std::vector<u32> m_VBOffsets;
	std::vector<u32> m_IBOffsets;
	std::vector<u32> m_VFIndex;
	std::vector<MeshBones> m_meshBones;

	// Collision geometry

	bool m_hasCollisionGeometry;
};

void WriteVertex(ueEndianess endianess, std::vector<u8>& data, const VertexFormat* vf, std::map<u32, u32>& meshNodeMapping, const ueToolModel::Vertex* v)
{
	u8 vertexData[1024];
	UE_ASSERT(vf->m_stride <= UE_ARRAY_SIZE(vertexData));

	for (u32 i = 0; i < vf->m_elements.size(); i++)
	{
		const glVertexElement elem = vf->m_elements[i];
		const ueBool isNormalized = elem.m_isNormalized ? UE_TRUE : UE_FALSE;

		switch (elem.m_semantic)
		{
			case glSemantic_Position:
				ueNumType_FromF32(3, (const f32*) v->m_pos, (ueNumType) elem.m_type, elem.m_count, isNormalized, &vertexData[elem.m_offset], endianess);
				break;
			case glSemantic_Normal:
			{
				const ueVec3 normal = v->m_normal * 0.5f + 0.5f;
				ueNumType_FromF32(3, (const f32*) normal, (ueNumType) elem.m_type, elem.m_count, isNormalized, &vertexData[elem.m_offset], endianess);
				break;
			}
			case glSemantic_Tangent:
			{
				const ueVec3 tangent = v->m_tangent * 0.5f + 0.5f;
				ueNumType_FromF32(3, (const f32*) tangent, (ueNumType) elem.m_type, elem.m_count, isNormalized, &vertexData[elem.m_offset], endianess);
				break;
			}
			case glSemantic_Binormal:
			{
				const ueVec3 binormal = v->m_binormal * 0.5f + 0.5f;
				ueNumType_FromF32(3, (const f32*) binormal, (ueNumType) elem.m_type, elem.m_count, isNormalized, &vertexData[elem.m_offset], endianess);
				break;
			}
			case glSemantic_TexCoord:
				ueNumType_FromF32(2, (const f32*) v->m_tex[elem.m_usageIndex], (ueNumType) elem.m_type, elem.m_count, isNormalized, &vertexData[elem.m_offset], endianess);
				break;
			case glSemantic_BoneWeights:
			{
				f32 boneWeights[ueToolModel::Vertex::MAX_BONES_PER_VERTEX] = {0.0f};
				for (u32 i = 0; i < v->m_numBones; i++)
					boneWeights[i] = v->m_boneWeights[i];
				ueNumType_FromF32(4, (const f32*) v->m_boneWeights, (ueNumType) elem.m_type, elem.m_count, isNormalized, &vertexData[elem.m_offset], endianess);
				break;
			}
			case glSemantic_BoneIndices:
			{
				// Remap indices

				s32 indices[ueToolModel::Vertex::MAX_BONES_PER_VERTEX] = {0};
				for (u32 j = 0; j < v->m_numBones; j++)
				{
					const u32* localIndex = map_find(meshNodeMapping, v->m_boneIndices[j]);
					UE_ASSERT(localIndex);
					indices[j] = *localIndex;
				}

				ueNumType_FromS32(4, indices, (ueNumType) elem.m_type, elem.m_count, isNormalized, &vertexData[elem.m_offset], endianess);
				break;
			}
		}
	}

	vector_u8_push_data(data, vertexData, vf->m_stride);
}

void AddVertexElement(VertexFormat& vf, glSemantic semantic, u32 usageIndex, ueNumType type, u32 count, ueBool isNormalized)
{
	glVertexElement& elem = vector_push(vf.m_elements);
	elem.m_semantic = (u8) semantic;
	elem.m_usageIndex = (u8) usageIndex;
	elem.m_type = (u8) type;
	elem.m_count = (u8) count;
	elem.m_isNormalized = isNormalized ? 1 : 0;
	elem.m_offset = (u8) vf.m_stride;

	vf.m_stride += ueNumType_GetTypeSize(type, count, isNormalized);
}

void BuildMeshBoneInfo(const ueToolModel::Mesh* mesh, MeshBones* bones)
{
	std::set<u32> uniqueIndices;
	for (u32 i = 0; i < mesh->m_verts.size(); i++)
	{
		const ueToolModel::Vertex& vertex = mesh->m_verts[i];
		for (u32 j = 0; j < vertex.m_numBones; j++)
			uniqueIndices.insert(vertex.m_boneIndices[j]);
	}

	if (uniqueIndices.size() > 0)
		for (std::set<u32>::iterator iter = uniqueIndices.begin(); iter != uniqueIndices.end(); iter++)
		{
			const u32 boneIndex = *iter;

			bones->m_mapping[boneIndex] = (u32) bones->m_indices.size();
			bones->m_indices.push_back(boneIndex);
		}
	else if (mesh->m_nodeIndex != U32_MAX)
		bones->m_indices.push_back(mesh->m_nodeIndex);
}

void BuildLODData(ueEndianess endianess, const ueToolModel::LOD* lod, LODData& data)
{
	// Build mesh node information

	data.m_meshBones.resize(lod->m_meshes.size());
	for (u32 i = 0; i < lod->m_meshes.size(); i++)
	{
		const ueToolModel::Mesh* mesh = &lod->m_meshes[i];
		if (mesh->IsDrawableTriMesh())
			BuildMeshBoneInfo(mesh, &data.m_meshBones[i]);
	}

	// Build vertex formats

	ueNumType primaryTexCoordType = ueNumType_F16;
	ueNumType secondaryTexCoordType = ueNumType_F16;
	ueNumType tangentSpaceVectorType = ueNumType_U8;
	if (s_settings->m_noF16)
		primaryTexCoordType = secondaryTexCoordType = ueNumType_F32;

	data.m_VFIndex.resize(lod->m_meshes.size());
	for (u32 i = 0; i < lod->m_meshes.size(); i++)
	{
		const ueToolModel::Mesh* mesh = &lod->m_meshes[i];
		if (!mesh->IsDrawableTriMesh())
			continue;

		// Determine mesh format

		u32 vertexFlags = mesh->m_verts[0].m_flags;
		ueBool hasBones = mesh->m_verts[0].m_numBones > 0;
		for (u32 j = 1; j < mesh->m_verts.size(); j++)
		{
			vertexFlags |= mesh->m_verts[j].m_flags;
			hasBones |= mesh->m_verts[j].m_numBones > 0;
		}

		// Build format description

		VertexFormat vf;
		vf.m_stride = 0;
		AddVertexElement(vf, glSemantic_Position, 0, ueNumType_F32, 3, UE_FALSE);
		if (vertexFlags & ueToolModel::Vertex::Flags_HasNormal)
			AddVertexElement(vf, glSemantic_Normal, 0, tangentSpaceVectorType, 4, UE_TRUE);
		if (vertexFlags & ueToolModel::Vertex::Flags_HasTangent)
			AddVertexElement(vf, glSemantic_Tangent, 0, tangentSpaceVectorType, 4, UE_TRUE);
		if (vertexFlags & ueToolModel::Vertex::Flags_HasBinormal)
			AddVertexElement(vf, glSemantic_Binormal, 0, tangentSpaceVectorType, 4, UE_TRUE);
		if (vertexFlags & ueToolModel::Vertex::Flags_HasTex0)
			AddVertexElement(vf, glSemantic_TexCoord, 0, primaryTexCoordType, 2, UE_FALSE);
		if (vertexFlags & ueToolModel::Vertex::Flags_HasTex1)
			AddVertexElement(vf, glSemantic_TexCoord, 1, secondaryTexCoordType, 2, UE_FALSE);
		if (vertexFlags & ueToolModel::Vertex::Flags_HasTex2)
			AddVertexElement(vf, glSemantic_TexCoord, 2, secondaryTexCoordType, 2, UE_FALSE);
		if (vertexFlags & ueToolModel::Vertex::Flags_HasTex3)
			AddVertexElement(vf, glSemantic_TexCoord, 3, secondaryTexCoordType, 2, UE_FALSE);
		if (hasBones)
		{
			AddVertexElement(vf, glSemantic_BoneIndices, 0, ueNumType_U8, 4, UE_FALSE);
			AddVertexElement(vf, glSemantic_BoneWeights, 0, ueNumType_U8, 4, UE_TRUE);
		}

		// Check if identical vertex format didn't exist (if not add new one)

		bool foundIdentical = false;
		for (u32 j = 0; j < data.m_VFs.size(); j++)
			if (data.m_VFs[j] == vf)
			{
				data.m_VFIndex[i] = j;
				foundIdentical = true;
				break;
			}
		if (!foundIdentical)
		{
			data.m_VFIndex[i] = (u32) data.m_VFs.size();
			data.m_VFs.push_back(vf);
		}
	}

	// See if we can use 16-bit indices

	data.m_indexSize = sizeof(u16);
	for (u32 i = 0; i < lod->m_meshes.size(); i++)
	{
		const ueToolModel::Mesh* mesh = &lod->m_meshes[i];
		if (!mesh->IsDrawableTriMesh())
			continue;

		if (mesh->m_verts.size() >= (1 << 16))
		{
			data.m_indexSize = sizeof(u32);
			break;
		}
	}

	// Write out verts & indices

	data.m_VBOffsets.resize(lod->m_meshes.size());
	data.m_IBOffsets.resize(lod->m_meshes.size());
	for (u32 i = 0; i < lod->m_meshes.size(); i++)
	{
		const ueToolModel::Mesh* mesh = &lod->m_meshes[i];
		if (!mesh->IsDrawableTriMesh())
			continue;

		const VertexFormat* vf = &data.m_VFs[data.m_VFIndex[i]];
		MeshBones* bones = &data.m_meshBones[i];

		// Vertices

		data.m_VBOffsets[i] = (u32) data.m_VB.size();

		for (u32 j = 0; j < mesh->m_verts.size(); j++)
			WriteVertex(endianess, data.m_VB, vf, bones->m_mapping, &mesh->m_verts[j]);

		// Indices

		data.m_IBOffsets[i] = (u32) data.m_IB.size() / data.m_indexSize;

		for (u32 j = 0; j < mesh->m_indices.size(); j++)
			if (data.m_indexSize == sizeof(u16))
			{
				u16 index = mesh->m_indices[j];
				index = ueToEndianess(index, endianess);
				vector_u8_push_data(data.m_IB, &index, sizeof(index));
			}
			else
			{
				u32 index = mesh->m_indices[j];
				index = ueToEndianess(index, endianess);
				vector_u8_push_data(data.m_IB, &index, sizeof(index));
			}
	}

	// Collision geometry

	data.m_hasCollisionGeometry = lod->m_prims.size() > 0;

	if (!data.m_hasCollisionGeometry)
		for (u32 i = 0; i < lod->m_meshes.size(); i++)
			if (lod->m_meshes[i].IsCollision())
			{
				data.m_hasCollisionGeometry = true;
				break;
			}
}

bool ueToolModel::Serialize(ioPackageWriter* pw, ImportSettings& settings)
{
	ueLogI("Serializing...");

	s_settings = &settings;

	// Preprocess all LODs

	std::vector<LODData> lodDatas(m_lods.size());
	for (u32 i = 0; i < m_lods.size(); i++)
	{
		const LOD* lod = &m_lods[i];
		BuildLODData(pw->GetConfig().m_endianess, lod, lodDatas[i]);

		if (lodDatas[i].m_hasCollisionGeometry && !settings.m_physicsCompilerFuncs)
		{
			ueLogE("Can't export physics, reason: no physics compiler specified for the model.");
			return false;
		}
	}

	// Write out 3 data segments:
	// (1) persistent C++ data
	// (2) hardware buffer data (index and vertex buffers; requires special memory types and alignments on some platforms)
	// (3) initialization only data - unloaded immediately after initializing the model

	ioSegmentParams segmentParams;
	segmentParams.m_symbol = UE_BE_4BYTE_SYMBOL('m','o','d','l');

	ioSegmentWriter sw;
	pw->BeginSegment(sw, segmentParams);

	// Write out all LODs

	std::vector<ioPtr> drawMeshesPtrs;
	std::vector<ioPtr> collisionGeomsPtrs(m_lods.size());
	std::vector<ioPtr> materialsPtrs;
	std::vector<ioPtr> vertexFormatsPtrs;
	std::vector<ioPtr> skeletonPtrs(m_lods.size());

	for (u32 i = 0; i < m_lods.size(); i++)
	{
		const LOD* lod = &m_lods[i];
		const LODData* lodData = &lodDatas[i];

		// gxModel::LOD

		sw.WriteNumber<f32>(1.0f);						// LOD value

		sw.WriteNumber<u32>((u32) lod->m_meshes.size());// u32 m_numMeshes
		drawMeshesPtrs.push_back(sw.WritePtr());		// Mesh* m_meshes

		if (lodData->m_hasCollisionGeometry)
			collisionGeomsPtrs[i] = sw.WritePtr();		// gxCollisionGeometry* m_collisionGeometry
		else
			sw.WriteNullPtr();

		sw.WriteNumber<u32>((u32) lod->m_materials.size());	// u32 m_numMaterials
		materialsPtrs.push_back(sw.WritePtr());			// Material* m_materials

		if (lod->m_nodes.size() > 0)
			skeletonPtrs[i] = sw.WritePtr();			// gxSkeleton* m_skeleton;
		else
			sw.WriteNullPtr();

		sw.WriteNumber<u32>((u32) lodData->m_VFs.size());// u32 m_numVertexFormats
		vertexFormatsPtrs.push_back(sw.WritePtr());		// glStreamFormat** m_vertexFormats

		sw.WriteNullPtr();								// glVertexBuffer* m_VB
		sw.WriteNullPtr();								// glIndexBuffer* m_IB

		// FIXME: X360 & PS3 only
		//sw.WriteNullPtr();							// void* m_hardwareBuffersData
	}

	for (u32 i = 0; i < m_lods.size(); i++)
	{
		const LOD* lod = &m_lods[i];
		const LODData* lodData = &lodDatas[i];

		// Drawable triangle meshes

		std::vector<ioPtr> bonePtrs(lod->m_meshes.size());

		sw.BeginPtr(drawMeshesPtrs[i]);
		for (u32 j = 0; j < lod->m_meshes.size(); j++)
		{
			const Mesh* mesh = &lod->m_meshes[j];
			if (!mesh->IsDrawableTriMesh())
				continue;

			// TODO: Optimize by creating separate 'geometry' meshes where possible (i.e. except for transparent meshes)
			const u32 meshFlags = 0;//gxModelMeshFlags_Color | gxModelMeshFlags_Geometry;

			// gxModel::Mesh

			sw.WriteNullPtr();								// const char* m_name
			sw.WriteNumber<u32>(meshFlags);					// u32 m_meshFlags

			WriteBox(&sw, mesh->m_box);						// ueBox m_box

			sw.WriteNumber<u32>(glPrimitive_TriangleList);	// glPrimType m_primType

			sw.WriteNumber<u32>(lodData->m_VFIndex[j]);		// u32 m_vertexFormatIndex
			sw.WriteNumber<u32>(mesh->m_materialIndex);		// u32 m_materialIndex

			sw.WriteNumber<u32>(lodData->m_VBOffsets[j]);	// u32 m_vertexBufferOffset
			sw.WriteNumber<u32>(0);							// u32 m_baseVertex
			sw.WriteNumber<u32>(0);							// u32 m_firstVertex
			sw.WriteNumber<u32>((u32) mesh->m_verts.size());// u32 m_numVerts

			sw.WriteNumber<u32>(lodData->m_IBOffsets[j]);	// u32 m_firstIndex
			sw.WriteNumber<u32>((u32) mesh->m_indices.size()); // u32 m_numIndices

			sw.WriteNumber<u32>((u32) lodData->m_meshBones[j].m_indices.size()); // u32 m_numNodes
			bonePtrs[j] = sw.WritePtr();					// u32* m_boneIndices

			sw.WriteAlignVec();
		}

		// Per mesh node indices

		for (u32 j = 0; j < lod->m_meshes.size(); j++)
		{
			const Mesh* mesh = &lod->m_meshes[j];
			if (!mesh->IsDrawableTriMesh())
				continue;

			sw.BeginPtr(bonePtrs[j]);

			const MeshBones* bones = &lodData->m_meshBones[j];
			for (u32 k = 0; k < bones->m_indices.size(); k++)
				sw.WriteNumber<u32>(bones->m_indices[k]);
		}

		// Materials

		sw.BeginPtr(materialsPtrs[i]);
		for (u32 j = 0; j < lod->m_materials.size(); j++)
		{
			const Material* material = &lod->m_materials[j];

			// gxModel::MaterialInfo

			sw.WriteResourceHandle(gxTexture_TYPE_ID, material->m_samplers[Material::SamplerType_Color].m_textureName.c_str()); // ueResourceHandle<gxTexture> m_colorMap
			sw.WriteNumber<f32>(1.0f);		// f32 m_mipLevelsUsage
		}

		// Vertex format ptrs (with actual formats created at runtime)

		sw.BeginPtr(vertexFormatsPtrs[i]);
		for (u32 j = 0; j < lodData->m_VFs.size(); j++)
			sw.WriteNullPtr();

		// Skeletons

		if (lod->m_nodes.size() > 0)
		{
			sw.BeginPtr(skeletonPtrs[i]);

			// gxSkeleton

			sw.WriteNumber<u32>((u32) lod->m_nodes.size());	// u32 m_numNodes
			ioPtr bonesPtr = sw.WritePtr();					// gxSkeleton::Node* m_nodes
			WriteEmptyList(&sw);							// ueList<AnimationBinding> m_bindings

			// Bones

			std::vector<ioPtr> boneNamePtrs;

			sw.WriteAlignVec();
			sw.BeginPtr(bonesPtr);
			for (u32 j = 0; j < lod->m_nodes.size(); j++)
			{
				const Node& node = lod->m_nodes[j];

				// gxSkeleton::Node

				boneNamePtrs.push_back(sw.WritePtr());		// const char* m_name
				sw.WriteNumber<s32>(node.m_parentIndex);	// u32 m_parentNodeIndex
				WriteMat44(&sw, node.m_localTransform);		// ueMat44 m_localTransform
				WriteMat44(&sw, node.m_invBindPose);		// ueMat44 m_invBindPose
			}

			// Node names

			for (u32 j = 0; j < lod->m_nodes.size(); j++)
			{
				const Node& node = lod->m_nodes[j];

				sw.BeginPtr(boneNamePtrs[j]);
				sw.WriteData(node.m_name.c_str(), node.m_name.length() + 1);
			}
		}
	}

	// Write out remaining data

	ioSegmentWriter initSw;
	pw->BeginSegment(initSw, segmentParams);

	// gxModel_InitData

	WriteBox(&initSw, m_box);						// ueBox m_box
	initSw.WriteNumber<u32>((u32) m_lods.size());	// u32 m_numLODs
	ioPtr initLODsPtr = initSw.WritePtr();			// gxModel_InitData::LOD* m_LODs

	// Array of gxModel_InitData::LOD

	std::vector<ioPtr> formatDescPtrs(m_lods.size());
	std::vector<ioPtr> collisionGeomInitPtrs(m_lods.size());

	u32 hardwareBufferOffset = 0;

	initSw.BeginPtr(initLODsPtr);
	for (u32 i = 0; i < m_lods.size(); i++)
	{
		const LOD* lod = &m_lods[i];
		const LODData* lodData = &lodDatas[i];

		// gxModel_InitData::LOD

		formatDescPtrs[i] = initSw.WritePtr();			// glStreamFormatDesc* m_formatDescs

		ioSegmentWriter buffersSw;
		pw->BeginSegment(buffersSw, segmentParams);

		// gxModel_InitData::VertexBufferDesc
		{
			initSw.WriteNumber<u32>(hardwareBufferOffset);	// u32 m_readOffset
			initSw.WriteNumber<u32>((u32) lodData->m_VB.size()); // u32 m_size
			buffersSw.WriteData(&lodData->m_VB[0], lodData->m_VB.size());
			hardwareBufferOffset += (u32) lodData->m_VB.size();
		}

		// gxModel_InitData::IndexBufferDesc
		{
			initSw.WriteNumber<u32>(hardwareBufferOffset);	// u32 m_readOffset
			initSw.WriteNumber<u32>(lodData->m_indexSize);	// u32 m_indexSize
			initSw.WriteNumber<u32>((u32) lodData->m_IB.size() / lodData->m_indexSize); // u32 m_numIndices
			buffersSw.WriteData(&lodData->m_IB[0], lodData->m_IB.size());
			hardwareBufferOffset += (u32) lodData->m_IB.size();
		}

		buffersSw.EndSegment();

		// Collision geometry

		if (lodData->m_hasCollisionGeometry)
			collisionGeomInitPtrs[i] = initSw.WritePtr();		// gxCollisionGeometry_InitData* m_collisionGeomInitData
		else
			initSw.WriteNullPtr();
	}

	// Write collision geometry

	for (u32 i = 0; i < m_lods.size(); i++)
	{
		const LOD* lod = &m_lods[i];
		const LODData* lodData = &lodDatas[i];

		if (!lodData->m_hasCollisionGeometry)
			continue;

		ueLogI("Serializing collision geometry using '%s' physics compiler...", settings.m_physicsCompilerName.c_str());

		ueToolCollisionGeometry collGeom;
		collGeom.Build(lod);

		sw.BeginPtr(collisionGeomsPtrs[i]);
		initSw.BeginPtr(collisionGeomInitPtrs[i]);

		settings.m_physicsCompilerFuncs->m_writeCollisionGeometryFunc(&sw, &initSw, &collGeom);
	}

	// Write vertex format descs

	for (u32 i = 0; i < m_lods.size(); i++)
	{
		const LODData* lodData = &lodDatas[i];

		// Array of glStreamFormatDesc

		std::vector<ioPtr> vertexFormatElementPtrs;

		initSw.BeginPtr(formatDescPtrs[i]);
		for (u32 j = 0; j < lodData->m_VFs.size(); j++)
		{
			const VertexFormat* vf = &lodData->m_VFs[j];

			// glStreamFormatDesc

			initSw.WriteNumber<u32>(vf->m_stride);					// u32 m_stride
			initSw.WriteNumber<u32>((u32) vf->m_elements.size());	// u32 m_numElements
			vertexFormatElementPtrs.push_back(initSw.WritePtr());	// const glVertexElement* m_elements
		}

		// Array of vertex element arrays

		for (u32 j = 0; j < lodData->m_VFs.size(); j++)
		{
			const VertexFormat* vf = &lodData->m_VFs[j];

			initSw.BeginPtr(vertexFormatElementPtrs[j]);
			for (u32 k = 0; k < vf->m_elements.size(); k++)
			{
				const glVertexElement elem = vf->m_elements[k];

				// glVertexElement

				initSw.WriteData(&elem, sizeof(elem)); // Can write without endian correctness because it only contains bytes
			}
		}
	}

	return true;
}

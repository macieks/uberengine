#include "ModelCompiler_Common/ueToolModel.h"
#include "IO/ioFile.h"

u32 ueToolModel::VERSION = 1;

ueToolModel::ueToolModel()
{
	Reset();
}

void ueToolModel::Reset()
{
	m_version = VERSION;
	m_lods.clear();
}

// Saving

bool ueToolModel::Save(const char* path)
{
/*	OutFileStream fs;
	if (!fs.Open(path)) return false;
	m_version = s_currentVersion;
	fs << m_version << m_lods;
	return fs.IsError();
*/
	return false;
}

// Loading

bool ueToolModel::Load(const char* path)
{
/*
	InFileStream fs;
	if (!fs.Open(path)) return false;
	fs << m_version;
	if (m_version != s_currentVersion) return false;
	fs << m_lods;
	return fs.IsError();
*/
	return false;
}

// Vertex

void ueToolModel::Vertex::AddWeight(u32 boneIndex, f32 weight)
{
	u32 weightIndex = 0;

	if (m_numBones < MAX_BONES_PER_VERTEX)
		weightIndex = m_numBones++;
	else
	{
		u32 smallestWeightIndex = 0;
		for (u32 i = 1; i < m_numBones; i++)
			if (m_boneWeights[smallestWeightIndex] > m_boneWeights[i])
				smallestWeightIndex = i;
		if (m_boneWeights[smallestWeightIndex] >= weight)
			return;
	}

	m_boneIndices[weightIndex] = boneIndex;
	m_boneWeights[weightIndex] = weight;
}

// Model

ueToolModel* ueToolModel::Merge(u32 numModels, ueToolModel** models)
{
	ueToolModel* dst = new ueToolModel();

	// Models

	for (u32 i = 0; i < numModels; i++)
	{
		ueToolModel* src = models[i];

		// LODs

		for (u32 j = 0; j < src->m_lods.size(); j++)
		{
			LOD* srcLOD = &src->m_lods[j];

			while (dst->m_lods.size() < src->m_lods.size())
				vector_push(dst->m_lods);
			LOD* dstLOD = &dst->m_lods[ dst->m_lods.size() - 1 ];

			const u32 boneOffset = (u32) dstLOD->m_nodes.size();
			const u32 materialOffset = (u32) dstLOD->m_materials.size();

			// Bones

			for (u32 k = 0; k < srcLOD->m_nodes.size(); k++)
			{
				dstLOD->m_nodes.push_back(srcLOD->m_nodes[k]);
				Node* dstBone = &dstLOD->m_nodes[ dstLOD->m_nodes.size() - 1 ];
				dstBone->m_parentIndex += boneOffset;
			}

			// Meshes

			for (u32 k = 0; k < srcLOD->m_meshes.size(); k++)
			{
				dstLOD->m_meshes.push_back(srcLOD->m_meshes[k]);
				Mesh* dstMesh = &dstLOD->m_meshes[ dstLOD->m_meshes.size() - 1 ];
				dstMesh->m_materialIndex += materialOffset;
				dstMesh->m_nodeIndex += boneOffset;

				// Per-vertex bone indices

				for (u32 l = 0; l < dstMesh->m_verts.size(); l++)
				{
					Vertex& v = dstMesh->m_verts[l];
					for (u32 m = 0; m < v.m_numBones; m++)
						v.m_boneIndices[m] += boneOffset;
				}
			}

			// Materials

			for (u32 k = 0; k < srcLOD->m_materials.size(); k++)
				dstLOD->m_materials.push_back(srcLOD->m_materials[k]);

			// Primitives

			for (u32 k = 0; k < srcLOD->m_prims.size(); k++)
			{
				dstLOD->m_prims.push_back(srcLOD->m_prims[k]);
				Primitive* dstPrim = &dstLOD->m_prims[ dstLOD->m_prims.size() - 1 ];
				dstPrim->m_nodeIndex += boneOffset;
			}
		}
	}

	return dst;
}

u32 ueToolModel::LOD::GetNodeIndex(const char* name) const
{
	for (u32 i = 0; i < m_nodes.size(); i++)
		if (m_nodes[i].m_name == name)
			return (s32) i;
	return U32_MAX;
}

u32 ueToolModel::LOD::GetMaterialIndex(const char* name) const
{
	for (u32 i = 0; i < m_nodes.size(); i++)
		if (m_materials[i].m_name == name)
			return (s32) i;
	return U32_MAX;
}

u32 ueToolModel::LOD::GetAddMaterial(ueToolModel::Material& mat)
{
	for (u32 i = 0; i < m_materials.size(); i++)
		if (m_materials[i] == mat)
			return i;

	// Add new one (make sure its name is unique)

	u32 counter = 0;
	std::string uniqueNameCandidate = mat.m_name;
	while (map_find(m_materialToIndexMap, uniqueNameCandidate))
		string_format(uniqueNameCandidate, "%s_%d", mat.m_name.c_str(), counter++);
	mat.m_name = uniqueNameCandidate;

	const u32 materialIndex = (u32) m_materials.size();
	m_materials.push_back(mat);
	m_materialToIndexMap[mat.m_name] = materialIndex;
	return materialIndex;
}

void ueToolModel::CalculateBox()
{
	m_box.Reset();
	FOR_EACH_MESH(mesh)
	{
		mesh->m_box.Reset();
		for (u32 k = 0; k < mesh->m_verts.size(); k++)
		{
			const Vertex* v = &mesh->m_verts[k];
			mesh->m_box.Extend(v->m_pos);
		}
		m_box.Extend(mesh->m_box);
	}
}

void ueToolModel::Vertex::Transform(const ueMat44& t)
{
	t.TransformCoord(m_pos);
	if (m_flags & ueToolModel::Vertex::Flags_HasNormal)
	{
		t.TransformNormal(m_normal);
		m_normal.Normalize();
	}
	if (m_flags & ueToolModel::Vertex::Flags_HasBinormal)
	{
		t.TransformNormal(m_binormal);
		m_binormal.Normalize();
	}
	if (m_flags & ueToolModel::Vertex::Flags_HasTangent)
	{
		t.TransformNormal(m_tangent);
		m_tangent.Normalize();
	}
}

void ueToolModel::CalcGlobalTransforms(const LOD* lod, std::vector<ueMat44>& globalTransforms)
{
	globalTransforms.resize(lod->m_nodes.size());
	for (u32 i = 0; i < lod->m_nodes.size(); i++)
	{
		const u32 parentIndex = lod->m_nodes[i].m_parentIndex;
		ueMat44::Mul(globalTransforms[i],
			lod->m_nodes[i].m_localTransform,
			parentIndex == U32_MAX ? ueMat44::Identity : globalTransforms[parentIndex]);
	}
}

void ueToolModel::RemoveBindPose()
{
	UE_NOT_IMPLEMENTED();

	ueLogI("Removing bind pose from vertices...");

#if 0
	for (u32 i = 0; i < m_lods.size(); i++)
	{
		LOD* lod = &m_lods[i];
		if (lod->m_nodes.size() == 0)
			continue;

		// Transform all verts by skinned inv bind pose

		for (u32 j = 0; j < lod->m_meshes.size(); j++)
		{
			Mesh* mesh = &lod->m_meshes[j];

			for (u32 k = 0; k < mesh->m_verts.size(); k++)
			{
				Vertex& v = mesh->m_verts[k];

				if (v.m_numNodes == 1)
				{
					v.Transform( lod->m_nodes[ v.m_boneIndices[0] ].m_invBindPose );
					continue;
				}
				else if (v.m_numNodes > 1)
				{
					ueMat44 skinnedInvBindPose;
					memset(&skinnedInvBindPose, 0, sizeof(skinnedInvBindPose));
					for (u32 b = 0; b < v.m_numNodes; b++)
					{
						ueMat44 temp = bindPose[ v.m_boneIndices[b] ];
						temp *= v.m_boneWeights[b];
						skinnedInvBindPose += temp;
					}
					skinnedInvBindPose.Invert();

					v.Transform(skinnedInvBindPose);
				}
			}
		}
	}
#endif
}

void ueToolModel::MakeStatic(ueBool preserveUnusedBones)
{
	ueLogI("Making static (preserve unused bones = %s)...", preserveUnusedBones ? "YES" : "NO");

	for (u32 lodIndex = 0; lodIndex < m_lods.size(); lodIndex++)
	{
		LOD* lod = &m_lods[lodIndex];
		if (lod->m_nodes.size() == 0)
			continue;

		std::vector<ueMat44> globalTransforms;
		CalcGlobalTransforms(lod, globalTransforms);

		// Transform all verts to model space

		for (u32 i = 0; i < lod->m_meshes.size(); i++)
		{
			ueToolModel::Mesh* mesh = &lod->m_meshes[i];
			const ueMat44* meshTransform = mesh->m_nodeIndex != U32_MAX ? &globalTransforms[mesh->m_nodeIndex] : NULL;

			for (u32 j = 0; j < mesh->m_verts.size(); j++)
			{
				ueToolModel::Vertex& v = mesh->m_verts[j];
#if 0
				if (v.m_numNodes == 1)
					v.Transform(bindPose[v.m_boneIndices[0]]);
				else if (v.m_numNodes > 1)
				{
					ueMat44 skinnedBindPose;
					memset(&skinnedBindPose, 0, sizeof(ueMat44));
					for (u32 b = 0; b < v.m_numNodes; b++)
					{
						ueMat44 temp = bindPose[ v.m_boneIndices[b] ];
						temp *= v.m_boneWeights[b];
						skinnedBindPose += temp;
					}

					v.Transform(skinnedBindPose);
				}
				else
#endif
				if (meshTransform)
					v.Transform(*meshTransform);

				v.m_numBones = 0;
			}

			mesh->m_nodeIndex = U32_MAX;
		}

		// Remove bones - they're not needed anymore

		if (!preserveUnusedBones)
			lod->m_nodes.clear();
	}
}

void ueToolModel::AddUVStreams(u32 vertexFlags)
{
	ueLogI("Adding UV streams (vertex flags = %u) ...", vertexFlags);

	FOR_EACH_MESH(mesh)
		for (u32 j = 0; j < mesh->m_verts.size(); j++)
		{
			ueToolModel::Vertex& v = mesh->m_verts[j];

			for (u32 k = 0; k < ueToolModel::Vertex::MAX_TEX_STREAMS; k++)
			{
				const u32 texMask = (ueToolModel::Vertex::Flags_HasTex0 << k);
				if ((vertexFlags & texMask) && !(v.m_flags & texMask))
				{
					v.m_flags |= texMask;
					v.m_tex[k].Set(v.m_pos[0], v.m_pos[1] + v.m_pos[2]);
				}
			}
		}
}

void ueToolModel::GenerateTangentSpace(ueBool includeBinormal, ueBool includeTangent, ueBool overwriteExisting)
{
	ueLogI("Generating tangent space (overwrite existing = %s; include binormals = %s; include tangents = %s) ...",
		overwriteExisting ? "YES" : "NO",
		includeBinormal ? "YES" : "NO",
		includeTangent ? "YES" : "NO");

	// Smooth normals space across all meshes

	std::map<Vertex, ueVec3, Vertex::CmpByPosUVsAndColor> vertexToNormal;

	FOR_EACH_MESH(mesh)
	{
		if (!mesh->IsDrawableTriMesh())
			continue;

		// Accumulate face normals into vertex normals

		const u32 numFaces = (u32) mesh->m_indices.size() / 3;

		for (u32 j = 0; j < numFaces; j++)
		{
			const u32 indices[3] =
			{
				mesh->m_indices[j * 3],
				mesh->m_indices[j * 3 + 1],
				mesh->m_indices[j * 3 + 2]
			};
			const Vertex* v[3] =
			{
				&mesh->m_verts[indices[0]],
				&mesh->m_verts[indices[1]],
				&mesh->m_verts[indices[2]]
			};

			ueVec3 faceNormal;
			ueVec3::Cross(faceNormal, v[0]->m_pos - v[1]->m_pos, v[0]->m_pos - v[2]->m_pos);
			faceNormal.Normalize();

			const f32 faceArea =
				1.0f;
				// Triangle area shall probably be replaced with triangle's inner angle at each vertex
				//calc_triangle_area(v[0]->m_pos, v[1]->m_pos, v[2]->m_pos);

			faceNormal *= faceArea;
			for (u32 k = 0; k < 3; k++)
			{
				ueVec3* existingNormal = map_find(vertexToNormal, *(v[k]));
				if (existingNormal)
					*existingNormal += faceNormal;
				else
					vertexToNormal[*(v[k])] = faceNormal;
			}
		}

		// Set vertex normals (and renormalize)

		for (u32 j = 0; j < mesh->m_verts.size(); j++)
		{
			Vertex& v = mesh->m_verts[j];

			// Make sure normal vector is valid

			if (v.m_flags & ueToolModel::Vertex::Flags_HasNormal)
			{
				v.m_normal.Normalize();
				if (v.m_normal == ueVec3::Zero)
					v.m_flags &= ~ueToolModel::Vertex::Flags_HasNormal;
			}

			// Set new calculated normal

			if (overwriteExisting || !(v.m_flags & Vertex::Flags_HasNormal))
			{
				ueVec3* existingNormal = map_find(vertexToNormal, v);
				UE_ASSERT(existingNormal);

				v.m_flags |= Vertex::Flags_HasNormal;
				v.m_normal = *existingNormal;
				v.m_normal.Normalize();
			}
		}
	}
}

void ueToolModel::RemoveDuplicateVertices()
{
	ueLogI("Removing duplicate vertices ...");

	u32 totalBefore = 0;
	u32 totalAfter = 0;

	FOR_EACH_MESH(mesh)
	{
		totalBefore += (u32) mesh->m_verts.size();

		// Find unique verts

		std::map<Vertex, u32> uniqueVertexToIndex;
		std::vector<u32> indexRemapping(mesh->m_verts.size());

		for (u32 i = 0; i < mesh->m_verts.size(); i++)
		{
			const Vertex& v = mesh->m_verts[i];
			const u32* existingIndex = map_find(uniqueVertexToIndex, v);
			if (!existingIndex)
			{
				const u32 newIndex = (u32) uniqueVertexToIndex.size();
				uniqueVertexToIndex[v] = newIndex;
				indexRemapping[i] = newIndex;
			}
			else
				indexRemapping[i] = *existingIndex;
		}

		// Remap

		mesh->m_verts.resize(uniqueVertexToIndex.size());
		for (std::map<Vertex, u32>::iterator iter = uniqueVertexToIndex.begin(); iter != uniqueVertexToIndex.end(); iter++)
			mesh->m_verts[iter->second] = iter->first;

		for (u32 i = 0; i < mesh->m_indices.size(); i++)
			mesh->m_indices[i] = indexRemapping[ mesh->m_indices[i] ];

		totalAfter += (u32) mesh->m_verts.size();
	}

	ueLogI("Reduced number of verts by %d (%d -> %d)", totalBefore - totalAfter, totalBefore, totalAfter);
}

bool ueToolModel::DetermineSamplerTexture(const char* srcPath, ueToolModel::Sampler& dstSampler)
{
	UE_ASSERT_MSG(!dstSampler.HasValidPath(), "Valid texture already determined.");

	// Set model based texture name

	ioPath fileNameNoExt;
	ioPath_ExtractFileNameWithoutExtension(srcPath, fileNameNoExt, UE_ARRAY_SIZE(fileNameNoExt));
	string_format(dstSampler.m_textureName, "%s/textures/%s", m_name.c_str(), fileNameNoExt);
	string_to_lower(dstSampler.m_textureName);

	// Check if texture exists

	if (ioFile_Exists(srcPath))
	{
		dstSampler.m_sourcePath = srcPath;
		return true;
	}

	// Search in model's sub directories (match HDD dirs and src path dirs)

	std::vector<std::string> srcPathParts;
	string_split(srcPathParts, srcPath, "\\/");
	for (s32 i = (s32) srcPathParts.size() - 1; i >= 0; i--)
	{
		const char* fileName = ioPath_ExtractFileName(srcPath);
		std::string realSrcPath = m_sourceDir;
		for (u32 j = (u32) i; j < srcPathParts.size(); j++)
		{
			realSrcPath += "/";
			realSrcPath += srcPathParts[j];
		}

		if (ioFile_Exists(realSrcPath.c_str()))
		{
			dstSampler.m_sourcePath = realSrcPath;
			return true;
		}
	}

	return false;
}

void ueToolModel::CalculateMissingInvBindPoseTransforms()
{
	std::vector<ueMat44> globalTransforms;

	for (u32 lodIndex = 0; lodIndex < m_lods.size(); lodIndex++)
	{
		LOD* lod = &m_lods[lodIndex];

		ueToolModel::CalcGlobalTransforms(lod, globalTransforms);

		for (u32 i = 0; i < lod->m_nodes.size(); i++)
		{
			ueToolModel::Node& node = lod->m_nodes[i];
			if (!node.m_hasInvBindPose)
			{
				ueMat44::Invert(node.m_invBindPose, globalTransforms[i]);
				node.m_hasInvBindPose = true;
			}
		}
	}
}

void ueToolModel::Transform(const ueMat44& transform)
{
	if (transform == ueMat44::Identity)
		return;

	ueLogW("Model transformation not (yet) supported.");

	for (u32 lodIndex = 0; lodIndex < m_lods.size(); lodIndex++)
	{
		LOD* lod = &m_lods[lodIndex];

		// TODO: Implement me
		// Note: Include collision geometry transformation
	}
}

void ueToolModel::DumpSettings()
{
	DumpSettingHeader();
	DumpSettingString("source", 0, "Source model file name relative to root source directory (\"root_src\"), e.g. 'my_dir/my_model.3ds'");
	DumpSettingBool("generateNormals", Setting_Optional, "Enables generation of normal vectors for the mesh");
	DumpSettingBool("removeDuplicateVertices", Setting_Optional, "Enables duplicate vertices removal");
	DumpSettingBool("noFloat16", Setting_Optional, "Avoids 16-bit floats being used for vertex data (some GPUs don't support it)");
	DumpSettingBool("physicsCompiler", Setting_Optional, "Physics compiler executable to be used for collision geometry compilation");
	DumpSettingBool("generateUV0", Setting_Optional, "Enables UV 0 generation (if not present)");
	DumpSettingBool("generateUV1", Setting_Optional, "Enables UV 1 generation (if not present)");
	DumpSettingBool("generateUV2", Setting_Optional, "Enables UV 2 generation (if not present)");
	DumpSettingBool("generateUV3", Setting_Optional, "Enables UV 3 generation (if not present)");
	DumpSettingBool("makeStatic", Setting_Optional, "Makes the model static (no animation support)");
	DumpSettingBool("preserveUnusedBones", Setting_Optional, "Preserves unused bones / nodes during optimization stages");
	DumpSettingF32("maxMeshBoxDim", Setting_Optional, "Mesh splitting/merging parameter: maximal single mesh box dimension");
	DumpSettingU32("minMeshTris", Setting_Optional, "Mesh splitting/merging parameter: minimal single mesh triangle count");
	DumpSettingU32("maxMeshTris", Setting_Optional, "Mesh splitting/merging parameter: maximal single mesh triangle count");
	DumpSettingU32("maxMeshBones", Setting_Optional, "Mesh splitting/merging parameter: maximal single mesh bone count (useful for limiting bone count per mesh based on target GPU's max. vertex shader constant count)");
	DumpSettingVec3("translation", Setting_Optional, "Translation to be applied to model during export");
	DumpSettingVec3("scale", Setting_Optional, "Scale to be applied to model during export");
	DumpSettingBool("rotationX", Setting_Optional, "Rotation around X axis to be applied to model during export");
	DumpSettingBool("rotationY", Setting_Optional, "Rotation around Y axis to be applied to model during export");
	DumpSettingBool("rotationZ", Setting_Optional, "Rotation around Z axis to be applied to model during export");
}

bool ueToolModel::ParseImportSettings(ImportSettings& settings, ueAssetParams& params)
{
	settings.m_sourcePath = params.GetParam("source");

	params.GetBoolParam("generateNormals", settings.m_generateNormals);
	params.GetBoolParam("removeDuplicateVertices", settings.m_removeDuplicateVertices);
	params.GetBoolParam("noFloat16", settings.m_noF16);
	params.GetParam("physicsCompiler", settings.m_physicsCompilerName);

	char buffer[] = "generateUV?";
	for (u32 i = 0; i < UE_ARRAY_SIZE(settings.m_generateUVs); i++)
	{
		buffer[UE_ARRAY_SIZE(buffer) - 2] = '0' + i;
		params.GetBoolParam(buffer, settings.m_generateUVs[i]);
	}

	params.GetBoolParam("makeStatic", settings.m_optimizeSettings.m_makeStatic);
	params.GetBoolParam("preserveUnusedBones", settings.m_optimizeSettings.m_preserveUnusedBones);

	params.GetF32Param("maxMeshBoxDim", settings.m_optimizeSettings.m_maxMeshBoxDim);
	params.GetU32Param("minMeshTris", settings.m_optimizeSettings.m_minMeshTris);
	params.GetU32Param("maxMeshTris", settings.m_optimizeSettings.m_maxMeshTris);
	params.GetU32Param("maxMeshBones", settings.m_optimizeSettings.m_maxMeshBones);

	// Transformation

	ueVec3 translation = ueVec3::Zero;
	ueVec3 scale = ueVec3::One;
	f32 rotX = 0;
	f32 rotY = 0;
	f32 rotZ = 0;

	params.GetVec3Param("translation", translation);
	params.GetVec3Param("scale", scale);
	params.GetF32Param("rotationX", rotX);
	params.GetF32Param("rotationY", rotY);
	params.GetF32Param("rotationZ", rotZ);

	if (rotY != 0)
		settings.m_transformation.Rotate(ueVec3(0, 1, 0), ueDegToRad(rotY));
	if (rotX != 0)
		settings.m_transformation.Rotate(ueVec3(1, 0, 0), ueDegToRad(rotX));
	if (rotZ != 0)
		settings.m_transformation.Rotate(ueVec3(0, 0, 1), ueDegToRad(rotZ));
	if (translation != ueVec3::Zero)
		settings.m_transformation.Translate(translation);
	if (scale != ueVec3::One)
		settings.m_transformation.Scale(scale);

	return true;
}

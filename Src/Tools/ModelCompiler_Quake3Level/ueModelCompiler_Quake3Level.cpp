#include "ModelCompiler_Common/ueToolModel.h"
#include "IO/ioFile.h"

#include "ueQuake3Level.h"

static ueToolModel* s_dstModel = NULL;
static ueToolModel::LOD* s_dstLOD = NULL;
static ueMat44 s_quake3Transform;

void FromQuake3Vertex(ueToolModel::Vertex& dst, const bsp_vertex_t& src, bool hasLightmap)
{
	dst.m_flags = ueToolModel::Vertex::Flags_HasNormal | ueToolModel::Vertex::Flags_HasColor | ueToolModel::Vertex::Flags_HasTex0;
	dst.m_pos.Set((const f32*) src.point);
	dst.m_normal.Set((const f32*) src.normal);
	dst.m_color = ueColor32(src.color[0], src.color[1], src.color[2], src.color[3]);
	dst.m_tex[0].Set((const f32*) src.texture);
	if (hasLightmap)
	{
		dst.m_flags |= ueToolModel::Vertex::Flags_HasTex1;
		dst.m_tex[1].Set((const f32*) src.lightmap);
	}
	dst.Transform(s_quake3Transform);
}

u32 GetMaterialIndex(const bsp_face_t& face, const bsp_shader_t& shader)
{
	ueToolModel::Material mat;
	mat.m_name = shader.name;

	ueToolModel::Sampler& colorSampler = mat.m_samplers[ueToolModel::Material::SamplerType_Color];
	s_dstModel->DetermineSamplerTexture(shader.name, colorSampler);

//	dstMat.m_hasLightmap = face.lm_texture >= 0;

	return s_dstLOD->GetAddMaterial(mat);
}

bool ImportFromFile(ueToolModel& model, const ueToolModel::ImportSettings& settings, const ueAssetParams& params)
{
	s_dstModel = &model;

	s_quake3Transform.SetIdentity();
	s_quake3Transform.Rotate(ueVec3(1, 0, 0), -UE_PI * 0.5f);
	s_quake3Transform.Scale(0.3f);

	// Load whole file

	ueLogI("Loading Quake 3 BSP Level from '%s'", model.m_sourcePath.c_str());
	std::vector<u8> dataVector;
	if (!vector_u8_load_file(model.m_sourcePath.c_str(), dataVector))
	{
		ueLogE("Failed to load file '%s'", model.m_sourcePath.c_str());
		return false;
	}

	const u8* data = &dataVector[0];

	// Get header

	bsp_header_t* header = (bsp_header_t*) data;

	// Get data

#define GET_LUMP_SIZE(index) header->lumps[index].size
#define GET_LUMP_DATA(index) (data + /*sizeof(bsp_header_t) + */header->lumps[index].offset)

	const int sizeEntities	= GET_LUMP_SIZE(BSP_ENTITIES_LUMP);
	const int numElements	= GET_LUMP_SIZE(BSP_ELEMENTS_LUMP) / sizeof(int);
	const int numFaces		= GET_LUMP_SIZE(BSP_FACES_LUMP) / sizeof(bsp_face_t);
	const int numLeafFaces	= GET_LUMP_SIZE(BSP_LFACES_LUMP) / sizeof(int);
	const int numLeaves		= GET_LUMP_SIZE(BSP_LEAVES_LUMP) / sizeof(bsp_leaf_t);
	const int numLightmaps	= GET_LUMP_SIZE(BSP_LIGHTMAPS_LUMP) / BSP_LIGHTMAP_BANKSIZE;
	const int numModels		= GET_LUMP_SIZE(BSP_MODELS_LUMP) / sizeof(bsp_model_t);
	const int numNodes		= GET_LUMP_SIZE(BSP_NODES_LUMP) / sizeof(bsp_node_t);
	const int numPlanes		= GET_LUMP_SIZE(BSP_PLANES_LUMP) / sizeof(bsp_plane_t);
	const int numShaders	= GET_LUMP_SIZE(BSP_SHADERS_LUMP) / sizeof(bsp_shader_t);
	const int numVertices	= GET_LUMP_SIZE(BSP_VERTICES_LUMP) / sizeof(bsp_vertex_t);
	const int numLeafBrushes= GET_LUMP_SIZE(BSP_LBRUSHES_LUMP) / sizeof(int);
	const int numBrushes	= GET_LUMP_SIZE(BSP_BRUSH_LUMP) / sizeof(bsp_brush_t);
	const int numBrushSides	= GET_LUMP_SIZE(BSP_BRUSHSIDES_LUMP) / sizeof(bsp_brushside_t);

	const char* entities			= (char*) GET_LUMP_DATA(BSP_ENTITIES_LUMP);
	const int* elements				= (int*) GET_LUMP_DATA(BSP_ELEMENTS_LUMP);
	const bsp_face_t* faces			= (bsp_face_t*) GET_LUMP_DATA(BSP_FACES_LUMP);
	const int* leafFaces			= (int*) GET_LUMP_DATA(BSP_LFACES_LUMP);
	const bsp_leaf_t* leaves		= (bsp_leaf_t*) GET_LUMP_DATA(BSP_LEAVES_LUMP);
	const unsigned char* lightmaps	= (unsigned char*) GET_LUMP_DATA(BSP_LIGHTMAPS_LUMP);
	const bsp_model_t* models		= (bsp_model_t*) GET_LUMP_DATA(BSP_MODELS_LUMP);
	const bsp_node_t* nodes			= (bsp_node_t*) GET_LUMP_DATA(BSP_NODES_LUMP);
	const bsp_plane_t* planes		= (bsp_plane_t*) GET_LUMP_DATA(BSP_PLANES_LUMP);
	const bsp_shader_t* shaders		= (bsp_shader_t*) GET_LUMP_DATA(BSP_SHADERS_LUMP);
	const bsp_vis_t* vis			= (bsp_vis_t*) GET_LUMP_DATA(BSP_VISIBILITY_LUMP);
	const bsp_vertex_t* vertices	= (bsp_vertex_t*) GET_LUMP_DATA(BSP_VERTICES_LUMP);
	const int* leafBrushes			= (int*) GET_LUMP_DATA(BSP_LBRUSHES_LUMP);
	const bsp_brush_t* brushes		= (bsp_brush_t*) GET_LUMP_DATA(BSP_BRUSH_LUMP);
	const bsp_brushside_t* brushSides=(bsp_brushside_t*) GET_LUMP_DATA(BSP_BRUSHSIDES_LUMP);

	// Create single LOD

	s_dstLOD = &vector_push(s_dstModel->m_lods);

	// Extract meshes and materials

	ueLogI("Processing faces (%d) ...", numFaces);

	for (int i = 0; i < numFaces; i++)
	{
		const bsp_face_t& srcFace = faces[i];

		UE_ASSERT(srcFace.shader >= 0);
		const bsp_shader_t& shader = shaders[srcFace.shader];
		if (shader.surface_flags & SURF_SKIP)
			continue;

		const bool isDrawable = !(shader.surface_flags & SURF_NODRAW);

		const bool isTransparent = !(shader.content_flags & CONTENTS_SOLID) || (shader.content_flags & CONTENTS_TRANSLUCENT);
		const bool hasLightmap = srcFace.lm_texture >= 0;

		ueToolModel::Mesh* mesh = NULL;

		switch (srcFace.type)
		{
			// Regular mesh

			case BSP_FACETYPE_PLANAR:
			case BSP_FACETYPE_MESH:
			{
				mesh = &vector_push(s_dstLOD->m_meshes);

				for (s32 j = srcFace.vert_start; j < srcFace.vert_start + srcFace.vert_count; j++)
				{
					ueToolModel::Vertex& v = vector_push(mesh->m_verts);
					FromQuake3Vertex(v, vertices[j], hasLightmap);
				}

				for (s32 j = srcFace.elem_start; j < srcFace.elem_start + srcFace.elem_count; j += 3)
				{
					// Flip triangle indices

					mesh->m_indices.push_back(elements[j]);
					mesh->m_indices.push_back(elements[j + 2]);
					mesh->m_indices.push_back(elements[j + 1]);
				}
				break;
			}

			// Patch mesh

			case BSP_FACETYPE_PATCH:
			{
				mesh = &vector_push(s_dstLOD->m_meshes);
				mesh->m_flags = ueToolModel::Mesh::Flags_IsPatch;

				mesh->m_patch.m_width = srcFace.mesh_cp[0];
				mesh->m_patch.m_height = srcFace.mesh_cp[1];

				for (s32 j = srcFace.vert_start; j < srcFace.vert_start + srcFace.vert_count; j++)
				{
					ueToolModel::Vertex& v = vector_push(mesh->m_patch.m_controlPoints);
					FromQuake3Vertex(v, vertices[j], hasLightmap);
				}
				break;
			}
			default:
				break;
		}

		// Set up material

		if (mesh)
		{
			mesh->m_flags |= ueToolModel::Mesh::Flags_IsCollision;
			string_format(mesh->m_name, "%d_%s", i, shader.name);
			if (isDrawable)
			{
				mesh->m_flags |= ueToolModel::Mesh::Flags_IsDrawable;
//				if (isTransparent)
//					mesh->m_flags |= IsTransparent;
				mesh->m_materialIndex = GetMaterialIndex(srcFace, shader);
//				mesh->m_lightmapId = srcFace.lm_texture;
			}
		}
	}

	// Extract lightmaps
#if 0
	ueLogI("Processing lightmaps (%d) ...", numLightmaps);

	dstModel.m_lightmaps.resize(numLightmaps);
	for (int i = 0; i < numLightmaps; ++i)
	{
		LightMap& lm = dstModel.m_lightmaps[i];;
		lm.m_texture.Create(BSP_LIGHTMAP_DIMENSION, BSP_LIGHTMAP_DIMENSION, Format_R8G8B8, lightmapData + i * BSP_LIGHTMAP_BANKSIZE);
	}
#endif

	// Optionally extract spawn points

	bool extractSpawnPoints = false;
	params.GetBoolParam("quake3_extractSpawnPoints", extractSpawnPoints);
	if (extractSpawnPoints)
	{
		std::vector<spawnpoint_t> spawnPoints;
		Q3_GetSpawnPoints(entities, sizeEntities, spawnPoints);

		for (u32 i = 0; i < spawnPoints.size(); i++)
		{
			const spawnpoint_t& p = spawnPoints[i];
			
			ueToolModel::Node& node = vector_push(s_dstLOD->m_nodes);
			string_format(node.m_name, "spawn_point_%d", i);
			
			node.m_localTransform.SetAxisRotation(ueVec3(0, 1, 0), ueRadToDeg(p.m_angleY));

			ueVec3 pos(p.m_pos);
			s_quake3Transform.TransformCoord(pos);
			node.m_localTransform.Translate(pos);
		}
	}

	return true;
}
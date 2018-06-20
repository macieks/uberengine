#pragma once

#include "ContentPipeline/ueContentPipeline.h"
#include "Math/ueShapes.h"
#include "GL/glLib_Shared.h"

class ioPackageWriter;
struct uePhysicsCompilerFuncs;

// Tool side model
struct ueToolModel
{
	static u32 VERSION;

	struct OptimizeSettings
	{
		ueBool m_makeStatic;
		ueBool m_preserveUnusedBones;

		f32 m_maxMeshBoxDim;
		u32 m_minMeshTris;
		u32 m_maxMeshTris;
		u32 m_maxMeshBones;

		OptimizeSettings() :
			m_makeStatic(UE_FALSE),
			m_preserveUnusedBones(UE_FALSE),
			m_maxMeshBoxDim(100000.0f),
			m_minMeshTris(256),
			m_maxMeshTris(65536 / 3),
			m_maxMeshBones(48)
		{}
	};

	struct ImportSettings
	{
		std::string m_sourcePath;

		ueBool m_generateNormals;
		ueBool m_removeDuplicateVertices;
		ueBool m_generateUVs[4];
		ueBool m_noF16;
		OptimizeSettings m_optimizeSettings;

		ueMat44 m_transformation;

		std::string m_physicsCompilerName;
		uePhysicsCompilerFuncs* m_physicsCompilerFuncs;

		ImportSettings() :
			m_generateNormals(UE_FALSE),
			m_removeDuplicateVertices(UE_TRUE),
			m_noF16(UE_FALSE),
			m_physicsCompilerFuncs(NULL),
			m_transformation(ueMat44::Identity)
		{
			for (u32 i = 0; i < UE_ARRAY_SIZE(m_generateUVs); i++)
				m_generateUVs[i] = UE_FALSE;
		}
	};

	struct Primitive
	{
		enum Flags
		{
			Flags_IsCollision = (1 << 0),
		};

		enum Type
		{
			Type_Box = 0,
			Type_Sphere,
			Type_Cylinder,
			Type_Capsule
		};

		union Data
		{
			struct Box
			{
				f32 m_sizeX;
				f32 m_sizeY;
				f32 m_sizeZ;
			} m_box;

			struct Sphere
			{
				f32 m_radius;
			} m_sphere;

			struct Cylinder
			{
				f32 m_radius;
				f32 m_height;
			} m_cylinder;

			struct Capsule
			{
				f32 m_radius;
				f32 m_height;
			} m_capsule;
		};

		std::string m_name;
		Type m_type;
		u32 m_flags;
		u32 m_nodeIndex;
		Data m_data;
		ueMat44 m_transform;

		UE_INLINE Primitive() :
			m_flags(0),
			m_nodeIndex(~0),
			m_transform(ueMat44::Identity)
		{}

		UE_INLINE bool IsCollision() const { return (m_flags & Flags_IsCollision) != 0; }
	};

	struct Vertex
	{
		static const u32 MAX_TEX_STREAMS = 4;
		static const u32 MAX_BONES_PER_VERTEX = 4;

		enum Flags
		{
			Flags_HasNormal		= UE_POW2(0),
			Flags_HasTangent	= UE_POW2(1),
			Flags_HasBinormal	= UE_POW2(2),
			Flags_HasColor		= UE_POW2(3),
			Flags_HasTex0		= UE_POW2(4),
			Flags_HasTex1		= UE_POW2(5),
			Flags_HasTex2		= UE_POW2(6),
			Flags_HasTex3		= UE_POW2(7),
		};

		u32 m_flags;

		ueVec3 m_pos;

		ueVec3 m_normal;
		ueVec3 m_tangent;
		ueVec3 m_binormal;

		ueColor32 m_color;

		ueVec2 m_tex[MAX_TEX_STREAMS];

		u32 m_numBones;
		u32 m_boneIndices[MAX_BONES_PER_VERTEX];
		f32 m_boneWeights[MAX_BONES_PER_VERTEX];

		UE_INLINE Vertex() : m_flags(0), m_numBones(0) {}

		void AddWeight(u32 boneIndex, f32 weight);
		void Transform(const ueMat44& t);

		struct CmpByPosUVsAndColor
		{
			UE_INLINE bool operator () (const Vertex& a, const Vertex& b) const
			{
				s32 x;
	
				if (x = memcmp(&a.m_pos, &b.m_pos, sizeof(a.m_pos)))
					return x < 0;

				for (u32 i = 0; i < MAX_TEX_STREAMS; i++)
					if ((a.m_flags & (Flags_HasTex0 << i)) && (x = memcmp(&a.m_tex[i], &b.m_tex[i], sizeof(a.m_tex[i]))))
						return x < 0;

				if ((a.m_flags & Flags_HasColor) && (x = memcmp(&a.m_color, &b.m_color, sizeof(a.m_color))))
					return x < 0;

				return false;
			}
		};

		UE_INLINE bool operator < (const Vertex& b) const
		{
			s32 x;
	
			if (x = memcmp(&m_pos, &b.m_pos, sizeof(m_pos)))
				return x < 0;

			if ((m_flags & Flags_HasNormal) && (x = memcmp(&m_normal, &b.m_normal, sizeof(m_normal))))
				return x < 0;
			if ((m_flags & Flags_HasBinormal) && (x = memcmp(&m_binormal, &b.m_binormal, sizeof(m_binormal))))
				return x < 0;
			if ((m_flags & Flags_HasTangent) && (x = memcmp(&m_tangent, &b.m_tangent, sizeof(m_tangent))))
				return x < 0;

			for (u32 i = 0; i < MAX_TEX_STREAMS; i++)
				if ((m_flags & (Flags_HasTex0 << i)) && (x = memcmp(&m_tex[i], &b.m_tex[i], sizeof(m_tex[i]))))
					return x < 0;

			if ((m_flags & Flags_HasColor) && (x = memcmp(&m_color, &b.m_color, sizeof(m_color))))
				return x < 0;

			if (x = ((s32) m_numBones - (s32) b.m_numBones))
				return x < 0;

			for (u32 i = 0; i < m_numBones; i++)
			{
				if (x = ((s32) m_boneIndices[i] - (s32) b.m_boneIndices[i]))
					return x < 0;

				const f32 y = m_boneWeights[i] - b.m_boneWeights[i];
				if (y != 0.0f)
					return y < 0.0f;
			}

			return false;

		}
	};

	struct Mesh
	{
		enum Flags
		{
			Flags_IsDrawable	= UE_POW2(0),
			Flags_IsCollision	= UE_POW2(1),
			Flags_IsConvex		= UE_POW2(2),
			Flags_IsWater		= UE_POW2(3),
			Flags_IsPatch		= UE_POW2(4)
		};

		std::string m_name;
		u32 m_flags;
		ueBox m_box;
		u32 m_materialIndex;
		u32 m_nodeIndex;
		std::vector<Vertex> m_verts;
		std::vector<u32> m_indices;

		struct Patch
		{
			u32 m_width;
			u32 m_height;
			std::vector<Vertex> m_controlPoints;

			Patch() : m_width(0), m_height(0) {}
		} m_patch;

		UE_INLINE Mesh() :
			m_flags(0),
			m_materialIndex(0),
			m_nodeIndex(~0)
		{}

		UE_INLINE bool IsDrawable() const { return (m_flags & Flags_IsDrawable) != 0; }
		UE_INLINE bool IsCollision() const { return (m_flags & Flags_IsCollision) != 0; }
		UE_INLINE bool IsPatch() const { return (m_flags & Flags_IsPatch) != 0; }
		UE_INLINE bool IsDrawableTriMesh() const { return IsDrawable() && !IsPatch(); }
	};

	struct Node
	{
		u32 m_parentIndex;
		std::string m_name;
		ueMat44 m_localTransform;
		ueMat44 m_invBindPose;
		bool m_hasInvBindPose;

		Node() :
			m_parentIndex(~0),
			m_hasInvBindPose(false)
		{}
	};

	struct Sampler
	{
		std::string m_textureName;
		std::string m_sourcePath;

		glSamplerParams m_desc;

		bool operator == (const Sampler& other) const
		{
			return m_textureName == other.m_textureName &&
				m_sourcePath == other.m_sourcePath &&
				!ueMemCmp(&m_desc, &other.m_desc, sizeof(glSamplerParams));
		}

		UE_INLINE bool HasValidPath() const { return m_sourcePath.length() > 0; }
	};

	struct Material
	{
		std::string m_name;

		enum SamplerType
		{
			SamplerType_Color = 0,
			SamplerType_Lightmap,
			SamplerType_Normal,
			SamplerType_Height,
			SamplerType_Specular,
			SamplerType_Detail,

			SamplerType_MAX
		};
		Sampler m_samplers[SamplerType_MAX];

		enum Flags
		{
			Flags_TwoSided = 1 << 0
		};
		u32 m_flags;

		ueVec3 m_ambient;
		ueVec3 m_diffuse;
		ueVec3 m_specular;
		f32 m_specularPower;
		f32 m_opacity;

		Material() :
			m_flags(0),
			m_ambient(0, 0, 0),
			m_diffuse(1, 1, 1),
			m_specular(0, 0, 0),
			m_specularPower(1),
			m_opacity(1.0f)
		{}

		bool operator == (const Material& other) const
		{
			if (!(m_name == other.m_name &&
				m_flags == other.m_flags &&
				m_ambient == other.m_ambient &&
				m_diffuse == other.m_diffuse &&
				m_specular == other.m_specular &&
				m_specularPower == other.m_specularPower))
				return false;

			for (int i = 0; i < SamplerType_MAX; i++)
			{
				const bool x1 = m_samplers[i] == other.m_samplers[i];
				if (!x1)
					return false;
			}

			return true;
		}
	};

	struct LOD
	{
		std::vector<Node> m_nodes;
		std::vector<Mesh> m_meshes;
		std::vector<Primitive> m_prims;
		std::vector<Material> m_materials;
		std::map<std::string, u32> m_materialToIndexMap;

		u32 GetNodeIndex(const char* name) const;
		u32 GetMaterialIndex(const char* name) const;
		u32 GetAddMaterial(ueToolModel::Material& mat);
	};

	u32 m_version;
	std::string m_name;
	std::string m_sourcePath;
	std::string m_sourceDir;

	ueBox m_box;
	std::vector<LOD> m_lods;

	ueToolModel();
	void Reset();
	bool Save(const char* path);
	bool Load(const char* path);

	bool Serialize(ioPackageWriter* pw, ImportSettings& settings);

	void Optimize(const OptimizeSettings& settings);

	void SubdividePatchMesh(Mesh* mesh, u32 maxDimension = 128, f32 subdivisions = 4);
	void ConvertPatchesToTrimeshes();

	void Transform(const ueMat44& transform);
	void MakeStatic(ueBool preserveUnusedBones);
	void RemoveBindPose();
	void CalculateMissingInvBindPoseTransforms();
	void AddUVStreams(u32 vertexFlags); // Vertex::Flags_*
	void GenerateTangentSpace(ueBool includeBinormal = UE_FALSE, ueBool includeTangent = UE_FALSE, ueBool overwriteExisting = UE_FALSE);
	void RemoveDuplicateVertices();

	void CalculateBox();
	bool DetermineSamplerTexture(const char* srcPath, ueToolModel::Sampler& dstSampler);

	static void CalcGlobalTransforms(const LOD* lod, std::vector<ueMat44>& globalTransforms);
	static ueToolModel* Merge(u32 numModels, ueToolModel** models);

	static void DumpSettings();
	static bool ParseImportSettings(ImportSettings& settings, ueAssetParams& params);
};

class ueToolModelMeshIterator
{
public:
	ueToolModelMeshIterator(ueToolModel* model) : m_model(model), m_lodIndex(0), m_meshIndex(0) {}
	ueToolModel::Mesh* Next()
	{
		while (1)
		{
			if (m_lodIndex == m_model->m_lods.size())
				return NULL;
			if (m_meshIndex < m_model->m_lods[m_lodIndex].m_meshes.size())
				return &m_model->m_lods[m_lodIndex].m_meshes[m_meshIndex++];
			m_lodIndex++;
			m_meshIndex = 0;
		}
	}
private:
	ueToolModel* m_model;
	u32 m_lodIndex;
	u32 m_meshIndex;
};

#define FOR_EACH_MESH(mesh) \
	ueToolModelMeshIterator iter(this); \
	while (ueToolModel::Mesh* mesh = iter.Next())

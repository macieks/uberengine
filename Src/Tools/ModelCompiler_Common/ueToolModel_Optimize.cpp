#include "ModelCompiler_Common/ueToolModel.h"

#include <set>
#include <algorithm>

#include "NvTriStrip.h"

struct Triangle
{
	ueVec3 m_center;
	ueToolModel::Vertex v[3];

	struct CmpByAxis
	{
		u32 m_axis;

		CmpByAxis(u32 axis) : m_axis(axis) {}

		inline bool operator() (const Triangle& a, const Triangle& b) const
		{
			return a.m_center[m_axis] < b.m_center[m_axis];
		}
	};
};

// Set of triangles with similar mesh-level properties
struct Bucket
{
	std::string m_name;
	u32 m_materialIndex;
	u32 m_vertexFlags;
	u32 m_meshFlags;

	std::vector<Triangle> m_tris;
};

static ueToolModel::LOD* s_lod = NULL;
static const ueToolModel::OptimizeSettings* s_settings = NULL;
static Bucket* s_bucket = NULL;

void BuildBuckets(std::vector<Bucket>& buckets)
{
	ueLogI("Building buckets...");

	std::vector<ueToolModel::Mesh> meshes = s_lod->m_meshes;
	s_lod->m_meshes.clear();

	for (u32 i = 0; i < meshes.size(); i++)
	{
		ueToolModel::Mesh* mesh = &meshes[i];
		if (!mesh->IsDrawableTriMesh())
		{
			// Make sure all non-processed meshes remain in lod

			s_lod->m_meshes.push_back(*mesh);
			continue;
		}

		const u32 meshFlags = mesh->m_flags;
		const u32 materialIndex = mesh->m_materialIndex;

		// Get node and its bind pose transform

		const ueToolModel::Node* node = mesh->m_nodeIndex != U32_MAX ? &s_lod->m_nodes[mesh->m_nodeIndex] : NULL;
		ueMat44 bindPoseTransform = ueMat44::Identity;
		if (node)
			ueMat44::Invert(bindPoseTransform, node->m_invBindPose);

		// Build triangles and insert them into buckets

		for (u32 j = 0; j < mesh->m_indices.size(); j += 3)
		{
			Triangle t;
			for (u32 k = 0; k < 3; k++)
				t.v[k] = mesh->m_verts[ mesh->m_indices[j + k] ];
			t.m_center = (t.v[0].m_pos + t.v[1].m_pos + t.v[2].m_pos) / 3.0f;

			const u32 vertexFlags = t.v[0].m_flags;

			// Set up single "rigid" bone (if non-skinned, non-static and mesh has parent bone)

			if (node && t.v[0].m_numBones == 0)
				for (u32 k = 0; k < 3; k++)
				{
					t.v[k].m_numBones = 1;
					t.v[k].m_boneIndices[0] = mesh->m_nodeIndex;
					t.v[k].m_boneWeights[0] = 1.0f;

					// Transform into bind pose

					t.v[k].Transform(bindPoseTransform);
				}

			// Find or create suitable bucket

			Bucket* bucket = NULL;
			for (u32 k = 0; k < buckets.size(); k++)
			{
				Bucket* b = &buckets[k];
				if (b->m_materialIndex == materialIndex &&
					b->m_meshFlags == meshFlags &&
					b->m_vertexFlags == vertexFlags)
				{
					bucket = b;
					break;
				}
			}

			if (!bucket)
			{
				bucket = &vector_push(buckets);
				bucket->m_name = mesh->m_name;
				bucket->m_materialIndex = materialIndex;
				bucket->m_meshFlags = meshFlags;
				bucket->m_vertexFlags = vertexFlags;
			}

			// Add triangle to bucket

			bucket->m_tris.push_back(t);
		}
	}

	ueLogI("Built %u buckets (%d non-bucketed meshes)", buckets.size(), s_lod->m_meshes.size());
}

void OptimizePrimitives(ueToolModel::Mesh* mesh)
{
	// No support for 32-bit indices

	const u32 numVerts = (u32) mesh->m_verts.size();
	if (numVerts > (1 << 16))
		return;

	// Disable NvTriStrip's output (spam!)

	NvTriStrip_EnableStdOutput(false);

	// Require triangle lists

	NvTriStrip_SetListsOnly(true);

	// Convert indices to 16 bits

	u32 numIndices = (u32) mesh->m_indices.size();
	std::vector<u16> indices_u16(numIndices);
	for (u32 i = 0; i < numIndices; i++)
		indices_u16[i] = (u16) mesh->m_indices[i];

	// Optimize indices

	PrimitiveGroup* primGroups = NULL;
	u16 numGroups = 0;
	const bool result = NvTriStrip_GenerateStrips(&indices_u16[0], numIndices, &primGroups, &numGroups, false /* disable validation */);
	UE_ASSERT(result && primGroups && numGroups == 1 && primGroups[0].type == PT_LIST);

	// Trim indices array (some indices could have been removed - not sure when - maybe for invalid triangles?)

	numIndices = primGroups[0].numIndices;
	mesh->m_indices.resize(numIndices);

	// Reorder verts

	PrimitiveGroup* remappedGroups = NULL;
	NvTriStrip_RemapIndices(primGroups, numGroups, numVerts, &remappedGroups);

	const std::vector<ueToolModel::Vertex> tempVerts = mesh->m_verts;
	for (u32 i = 0; i < numIndices; i++)
	{
		mesh->m_verts[ remappedGroups->indices[i] ] = tempVerts[ primGroups->indices[i] ];
		mesh->m_indices[i] = remappedGroups->indices[i];
	}

	// Clean up

	delete[] primGroups;
	delete[] remappedGroups;
}

void BuildMesh(u32 first, u32 count, const ueBox& box, u32 nodeIndex)
{
	ueLogI("Building mesh %u (name: '%s' tris: %u) ...", (u32) s_lod->m_meshes.size(), s_bucket->m_name.c_str(), count);

	// Create new mesh

	ueToolModel::Mesh* mesh = &vector_push(s_lod->m_meshes);
	mesh->m_flags = s_bucket->m_meshFlags;
	mesh->m_materialIndex = s_bucket->m_materialIndex;
	mesh->m_box = box;
	mesh->m_nodeIndex = nodeIndex;
	mesh->m_name = s_bucket->m_name;

	// Determine unique verts & build indices

	const bool isRigidSkinned = !s_settings->m_makeStatic && nodeIndex != U32_MAX;
	const ueToolModel::Node* node = nodeIndex != U32_MAX ? &s_lod->m_nodes[nodeIndex] : NULL;

	std::map<ueToolModel::Vertex, u32> uniqueVerts;

	const u32 end = first + count;
	for (u32 i = first; i < end; i++)
		for (u32 j = 0; j < 3; j++)
		{
			ueToolModel::Vertex& v = s_bucket->m_tris[i].v[j];

			// Remove bone index from vertex if rigid-skinned

			if (isRigidSkinned)
			{
				v.m_numBones = 0;

				// Transform back into non-bind pose

				v.Transform(node->m_invBindPose);
			}

			// Add new vertex (if not found equal) & new index

			const std::map<ueToolModel::Vertex, u32>::const_iterator iter = uniqueVerts.find(v);
			if (iter != uniqueVerts.end())
				mesh->m_indices.push_back(iter->second);
			else
			{
				const u32 newVertexIndex = (u32) uniqueVerts.size();
				mesh->m_indices.push_back(newVertexIndex);
				uniqueVerts[v] = newVertexIndex;
			}
		}

	// Build vertices

	mesh->m_verts.resize(uniqueVerts.size());
	for (std::map<ueToolModel::Vertex, u32>::const_iterator iter = uniqueVerts.begin(); iter != uniqueVerts.end(); iter++)
		mesh->m_verts[iter->second] = iter->first;

	// Optimize indices & vertices

	OptimizePrimitives(mesh);
}

void SplitBucket(u32 first, u32 count)
{
	// Calc bbox

	ueBox box;
	box.Reset();
	const u32 end = first + count;
	for (u32 i = first; i < end; i++)
		for (u32 j = 0; j < 3; j++)
			box.Extend(s_bucket->m_tris[i].v[j].m_pos);

	ueVec3 boxSize;
	box.GetExtents(boxSize);
	const f32 maxBoxDim = ueMin(ueMin(boxSize[0], boxSize[1]), boxSize[1]);

	// Calc number of unique bones used (and figure out parent bone, if any)

	u32 nodeIndex = U32_MAX;

	u32 numNodes = 0;
	if (!s_settings->m_makeStatic)
	{
		std::set<u32> nodeIds;
		for (u32 i = first; i < end; i++)
			for (u32 j = 0; j < 3; j++)
			{
				const ueToolModel::Vertex& v = s_bucket->m_tris[i].v[j];
				for (u32 k = 0; k < v.m_numBones; k++)
					nodeIds.insert(v.m_boneIndices[k]);
			}
		numNodes = (u32) nodeIds.size();

		if (nodeIds.size() == 1)
			nodeIndex = *nodeIds.begin();
	}

	// Determine properties of triangle set

	const bool largerThanMaxBox = maxBoxDim > s_settings->m_maxMeshBoxDim;

	const bool aboveMaxTriCount = count > s_settings->m_maxMeshTris;
	const bool belowMinTriCount = count < s_settings->m_minMeshTris;

	const bool aboveMaxBoneCount = numNodes > s_settings->m_maxMeshBones;

	// Decide whether to split the set

	const bool split = aboveMaxBoneCount || aboveMaxTriCount || (largerThanMaxBox && !belowMinTriCount);

	// No split? - generate final mesh

	if (!split)
	{
		BuildMesh(first, count, box, nodeIndex);
		return;
	}

	// TODO: For skinned models split in a way that would separate skinned meshes from non-skinned ones - how???

	// Split recursively along longest axis

	u32 splitAxis = 0;
	for (u32 i = 1; i < 3; i++)
		if (boxSize[i] > boxSize[splitAxis])
			splitAxis = i;

	std::sort(&s_bucket->m_tris[first], &s_bucket->m_tris[first] + count, Triangle::CmpByAxis(splitAxis));

	const u32 halfCount = count / 2;
	SplitBucket(first, halfCount);
	SplitBucket(first + halfCount, count - halfCount);
}

void OptimizeLOD()
{
	// Build triangle buckets

	std::vector<Bucket> buckets;
	BuildBuckets(buckets);

	// Optimize buckets and build new meshes

	for (u32 i = 0; i < buckets.size(); i++)
	{
		s_bucket = &buckets[i];
		SplitBucket(0, (u32) s_bucket->m_tris.size());
	}
}

void ueToolModel::Optimize(const OptimizeSettings& settings)
{
	s_settings = &settings;
	for (u32 i = 0; i < m_lods.size(); i++)
	{
		s_lod = &m_lods[i];

		ueLogI("Optimizing geometry for LOD %u...", i);
		OptimizeLOD();
	}
}

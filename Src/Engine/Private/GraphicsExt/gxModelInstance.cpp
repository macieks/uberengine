#include "GraphicsExt/gxSkeleton.h"
#include "GraphicsExt/gxAnimationInstance_Private.h"
#include "GraphicsExt/gxModelInstance_Private.h"
#include "GraphicsExt/gxModel_Private.h"
#include "GraphicsExt/gxCollision.h"

f32 gxModelInstance::m_nodeWeightsAccumulator[MAX_BONES];

void gxModelInstance_Destroy(gxModelInstance* mi)
{
	gxModelInstance_RemoveFromScene(mi);
	gxModelInstance_DestroyAllAnimations(mi);
	mi->m_model.Release();
	gxModelMgr_GetAllocator()->Free(mi);
}

gxModel* gxModelInstance_GetModel(gxModelInstance* mi) { return *mi->m_model; }

gxAnimationInstance* gxModelInstance_PlayAnimation(gxModelInstance* mi, gxAnimation* animation, const gxAnimationConfig* config)
{
	UE_ASSERT(config);

	if (config->m_mixMode == gxAnimationMixMode_Overwrite)
	{
		// Fade out all running animations at the speed of the fade in of the new one

		gxAnimationInstance* current = mi->m_skeletalAnimationInstances.Front();
		while (current)
		{
			gxAnimationInstance_Stop(current, config->m_blendInTime, UE_TRUE);
			current = current->Next();
		}

		// Remove all done animations

		current = mi->m_skeletalAnimationInstances.Front();
		while (current)
		{
			gxAnimationInstance* next = current->Next();
			if (gxAnimationInstance_ToBeDestroyed(current))
				gxAnimationInstance_Destroy(current);
			current = next;
		}
	}

	// Create new animation

	gxAnimationInstance* ai = gxAnimationInstance_Create(mi, animation, config);
	mi->m_skeletalAnimationInstances.PushFront(ai);

	for (u32 i = 0; i < mi->m_model->m_numLODs; i++)
		mi->m_LODs[i].m_nodeTransformsDirty = UE_TRUE;

	return ai;
}

void gxModelInstance_DestroyAllAnimations(gxModelInstance* mi)
{
	while (gxAnimationInstance* ai = mi->m_skeletalAnimationInstances.PopFront())
		gxAnimationInstance_Destroy(ai);

	for (u32 i = 0; i < mi->m_model->m_numLODs; i++)
		mi->m_LODs[i].m_nodeTransformsDirty = UE_TRUE;
}

void gxModelInstance_UpdateAnimations(gxModelInstance* mi, f32 dt)
{
	if (mi->m_skeletalAnimationInstances.IsEmpty())
		return;

	// Update all active animations

	gxAnimationInstance* current = mi->m_skeletalAnimationInstances.Front();
	while (current)
	{
		gxAnimationInstance_Update(current, dt);
		current = current->Next();
	}

	// Remove all done animations

	current = mi->m_skeletalAnimationInstances.Front();
	while (current)
	{
		gxAnimationInstance* next = current->Next();
		if (gxAnimationInstance_ToBeDestroyed(current))
			gxAnimationInstance_Destroy(current);
		current = next;
	}

	for (u32 i = 0; i < mi->m_model->m_numLODs; i++)
		mi->m_LODs[i].m_nodeTransformsDirty = UE_TRUE;
}

void gxModelInstance_SetBaseTransform(gxModelInstance* mi, const ueMat44& baseTransform)
{
	if (mi->m_baseTransform == baseTransform)
		return;

	mi->m_baseTransform = baseTransform;
	for (u32 i = 0; i < mi->m_model->m_numLODs; i++)
		mi->m_LODs[i].m_nodeTransformsDirty = UE_TRUE;
}

u32 gxModelInstance_GetNumNodes(gxModelInstance* mi, u32 lodIndex) { return mi->m_model->m_LODs[lodIndex].m_skeleton->m_numNodes; }

void gxModelInstance_UpdateNodeTransforms(gxModelInstance* mi, u32 lodIndex)
{
	gxModelInstanceLOD* lod = &mi->m_LODs[lodIndex];
	if (!lod->m_nodeTransformsDirty)
		return;

	const gxSkeleton* skeleton = mi->m_model->m_LODs[lodIndex].m_skeleton;
	const gxSkeletonNode* skeletonBones = skeleton->m_nodes;
	ueMat44* nodeTransforms = lod->m_nodeTransforms;

	// Zero out transformations

	ueMat44 zeroTransform(ueMat44::Zero);
	for (u32 i = 0; i < skeleton->m_numNodes; i++)
	{
		nodeTransforms[i] = zeroTransform;
		mi->m_nodeWeightsAccumulator[i] = 0;
	}

	// Accumulate transformations

	gxAnimationInstance* current = mi->m_skeletalAnimationInstances.Front();
	while (current)
	{
		gxAnimationInstance_AccumulateBoneTransforms(current, lodIndex);
		current = current->Next();
	}

	// Normalize the transformations to maintain weight 1.0 for each node

	for (u32 i = 0; i < skeleton->m_numNodes; i++)
		if (mi->m_nodeWeightsAccumulator[i] > 1.0f)
			nodeTransforms[i] /= mi->m_nodeWeightsAccumulator[i];
		else if (mi->m_nodeWeightsAccumulator[i] < 1.0f)
		{
			ueMat44 localTransformWeighted(skeleton->m_nodes[i].m_localTransform);
			localTransformWeighted *= 1.0f - mi->m_nodeWeightsAccumulator[i];

			nodeTransforms[i] += localTransformWeighted;
		}

	// Concatenate transformations for all nodes

	for (u32 i = 0; i < skeleton->m_numNodes; i++)
	{
		const u32 parentNodeIndex = skeletonBones[i].m_parentNodeIndex;
		nodeTransforms[i] *= (parentNodeIndex != U32_MAX) ? nodeTransforms[parentNodeIndex] : mi->m_baseTransform;
	}

	lod->m_nodeTransformsDirty = UE_FALSE;
}

const ueMat44* gxModelInstance_GetBaseTransform(gxModelInstance* mi) { return &mi->m_baseTransform; }

const ueMat44* gxModelInstance_GetNodeTransforms(gxModelInstance* mi, u32 lodIndex)
{
	gxModelInstance_UpdateNodeTransforms(mi, lodIndex);
	return mi->m_LODs[lodIndex].m_nodeTransforms;
}

void gxModelInstance_GetSkinningNodeTransforms(gxModelInstance* mi, u32 lodIndex, u32 numNodes, const u32* nodeIndices, ueMat44* dstArray)
{
	gxModelInstance_UpdateNodeTransforms(mi, lodIndex);

	const ueMat44* nodeTransforms = mi->m_LODs[lodIndex].m_nodeTransforms;
#if 0
	for (u32 i = 0; i < numNodes; i++)
		dstArray[i] = nodeTransforms[nodeIndices[i]];

#else // Testing

	const gxSkeleton* skeleton = mi->m_model->m_LODs[lodIndex].m_skeleton;
	for (u32 i = 0; i < numNodes; i++)
	{
		const u32 boneIndex = nodeIndices[i];
		ueMat44::Mul(dstArray[i], skeleton->m_nodes[boneIndex].m_invBindPose, nodeTransforms[boneIndex]);
	}

#endif
}

void gxModelInstance_UpdatePhysics(gxModelInstance* mi)
{
	// if (animation drives transformations)
	// [...]
	// else if (physics drives transformations)
	// [...]
}

void gxModelInstance_AddToScene(gxModelInstance* mi, phScene* scene)
{
	gxModelInstanceLOD* lod = &mi->m_LODs[0]; // Only using LOD 0 for collision
	if (lod->m_collGeomInstance)
		lod->m_collGeomInstance->AddToScene(scene);
}

void gxModelInstance_RemoveFromScene(gxModelInstance* mi)
{
	gxModelInstanceLOD* lod = &mi->m_LODs[0]; // Only using LOD 0 for collision
	if (lod->m_collGeomInstance)
		lod->m_collGeomInstance->RemoveFromScene();
}

void gxModelInstance_Update(gxModelInstance* mi, f32 dt)
{
	gxModelInstance_UpdateAnimations(mi, dt);
	gxModelInstance_UpdatePhysics(mi);
}

#if 0

struct gxModelInstancePacket : gxPacket
{
	u32 m_lodIndex;
};

struct gxMeshInstancePacket : gxPacket
{
	u32 m_lodIndex;
	u32 m_meshIndex;
};

struct gxSortKeyData
{
	f32 m_distance;
	u32 m_materialIndex;
	u32 m_numNodes;

	UE_INLINE void SetSortKey(gxPacket* packet) const
	{
		u32 key;
		SetBits(key, 0, 16, (u32) ueMin(m_distance, 65535.0f));
		SetBits(key, 16, 2, ueMin(m_numNodes, 2));
		SetBits(key, 18, 14, m_materialIndex);
		packet->m_sortKey[0] = key;
	}
};

void gxModelInstance_DrawSkeleton(gxModelInstancePacket* packet, gxContext* ctx);
void gxModelInstance_DrawNormals(gxModelInstancePacket* packet, gxContext* ctx);
void gxModelInstance_DrawDepth(gxModelInstancePacket* packet, gxContext* ctx);
void gxModelInstance_DrawShadow(gxModelInstancePacket* packet, gxContext* ctx);
void gxModelInstance_DrawVelocity(gxModelInstancePacket* packet, gxContext* ctx);
void gxModelInstance_DrawMesh(gxMeshInstancePacket* packet, gxContext* ctx);

UE_INLINE void gxModelInstance_CreatePacket(gxModelInstance* mi, gxFrame* frame, u32 lodIndex, f32 distance, gxPass pass, gxDrawFunc func)
{
	if (gxModelInstancePacket* packet = frame->AddPacket<gxModelInstancePacket>(pass))
	{
		packet->m_userData = mi;
		packet->m_drawFunc = func;
		packet->m_lodIndex = lodIndex;

		gxSortKeyData keyData;
		keyData.m_distance = distance;
		keyData.SetSortKey(packet);
	}
}

void gxModelInstance_SubmitRenderPackets(gxModelInstance* mi, gxFrame* frame)
{
	const f32 distance = ueVec3::Dist(frame->m_cameraPos, bbox.GetCenter());
	const u32 lodIndex = model->GetLODIndexForDistanceFromCamera(distance);
	gxModelLOD* lod = &model->m_LODs[lodIndex];

	if (frame->HasPass(gxPass_Debug))
	{
		if (showBBox)
			if (rdDebugBoxPacket* packet = frame->AddPacket<rdDebugBoxPacket>(gxPass_Debug))
				gxModelInstance_GetBBox(m_modelInstance, packet->m_box)

		if (showSkeleton)
			gxModelInstance_CreatePacket(mi, frame, lodIndex, distance, gxPass_Debug, (gxDrawFunc) gxModelInstamce_DrawSkeleton);

		if (showNormals)
			gxModelInstance_CreatePacket(mi, frame, lodIndex, distance, gxPass_Debug, (gxDrawFunc) gxModelInstamce_DrawNormals);
	}

	if (showModel)
		for (u32 i = 0; i < lod->m_numMeshes; i++)
		{
			gxModelMesh* mesh = &lod->m_meshes[i];
			gxModelMaterialInfo* materialInfo = &lod->m_materials[mesh->m_materialIndex];

			const ueBool isTransparent = false;
			const u32 materialIndex = 0;

			gxSortKeyData keyData;
			keyData.m_distance = mesh->m_numNodes != 1 ?
					distance : // Skinning or static case
					ueVec3::Dist(frame->m_cameraPos, gxModelInstance_GetNodePosition(mi, mesh->m_boneIndices[0])); // Rigid skinning case
			keyData.m_numNodes = mesh->m_numNodes;
			keyData.m_materialIndex = materialIndex;

			if (gxMeshInstancePacket* packet = frame->AddPacket<gxMeshInstancePacket>(isTransparent ? gxPass_Transparent : gxPass_Opaque))
			{
				packet->m_userData = mi;
				packet->m_drawFunc = (gxDrawFunc) gxModelInstance_DrawMesh;
				packet->m_lodIndex = lodIndex;
				packet->m_meshIndex = meshIndex;
				keyData.SetSortKey(packet);
			}
		}
}

#endif
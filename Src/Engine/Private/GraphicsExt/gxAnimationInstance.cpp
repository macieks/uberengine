#include "GraphicsExt/gxSkeleton.h"
#include "GraphicsExt/gxAnimationInstance_Private.h"
#include "GraphicsExt/gxModel.h"
#include "GraphicsExt/gxModel_Private.h"
#include "GraphicsExt/gxAnimation_Private.h"
#include "GraphicsExt/gxModelInstance_Private.h"

gxAnimationConfig gxAnimationConfig::Default;

gxAnimationInstance* gxAnimationInstance_Create(gxModelInstance* mi, gxAnimation* animation, const gxAnimationConfig* config)
{
	const u32 memorySize = sizeof(gxAnimationInstance) + mi->m_model->m_numLODs * sizeof(gxAnimationInstance::LOD);
	u8* memory = (u8*) gxModelMgr_GetAllocator()->Alloc(memorySize);
	UE_ASSERT(memory);

	gxAnimationInstance* ai = new(memory) gxAnimationInstance();
	memory += sizeof(gxAnimationInstance);

	ai->m_modelInstance = mi;
	ai->m_animation = animation;
	ai->m_realTime = 0;
	ai->m_totalTime = 0;

	ai->m_time = ueMod(config->m_startTimeOffset, ai->m_animation->m_data->m_length);
	ai->m_numLoops = config->m_numLoops;
	ai->m_weight = 0;
	ai->m_targetWeight = config->m_weight;
	ai->m_blendInTime = config->m_blendInTime;
	ai->m_blendOutTime = config->m_blendOutTime;
	ai->m_speed = config->m_speed;
	ai->m_playMode = config->m_playMode;
	ai->m_mixMode = config->m_mixMode;
	ai->m_state = gxAnimationInstanceState_Playing;
	ai->m_isPlayingForward = config->m_playMode != gxAnimationPlayMode_Backward;
	ai->m_destroyWhenStops = config->m_destroyWhenStops;

	ai->m_LODs = (gxAnimationInstance::LOD*) memory;

	gxModel* model = *ai->m_modelInstance->m_model;
	for (u32 i = 0; i < model->m_numLODs; i++)
	{
		gxSkeleton* skeleton = model->m_LODs[i].m_skeleton;
		if (skeleton)
		{
			ai->m_LODs[i].m_binding = skeleton->GetBinding(animation);
			UE_ASSERT(ai->m_LODs[i].m_binding);
		}
		else
			ai->m_LODs[i].m_binding = NULL;
	}

	return ai;
}

void gxAnimationInstance_Destroy(gxAnimationInstance* ai)
{
	gxModel* model = *ai->m_modelInstance->m_model;
	for (u32 i = 0; i < model->m_numLODs; i++)
	{
		gxSkeleton* skeleton = model->m_LODs[i].m_skeleton;
		if (skeleton)
			skeleton->ReleaseBinding(ai->m_LODs[i].m_binding);
	}

	ai->m_modelInstance->m_skeletalAnimationInstances.Remove(ai);
	ueDelete(ai, gxModelMgr_GetAllocator());
}

void gxAnimationInstance_Update(gxAnimationInstance* ai, f32 deltaTime)
{
	if (ai->m_state == gxAnimationInstanceState_Stopped)
		return;

	ai->m_realTime += deltaTime;

	// Update weight

	if (ai->m_state == gxAnimationInstanceState_Stopping)
	{
		ai->m_weight -= deltaTime / ai->m_blendOutTime;
		if (ai->m_weight <= 0)
		{
			ai->m_weight = 0;
			ai->m_state = gxAnimationInstanceState_Stopped;
			return;
		}
	}
	else
	{
		ai->m_weight += deltaTime / ai->m_blendInTime;
		if (ai->m_weight >= ai->m_targetWeight)
			ai->m_weight = ai->m_targetWeight;
	}

	// Update time

	ai->m_time += deltaTime * ai->m_speed * (ai->m_isPlayingForward ? 1.0f : -1.0f);
	ai->m_totalTime += deltaTime * ai->m_speed;

	// Shall we start stopping this animation?

	if (ai->m_state == gxAnimationInstanceState_Playing &&
		ai->m_numLoops != 0.0f &&
		ai->m_totalTime + ai->m_blendOutTime >= ai->m_numLoops * ai->m_animation->m_data->m_length)
	{
		ai->m_state = gxAnimationInstanceState_Stopping;
		ai->m_targetWeight = 0;
	}

	// Wrap / reverse animation time

	switch (ai->m_playMode)
	{
		case gxAnimationPlayMode_Forward:
		{
			if (ai->m_time > ai->m_animation->m_data->m_length)
				ai->m_time = ueMod(ai->m_time, ai->m_animation->m_data->m_length);
			break;
		}
		case gxAnimationPlayMode_Backward:
		{
			if (ai->m_time < 0)
				ai->m_time = ueMod(ai->m_time, ai->m_animation->m_data->m_length);
			break;
		}
		case gxAnimationPlayMode_PingPong:
		{
			if (ai->m_time > ai->m_animation->m_data->m_length)
			{
				ai->m_time = ai->m_animation->m_data->m_length - ueMod(ai->m_time, ai->m_animation->m_data->m_length);
				ai->m_isPlayingForward = UE_FALSE;
			}
			else if (ai->m_time < 0)
			{
				ai->m_time = ueMod(ai->m_time, ai->m_animation->m_data->m_length);
				ai->m_isPlayingForward = UE_TRUE;
			}
			break;
		}
	}
}

void gxAnimationInstance_Stop(gxAnimationInstance* ai, f32 blendOutTime, ueBool destroyWhenStops)
{
	if (ai->m_state == gxAnimationInstanceState_Stopped)
		return;

	ai->m_blendOutTime = blendOutTime;
	ai->m_targetWeight = 0;
	ai->m_state = (ai->m_blendOutTime == 0) ? gxAnimationInstanceState_Stopped : gxAnimationInstanceState_Stopping;
	ai->m_destroyWhenStops = destroyWhenStops;
}

void gxAnimationInstance_AccumulateBoneTransforms(gxAnimationInstance* ai, u32 lodIndex)
{
	gxSkeleton* skeleton = ai->m_modelInstance->m_model->m_LODs[lodIndex].m_skeleton;
	const u32 numNodes = skeleton->m_numNodes;
	gxSkeletonAnimationBinding* binding = ai->m_LODs[lodIndex].m_binding;
	gxModelInstanceLOD* lod = &ai->m_modelInstance->m_LODs[lodIndex];

	ueMat44 transform;
	for (u32 i = 0; i < numNodes; i++)
	{
		const u32 boneIndex = binding->m_animationNodeIndices[i];
		if (boneIndex == U32_MAX)
			continue;

		gxAnimation_SampleAt(ai->m_animation, ai->m_time, boneIndex, transform);

		transform *= ai->m_weight;
		lod->m_nodeTransforms[i] += transform;
		gxModelInstance::m_nodeWeightsAccumulator[i] += ai->m_weight;
	}
}

f32 gxAnimationInstance_GetTime(gxAnimationInstance* ai)
{
	return ai->m_time;
}

gxAnimationInstanceState gxAnimationInstance_GetState(gxAnimationInstance* ai)
{
	return ai->m_state;
}

#include "GraphicsExt/gxSkeleton.h"
#include "GraphicsExt/gxAnimation.h"
#include "GraphicsExt/gxAnimation_Private.h"
#include "IO/ioPackage.h"
#include "IO/ioFile.h"
#include "IO/ueBuffer.h"

// gxAnimationMgr

struct gxAnimationMgrData
{
	ueAllocator* m_allocator;
	ueResourceTypeDesc m_typeDesc;
};

static gxAnimationMgrData* s_data = NULL;

// Animation

ueResource* gxAnimation_CreateFunc(const char* name)
{
	UE_NEW_RESOURCE_RET(gxAnimation, r, s_data->m_allocator, name);
	r->m_state = ueResourceState_WaitingForAsyncInit;
	return r;
}

void gxAnimation_LoadFunc(gxAnimation* r, ueBool isSyncInit)
{
	uePath path;
	ueAssets_GetAssetPath(path, ueResource_GetName(r), "asset", s_data->m_typeDesc.m_config);

	// Load package

	ioPackageLoadParams loadParams;
	loadParams.m_allocator = s_data->m_allocator;
	loadParams.m_path = path;
	r->m_package = ioPackage_Load(&loadParams);
	if (!r->m_package ||
		!ioPackage_LoadSegment(r->m_package, 0))
	{
		ueLogW("Failed to load animation '%s', reason: can't load package '%s'", ueResource_GetName(r), path);
		if (r->m_package)
		{
			ioPackage_Destroy(r->m_package);
			r->m_package = NULL;
		}
		ueResourceMgr_OnInitStageDone(r, ueResourceState_FailedToInitialize, UE_FALSE);
		return;
	}
	ioPackage_DoneLoading(r->m_package);

	// Done!

	ueResourceMgr_OnInitStageDone(r, ueResourceState_WaitingForSyncInit, UE_FALSE);
}

void gxAnimation_AsyncLoadFunc(ueAsync* job, void* userData)
{
	gxAnimation_LoadFunc((gxAnimation*) userData, UE_FALSE);
}

void gxAnimation_InitFunc_Finalize(gxAnimation* r)
{
	if (!r->m_package)
		return;

	// Get data pointer

	UE_ASSERT(ioPackage_GetSegmentSymbol(r->m_package, 0) == ueResourceType<gxAnimation>::ID);
	r->m_data = (gxAnimation::Data*) ioPackage_AcquireSegmentData(r->m_package, 0);

	// Release package

	ioPackage_Destroy(r->m_package);
	r->m_package = NULL;

	// Done!

	ueResourceMgr_OnInitStageDone(r, ueResourceState_Ready, UE_FALSE);
}

void gxAnimation_InitFunc(gxAnimation* r, ueBool isSyncInit)
{
	r->m_state = ueResourceState_Initializing;

	if (isSyncInit)
	{
		gxAnimation_LoadFunc(r, isSyncInit);
		gxAnimation_InitFunc_Finalize(r);
		return;
	}

	if (!r->m_package)
	{
#if 0
		ioJobDesc jobDesc;
		jobDesc.m_func = gxAnimation::AsyncLoadFunc;
		jobDesc.m_userData = r;
		if (!ioJob_Start(jobDesc))
			ueResourceMgr_OnInitStageDone(r, State_WaitingForAsyncInit, !isSyncInit);
#else
		UE_NOT_IMPLEMENTED();
#endif
	}
	else
		gxAnimation_InitFunc_Finalize(r);
}

void gxAnimation_DestroyFunc(gxAnimation* r)
{
	if (r->m_package)
	{
		ioPackage_Destroy(r->m_package);
		r->m_package = NULL;
	}

	if (r->m_data)
	{
		ueDelete(r->m_data, s_data->m_allocator);
		r->m_data = NULL;
	}

	ueDelete(r, s_data->m_allocator);
}

void gxAnimationMgr_Startup(ueAllocator* allocator, const char* config)
{
	UE_ASSERT(!s_data);

	s_data = new(allocator) gxAnimationMgrData();
	UE_ASSERT(s_data);
	s_data->m_allocator = allocator;

	s_data->m_typeDesc.m_id = ueResourceType<gxAnimation>::ID;
	s_data->m_typeDesc.m_name = "gxAnimation";
	s_data->m_typeDesc.m_createFunc = gxAnimation_CreateFunc;
	s_data->m_typeDesc.m_initFunc = (ueResourceInitFunc) gxAnimation_InitFunc;
	s_data->m_typeDesc.m_destroyFunc = (ueResourceDestroyFunc) gxAnimation_DestroyFunc;
	s_data->m_typeDesc.m_needsSyncDeinit = UE_FALSE;
	s_data->m_typeDesc.m_config = config;
	ueResourceMgr_RegisterType(&s_data->m_typeDesc);
}

void gxAnimationMgr_Shutdown()
{
	UE_ASSERT(s_data);
	ueResourceMgr_UnregisterType(&s_data->m_typeDesc);

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

// gxSkeleton

void gxSkeleton::DestroyAllBindings()
{
	gxSkeletonAnimationBinding* binding = m_bindings.Front();
	while (binding)
	{
		gxSkeletonAnimationBinding* next = binding->Next();

		m_bindings.Remove(binding);
		ueDelete(binding, s_data->m_allocator);

		binding = next;
	}
}

gxSkeletonAnimationBinding* gxSkeleton::AddBinding(gxAnimation* animation)
{
#if defined(UE_ENABLE_ASSERTION)
	{
		gxSkeletonAnimationBinding* binding = m_bindings.Front();
		while (binding && *binding->m_animation != animation)
			binding = binding->Next();
		UE_ASSERT(!binding);
	}
#endif

	const u32 memorySize = sizeof(gxSkeletonAnimationBinding) + sizeof(u32) * m_numNodes;
	u8* memory = (u8*) s_data->m_allocator->Alloc(memorySize);
	UE_ASSERT(memory);

	gxSkeletonAnimationBinding* binding = new(memory) gxSkeletonAnimationBinding();
	m_bindings.PushFront(binding);
	memory += sizeof(gxSkeletonAnimationBinding);

	binding->m_refCount = 0;
	binding->m_animation = animation;
	binding->m_animationNodeIndices = (u32*) memory;

	// Match bones from skeleton with bones in animation

	for (u32 i = 0; i < m_numNodes; i++)
	{
		binding->m_animationNodeIndices[i] = U32_MAX;
		for (u32 j = 0; j < animation->m_data->m_numNodes; j++)
			if (!ueStrCmp(m_nodes[i].m_name, animation->m_data->m_nodes[j].m_name))
			{
				binding->m_animationNodeIndices[i] = j;
				break;
			}
	}

	return binding;
}

void gxSkeleton::RemoveBinding(gxSkeletonAnimationBinding* binding)
{
	m_bindings.Remove(binding);
	ueDelete(binding, s_data->m_allocator);
}

void gxSkeleton::RemoveBinding(gxAnimation* animation)
{
	gxSkeletonAnimationBinding* binding = GetBinding(animation);
	UE_ASSERT(binding);
	RemoveBinding(binding);
}

gxSkeletonAnimationBinding* gxSkeleton::GetBinding(gxAnimation* animation)
{
	gxSkeletonAnimationBinding* binding = m_bindings.Front();
	while (binding && *binding->m_animation != animation)
		binding = binding->Next();
	if (!binding)
		binding = AddBinding(animation);
	binding->m_refCount++;
	return binding;
}

void gxSkeleton::ReleaseBinding(gxSkeletonAnimationBinding* binding)
{
	UE_ASSERT(binding->m_refCount > 0);
	if (--binding->m_refCount == 0)
		RemoveBinding(binding);
}

// gxAnimation

template <typename TYPE, typename TYPE_ARG, typename LERPER_TYPE>
void SampleTrack(TYPE_ARG out, const f32* times, const TYPE* keys, u32 numKeys, f32 time, f32 frequencyInv)
{
	UE_ASSERT(numKeys > 0);

	if (numKeys == 1 || time <= times[0])
	{
		out = keys[0];
		return;
	}
	if (times[numKeys - 1] <= time)
	{
		out = keys[numKeys - 1];
		return;
	}
	if (numKeys == 2)
	{
		LERPER_TYPE::Lerp(out, keys[0], keys[1], (time - times[0]) / (times[1] - times[0]));
		return;
	}

	const f32 indexF = time * frequencyInv;
	const f32 indexFloorF = ueFloor(indexF);
	const u32 indexFloor = (u32) indexFloorF;
	if (indexFloor == numKeys - 1)
	{
		out = keys[numKeys - 1];
		return;
	}

	LERPER_TYPE::Lerp(out, keys[indexFloor], keys[indexFloor + 1], indexF - indexFloorF);
}

void gxAnimation_SampleAt(gxAnimation* r, f32 time, u32 boneIndex, ueMat44& transform)
{
	const gxAnimation::Node& node = r->m_data->m_nodes[boneIndex];

	transform.SetIdentity();

	if (node.m_numScaleKeys > 0)
	{
		ueVec3 scale;
		SampleTrack<ueVec3, ueVec3&, ueVec3Lerper>(scale, node.m_scaleTimes, node.m_scaleKeys, node.m_numScaleKeys, time, node.m_scaleFrequencyInv);
		transform.Scale(scale);
	}

	if (node.m_numRotationKeys > 0)
	{
		ueQuat rotation;
		SampleTrack<ueQuat, ueQuat&, ueQuatLerper>(rotation, node.m_rotationTimes, node.m_rotationKeys, node.m_numRotationKeys, time, node.m_rotationFrequencyInv);
		transform.Rotate(rotation);
	}

	if (node.m_numTranslationKeys > 0)
	{
		ueVec3 translation;
		SampleTrack<ueVec3, ueVec3&, ueVec3Lerper>(translation, node.m_translationTimes, node.m_translationKeys, node.m_numTranslationKeys, time, node.m_translationFrequencyInv);
		transform.Translate(translation);
	}
}

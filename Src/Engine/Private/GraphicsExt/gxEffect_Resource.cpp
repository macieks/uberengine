#include "GraphicsExt/gxEffect_Private.h"
#include "Base/ueResource_Private.h"
#include "IO/ioPackage.h"
#include "IO/ioFile.h"
#include "IO/ueBuffer.h"
#include "Base/ueJobSys.h"
#include "Graphics/glLib.h"

// gxEffectType

ueResource* gxEffectType_CreateFunc(const char* name)
{
	UE_NEW_RESOURCE_RET(gxEffectType, r, s_fxSysData->m_allocator, name);
	r->m_state = ueResourceState_WaitingForAsyncInit;
	return r;
}

u32 gxEffectType_CalcInstanceSize(gxEffectTypeData* data)
{
	u32 size = (u32) sizeof(gxEffect) + (u32) sizeof(gxEmitter) * data->m_numEmitterTypes;
	for (u32 i = 0; i < data->m_numEmitterTypes; i++)
	{
		gxEmitterType* et = &data->m_emitterTypes[i];
		size += et->m_maxParticles * sizeof(gxParticle);
	}
	return size;
}

void gxEffectType_InitFunc_Finalize(gxEffectType* r)
{
	if (!r->m_package)
		return;

	UE_ASSERT(ioPackage_GetSegmentSymbol(r->m_package, 0) == ueResourceType<gxEffectType>::ID);

	// Create effect type

	r->m_data = (gxEffectTypeData*) ioPackage_AcquireSegmentData(r->m_package, 0);
	UE_ASSERT(r->m_data);
	r->m_data->m_instanceSize = gxEffectType_CalcInstanceSize(r->m_data);

	// Release package

	ioPackage_Destroy(r->m_package);
	r->m_package = NULL;

	// Done!

	ueResourceMgr_OnInitStageDone(r, ueResourceState_Ready, UE_FALSE);
}

void gxEffectType_LoadFunc(gxEffectType* r, ueBool isSyncInit)
{
	uePath path;
	ueAssets_GetAssetPath(path, ueResource_GetName(r), "asset", s_fxSysData->m_typeDesc.m_config, s_fxSysData->m_typeDesc.m_assetPathType);

	// Load package

	ioPackageLoadParams loadParams;
	loadParams.m_allocator = s_fxSysData->m_allocator;
	loadParams.m_path = path;
	r->m_package = ioPackage_Load(&loadParams);
	if (!r->m_package ||
		!ioPackage_LoadSegment(r->m_package, 0))
	{
		ueLogW("Failed to load texture '%s', reason: can't load package '%s'", ueResource_GetName(r), path);
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

	ueResourceMgr_OnInitStageDone(r, ueResourceState_WaitingForSyncInit, !isSyncInit);
}

void gxEffectType_AsyncLoadFunc(ueAsync* job, void* userData)
{
	gxEffectType_LoadFunc((gxEffectType*) userData, UE_FALSE);
}

void gxEffectType_InitFunc(gxEffectType* r, ueBool isSyncInit)
{
	r->m_state = ueResourceState_Initializing;

	if (isSyncInit)
	{
		gxEffectType_LoadFunc(r, UE_TRUE);
		gxEffectType_InitFunc_Finalize(r);
		return;
	}

	if (!r->m_package)
	{
		ueJobDesc desc;
		desc.m_enableAutoRelease = UE_TRUE;
		desc.m_workFunc = gxEffectType_AsyncLoadFunc;
		desc.m_userData = r;
		r->m_job = thGlobalJobSys_StartJob(&desc);
		if (!r->m_job)
			ueResourceMgr_OnInitStageDone(r, ueResourceState_FailedToInitialize, UE_FALSE);
	}
	else
		gxEffectType_InitFunc_Finalize(r);
}

void gxEffectType_DestroyFunc(gxEffectType* r)
{
	if (r->m_package)
	{
		ioPackage_Destroy(r->m_package);
		r->m_package = NULL;
	}

	ueDelete(r, s_fxSysData->m_allocator);
}
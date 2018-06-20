#include "GraphicsExt/gxModel.h"
#include "GraphicsExt/gxModel_Private.h"
#include "GraphicsExt/gxModel_InitData.h"
#include "Base/ueResource_Private.h"
#include "IO/ioPackage.h"
#include "Base/ueAsync.h"
#include "Graphics/glLib.h"
#include "GraphicsExt/gxModelInstance_Private.h"
#include "GraphicsExt/gxSkeleton.h"
#include "GraphicsExt/gxCollision.h"

struct gxModelMgrData
{
	ueAllocator* m_allocator;
	ueResourceTypeDesc m_typeDesc;
};

static gxModelMgrData* s_data = NULL;

// gxModel

u32 gxModel_CalcInstanceSize(gxModel* r)
{
	u32 memorySize = sizeof(gxModelInstance) + r->m_numLODs * sizeof(gxModelInstanceLOD);
	memorySize = ueAlignPow2(memorySize, (u32) UE_DEFAULT_ALIGNMENT);
	for (u32 i = 0; i < r->m_numLODs; i++)
	{
		gxSkeleton* skeleton = r->m_LODs[i].m_skeleton;
		if (skeleton)
			memorySize += skeleton->m_numNodes * sizeof(ueMat44);

		gxCollisionGeometry* collGeom = r->m_LODs[i].m_collisionGeometry;
		if (collGeom)
			memorySize += collGeom->GetInstanceSize();
	}
	return memorySize;
}

gxModelInstance* gxModel_CreateInstance(gxModel* r)
{
	// Alocate instance
		
	u8* memory = (u8*) s_data->m_allocator->Alloc(r->m_instanceSize);
	UE_ASSERT(memory);

	gxModelInstance* mi = new(memory) gxModelInstance();
	memory += sizeof(gxModelInstance);
	mi->m_LODs = (gxModelInstanceLOD*) memory;
	memory += r->m_numLODs * sizeof(gxModelInstanceLOD);

	mi->m_baseTransform.SetIdentity();

	// Initialize LODs' matrices

	memory = (u8*) ueAlignPow2(memory, UE_DEFAULT_ALIGNMENT);
	for (u32 i = 0; i < r->m_numLODs; i++)
	{
		gxModelInstanceLOD* lod = &mi->m_LODs[i];

		gxSkeleton* skeleton = r->m_LODs[i].m_skeleton;
		if (skeleton)
		{
			lod->m_nodeTransforms = (ueMat44*) memory;
			memory += skeleton->m_numNodes * sizeof(ueMat44);
			lod->m_nodeTransformsDirty = UE_TRUE;
		}
	}

	// Initialize LODs' collision geometry instances

	for (u32 i = 0; i < r->m_numLODs; i++)
	{
		gxModelInstanceLOD* lod = &mi->m_LODs[i];

		gxCollisionGeometry* collGeom = r->m_LODs[i].m_collisionGeometry;
		if (collGeom)
		{
			const u32 collGeomInstanceSize = collGeom->GetInstanceSize();
			lod->m_collGeomInstance = collGeom->CreateInstanceInPlace(memory, collGeomInstanceSize);
			memory += collGeomInstanceSize;
		}
		else
			lod->m_collGeomInstance = NULL;
	}

	mi->m_model = r;

	return mi;
}

u32 gxModel_GetNumLODs(gxModel* m)
{
	return m->m_numLODs;
}

gxModelLOD* gxModel_GetLOD(gxModel* m, u32 index)
{
	UE_ASSERT(index < m->m_numLODs);
	return m->m_LODs + index;
}

gxModel::gxModel() :
	m_numLODs(0),
	m_LODs(NULL),
	m_package(NULL)
{}

gxModel::~gxModel()
{
	UE_ASSERT(!m_LODs);
	UE_ASSERT(!m_package);
}

ueResource* gxModel_CreateFunc(const char* name)
{
	UE_NEW_RESOURCE_RET(gxModel, r, s_data->m_allocator, name);
	r->m_state = ueResourceState_WaitingForAsyncInit;
	return r;
}

void gxModel_LoadFunc(gxModel* r, ueBool isSyncInit)
{
	uePath path;
	ueAssets_GetAssetPath(path, ueResource_GetName(r), "asset", s_data->m_typeDesc.m_config);

	// Load package

	ioPackageLoadParams loadParams;
	loadParams.m_allocator = s_data->m_allocator;
	loadParams.m_path = path;
	r->m_package = ioPackage_Load(&loadParams);
	if (!r->m_package ||
		!ioPackage_LoadAllSegments(r->m_package))
	{
		ueLogW("Failed to load model '%s', reason: can't load package '%s'", ueResource_GetName(r), path);
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

void gxModel_InitFunc_Finalize(gxModel* r)
{
	if (!r->m_package)
		return;

	UE_ASSERT(ioPackage_GetSegmentSymbol(r->m_package, 0) == ueResourceType<gxModel>::ID);

	// Get main data pointer

	r->m_LODs = (gxModelLOD*) ioPackage_AcquireSegmentData(r->m_package, 0);

	// Get init data

	gxModel_InitData* initData;
	ioPackage_GetSegmentData(r->m_package, 1, (void**) &initData);

	// Initialize the model using init data and hardware buffers data

	r->m_box = initData->m_box;
	r->m_numLODs = initData->m_numLODs;

	for (u32 i = 0; i < r->m_numLODs; i++)
	{
		gxModelLOD* lod = &r->m_LODs[i];
		gxModel_InitData::LOD* initLOD = &initData->m_LODs[i];

		// Get hardware buffer data pointer

		const u32 hardwareBufferSegmentIndex = i + 2;

		u8* hardwareBuffersData;
		ioPackage_GetSegmentData(r->m_package, hardwareBufferSegmentIndex, (void**) &hardwareBuffersData);

		// Create vertex formats

		for (u32 j = 0; j < lod->m_numVertexFormats; j++)
			lod->m_vertexFormats[j] = glStreamFormat_Create(&initLOD->m_formatDescs[j]);

		// Create VB

		glVertexBufferDesc vbDesc;
		vbDesc.m_size = initLOD->m_vbDesc.m_size;
		lod->m_VB = glVertexBuffer_Create(&vbDesc, hardwareBuffersData + initLOD->m_vbDesc.m_readOffset);
		UE_ASSERT(lod->m_VB);

		// Create IB

		glIndexBufferDesc ibDesc;
		ibDesc.m_indexSize = initLOD->m_ibDesc.m_indexSize;
		ibDesc.m_numIndices = initLOD->m_ibDesc.m_numIndices;
		lod->m_IB = glIndexBuffer_Create(&ibDesc, hardwareBuffersData + initLOD->m_ibDesc.m_readOffset);
		UE_ASSERT(lod->m_IB);

		ioPackage_UnloadSegment(r->m_package, hardwareBufferSegmentIndex);

		// Create physics shape

		if (lod->m_collisionGeometry)
			lod->m_collisionGeometry->Init(initLOD->m_collisionGeomInitData);
	}

	// Calculate instance size

	r->m_instanceSize = gxModel_CalcInstanceSize(r);

	// Release package

	ioPackage_Destroy(r->m_package);
	r->m_package = NULL;

	// Done!

	ueResourceMgr_OnInitStageDone(r, ueResourceState_Ready, UE_FALSE);
}

void gxModel_AsyncLoadFunc(ueAsync* job, void* userData)
{
	gxModel_LoadFunc((gxModel*) userData, UE_FALSE);
}

void gxModel_InitFunc(gxModel* r, ueBool isSyncInit)
{
	r->m_state = ueResourceState_Initializing;

	if (isSyncInit)
	{
		gxModel_LoadFunc(r, isSyncInit);
		gxModel_InitFunc_Finalize(r);
		return;
	}

	if (!r->m_package)
	{
#if 0
		ioJobDesc jobDesc;
		jobDesc.m_func = gxModel::AsyncLoadFunc;
		jobDesc.m_userData = r;
		if (!ioJob_Start(jobDesc))
			ueResourceMgr_OnInitStageDone(r, State_WaitingForAsyncInit, !isSyncInit);
#else
		UE_NOT_IMPLEMENTED();
#endif
	}
	else
		gxModel_InitFunc_Finalize(r);
}

void gxModel_DestroyFunc(gxModel* r)
{
	if (r->m_package)
	{
		ioPackage_Destroy(r->m_package);
		r->m_package = NULL;
	}

	if (r->m_LODs)
	{
		for (u32 i = 0; i < r->m_numLODs; i++)
		{
			gxModelLOD* lod = &r->m_LODs[i];

			// Destroy animation bindings

			if (lod->m_skeleton)
				lod->m_skeleton->DestroyAllBindings();

			// Destroy textures

			for (u32 j = 0; j < lod->m_numMaterials; j++)
			{
				gxModelMaterialInfo& mat = lod->m_materials[j];
				mat.m_colorMap.Release();
			}

			// Destroy stream formats

			for (u32 j = 0; j < lod->m_numVertexFormats; j++)
				glStreamFormat_Destroy(lod->m_vertexFormats[j]);

			// Destroy vertex/index buffers

			glVertexBuffer_Destroy(lod->m_VB);
			glIndexBuffer_Destroy(lod->m_IB);

			// Destroy collision geometry

			if (lod->m_collisionGeometry)
				lod->m_collisionGeometry->Deinit();
		}

		ueDelete(r->m_LODs, s_data->m_allocator);
		r->m_LODs = NULL;
	}

	ueDelete(r, s_data->m_allocator);
}

// gxModelMgr

void gxModelMgr_Startup(ueAllocator* allocator, const char* config)
{
	UE_ASSERT(!s_data);

	s_data = new(allocator) gxModelMgrData();
	UE_ASSERT(s_data);

	s_data->m_typeDesc.m_id = ueResourceType<gxModel>::ID;
	s_data->m_typeDesc.m_name = "gxModel";
	s_data->m_typeDesc.m_createFunc = gxModel_CreateFunc;
	s_data->m_typeDesc.m_initFunc = (ueResourceInitFunc) gxModel_InitFunc;
	s_data->m_typeDesc.m_destroyFunc = (ueResourceDestroyFunc) gxModel_DestroyFunc;
	s_data->m_typeDesc.m_needsSyncDeinit = UE_TRUE;
	s_data->m_typeDesc.m_config = config;
	ueResourceMgr_RegisterType(&s_data->m_typeDesc);

	s_data->m_allocator = allocator;
}

void gxModelMgr_Shutdown()
{
	UE_ASSERT(s_data);

	ueResourceMgr_UnregisterType(&s_data->m_typeDesc);

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

void gxModelLOD_GetStats(gxModelLOD* lod, u32& numVerts, u32& numIndices, u32& numTris)
{
	numVerts = 0;
	numIndices = 0;
	numTris = 0;

	for (u32 i = 0; i < lod->m_numMeshes; i++)
	{
		const gxModelMesh& mesh = lod->m_meshes[i];
		numVerts += mesh.m_numVerts;
		numIndices += mesh.m_numIndices;
		numTris += glUtils_CalcPrimCount((glPrimitive) mesh.m_primType, mesh.m_numIndices);
	}
}

ueAllocator* gxModelMgr_GetAllocator()
{
	return s_data->m_allocator;
}
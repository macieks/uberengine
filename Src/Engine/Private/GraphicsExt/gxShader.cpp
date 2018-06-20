#include "GraphicsExt/gxShader.h"
#include "Base/ueResource_Private.h"
#include "IO/ioPackage.h"
#include "IO/ioFile.h"
#include "Graphics/glLib.h"

struct gxShaderMgrData
{
	ueAllocator* m_allocator;
	ueResourceTypeDesc m_typeDesc;
};

static gxShaderMgrData* s_data = NULL;

// gxShader

struct gxShader : ueResource
{
	gxShader();
	~gxShader();

	glShader* m_shader;

	// Loading only data

	ioPackage* m_package;
};

glShader* gxShader_GetShaderObject(gxShader* s) { return s->m_shader; }

gxShader::gxShader() :
	m_shader(NULL),
	m_package(NULL)
{}

gxShader::~gxShader()
{
	UE_ASSERT(!m_shader);
}

ueResource* gxShader_CreateFunc(const char* name)
{
	UE_NEW_RESOURCE_RET(gxShader, r, s_data->m_allocator, name);
	r->m_state = ueResourceState_WaitingForAsyncInit;
	return r;
}

void gxShader_LoadFunc(gxShader* r, ueBool isSyncInit)
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
		ueLogW("Failed to load shader '%s', reason: can't load package '%s'", ueResource_GetName(r), path);
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
	ueResourceMgr_OnInitStageDone(r, ueResourceState_Ready, UE_FALSE);
}

void gxShader_AsyncLoadFunc(ueAsync* job, void* userData)
{
	gxShader_LoadFunc((gxShader*) userData, UE_FALSE);
}

void gxShader_InitFunc_Finalize(gxShader* r)
{
	if (!r->m_package)
		return;

	UE_ASSERT(ioPackage_GetSegmentSymbol(r->m_package, 0) == ueResourceType<gxShader>::ID);

	glShaderDesc* shaderDesc;
	ioPackage_GetSegmentData(r->m_package, 0, (void**) &shaderDesc);

	// Create shader

	r->m_shader = glShader_Create(shaderDesc);
	if (!r->m_shader)
	{
		ioPackage_Destroy(r->m_package);
		r->m_package = NULL;

		ueResourceMgr_OnInitStageDone(r, ueResourceState_FailedToInitialize, UE_FALSE);
		return;
	}

	// Release package
	
	ioPackage_Destroy(r->m_package);
	r->m_package = NULL;

	// Done!

	ueResourceMgr_OnInitStageDone(r, ueResourceState_Ready, UE_FALSE);
}

void gxShader_InitFunc(gxShader* r, ueBool isSyncInit)
{
	r->m_state = ueResourceState_Initializing;

	if (isSyncInit)
	{
		gxShader_LoadFunc(r, isSyncInit);
		gxShader_InitFunc_Finalize(r);
		return;
	}

	if (!r->m_package)
	{
#if 0
		ioJobDesc jobDesc;
		jobDesc.m_func = gxShader::AsyncLoadFunc;
		jobDesc.m_userData = r;
		if (!ioJob_Start(jobDesc))
			ueResourceMgr_OnInitStageDone(r, State_WaitingForAsyncInit, !isSyncInit);
#else
		UE_NOT_IMPLEMENTED();
#endif
	}
	else
		gxShader_InitFunc_Finalize(r);
}

void gxShader_DestroyFunc(gxShader* r)
{
	if (r->m_package)
	{
		ioPackage_Destroy(r->m_package);
		r->m_package;
	}

	if (r->m_shader)
	{
		glShader_Destroy(r->m_shader);
		r->m_shader = NULL;
	}

	ueDelete(r, s_data->m_allocator);
}

// gxShaderMgr

void gxShaderMgr_Startup(ueAllocator* allocator, const char* config)
{
	UE_ASSERT(!s_data);

	s_data = new(allocator) gxShaderMgrData();
	UE_ASSERT(s_data);
	s_data->m_allocator = allocator;

	s_data->m_typeDesc.m_id = ueResourceType<gxShader>::ID;
	s_data->m_typeDesc.m_name = "gxShader";
	s_data->m_typeDesc.m_createFunc = gxShader_CreateFunc;
	s_data->m_typeDesc.m_initFunc = (ueResourceInitFunc) gxShader_InitFunc;
	s_data->m_typeDesc.m_destroyFunc = (ueResourceDestroyFunc) gxShader_DestroyFunc;
	s_data->m_typeDesc.m_needsSyncDeinit = UE_TRUE;
	s_data->m_typeDesc.m_config = config;
	ueResourceMgr_RegisterType(&s_data->m_typeDesc);
}

void gxShaderMgr_Shutdown()
{
	UE_ASSERT(s_data);

	ueResourceMgr_UnregisterType(&s_data->m_typeDesc);

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}
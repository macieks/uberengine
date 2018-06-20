#include "GraphicsExt/gxTexture.h"
#include "Base/ueResource_Private.h"
#include "IO/ioPackage.h"
#include "IO/ioFile.h"
#include "IO/ueBuffer.h"
#include "Base/ueJobSys.h"
#include "Graphics/glLib.h"

#ifdef UE_MARMALADE
	#include "MessageBox.h"
#endif

struct gxTextureMgrData
{
	ueAllocator* m_allocator;
	ueResourceTypeDesc m_typeDesc;
};

static gxTextureMgrData* s_data = NULL;

// gxTexture

//! Texture resource
struct gxTexture : ueResource
{
	gxTexture();
	~gxTexture();

	static ueResource* CreateFunc(const char* name);
	static void InitAtlasEntry(gxTexture* t);
	static void InitAtlas(gxTexture* t);
	static void InitFunc_Finalize(gxTexture* r);
	static void LoadFunc(gxTexture* r, ueBool isSyncInit);
	static void AsyncLoadFunc(ueAsync* job, void* userData);
	static void InitFunc(gxTexture* r, ueBool isSyncInit);
	static void DestroyFunc(gxTexture* r);

	glTextureBuffer* m_tb;				//!< The actual texture buffer

	// Texture atlasing

	ueBool m_isAtlas;
	ueBool m_isAtlasEntry;
	gxTextureAtlasInfo* m_atlas;		//!< Optional texture atlas (in case this texture is texture atlas or belongs to texture atlas)
	gxTextureAtlasEntry* m_atlasEntry;	//!< Optional texture atlas entry (in case this texture belongs to texture atlas)

	// Used when loading

	ueAsync* m_job;
	ioPackage* m_package;
};

glTextureBuffer* gxTexture_GetBuffer(gxTexture* t) { return t->m_tb; }

u32 gxTexture_GetWidth(const gxTexture* t)
{
	return gxTexture_IsInAtlas(t) ? t->m_atlasEntry->m_rect.m_width : glTextureBuffer_GetDesc(t->m_tb)->m_width;
}

u32 gxTexture_GetHeight(const gxTexture* t)
{
	return gxTexture_IsInAtlas(t) ? t->m_atlasEntry->m_rect.m_height : glTextureBuffer_GetDesc(t->m_tb)->m_height;
}

gxTextureAtlasEntry* gxTexture_GetAtlasEntry(gxTexture* t) { return t->m_atlasEntry; }
ueBool gxTexture_IsAtlas(const gxTexture* t) { return t->m_isAtlas; }
ueBool gxTexture_IsInAtlas(const gxTexture* t) { return t->m_isAtlasEntry; }

gxTexture::gxTexture() :
	m_tb(NULL),
	m_package(NULL),
	m_isAtlas(UE_FALSE),
	m_isAtlasEntry(UE_FALSE),
	m_atlas(NULL),
	m_atlasEntry(NULL)
{}

gxTexture::~gxTexture()
{
	UE_ASSERT(!m_package);
}

ueResource* gxTexture::CreateFunc(const char* name)
{
	UE_NEW_RESOURCE_RET(gxTexture, r, s_data->m_allocator, name);
	r->m_state = ueResourceState_WaitingForAsyncInit;
	return r;
}

void gxTexture::InitAtlasEntry(gxTexture* t)
{
	UE_ASSERT(ioPackage_GetSegmentSymbol(t->m_package, 0) == gxTexture_ATLAS_ENTRY_TYPE_ID);

	// Get texture atlas name

	char* atlasName;
	ioPackage_GetSegmentData(t->m_package, 0, (void**) &atlasName);

	// Load texture atlas (this will also initialize this atlas entry)

	gxTexture* atlasTexture = (gxTexture*) ueResourceMgr_Get(ueResourceType<gxTexture>::ID, atlasName, UE_TRUE);
	UE_ASSERT(atlasTexture);
}

void gxTexture::InitAtlas(gxTexture* t)
{
	if (ioPackage_GetNumSegments(t->m_package) < 2)
		return;

	// Get segment data
	
	UE_ASSERT(ioPackage_GetSegmentSymbol(t->m_package, 1) == gxTexture_ATLAS_TYPE_ID);
	t->m_atlas = (gxTextureAtlasInfo*) ioPackage_AcquireSegmentData(t->m_package, 1);
	UE_ASSERT(t->m_atlas);

	t->m_isAtlas = UE_TRUE;

	// Create texture for each entry

	for (u32 i = 0; i < t->m_atlas->m_numEntries; i++)
	{
		gxTextureAtlasEntry* entry = &t->m_atlas->m_entries[i];

		// Check for special case atlas (single texture)

		if (!entry->m_name || !entry->m_name[0])
		{
			entry->m_texture = t;
			t->m_isAtlasEntry = UE_TRUE;
			t->m_atlasEntry = entry;
			UE_ASSERT(t->m_atlas->m_numEntries == 1);
			break;
		}

		// Check if resource exists first (it's possible that it has been created in the function requesting atlas creation)

		gxTexture* existingEntryTexture = (gxTexture*) ueResourceMgr_Get(ueResourceType<gxTexture>::ID, entry->m_name, UE_FALSE, UE_FALSE);
		if (existingEntryTexture)
			entry->m_texture = existingEntryTexture;
		else
		{
			entry->m_texture = new(s_data->m_allocator) gxTexture();
			entry->m_texture->m_type = &s_data->m_typeDesc;
			entry->m_texture->m_refCount = 1;
			entry->m_texture->m_state = ueResourceState_Ready;
			entry->m_texture->m_isAtlasEntry = UE_TRUE;
		}

		entry->m_texture->m_name = entry->m_name;
		entry->m_texture->m_tb = t->m_tb;
		entry->m_texture->m_atlas = t->m_atlas;
		entry->m_texture->m_atlasEntry = entry;

		if (!existingEntryTexture)
			ueResourceMgr_Add(entry->m_texture);
	}
}

void gxTexture::InitFunc_Finalize(gxTexture* r)
{
	if (!r->m_package)
		return;

	const u32 packageSymbol = ioPackage_GetSegmentSymbol(r->m_package, 0);

	// Texture or atlas

	if (packageSymbol == ueResourceType<gxTexture>::ID)
	{
		glTextureFileDesc* textureFileDesc;
		ioPackage_GetSegmentData(r->m_package, 0, (void**) &textureFileDesc);

		// Create texture

		r->m_tb = glTextureBuffer_Create(textureFileDesc);
		if (!r->m_tb)
		{
			ioPackage_Destroy(r->m_package);
			r->m_package = NULL;

			ueResourceMgr_OnInitStageDone(r, ueResourceState_FailedToInitialize, UE_FALSE);
			return;
		}

		glTextureBuffer_SetDebugName(r->m_tb, r->m_name);

		// Optionally load texture atlas description and create textures within atlas

		gxTexture::InitAtlas(r);
	}

	// Otherwise it's a texture atlas entry

	else
	{
		UE_ASSERT(packageSymbol == gxTexture_ATLAS_ENTRY_TYPE_ID);
		gxTexture::InitAtlasEntry(r);
	}

	// Release package

	ioPackage_Destroy(r->m_package);
	r->m_package = NULL;

	// Done!

	ueResourceMgr_OnInitStageDone(r, ueResourceState_Ready, UE_FALSE);
}

void gxTexture::LoadFunc(gxTexture* r, ueBool isSyncInit)
{
	uePath path;
	ueAssets_GetAssetPath(path, ueResource_GetName(r), "asset", s_data->m_typeDesc.m_config, s_data->m_typeDesc.m_assetPathType);

	// Load package

	ioPackageLoadParams loadParams;
	loadParams.m_allocator = s_data->m_allocator;
	loadParams.m_path = path;
	r->m_package = ioPackage_Load(&loadParams);
	if (!r->m_package ||
		!ioPackage_LoadAllSegments(r->m_package))
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

void gxTexture::AsyncLoadFunc(ueAsync* job, void* userData)
{
	gxTexture::LoadFunc((gxTexture*) userData, UE_FALSE);
}

void gxTexture::InitFunc(gxTexture* r, ueBool isSyncInit)
{
	r->m_state = ueResourceState_Initializing;

	if (isSyncInit)
	{
		gxTexture::LoadFunc(r, UE_TRUE);
		gxTexture::InitFunc_Finalize(r);
		return;
	}

	if (!r->m_package)
	{
		ueJobDesc desc;
		desc.m_enableAutoRelease = UE_TRUE;
		desc.m_workFunc = gxTexture::AsyncLoadFunc;
		desc.m_userData = r;
		r->m_job = thGlobalJobSys_StartJob(&desc);
		if (!r->m_job)
			ueResourceMgr_OnInitStageDone(r, ueResourceState_FailedToInitialize, UE_FALSE);
	}
	else
		gxTexture::InitFunc_Finalize(r);
}

void gxTexture::DestroyFunc(gxTexture* r)
{
	if (r->m_package)
	{
		ioPackage_Destroy(r->m_package);
		r->m_package = NULL;
	}

	if (r->m_tb && (!r->m_isAtlasEntry || r->m_isAtlas))
	{
		glTextureBuffer_Destroy(r->m_tb);
		r->m_tb = NULL;
	}

	if (r->m_isAtlas)
	{
		if (!r->m_isAtlasEntry)
			for (u32 i = 0; i < r->m_atlas->m_numEntries; i++)
			{
				gxTextureAtlasEntry* entry = &r->m_atlas->m_entries[i];
				UE_ASSERT(entry->m_texture->m_refCount == 1);
				ueResourceMgr_Release(entry->m_texture);
			}

		s_data->m_allocator->Free(r->m_atlas);
		r->m_atlas = NULL;
	}

	if (r->m_isAtlasEntry)
	{
		r->m_atlas = NULL;
		r->m_atlasEntry = NULL;
	}

	ueDelete(r, s_data->m_allocator);
}

// gxTextureMgr

void gxTextureMgr_Startup(ueAllocator* allocator, const char* config, ueBool supportLocalization)
{
	UE_ASSERT(!s_data);

	s_data = new(allocator) gxTextureMgrData();
	UE_ASSERT(s_data);
	s_data->m_allocator = allocator;

	s_data->m_typeDesc.m_id = ueResourceType<gxTexture>::ID;
	s_data->m_typeDesc.m_name = "gxTexture";
	s_data->m_typeDesc.m_createFunc = gxTexture::CreateFunc;
	s_data->m_typeDesc.m_initFunc = (ueResourceInitFunc) gxTexture::InitFunc;
	s_data->m_typeDesc.m_destroyFunc = (ueResourceDestroyFunc) gxTexture::DestroyFunc;
	s_data->m_typeDesc.m_needsSyncDeinit = UE_TRUE;
	s_data->m_typeDesc.m_config = config;
	s_data->m_typeDesc.m_assetPathType = supportLocalization ? ueAssetPath_LocalizedFirst : ueAssetPath_NonlocalizedOnly;
	ueResourceMgr_RegisterType(&s_data->m_typeDesc);
}

void gxTextureMgr_Shutdown()
{
	UE_ASSERT(s_data);

	ueResourceMgr_UnregisterType(&s_data->m_typeDesc);

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}
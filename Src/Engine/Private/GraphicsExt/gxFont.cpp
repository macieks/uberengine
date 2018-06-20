#include "GraphicsExt/gxFont.h"
#include "GraphicsExt/gxFont_Private.h"
#include "IO/ioPackage.h"
#include "IO/ioFile.h"
#include "IO/ueBuffer.h"
#include "Graphics/glLib.h"

struct gxFontMgrData
{
	ueAllocator* m_allocator;
	ueResourceTypeDesc m_typeDesc;
};

static gxFontMgrData* s_data = NULL;

// gxFont

gxFont::gxFont() :
	m_tb(NULL),
	m_package(NULL)
{}

gxFont::~gxFont()
{
	UE_ASSERT(!m_package);
	UE_ASSERT(!m_tb);
}

ueResource* gxFont_CreateFunc(const char* name)
{
	UE_NEW_RESOURCE_RET(gxFont, r, s_data->m_allocator, name);
	r->m_state = ueResourceState_WaitingForAsyncInit;
	return r;
}

void gxFont_LoadFunc(gxFont* r, ueBool isSyncInit)
{
	uePath path;
	ueAssets_GetAssetPath(path, ueResource_GetName(r), "asset", s_data->m_typeDesc.m_config);

	// Load package

	ioPackageLoadParams loadParams;
	loadParams.m_allocator = s_data->m_allocator;
	loadParams.m_path = path;
	r->m_package = ioPackage_Load(&loadParams);
	if (!r->m_package ||
		!ioPackage_LoadSegment(r->m_package, 0) ||
		!ioPackage_LoadSegment(r->m_package, 1))
	{
		ueLogW("Failed to load font '%s', reason: can't load package '%s'", ueResource_GetName(r), path);
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

void gxFont_AsyncLoadFunc(ueAsync* job, void* userData)
{
	gxFont_LoadFunc((gxFont*) userData, UE_FALSE);
}

void gxFont_InitFunc_Finalize(gxFont* r)
{
	if (!r->m_package)
		return;

	// Read font data

	UE_ASSERT(ioPackage_GetSegmentSymbol(r->m_package, 0) == gxFont::TYPE_ID);

	void* fontData;
	u32 fontDataSize;
	ioPackage_GetSegmentData(r->m_package, 0, &fontData, &fontDataSize);
	ueBuffer buffer(fontData, fontDataSize, ueBuffer::Mode_Read);

	UE_ASSERT_FUNC( buffer.ReadAny<u32>(r->m_sizeInPixels) );
	UE_ASSERT_FUNC( buffer.ReadAny<u32>(r->m_options) );

	u32 numGlyphs;
	UE_ASSERT_FUNC( buffer.ReadAny<u32>(numGlyphs) );

	UE_ASSERT_FUNC( r->m_glyphs.Init(s_data->m_allocator, numGlyphs) );

	f32 fixedAdvanceX = 0;
	u32 code;
	gxFontGlyph glyph;
	for (u32 i = 0; i < numGlyphs; i++)
	{
		UE_ASSERT_FUNC( buffer.ReadAny<u32>(code) );
		UE_ASSERT_FUNC( buffer.ReadAny<gxFontGlyph>(glyph) );

		r->m_glyphs.Insert(code, glyph);

		// Determine fixed font width (check if equal among all characters)

		if (fixedAdvanceX != -1.0f)
		{
			if (fixedAdvanceX == 0.0f)
				fixedAdvanceX = glyph.m_advanceX;
			else if (fixedAdvanceX != glyph.m_advanceX)
				fixedAdvanceX = -1.0f;
		}
	}

	if (fixedAdvanceX > 0.0f)
		r->m_widthInPixels = (u32) fixedAdvanceX;
	else
		r->m_widthInPixels = 0;

	// Read texture data

	void* textureData;
	u32 textureDataSize;
	ioPackage_GetSegmentData(r->m_package, 1, &textureData, &textureDataSize);

	r->m_tb = glTextureBuffer_Create((const glTextureFileDesc*) textureData);
	if (!r->m_tb)
	{
		ioPackage_Destroy(r->m_package);
		r->m_package = NULL;

		ueResourceMgr_OnInitStageDone(r, ueResourceState_FailedToInitialize, UE_FALSE);
		return;
	}

	glTextureBuffer_SetDebugName(r->m_tb, r->m_name);

	// Release package

	ioPackage_Destroy(r->m_package);
	r->m_package = NULL;

	// Done!

	ueResourceMgr_OnInitStageDone(r, ueResourceState_Ready, UE_FALSE);
}

void gxFont_InitFunc(gxFont* r, ueBool isSyncInit)
{
	r->m_state = ueResourceState_Initializing;

	if (isSyncInit)
	{
		gxFont_LoadFunc(r, isSyncInit);
		gxFont_InitFunc_Finalize(r);
		return;
	}

	if (!r->m_package)
	{
#if 0
		ioJobDesc jobDesc;
		jobDesc.m_func = gxFont_AsyncLoadFunc;
		jobDesc.m_userData = r;
		if (!ioJob_Start(jobDesc))
			ueResourceMgr_OnInitStageDone(r, State_WaitingForAsyncInit, !isSyncInit);
#else
		UE_NOT_IMPLEMENTED();
#endif
	}
	else
		gxFont_InitFunc_Finalize(r);
}

void gxFont_DestroyFunc(gxFont* r)
{
	if (r->m_package)
	{
		ioPackage_Destroy(r->m_package);
		r->m_package = NULL;
	}

	if (r->m_tb)
	{
		glTextureBuffer_Destroy(r->m_tb);
		r->m_tb = NULL;
	}

	r->m_glyphs.Deinit();

	ueDelete(r, s_data->m_allocator);
}

ueBool gxFont_IsFixedWidth(const gxFont* r) { return r->m_widthInPixels != 0; }
u32 gxFont_GetWidthInPixels(const gxFont* r) { return r->m_widthInPixels; }
u32 gxFont_GetSizeInPixels(const gxFont* r) { return r->m_sizeInPixels; }

const gxFontGlyph* gxFont_GetGlyphForCode(gxFont* r, u32 code) { return r->m_glyphs.Find(code); }

// gxFontMgr

void gxFontMgr_Startup(ueAllocator* allocator, const char* config)
{
	UE_ASSERT(!s_data);

	s_data = new(allocator) gxFontMgrData();
	UE_ASSERT(s_data);
	s_data->m_allocator = allocator;

	s_data->m_typeDesc.m_id = gxFont::TYPE_ID;
	s_data->m_typeDesc.m_name = "gxFont";
	s_data->m_typeDesc.m_createFunc = gxFont_CreateFunc;
	s_data->m_typeDesc.m_initFunc = (ueResourceInitFunc) gxFont_InitFunc;
	s_data->m_typeDesc.m_destroyFunc = (ueResourceDestroyFunc) gxFont_DestroyFunc;
	s_data->m_typeDesc.m_needsSyncDeinit = UE_TRUE;
	s_data->m_typeDesc.m_config = config;
	ueResourceMgr_RegisterType(&s_data->m_typeDesc);
}

void gxFontMgr_Shutdown()
{
	UE_ASSERT(s_data);

	ueResourceMgr_UnregisterType(&s_data->m_typeDesc);

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}
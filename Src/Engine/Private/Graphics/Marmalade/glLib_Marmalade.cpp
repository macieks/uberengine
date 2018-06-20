#include "GL/Marmalade/glLib_Private_Marmalade.h"
#include "GL/glLib_Private.h"
#include "IO/ioFile.h"

glDevice s_dev;

// Device

u32 glDevice_GetUsedTextureMemory()
{
	return s_dev.m_textureMemory;
}

void glDevice_PreStartup()
{
	// Do nothing
}

void glDevice_EnableFakeTextureDownsize2x()
{
	s_dev.m_fakeDownsizeTextures2x = UE_TRUE;
}

void glDevice_Startup(const glDeviceStartupParams* params)
{
	glDevice_Set(&s_dev);

	s_dev.m_stackAllocator = params->m_stackAllocator;
	s_dev.m_freqAllocator = params->m_freqAllocator;

	s_dev.m_startupParams = *params;
	s_dev.m_defCtx = &s_dev.m_ctx;

	s_dev.m_textureMemory = 0;

	ueAssertFunc(GLDEV->m_textureBuffersPool.Init(GL_STACK_ALLOC, sizeof(glTextureBuffer), params->m_maxTextureBuffers, UE_DEFAULT_ALIGNMENT));

	// Init Gx module

	IwGxInit();
	IwGxSetSortMode(IW_GX_SORT_NONE);

	s_dev.m_startupParams.m_width = IwGxGetDisplayWidth();
	s_dev.m_startupParams.m_height = IwGxGetDisplayHeight();

	// Set default render states

	IwGxSetColClear(0, 0, 0, 0);
	IwGxLightingOff();
	IwGxSetFarZNearZ(0x1000, 0x10);

	// Set up caps

	s_dev.m_caps.m_supportsOcclusionQuery = UE_FALSE;
	s_dev.m_caps.m_supportsPredicateQuery = UE_FALSE;
	s_dev.m_caps.m_supportsIndependentColorWriteMasks = UE_FALSE;
	s_dev.m_caps.m_supportsTwoSidedStencil = UE_FALSE;
	s_dev.m_caps.m_hasHalfPixelOffset = UE_FALSE;

	s_dev.m_caps.m_maxVertexStreams = 1;
	s_dev.m_caps.m_maxColorRenderTargets = 1;

	s_dev.m_caps.m_numSupportedMultisamplingLevels = 1;
	s_dev.m_caps.m_supportedMultisamplingLevels[0] = 1;
	s_dev.m_caps.m_maxAnisotropicFilteringlevel = 1;

	s_dev.m_caps.m_maxPrimitives = 1 << 16;
	s_dev.m_caps.m_maxUserClippingPlanes = 0;
}

void glDevice_Shutdown()
{
	ueAssertMsgP(GLDEV->m_textureBuffersPool.Size() == 0, "Unfreed glTextureBuffer instances (count = %u).", GLDEV->m_textureBuffersPool.Size());
	GLDEV->m_textureBuffersPool.Deinit();

	IwGxTerminate();
	glDevice_Set(NULL);
}

// Context

void glCtx_SetViewport(glCtx* ctx, const glViewportDesc& desc)
{
}

ueBool glCtx_Begin(glCtx* ctx)
{
	IwGxClear(IW_GX_COLOUR_BUFFER_F | IW_GX_DEPTH_BUFFER_F); // On iOS must also clear depth even if unused
	return UE_TRUE;
}

void glCtx_End(glCtx* ctx)
{
	IwGxFlush();
    IwGxSwapBuffers();
}

// Utils

void glUtils_ToMarmaladeMatrix(CIwFMat* dst, const ueMat44* src)
{
	ueVec4 tmp;
	for (u32 i = 0; i < 3; i++)
	{
		ueMat44_GetColumn(tmp, *src, i);
		ueVec4_GetPtr(dst->m[i], tmp);
	}

	ueVec3 translation;
	ueMat44_GetTranslation(translation, *src);
	ueVec3_GetPtr(&dst->t.x, translation);
}

void glUtils_FMatToMat(CIwMat& fixedMat, const CIwFMat& fmat)
{
    for(int y = 0; y < 4; y++)
    {
        for(int x = 0; x < 3; x++)
        {
            if( y < 3 )
            {
                // rotational components are based on fixed point
                fixedMat.m[x][y] = (int)(fmat.m[x][y] * IW_GEOM_ONE);
            }
            else
            {
                // translation components are based on whole part of float translation
                fixedMat.t[x] = (int)fmat.t[x] * (1 << 4);
            }
        }
    }
}

void glUtils_ToMarmaladeMatrix(CIwMat* dst, const ueMat44* src)
{
	CIwFMat tmp;
	glUtils_ToMarmaladeMatrix(&tmp, src);
	glUtils_FMatToMat(*dst, tmp);
}

CIwImage::Format glUtils_ToMarmaladeFormat(glBufferFormat format)
{
	switch (format)
	{
		case glBufferFormat_A8: return CIwImage::A_8;
		case glBufferFormat_A4R4G4B4: return CIwImage::ABGR_4444;
		case glBufferFormat_R5G6B5: return CIwImage::RGB_565;
		case glBufferFormat_R8G8B8: return CIwImage::RGB_888;
		case glBufferFormat_B8G8R8: return CIwImage::BGR_888;
		case glBufferFormat_A8B8G8R8: return CIwImage::ABGR_8888;
		case glBufferFormat_A8R8G8B8: return CIwImage::ARGB_8888;
		case glBufferFormat_PVRTC2_RGB:
		case glBufferFormat_PVRTC2_RGBA: return CIwImage::PVRTC_2;
		case glBufferFormat_PVRTC4_RGB:
		case glBufferFormat_PVRTC4_RGBA: return CIwImage::PVRTC_4;
		UE_INVALID_CASE(format);
	}
	return CIwImage::ARGB_8888;
}
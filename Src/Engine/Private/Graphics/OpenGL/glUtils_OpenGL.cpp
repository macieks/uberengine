#include "Base/ueBase.h"

#if defined(GL_OPENGL)

#include "Graphics/glLib_Private.h"
#include "Base/Containers/ueHashMap.h"
#include "Utils/ueWindow.h"

struct glInternalAdapterInfo : glAdapterInfo
{
#ifdef GL_OPENGL_ES
	// Nothing
#elif defined(UE_WIN32)
	HMONITOR m_monitor;
#endif
};

static u32 s_numAdapters = 0;
static glInternalAdapterInfo s_adapters[16];

#if defined(UE_WIN32) && !defined(GL_OPENGL_ES)

BOOL CALLBACK glUtils_MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	// Get monitor info

	MONITORINFOEX monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &monitorInfo);

	// Get associated device

	DISPLAY_DEVICE device;
	device.cb = sizeof(device);
	EnumDisplayDevices(monitorInfo.szDevice, 0, &device, 0);

	// Determine number of display modes

	DEVMODE dm;
	dm.dmSize = sizeof(dm);

	u32 numModes = 0;
	while (EnumDisplaySettings(monitorInfo.szDevice, numModes, &dm))
		numModes++;
	if (numModes == 0)
		return TRUE; // Skip adapter without any display modes

	// Add adapter info

	glInternalAdapterInfo& adapter = s_adapters[s_numAdapters++];
	adapter.m_monitor = hMonitor;
	adapter.m_numModes = numModes;
	ueStrCpyS(adapter.m_driver, "unknown");
	ueStrCpyS(adapter.m_description, device.DeviceString);
	ueStrCpyS(adapter.m_deviceName, device.DeviceName);
	adapter.m_flags =
		((device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) ? glAdapterInfo::Flags_IsPrimary : 0) |
		glAdapterInfo::Flags_HasMonitor |
		((monitorInfo.dwFlags & MONITORINFOF_PRIMARY) ? glAdapterInfo::Flags_HasPrimaryMonitor : 0);
	adapter.m_monitorWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	adapter.m_monitorHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
	adapter.m_aspectRatio = (f32) adapter.m_monitorWidth / (f32) adapter.m_monitorHeight;

	return TRUE;
}

#endif // defined(UE_WIN32)

void glUtils_BuildAdapterList()
{
	s_numAdapters = 0;

#if defined(GL_OPENGL_ES)

	glInternalAdapterInfo& adapter = s_adapters[s_numAdapters++];
	adapter.m_numModes = 1;
	ueStrCpyS(adapter.m_driver, "OpenGLES");
	ueStrCpyS(adapter.m_description, "OpenGLES");
	ueStrCpyS(adapter.m_deviceName, "OpenGLES");
	adapter.m_flags = glAdapterInfo::Flags_IsPrimary | glAdapterInfo::Flags_HasMonitor | glAdapterInfo::Flags_HasPrimaryMonitor;
	adapter.m_monitorWidth = 960;//IwGLGetInt(IW_GL_WIDTH);
	adapter.m_monitorHeight = 640;//IwGLGetInt(IW_GL_HEIGHT);
	adapter.m_aspectRatio = (f32) adapter.m_monitorWidth / (f32) adapter.m_monitorHeight;

#elif defined(UE_WIN32)

	EnumDisplayMonitors(NULL, NULL, glUtils_MonitorEnumProc, NULL);

#else
	UE_ASSERT(0);
#endif
	UE_ASSERT(s_numAdapters > 0);
}

u32 glUtils_GetNumAdapters()
{
	UE_ASSERT_MSG(s_numAdapters > 0, "Need to pre-startup render device - call glDevice::PreStartup()");
	return s_numAdapters;
}

ueBool glUtils_GetAdapterInfo(u32 adapterIndex, glAdapterInfo* info)
{
	UE_ASSERT_MSG(s_numAdapters > 0, "Need to pre-startup render device - call glDevice::PreStartup()");
	UE_ASSERT(adapterIndex < s_numAdapters);
	ueMemCpy(info, &s_adapters[adapterIndex], sizeof(glAdapterInfo));
	return UE_TRUE;
}

ueBool glUtils_GetOutputMode(u32 adapterIndex, u32 modeIndex, glOutputMode* mode)
{
	UE_ASSERT_MSG(s_numAdapters > 0, "Need to pre-startup render device - call glDevice::PreStartup()");
	UE_ASSERT(adapterIndex < s_numAdapters);
	UE_ASSERT(modeIndex < s_adapters[adapterIndex].m_numModes);

#if defined(UE_WIN32)
	DEVMODE dm;
	dm.dmSize = sizeof(dm);
	if (!EnumDisplaySettings(s_adapters[adapterIndex].m_deviceName, modeIndex, &dm))
		return UE_FALSE;

	mode->m_width = dm.dmPelsWidth;
	mode->m_height = dm.dmPelsHeight;
	mode->m_frequency = dm.dmDisplayFrequency;
	mode->m_bitsPerPixel = dm.dmBitsPerPel;
#elif defined(UE_MARMALADE)
	mode->m_width = 960;
	mode->m_height = 640;
	mode->m_frequency = 60;
	mode->m_bitsPerPixel = 32;
#else
	UE_ASSERT(0);
#endif
	return UE_TRUE;
}

ueBool glUtils_ToOpenGLFormat(glBufferFormat src, u32 textureFlags, glOpenGLBufferFormat& dst)
{
	const glDevice* dev = glDevice_Get();
	const glCaps* caps = glDevice_GetCaps();

	if ((textureFlags & glTextureBufferFlags_IsTexture) && (textureFlags & glTextureBufferFlags_IsRenderTarget))
	{
		dst = dev->m_bufferFormatMappings[src].m_rtTexFormat;
		return caps->m_formatInfos[src].m_supportedAsRTAndTexture;
	}
	else if (textureFlags & glTextureBufferFlags_IsTexture)
	{
		dst = dev->m_bufferFormatMappings[src].m_texFormat;
		return caps->m_formatInfos[src].m_supportedAsTextureOnly;
	}

	dst = dev->m_bufferFormatMappings[src].m_rtFormat;
	return caps->m_formatInfos[src].m_supportedAsRTOnly;
}

GLenum glUtils_ToOpenGLPrimitive(glPrimitive primitive)
{
	switch (primitive)
	{
		case glPrimitive_PointList: return GL_POINTS;
		case glPrimitive_LineList: return GL_LINES;
		case glPrimitive_LineStrip: return GL_LINE_STRIP;
		case glPrimitive_TriangleList: return GL_TRIANGLES;
		case glPrimitive_TriangleStrip: return GL_TRIANGLE_STRIP;
		UE_INVALID_CASE(primitive);
	}
	return GL_NONE;
}

void glUtils_ToOpenGLType(ueNumType src, GLenum& dst)
{
	switch (src)
	{
		case ueNumType_F32: dst = GL_FLOAT; break;
#if !defined(GL_OPENGL_ES)
		case ueNumType_F16: dst = GL_HALF_FLOAT_ARB; break;
		case ueNumType_S32: dst = GL_INT; break;
		case ueNumType_U32: dst = GL_UNSIGNED_INT; break;
#endif
		case ueNumType_S16: dst = GL_SHORT; break;
		case ueNumType_U16: dst = GL_UNSIGNED_SHORT; break;
		case ueNumType_S8: dst = GL_BYTE; break;
		case ueNumType_U8: dst = GL_UNSIGNED_BYTE; break;
#if !defined(GL_OPENGL_ES)
		case ueNumType_U10_3: dst = GL_UNSIGNED_INT_10_10_10_2; break;
#endif
		UE_INVALID_CASE(src);
	}
}

GLenum glUtils_ToOpenGLCmpFunc(glCmpFunc func)
{
	switch (func)
	{
		case glCmpFunc_Less: return GL_LESS;
		case glCmpFunc_LEqual: return GL_LEQUAL;
		case glCmpFunc_Equal: return GL_EQUAL;
		case glCmpFunc_GEqual: return GL_GEQUAL;
		case glCmpFunc_Greater: return GL_GREATER;
		case glCmpFunc_Never: return GL_NEVER;
		case glCmpFunc_Always: return GL_ALWAYS;
		UE_INVALID_CASE(func);
	}
	return GL_NONE;
}

GLenum glUtils_ToOpenGLBlendOp(glBlendOp blendOp)
{
	switch (blendOp)
	{
		case glBlendOp_Add: return GL_FUNC_ADD;
		case glBlendOp_Subtract: return GL_FUNC_SUBTRACT;
#if !defined(GL_OPENGL_ES)
		case glBlendOp_Min: return GL_MIN;
		case glBlendOp_Max: return GL_MAX;
#endif
		UE_INVALID_CASE(blendOp);
	}
	return GL_NONE;
}

GLenum glUtils_ToOpenGLBlendFunc(glBlendingFunc blendFunc)
{
	switch (blendFunc)
	{
		case glBlendingFunc_One: return GL_ONE;
		case glBlendingFunc_Zero: return GL_ZERO;
		case glBlendingFunc_SrcAlpha: return GL_SRC_ALPHA;
		case glBlendingFunc_InvSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
		case glBlendingFunc_SrcColor: return GL_SRC_COLOR;
		case glBlendingFunc_InvSrcColor: return GL_ONE_MINUS_SRC_COLOR;
		case glBlendingFunc_DstAlpha: return GL_DST_ALPHA;
		case glBlendingFunc_InvDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
		case glBlendingFunc_DstColor: return GL_DST_COLOR;
		case glBlendingFunc_InvDstColor: return GL_ONE_MINUS_DST_COLOR;
		UE_INVALID_CASE(blendFunc);
	}
	return GL_NONE;
}

GLenum glUtils_ToOpenGLStencilOp(glStencilOperation stencilOp)
{
	switch (stencilOp)
	{
		case glStencilOp_Zero: return GL_ZERO;
		case glStencilOp_Keep: return GL_KEEP;
		case glStencilOp_Replace: return GL_REPLACE;
		case glStencilOp_Incr: return GL_INCR;
		case glStencilOp_Decr: return GL_DECR;
		case glStencilOp_Invert: return GL_INVERT;
		UE_INVALID_CASE(stencilOp);
	}
	return GL_NONE;
}

GLenum glUtils_ToOpenGLCubeFace(glCubeFace face)
{
	switch (face)
	{
		case glCubeFace_PosX: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
		case glCubeFace_NegX: return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
		case glCubeFace_PosY: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
		case glCubeFace_NegY: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
		case glCubeFace_PosZ: return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
		case glCubeFace_NegZ: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
		UE_INVALID_CASE(face);
	}
	return GL_NONE;
}

GLenum glUtils_ToOpenGLTexAddr(glTexAddr addressMode)
{
	switch (addressMode)
	{
		case glTexAddr_Wrap: return GL_REPEAT;
		case glTexAddr_Clamp: return GL_CLAMP_TO_EDGE;
		case glTexAddr_ClampToBorder:
#if defined(GL_OPENGL_ES)
			return GL_CLAMP_TO_EDGE;
#else
			return GL_CLAMP_TO_BORDER;
#endif
		case glTexAddr_Mirror: return GL_MIRRORED_REPEAT;
		UE_INVALID_CASE(addressMode);
	}
	return GL_NONE;
}

void glUtils_ToOpenGLTexFilter(glTexFilter srcMinFilter, glTexFilter srcMagFilter, glTexFilter srcMipFilter, u32 numLevels, GLenum& dstMinFilter, GLenum& dstMagFilter)
{
	UE_ASSERT(numLevels);

	switch (srcMinFilter)
	{
		case glTexFilter_None:
		case glTexFilter_Nearest:
			if (numLevels == 1)
				dstMinFilter = GL_NEAREST;
			else switch (srcMipFilter)
			{
				case glTexFilter_None: dstMinFilter = GL_NEAREST; break;
				case glTexFilter_Nearest: dstMinFilter = GL_NEAREST_MIPMAP_NEAREST; break;
				case glTexFilter_Linear: dstMinFilter = GL_NEAREST_MIPMAP_LINEAR; break;
				UE_INVALID_CASE(srcMipFilter);
			}
			break;
		case glTexFilter_Linear:
			if (numLevels == 1)
				dstMinFilter = GL_LINEAR;
			else switch (srcMipFilter)
			{
				case glTexFilter_None: dstMinFilter = GL_LINEAR; break;
				case glTexFilter_Nearest: dstMinFilter = GL_LINEAR_MIPMAP_NEAREST; break;
				case glTexFilter_Linear: dstMinFilter = GL_LINEAR_MIPMAP_LINEAR; break;
				UE_INVALID_CASE(srcMipFilter);
			}
			break;
		UE_INVALID_CASE(srcMinFilter);
	}

	switch (srcMagFilter)
	{
		case glTexFilter_None:
		case glTexFilter_Nearest: dstMagFilter = GL_NEAREST; break;
		case glTexFilter_Linear: dstMagFilter = GL_LINEAR; break;
		UE_INVALID_CASE(srcMagFilter);
	}
}

const char* glUtils_GetErrorString(GLenum error)
{
	switch (error)
	{
#define CASE(value) case value: return #value;
		CASE(GL_INVALID_ENUM)
		CASE(GL_INVALID_VALUE)
		CASE(GL_INVALID_OPERATION)
#ifndef GL_OPENGL_ES
		CASE(GL_STACK_OVERFLOW)
		CASE(GL_STACK_UNDERFLOW)
#endif
		CASE(GL_OUT_OF_MEMORY)
		CASE(GL_INVALID_FRAMEBUFFER_OPERATION)
#undef CASE
	}
	return "<UNKNOWN>";
}

void glUtils_CheckGLErrors(const char* operation, const char* file, u32 line)
{
	GLenum error = glGetError();
	if (error == GL_NO_ERROR)
	{
		//ueLog("GL:: op: %s, file: %s:%u\n", operation, file, line);
		return;
	}

	ueLogE("OpenGL error: %s (%u)\nOPERATION: %s\nFILE: %s\nLINE: %u\n", glUtils_GetErrorString(error), (u32) error, operation, file, line);
	UE_DEBUG_BREAK();
}

ueBool glUtils_CheckFramebufferStatus(ueBool silentMode)
{
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	const char* statusString = NULL;
	switch (status)
	{
		// It's okay

		case GL_FRAMEBUFFER_COMPLETE_EXT:
			return UE_TRUE;

		// It's not okay

#define CASE(value) case value: statusString = #value; break;
		CASE(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT)
		CASE(GL_FRAMEBUFFER_UNSUPPORTED_EXT)
		CASE(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT)
		CASE(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT)
#ifndef GL_OPENGL_ES
		CASE(GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT)
		CASE(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT)
		CASE(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT)
#endif // GL_OPENGL_ES
#undef CASE
		default: statusString = "UNKNOWN"; break;
	}

	if (!silentMode)
		ueLogE("Invalid OpenGL framebuffer status = %s (%d)", statusString, (int) status);
	return UE_FALSE;
}

void glUtils_ReportUnsupportedFBO(glRenderGroup* group, const char* usageDesc)
{
	ueLogE("Unsupported FBO details (usage: %s): %ux%u, MSAA x %u", usageDesc, group->m_desc.m_width, group->m_desc.m_height, group->m_desc.m_MSAALevel);
	if (group->m_desc.m_hasDepthStencil)
		ueLogE("  depth-stencil buffer: format = %s, is texture = %s", ueEnumToString(glBufferFormat, group->m_desc.m_depthStencilDesc.m_format), glTextureBuffer_IsTexture(group->m_depthStencilRenderTarget) ? "YES" : "NO");
	for (u32 i = 0; i < group->m_desc.m_numColorBuffers; i++)
		ueLogE("  color buffer [%u]: format = %s", i, ueEnumToString(glBufferFormat, group->m_desc.m_colorDesc[i].m_format), glTextureBuffer_IsTexture(group->m_colorRenderTargets[i]) ? "YES" : "NO");

	char fboDesc[256];
	ueStrFormatS(fboDesc, "%ux%u, %uxAA, D: %s, C: %s, %s, %s, %s",
		group->m_desc.m_width, group->m_desc.m_height, group->m_desc.m_MSAALevel,
		group->m_desc.m_hasDepthStencil ? ueEnumToString(glBufferFormat, group->m_desc.m_depthStencilDesc.m_format) : "X",
		group->m_desc.m_numColorBuffers >= 1 ? ueEnumToString(glBufferFormat, group->m_desc.m_colorDesc[0].m_format) : "X",
		group->m_desc.m_numColorBuffers >= 2 ? ueEnumToString(glBufferFormat, group->m_desc.m_colorDesc[1].m_format) : "X",
		group->m_desc.m_numColorBuffers >= 3 ? ueEnumToString(glBufferFormat, group->m_desc.m_colorDesc[2].m_format) : "X",
		group->m_desc.m_numColorBuffers >= 4 ? ueEnumToString(glBufferFormat, group->m_desc.m_colorDesc[3].m_format) : "X");
	ueFatalErrorP("Unsupported FBO config (usage: %s): '%s'", usageDesc, fboDesc);
}

#if !defined(GL_FIXED_PIPELINE)

// Map from Cg attribute name (as exported from Cg/HLSL into GLSL) into semantic & usage

void glUtils_InitCgSemanticsMap()
{
	GLDEV->m_CgSemanticMap.Init(GL_STACK_ALLOC, 16, 32 /* Minimize likelihood of hash collisions - we want Find() to be real quick */);

#define ADD_SEMANTIC_RANGE(cgName, semantic, minUsageIndex, maxUsageIndex) GLDEV->m_CgSemanticMap.Insert(#cgName, glUsageAndSemantic(glSemantic_##semantic, minUsageIndex, maxUsageIndex, ueStrNCmp(#cgName, "gl_", 3) != 0));
#define ADD_SEMANTIC(cgName, semantic, usageIndex) ADD_SEMANTIC_RANGE(cgName, semantic, usageIndex, usageIndex)

	ADD_SEMANTIC(gl_Vertex, Position, 0)
	ADD_SEMANTIC_RANGE(POSITION, Position, 0, 7)
	ADD_SEMANTIC(BLENDWEIGHT, BoneWeights, 0)
	ADD_SEMANTIC(BLENDINDICES, BoneIndices, 0)
	ADD_SEMANTIC(gl_Normal, Normal, 0)
	ADD_SEMANTIC_RANGE(NORMAL, Normal, 0, 7) // Overlap with gl_Normal on purpose
	ADD_SEMANTIC(POINTSIZE, PointSize, 0)
	ADD_SEMANTIC_RANGE(gl_MultiTexCoord, TexCoord, 0, 31)
	ADD_SEMANTIC_RANGE(TEXCOORD, TexCoord, 0, 31)
	ADD_SEMANTIC(TANGENT, Tangent, 0)
	ADD_SEMANTIC(BINORMAL, Binormal, 0)
	ADD_SEMANTIC(TESSFACTOR, TessFactor, 0)
	ADD_SEMANTIC(gl_Color, Color, 0)
	ADD_SEMANTIC(gl_SecondaryColor, Color, 1)
	ADD_SEMANTIC_RANGE(COLOR, Color, 0, 7) // Overlap with gl_Color & gl_SecondaryColor on purpose
	ADD_SEMANTIC(gl_FogCoord, Fog, 0)
}

void glUtils_DeinitCgSemanticsMap()
{
	GLDEV->m_CgSemanticMap.Deinit();
}

void glUtils_GetUsageAndSemanticForCgAttrib(char* attrName, u32 attrNameLength, u8& semantic, u8& usageIndex, u8& isGeneric)
{
	// Get usage index

	const ueBool hasAttrIndex = ueIsDigit(attrName[attrNameLength - 1]) != 0;
	u32 attrIndex = 0;
	u32 attrMult = 1;
	while (ueIsDigit(attrName[attrNameLength - 1]))
	{
		attrIndex += (attrName[attrNameLength - 1] - '0') * attrMult;
		attrMult *= 10;
		attrNameLength--;
	}
	attrName[attrNameLength] = 0;

	// Find entry by name (without usage index)

	const glUsageAndSemantic* usageAndSemantic = GLDEV->m_CgSemanticMap.Find(attrName);
	UE_ASSERT(usageAndSemantic);
	UE_ASSERT(!hasAttrIndex || (usageAndSemantic->m_minUsage <= attrIndex && attrIndex <= usageAndSemantic->m_maxUsage));

	// Return result

	semantic = usageAndSemantic->m_semantic;
	usageIndex = hasAttrIndex ? attrIndex : usageAndSemantic->m_minUsage;
	isGeneric = usageAndSemantic->m_isGeneric;
}

#endif // !defined(GL_FIXED_PIPELINE)

#else // defined(GL_OPENGL)
	UE_NO_EMPTY_FILE
#endif
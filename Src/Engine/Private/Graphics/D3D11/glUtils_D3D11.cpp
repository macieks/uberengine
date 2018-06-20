#include "Base/ueBase.h"

#if defined(GL_D3D11)

#include "Graphics/glLib_Private.h"

u32 glUtils_GetNumAdapters()
{
	UE_ASSERT_MSG(GLDEV, "Need to pre-startup render device - call glDevice_PreStartup()");
	return GLDEV->m_numAdapters;
}

ueBool glUtils_GetAdapterInfo(u32 adapterIndex, glAdapterInfo* info)
{
	UE_ASSERT_MSG(GLDEV, "Need to pre-startup render device - call glDevice_PreStartup()");
	
	IDXGIAdapter1* adapter;
	if (GLDEV->m_dxgiFactory->EnumAdapters1(adapterIndex, &adapter) == DXGI_ERROR_NOT_FOUND)
		return UE_FALSE;

	DXGI_ADAPTER_DESC1 desc;
	if (FAILED(adapter->GetDesc1(&desc)))
	{
		adapter->Release();
		return UE_FALSE;
	}

	IDXGIOutput* output = NULL;
	info->m_numModes = 0;
	if (adapter->EnumOutputs(0, &output) == DXGI_ERROR_NOT_FOUND)
	{
		adapter->Release();
		return UE_FALSE;
	}
	adapter->Release();

	UINT numModes;
	if (FAILED(output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, NULL)))
	{
		output->Release();
		return UE_FALSE;
	}
	output->Release();

	info->m_flags = adapterIndex == 0 ? glAdapterInfo::Flags_IsPrimary : 0;
	ueStrCpyWToA(info->m_description, UE_ARRAY_SIZE(info->m_description), desc.Description);
	info->m_driver[0] = 0;
	info->m_deviceName[0] = 0;
	info->m_numModes = numModes;

	// Retrieve monitor info
#if 0
	HMONITOR monitor = D3D->GetAdapterMonitor(adapterIndex);
	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFO);
	if (!GetMonitorInfo(monitor, &monitorInfo))
	{
		info->m_monitorWidth = 0;
		info->m_monitorHeight = 0;
		return UE_FALSE;
	}

	info->m_flags |= glAdapterInfo::Flags_HasMonitor;
	info->m_flags |= (monitorInfo.dwFlags & MONITORINFOF_PRIMARY) != 0 ? glAdapterInfo::Flags_HasPrimaryMonitor : 0;
	info->m_monitorWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	info->m_monitorHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
	info->m_aspectRatio = (f32) info->m_monitorWidth / (f32) info->m_monitorHeight;
#endif
	return UE_TRUE;
}

ueBool glUtils_GetOutputMode(u32 adapterIndex, u32 modeIndex, glOutputMode* mode)
{
	UE_ASSERT_MSG(GLDEV, "Need to pre-startup render device - call glDevice::PreStartup()");

	IDXGIAdapter1* adapter;
	if (GLDEV->m_dxgiFactory->EnumAdapters1(adapterIndex, &adapter) == DXGI_ERROR_NOT_FOUND)
		return UE_FALSE;

	IDXGIOutput* output = NULL;
	if (adapter->EnumOutputs(0, &output) == DXGI_ERROR_NOT_FOUND)
	{
		adapter->Release();
		return UE_FALSE;
	}
	adapter->Release();

	DXGI_MODE_DESC modes[256];
	UINT numModes = UE_ARRAY_SIZE(modes);
	if (FAILED(output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, modes)))
	{
		output->Release();
		return UE_FALSE;
	}
	output->Release();

	if (modeIndex >= numModes)
		return UE_FALSE;

	mode->m_width = modes[modeIndex].Width;
	mode->m_height = modes[modeIndex].Height;
	mode->m_frequency = modes[modeIndex].RefreshRate.Numerator;
	return UE_TRUE;
}

ueBool glUtils_ToD3DFORMAT(glBufferFormat format, DXGI_FORMAT& d3dFormat)
{
	switch (format)
	{
		case glBufferFormat_A8: d3dFormat = DXGI_FORMAT_A8_UNORM; break;
		case glBufferFormat_A16: d3dFormat = DXGI_FORMAT_R16_UNORM; break;
		case glBufferFormat_B5G6R5: d3dFormat = DXGI_FORMAT_B5G6R5_UNORM; break;
		case glBufferFormat_B5G5R5A1: d3dFormat = DXGI_FORMAT_B5G5R5A1_UNORM; break;
		case glBufferFormat_B8G8R8: d3dFormat = DXGI_FORMAT_B8G8R8X8_UNORM; break;
		case glBufferFormat_B8G8R8A8: d3dFormat = DXGI_FORMAT_B8G8R8A8_UNORM; break;

		case glBufferFormat_D16: d3dFormat = DXGI_FORMAT_D16_UNORM; break;
		case glBufferFormat_D24S8: d3dFormat = DXGI_FORMAT_D24_UNORM_S8_UINT; break;
		case glBufferFormat_D32: d3dFormat = DXGI_FORMAT_D32_FLOAT; break;

		case glBufferFormat_DXT1: d3dFormat = DXGI_FORMAT_BC1_UNORM; break;
		case glBufferFormat_DXT3: d3dFormat = DXGI_FORMAT_BC2_UNORM; break;
		case glBufferFormat_DXT5: d3dFormat = DXGI_FORMAT_BC3_UNORM; break;

		case glBufferFormat_R16F: d3dFormat = DXGI_FORMAT_R16_FLOAT; break;
		case glBufferFormat_R16G16F: d3dFormat = DXGI_FORMAT_R16G16_FLOAT; break;
		case glBufferFormat_R16G16B16A16F: d3dFormat = DXGI_FORMAT_R16G16B16A16_FLOAT; break;

		case glBufferFormat_R32F: d3dFormat = DXGI_FORMAT_R32_FLOAT; break;
		case glBufferFormat_R32G32F: d3dFormat = DXGI_FORMAT_R32G32_FLOAT; break;
		case glBufferFormat_R32G32B32A32F: d3dFormat = DXGI_FORMAT_R32G32B32A32_FLOAT; break;

		case glBufferFormat_Unknown: d3dFormat = DXGI_FORMAT_UNKNOWN; break;
		default:
			return UE_FALSE;
	}
	return UE_TRUE;
}

ueBool glUtils_FromD3DFORMAT(glBufferFormat& format, DXGI_FORMAT d3dFormat)
{
	DXGI_FORMAT tempD3DFormat;
	for (u32 i = 0; i < glBufferFormat_MAX; i++)
		if (glUtils_ToD3DFORMAT((glBufferFormat) i, tempD3DFormat) && tempD3DFormat == d3dFormat)
		{
			format = (glBufferFormat) i;
			return UE_TRUE;
		}
	return UE_FALSE;
}

D3D_PRIMITIVE_TOPOLOGY glUtils_ToD3DPRIMITIVETYPE(glPrimitive primitiveType)
{
	switch (primitiveType)
	{
		case glPrimitive_PointList: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case glPrimitive_LineList: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case glPrimitive_LineStrip: return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case glPrimitive_TriangleList: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case glPrimitive_TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		UE_INVALID_CASE(primitiveType);
	}
	return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
}

const char* glUtils_ToD3DSemanticName(glSemantic semantic)
{
	switch (semantic)
	{
		case glSemantic_Position: return "POSITION";
		case glSemantic_BoneWeights: return "BLENDWEIGHT";
		case glSemantic_BoneIndices: return "BLENDINDICES";
		case glSemantic_Normal: return "NORMAL";
		case glSemantic_TexCoord: return "TEXCOORD";
		case glSemantic_Tangent: return "TANGENT";
		case glSemantic_Binormal: return "BINORMAL";
		case glSemantic_Color: return "COLOR";
		case glSemantic_Depth: return "DEPTH";
		case glSemantic_Sample: return "SAMPLE";
		UE_INVALID_CASE(semantic);
	};
	return "<unknown glSemantic>";
}

DXGI_FORMAT glUtils_ToD3DInputFormat(ueNumType type, u8 count, u8 normalized)
{
	switch (type)
	{
		case ueNumType_F32:
		{
			UE_ASSERT(!normalized);
			static DXGI_FORMAT formats[4] = {DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT};
			return formats[count - 1];
		}
		case ueNumType_F16:
			UE_ASSERT(!normalized);
			UE_ASSERT(count == 1 || count == 2 || count == 4);
			return count == 1 ? DXGI_FORMAT_R16_FLOAT : (count == 2 ? DXGI_FORMAT_R16G16_FLOAT : DXGI_FORMAT_R16G16B16A16_FLOAT);
		case ueNumType_U8:
			UE_ASSERT(count == 4);
			return normalized ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8G8B8A8_UINT;
		case ueNumType_S16:
			UE_ASSERT(normalized);
			UE_ASSERT(count == 1 || count == 2 || count == 4);
			return count == 1 ? DXGI_FORMAT_R16_SNORM : (count == 2 ? DXGI_FORMAT_R16G16_SNORM : DXGI_FORMAT_R16G16B16A16_SNORM);
		case ueNumType_U16:
			UE_ASSERT(normalized);
			UE_ASSERT(count == 1 || count == 2 || count == 4);
			return count == 1 ? DXGI_FORMAT_R16_UNORM : (count == 2 ? DXGI_FORMAT_R16G16_UNORM : DXGI_FORMAT_R16G16B16A16_UNORM);
		case ueNumType_S10_3:
			UE_ASSERT(count == 1);
			UE_ASSERT(normalized);
			return DXGI_FORMAT_R10G10B10A2_UNORM;
		case ueNumType_U10_3:
			UE_ASSERT(count == 1);
			UE_ASSERT(!normalized);
			return DXGI_FORMAT_R10G10B10A2_UINT;
		UE_INVALID_CASE(type);
	}
	return DXGI_FORMAT_UNKNOWN;
}

D3D11_COMPARISON_FUNC glUtils_ToD3DCMPFUNC(glCmpFunc ueFunc)
{
	switch (ueFunc)
	{
		case glCmpFunc_Less: return D3D11_COMPARISON_LESS;
		case glCmpFunc_LEqual: return D3D11_COMPARISON_LESS_EQUAL;
		case glCmpFunc_Equal: return D3D11_COMPARISON_EQUAL;
		case glCmpFunc_GEqual: return D3D11_COMPARISON_GREATER_EQUAL;
		case glCmpFunc_Greater: return D3D11_COMPARISON_GREATER;
		case glCmpFunc_Never: return D3D11_COMPARISON_NEVER;
		case glCmpFunc_Always: return D3D11_COMPARISON_ALWAYS;
		UE_INVALID_CASE(ueFunc);
	}
	return D3D11_COMPARISON_LESS;
}

D3D11_BLEND_OP glUtils_ToD3DBLENDOP(glBlendOp blendOp)
{
	switch (blendOp)
	{
		case glBlendOp_Add: return D3D11_BLEND_OP_ADD;
		case glBlendOp_Subtract: return D3D11_BLEND_OP_SUBTRACT;
		case glBlendOp_Min: return D3D11_BLEND_OP_MIN;
		case glBlendOp_Max: return D3D11_BLEND_OP_MAX;
		UE_INVALID_CASE(blendOp);
	}
	return D3D11_BLEND_OP_ADD;
}

D3D11_BLEND glUtils_ToD3DBLEND(glBlendingFunc blendFunc)
{
	switch (blendFunc)
	{
		case glBlendingFunc_One: return D3D11_BLEND_ONE;
		case glBlendingFunc_Zero: return D3D11_BLEND_ZERO;
		case glBlendingFunc_SrcAlpha: return D3D11_BLEND_SRC_ALPHA;
		case glBlendingFunc_InvSrcAlpha: return D3D11_BLEND_INV_SRC_ALPHA;
		case glBlendingFunc_SrcColor: return D3D11_BLEND_SRC_COLOR;
		case glBlendingFunc_InvSrcColor: return D3D11_BLEND_INV_SRC_COLOR;
		case glBlendingFunc_DstAlpha: return D3D11_BLEND_DEST_ALPHA;
		case glBlendingFunc_InvDstAlpha: return D3D11_BLEND_INV_DEST_ALPHA;
		case glBlendingFunc_DstColor: return D3D11_BLEND_DEST_COLOR;
		case glBlendingFunc_InvDstColor: return D3D11_BLEND_INV_DEST_COLOR;
		UE_INVALID_CASE(blendFunc);
	}
	return D3D11_BLEND_ONE;
}

D3D11_STENCIL_OP glUtils_ToD3DSTENCILOP(glStencilOperation stencilOp)
{
	switch (stencilOp)
	{
		case glStencilOp_Zero: return D3D11_STENCIL_OP_ZERO;
		case glStencilOp_Keep: return D3D11_STENCIL_OP_KEEP;
		case glStencilOp_Replace: return D3D11_STENCIL_OP_REPLACE;
		case glStencilOp_Incr: return D3D11_STENCIL_OP_INCR;
		case glStencilOp_Decr: return D3D11_STENCIL_OP_DECR;
		case glStencilOp_Invert: return D3D11_STENCIL_OP_INVERT;
		UE_INVALID_CASE(stencilOp);
	}
	return D3D11_STENCIL_OP_ZERO;
}

D3D11_TEXTURECUBE_FACE glUtils_ToD3DCUBEMAP_FACES(glCubeFace face)
{
	switch (face)
	{
		case glCubeFace_PosX: return D3D11_TEXTURECUBE_FACE_POSITIVE_X;
		case glCubeFace_NegX: return D3D11_TEXTURECUBE_FACE_NEGATIVE_X;
		case glCubeFace_PosY: return D3D11_TEXTURECUBE_FACE_POSITIVE_Y;
		case glCubeFace_NegY: return D3D11_TEXTURECUBE_FACE_NEGATIVE_Y;
		case glCubeFace_PosZ: return D3D11_TEXTURECUBE_FACE_POSITIVE_Z;
		case glCubeFace_NegZ: return D3D11_TEXTURECUBE_FACE_NEGATIVE_Z;
		UE_INVALID_CASE(face);
	}
	return D3D11_TEXTURECUBE_FACE_POSITIVE_X;
}

D3D11_TEXTURE_ADDRESS_MODE glUtils_ToD3DTEXTUREADDRESS(glTexAddr addressMode)
{
	switch (addressMode)
	{
		case glTexAddr_Wrap: return D3D11_TEXTURE_ADDRESS_WRAP;
		case glTexAddr_Clamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
		case glTexAddr_ClampToBorder: return D3D11_TEXTURE_ADDRESS_BORDER;
		case glTexAddr_Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
		case glTexAddr_MirrorOnce: return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
		UE_INVALID_CASE(addressMode);
	}
	return D3D11_TEXTURE_ADDRESS_WRAP;
}

D3D11_FILTER glUtils_ToD3DTEXTUREFILTERTYPE(glTexFilter min, glTexFilter mag, glTexFilter mip, ueBool comparison)
{
#define MERGE_FOR_SWITCH(_comparison_, _min_ , _mag_, _mip_ ) (((_comparison_ ? 1 : 0) << 16) | (_min_ << 8) | (_mag_ << 4) | (_mip_))
#define CASE_FILTER(_comparison_, _min_ , _mag_, _mip_, result) case MERGE_FOR_SWITCH(_comparison_, glTexFilter_##_min_, glTexFilter_##_mag_, glTexFilter_##_mip_): return result;

	switch (MERGE_FOR_SWITCH(comparison, min, mag, mip))
	{
		CASE_FILTER(UE_TRUE, Nearest, Nearest, Nearest, D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT)
		CASE_FILTER(UE_TRUE, Nearest, Nearest, Linear, D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR)
		CASE_FILTER(UE_TRUE, Nearest, Linear, Nearest, D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT)
		CASE_FILTER(UE_TRUE, Nearest, Linear, Linear, D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR)
		CASE_FILTER(UE_TRUE, Linear, Nearest, Nearest, D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT)
		CASE_FILTER(UE_TRUE, Linear, Nearest, Linear, D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR)
		CASE_FILTER(UE_TRUE, Linear, Linear, Nearest, D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT)
		CASE_FILTER(UE_TRUE, Linear, Linear, Linear, D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR)
		CASE_FILTER(UE_TRUE, Anisotropic, Anisotropic, Anisotropic, D3D11_FILTER_COMPARISON_ANISOTROPIC)
		CASE_FILTER(UE_FALSE, Nearest, Nearest, Nearest, D3D11_FILTER_MIN_MAG_MIP_POINT)
		CASE_FILTER(UE_FALSE, Nearest, Nearest, Linear, D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR)
		CASE_FILTER(UE_FALSE, Nearest, Linear, Nearest, D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT)
		CASE_FILTER(UE_FALSE, Nearest, Linear, Linear, D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR)
		CASE_FILTER(UE_FALSE, Linear, Nearest, Nearest, D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT)
		CASE_FILTER(UE_FALSE, Linear, Nearest, Linear, D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR)
		CASE_FILTER(UE_FALSE, Linear, Linear, Nearest, D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT)
		CASE_FILTER(UE_FALSE, Linear, Linear, Linear, D3D11_FILTER_MIN_MAG_MIP_LINEAR)
		CASE_FILTER(UE_FALSE, Anisotropic, Anisotropic, Anisotropic, D3D11_FILTER_ANISOTROPIC)
		default:
			return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	}	
}

#else // defined(GL_D3D11)
	UE_NO_EMPTY_FILE
#endif
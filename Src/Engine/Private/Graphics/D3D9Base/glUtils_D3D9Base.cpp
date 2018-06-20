#include "Base/ueBase.h"

#if defined(GL_D3D9) || defined(GL_X360)

#include "Graphics/glLib_Private.h"

ueBool glUtils_ToD3DFORMAT(glBufferFormat format, D3DFORMAT& d3dFormat)
{
	switch (format)
	{
		case glBufferFormat_A8: d3dFormat = D3DFMT_A8; break;
		case glBufferFormat_A16: d3dFormat = D3DFMT_L16; break;
		case glBufferFormat_B5G6R5: d3dFormat = D3DFMT_R5G6B5; break;
		case glBufferFormat_B4G4R4A4: d3dFormat = D3DFMT_A4R4G4B4; break;
		case glBufferFormat_B5G5R5A1: d3dFormat = D3DFMT_A1R5G5B5; break;
		case glBufferFormat_B8G8R8: d3dFormat = D3DFMT_R8G8B8; break;
//		case glBufferFormat_R8G8B8: d3dFormat = D3DFMT_B8G8R8; break;
		case glBufferFormat_B8G8R8A8: d3dFormat = D3DFMT_A8R8G8B8; break;

		case glBufferFormat_D16: d3dFormat = D3DFMT_D16; break;
		case glBufferFormat_D24: d3dFormat = D3DFMT_D24X8; break;
		case glBufferFormat_D24S8: d3dFormat = D3DFMT_D24S8; break;
		case glBufferFormat_D32: d3dFormat = D3DFMT_D32; break;

		case glBufferFormat_DXT1: d3dFormat = D3DFMT_DXT1; break;
		case glBufferFormat_DXT3: d3dFormat = D3DFMT_DXT3; break;
		case glBufferFormat_DXT5: d3dFormat = D3DFMT_DXT5; break;

		case glBufferFormat_R16F: d3dFormat = D3DFMT_R16F; break;
		case glBufferFormat_R16G16F: d3dFormat = D3DFMT_G16R16F; break;
		case glBufferFormat_R16G16B16A16F: d3dFormat = D3DFMT_A16B16G16R16F; break;

		case glBufferFormat_R32F: d3dFormat = D3DFMT_R32F; break;
		case glBufferFormat_R32G32F: d3dFormat = D3DFMT_G32R32F; break;
		case glBufferFormat_R32G32B32A32F: d3dFormat = D3DFMT_A32B32G32R32F; break;

		case glBufferFormat_Unknown: d3dFormat = D3DFMT_UNKNOWN; break;
		default:
			return UE_FALSE;
	}
	return UE_TRUE;
}

ueBool glUtils_FromD3DFORMAT(glBufferFormat& format, D3DFORMAT d3dFormat)
{
	D3DFORMAT tempD3DFormat;
	for (u32 i = 0; i < glBufferFormat_MAX; i++)
		if (glUtils_ToD3DFORMAT((glBufferFormat) i, tempD3DFormat) && tempD3DFormat == d3dFormat)
		{
			format = (glBufferFormat) i;
			return UE_TRUE;
		}
	return UE_FALSE;
}

D3DPRIMITIVETYPE glUtils_ToD3DPRIMITIVETYPE(glPrimitive primitiveType)
{
	switch (primitiveType)
	{
		case glPrimitive_PointList: return D3DPT_POINTLIST;
		case glPrimitive_LineList: return D3DPT_LINELIST;
		case glPrimitive_LineStrip: return D3DPT_LINESTRIP;
		case glPrimitive_TriangleList: return D3DPT_TRIANGLELIST;
		case glPrimitive_TriangleStrip: return D3DPT_TRIANGLESTRIP;
		UE_INVALID_CASE(primitiveType);
	}
	return D3DPT_POINTLIST;
}

D3DMULTISAMPLE_TYPE glUtils_ToD3DMULTISAMPLE_TYPE(u32 MSAALevel)
{
	switch (MSAALevel)
	{
		case 1: return D3DMULTISAMPLE_NONE;
		case 2: return D3DMULTISAMPLE_2_SAMPLES;
		case 4: return D3DMULTISAMPLE_4_SAMPLES;
#if defined(GL_D3D9)
		case 3: return D3DMULTISAMPLE_3_SAMPLES;
		case 5: return D3DMULTISAMPLE_5_SAMPLES;
		case 6: return D3DMULTISAMPLE_6_SAMPLES;
		case 7: return D3DMULTISAMPLE_7_SAMPLES;
		case 8: return D3DMULTISAMPLE_8_SAMPLES;
		case 9: return D3DMULTISAMPLE_9_SAMPLES;
		case 10: return D3DMULTISAMPLE_10_SAMPLES;
		case 11: return D3DMULTISAMPLE_11_SAMPLES;
		case 12: return D3DMULTISAMPLE_12_SAMPLES;
		case 13: return D3DMULTISAMPLE_13_SAMPLES;
		case 14: return D3DMULTISAMPLE_14_SAMPLES;
		case 15: return D3DMULTISAMPLE_15_SAMPLES;
		case 16: return D3DMULTISAMPLE_16_SAMPLES;
#endif
		UE_INVALID_CASE(MSAALevel);
	}
	return D3DMULTISAMPLE_NONE;
}

D3DDECLUSAGE glUtils_ToD3DDECLUSAGE(glSemantic semantic)
{
	switch (semantic)
	{
		case glSemantic_Position: return D3DDECLUSAGE_POSITION;
		case glSemantic_BoneWeights: return D3DDECLUSAGE_BLENDWEIGHT;
		case glSemantic_BoneIndices: return D3DDECLUSAGE_BLENDINDICES;
		case glSemantic_Normal: return D3DDECLUSAGE_NORMAL;
		case glSemantic_PointSize: return D3DDECLUSAGE_PSIZE;
		case glSemantic_TexCoord: return D3DDECLUSAGE_TEXCOORD;
		case glSemantic_Tangent: return D3DDECLUSAGE_TANGENT;
		case glSemantic_Binormal: return D3DDECLUSAGE_BINORMAL;
		case glSemantic_TessFactor: return D3DDECLUSAGE_TESSFACTOR;
		case glSemantic_PositionT: return D3DDECLUSAGE_POSITIONT;
		case glSemantic_Color: return D3DDECLUSAGE_COLOR;
		case glSemantic_Fog: return D3DDECLUSAGE_FOG;
		case glSemantic_Depth: return D3DDECLUSAGE_DEPTH;
		case glSemantic_Sample: return D3DDECLUSAGE_SAMPLE;
		UE_INVALID_CASE(semantic);
	};
	return D3DDECLUSAGE_SAMPLE;
}

ueBool glUtils_FromD3DDECLUSAGE(glSemantic& semantic, D3DDECLUSAGE d3dDeclUsage)
{
	for (u32 i = 0; i < glSemantic_MAX; i++)
		if (glUtils_ToD3DDECLUSAGE((glSemantic) i) == d3dDeclUsage)
		{
			semantic = (glSemantic) i;
			return UE_TRUE;
		}
	return UE_FALSE;
}

D3DDECLTYPE glUtils_ToD3DDECLTYPE(ueNumType type, u8 count, u8 normalized)
{
	switch (type)
	{
		case ueNumType_F32:
			UE_ASSERT(!normalized);
			return (D3DDECLTYPE) (D3DDECLTYPE_FLOAT1 + count - 1);
		case ueNumType_F16:
			UE_ASSERT(!normalized);
			UE_ASSERT(count == 2 || count == 4);
			return count == 2 ? D3DDECLTYPE_FLOAT16_2 : D3DDECLTYPE_FLOAT16_4;
		case ueNumType_U8:
			UE_ASSERT(count == 4);
			return normalized ? D3DDECLTYPE_UBYTE4N : D3DDECLTYPE_UBYTE4;
		case ueNumType_S16:
			UE_ASSERT(count == 2 || count == 4);
			return count == 2 ? (normalized ? D3DDECLTYPE_SHORT2N : D3DDECLTYPE_SHORT2) : (normalized ? D3DDECLTYPE_SHORT4N : D3DDECLTYPE_SHORT4);
		case ueNumType_U16:
			UE_ASSERT(count == 2 || count == 4);
			UE_ASSERT(normalized);
			return count == 2 ? D3DDECLTYPE_USHORT2N : D3DDECLTYPE_USHORT4N;
		case ueNumType_S10_3:
			UE_ASSERT(count == 1);
			UE_ASSERT(normalized);
			return D3DDECLTYPE_DEC3N;
		case ueNumType_U10_3:
			UE_ASSERT(count == 1);
			UE_ASSERT(!normalized);
			return D3DDECLTYPE_UDEC3;
		UE_INVALID_CASE(type);
	}
	return D3DDECLTYPE_UNUSED;
}

D3DCMPFUNC glUtils_ToD3DCMPFUNC(glCmpFunc ueFunc)
{
	switch (ueFunc)
	{
		case glCmpFunc_Less: return D3DCMP_LESS;
		case glCmpFunc_LEqual: return D3DCMP_LESSEQUAL;
		case glCmpFunc_Equal: return D3DCMP_EQUAL;
		case glCmpFunc_GEqual: return D3DCMP_GREATEREQUAL;
		case glCmpFunc_Greater: return D3DCMP_GREATER;
		case glCmpFunc_Never: return D3DCMP_NEVER;
		case glCmpFunc_Always: return D3DCMP_ALWAYS;
		UE_INVALID_CASE(ueFunc);
	}
	return D3DCMP_LESS;
}

D3DBLENDOP glUtils_ToD3DBLENDOP(glBlendOp blendOp)
{
	switch (blendOp)
	{
		case glBlendOp_Add: return D3DBLENDOP_ADD;
		case glBlendOp_Subtract: return D3DBLENDOP_SUBTRACT;
		case glBlendOp_Min: return D3DBLENDOP_MIN;
		case glBlendOp_Max: return D3DBLENDOP_MAX;
		UE_INVALID_CASE(blendOp);
	}
	return D3DBLENDOP_ADD;
}

D3DBLEND glUtils_ToD3DBLEND(glBlendingFunc blendFunc)
{
	switch (blendFunc)
	{
		case glBlendingFunc_One: return D3DBLEND_ONE;
		case glBlendingFunc_Zero: return D3DBLEND_ZERO;
		case glBlendingFunc_SrcAlpha: return D3DBLEND_SRCALPHA;
		case glBlendingFunc_InvSrcAlpha: return D3DBLEND_INVSRCALPHA;
		case glBlendingFunc_SrcColor: return D3DBLEND_SRCCOLOR;
		case glBlendingFunc_InvSrcColor: return D3DBLEND_INVSRCCOLOR;
		case glBlendingFunc_DstAlpha: return D3DBLEND_DESTALPHA;
		case glBlendingFunc_InvDstAlpha: return D3DBLEND_INVDESTALPHA;
		case glBlendingFunc_DstColor: return D3DBLEND_DESTCOLOR;
		case glBlendingFunc_InvDstColor: return D3DBLEND_INVDESTCOLOR;
		UE_INVALID_CASE(blendFunc);
	}
	return D3DBLEND_ONE;
}

D3DSTENCILOP glUtils_ToD3DSTENCILOP(glStencilOperation stencilOp)
{
	switch (stencilOp)
	{
		case glStencilOp_Zero: return D3DSTENCILOP_ZERO;
		case glStencilOp_Keep: return D3DSTENCILOP_KEEP;
		case glStencilOp_Replace: return D3DSTENCILOP_REPLACE;
		case glStencilOp_Incr: return D3DSTENCILOP_INCR;
		case glStencilOp_Decr: return D3DSTENCILOP_DECR;
		case glStencilOp_Invert: return D3DSTENCILOP_INVERT;
		UE_INVALID_CASE(stencilOp);
	}
	return D3DSTENCILOP_ZERO;
}

D3DCUBEMAP_FACES glUtils_ToD3DCUBEMAP_FACES(glCubeFace face)
{
	switch (face)
	{
		case glCubeFace_PosX: return D3DCUBEMAP_FACE_POSITIVE_X;
		case glCubeFace_NegX: return D3DCUBEMAP_FACE_NEGATIVE_X;
		case glCubeFace_PosY: return D3DCUBEMAP_FACE_POSITIVE_Y;
		case glCubeFace_NegY: return D3DCUBEMAP_FACE_NEGATIVE_Y;
		case glCubeFace_PosZ: return D3DCUBEMAP_FACE_POSITIVE_Z;
		case glCubeFace_NegZ: return D3DCUBEMAP_FACE_NEGATIVE_Z;
		UE_INVALID_CASE(face);
	}
	return D3DCUBEMAP_FACE_POSITIVE_X;
}

D3DTEXTUREADDRESS glUtils_ToD3DTEXTUREADDRESS(glTexAddr addressMode)
{
	switch (addressMode)
	{
		case glTexAddr_Wrap: return D3DTADDRESS_WRAP;
		case glTexAddr_Clamp: return D3DTADDRESS_CLAMP;
		case glTexAddr_ClampToBorder: return D3DTADDRESS_BORDER;
		case glTexAddr_Mirror: return D3DTADDRESS_MIRROR;
		case glTexAddr_MirrorOnce: return D3DTADDRESS_MIRRORONCE;
		UE_INVALID_CASE(addressMode);
	}
	return D3DTADDRESS_WRAP;
}

D3DTEXTUREFILTERTYPE glUtils_ToD3DTEXTUREFILTERTYPE(glTexFilter filter)
{
	switch (filter)
	{
		case glTexFilter_None: return D3DTEXF_NONE;
		case glTexFilter_Nearest: return D3DTEXF_POINT;
		case glTexFilter_Linear: return D3DTEXF_LINEAR;
		UE_INVALID_CASE(filter);
	}
	return D3DTEXF_POINT;
}

#else // defined(GL_D3D9) || defined(GL_X360)
	UE_NO_EMPTY_FILE
#endif
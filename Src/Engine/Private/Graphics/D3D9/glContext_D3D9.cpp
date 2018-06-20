#include "Base/ueBase.h"

#if defined(GL_D3D9)

#include "Graphics/glLib_Private.h"

void glCtx_Init(glCtx* ctx)
{
	glCtx_D3D9Base_Init(ctx);
}

void glCtx_Deinit(glCtx* ctx)
{
	glCtx_D3D9Base_Deinit(ctx);
}

ueBool glCtx_Begin(glCtx* ctx)
{
	return SUCCEEDED(ctx->m_handle->BeginScene());
}

void glCtx_End(glCtx* ctx)
{
	// Release unnecessary references to resources
	glCtx_Reset(ctx, glResetFlags_Resources);

	ctx->m_handle->EndScene();
}

void glCtx_Present(glCtx* ctx, glTextureBuffer* tb)
{
	UE_ASSERT_MSG(tb == glDevice_GetBackBuffer(), "On D3D9 can only present using back buffer.");
	ctx->m_handle->Present(NULL, NULL, NULL, NULL);
}

void glCtx_DrawIndexed(glCtx* ctx, glPrimitive primitive, u32 baseVertexIndex, u32 minVertexIndex, u32 numVerts, u32 startIndex, u32 numIndices)
{
	if (!glCtx_PreDraw(ctx)) return;
	ctx->m_handle->DrawIndexedPrimitive(glUtils_ToD3DPRIMITIVETYPE(primitive), baseVertexIndex, minVertexIndex, numVerts, startIndex, glUtils_CalcPrimCount(primitive, numIndices));
}

void glCtx_Draw(glCtx* ctx, glPrimitive primitive, u32 startVertex, u32 numVerts)
{
	if (!glCtx_PreDraw(ctx)) return;
	ctx->m_handle->DrawPrimitive(glUtils_ToD3DPRIMITIVETYPE(primitive), startVertex, glUtils_CalcPrimCount(primitive, numVerts));
}

void glCtx_ResolveRenderTargets(glCtx* ctx)
{
	UE_ASSERT(ctx->m_renderGroup);

	glRenderGroup* group = ctx->m_renderGroup;

	if (group->m_desc.m_MSAALevel == 1)
		return;

	if (group->m_desc.m_needDepthStencilContent)
	{
		glTextureBuffer* dstTexture = group->m_depthStencilTexture;
		IDirect3DSurface9* dstD3DSurface = NULL;
		UE_ASSERTWinCall(static_cast<IDirect3DTexture9*>(dstTexture->m_textureHandle)->GetSurfaceLevel(0, &dstD3DSurface));

		ctx->m_handle->StretchRect(
			group->m_depthStencilRenderTarget->m_surfaceHandle, NULL,
			group->m_depthStencilTexture->m_surfaceHandle, NULL,
			D3DTEXF_NONE);

		dstD3DSurface->Release();
	}

	if (group->m_desc.m_needColorContent)
		for (u32 i = 0; i < group->m_desc.m_numColorBuffers; i++)
		{
			glTextureBuffer* dstTexture = group->m_colorTextures[i];
			IDirect3DSurface9* dstD3DSurface = NULL;
			UE_ASSERTWinCall(static_cast<IDirect3DTexture9*>(dstTexture->m_textureHandle)->GetSurfaceLevel(0, &dstD3DSurface));

			ctx->m_handle->StretchRect(
				group->m_colorRenderTargets[i]->m_surfaceHandle, NULL,
				dstD3DSurface, NULL,
				D3DTEXF_NONE);

			dstD3DSurface->Release();
		}
}

ueBool glCtx_CopyBuffer(glCtx* ctx, glTextureBuffer* src, glTextureBuffer* dst)
{
	UE_ASSERT((src->m_desc.m_flags & glTextureBufferFlags_IsTexture) && src->m_desc.m_type == glTexType_2D);
	UE_ASSERT((dst->m_desc.m_flags & glTextureBufferFlags_IsTexture) && dst->m_desc.m_type == glTexType_2D);

	IDirect3DTexture9* sourceTexture = (IDirect3DTexture9*) src->m_textureHandle;
	IDirect3DSurface9* sourceSurface = NULL;
	HRESULT hr = sourceTexture->GetSurfaceLevel(0, &sourceSurface);
	if (FAILED(hr))
		return UE_FALSE;

	IDirect3DTexture9* destTexture = (IDirect3DTexture9*) dst->m_textureHandle;
	IDirect3DSurface9* destSurface = NULL;
	hr = destTexture->GetSurfaceLevel(0, &destSurface);
	if (FAILED(hr))
	{
		sourceSurface->Release();
		return UE_FALSE;
	}

	hr = ctx->m_handle->StretchRect(sourceSurface, NULL, destSurface, NULL, D3DTEXF_LINEAR);

	sourceSurface->Release();
	destSurface->Release();
	return SUCCEEDED(hr);
}

void glCtx_BeginConditionalRendering(glCtx* ctx, glOcclusionQuery* query)
{
	// Unsupported
}

void glCtx_EndConditionalRendering(glCtx* ctx)
{
	// Unsupported
}

glOcclusionQuery* glQuery_Create(glQueryType type)
{
	if (type != glQueryType_OcclusionNumSamples && type != glQueryType_OcclusionAnySample)
		return NULL;

	glCtx* ctx = glDevice_Get()->m_defCtx; // FIXME

	glOcclusionQuery* query = new(GLDEV->m_queriesPool) glOcclusionQuery;
	if (!query)
		return NULL;

	if (FAILED(ctx->m_handle->CreateQuery(D3DQUERYTYPE_OCCLUSION, &query->m_handle)))
	{
		GLDEV->m_queriesPool.Free(query);
		return NULL;
	}
	return query;
}

void glQuery_Destroy(glOcclusionQuery* query)
{
	if (query->m_handle)
	{
		const UINT refCount = query->m_handle->Release();
		UE_ASSERT(refCount == 0);
	}
}

void glCtx_BeginQuery(glCtx* ctx, glOcclusionQuery* query)
{
	if (!query->m_handle)
		return;

	HRESULT hr = query->m_handle->Issue(D3DISSUE_BEGIN);
}

void glCtx_EndQuery(glCtx* ctx, glOcclusionQuery* query)
{
	if (!query->m_handle)
		return;

	HRESULT hr = query->m_handle->Issue(D3DISSUE_END);
}

ueBool glCtx_GetQueryResult(glCtx* ctx, glOcclusionQuery* query, u32& numPixelsDrawn, ueBool forceFinish)
{
	if (!query->m_handle)
	{
		numPixelsDrawn = U32_MAX;
		return UE_TRUE;
	}

	if (!forceFinish)
		return query->m_handle->GetData(&numPixelsDrawn, sizeof(DWORD), 0) != S_FALSE;

	while (query->m_handle->GetData(&numPixelsDrawn, sizeof(DWORD), D3DGETDATA_FLUSH) == S_FALSE) {}
	return UE_TRUE;
}

void glOcclusionQuery_OnLostDevice(glOcclusionQuery* query)
{
	if (query->m_handle)
	{
		query->m_handle->Release();
		query->m_handle = NULL;
	}
}

void glOcclusionQuery_OnResetDevice(glOcclusionQuery* query)
{
	if (query->m_handle)
		return;
	glCtx* ctx = glDevice_Get()->m_defCtx; // FIXME
	ctx->m_handle->CreateQuery(D3DQUERYTYPE_OCCLUSION, &query->m_handle);
}

#else // defined(GL_D3D9)
	UE_NO_EMPTY_FILE
#endif
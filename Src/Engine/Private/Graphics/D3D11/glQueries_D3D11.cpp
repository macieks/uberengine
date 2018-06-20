#include "Base/ueBase.h"

#if defined(GL_D3D11)

#include "Graphics/glLib_Private.h"

void glCtx_BeginConditionalRendering(glCtx* ctx, glOcclusionQuery* query)
{
	UE_ASSERT(query->m_predicateHandle);
	ctx->m_handle->SetPredication(query->m_predicateHandle, FALSE);
}

void glCtx_EndConditionalRendering(glCtx* ctx)
{
	ctx->m_handle->SetPredication(NULL, FALSE);
}

glOcclusionQuery* glQuery_Create(glQueryType type)
{
	glOcclusionQuery* query = new(GLDEV->m_queriesPool) glOcclusionQuery;
	if (!query)
		return NULL;

	query->m_handle = NULL;
	query->m_predicateHandle = NULL;

	D3D11_QUERY_DESC desc;
	ueMemZeroS(desc);

	// Occlusion

	desc.Query = D3D11_QUERY_OCCLUSION;
	if (FAILED(D3DEV->CreateQuery(&desc, &query->m_handle)))
	{
		GLDEV->m_queriesPool.Free(query);
		return NULL;
	}

	// Predicate

	if (type == glQueryType_OcclusionPredicate)
	{
		desc.Query = D3D11_QUERY_OCCLUSION_PREDICATE;
		desc.MiscFlags = D3D11_QUERY_MISC_PREDICATEHINT;
		if (FAILED(D3DEV->CreatePredicate(&desc, &query->m_predicateHandle)))
		{
			if (query->m_handle)
				query->m_handle->Release();
			GLDEV->m_queriesPool.Free(query);
			return NULL;
		}
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

	if (query->m_predicateHandle)
	{
		const UINT refCount = query->m_predicateHandle->Release();
		UE_ASSERT(refCount == 0);
	}

	GLDEV->m_queriesPool.Free(query);
}

void glCtx_BeginQuery(glCtx* ctx, glOcclusionQuery* query)
{
	if (query->m_handle)
		ctx->m_handle->Begin(query->m_handle);
	if (query->m_predicateHandle)
		ctx->m_handle->Begin(query->m_predicateHandle);
}

void glCtx_EndQuery(glCtx* ctx, glOcclusionQuery* query)
{
	if (query->m_handle)
		ctx->m_handle->End(query->m_handle);
	if (query->m_predicateHandle)
		ctx->m_handle->End(query->m_predicateHandle);
}

ueBool glCtx_GetQueryResult(glCtx* ctx, glOcclusionQuery* query, u32& numPixelsDrawn, ueBool forceFinish)
{
	UE_ASSERT(query->m_handle);

	UINT64 data = 0;
	if (!forceFinish)
	{
		HRESULT hr = ctx->m_handle->GetData(query->m_handle, &data, sizeof(data), D3D11_ASYNC_GETDATA_DONOTFLUSH);
		numPixelsDrawn = (u32) data;
		return SUCCEEDED(hr);
	}

	HRESULT hr;
	while (1)
	{
		hr = ctx->m_handle->GetData(query->m_handle, &data, sizeof(data), 0);
		if (hr == S_OK)
			break;
	}
	numPixelsDrawn = (u32) data;
	return UE_TRUE;
}

#else // defined(GL_D3D11)
	UE_NO_EMPTY_FILE
#endif
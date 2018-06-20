#include "Base/ueBase.h"

#if defined(GL_OPENGL)

#include "Graphics/glLib_Private.h"

glOcclusionQuery* glQuery_Create(glQueryType type)
{
#ifdef GL_OPENGL_ES
	if (type != glQueryType_OcclusionAnySample || !GLEW_EXT_occlusion_query_boolean)
		return NULL;
#else
	switch (type)
	{
	case glQueryType_OcclusionPredicate:
		if (!GLEW_VERSION_3_0)
			return NULL;
		break;
	case glQueryType_OcclusionNumSamples:
	case glQueryType_OcclusionAnySample:
		if (!GLEW_ARB_occlusion_query)
			return NULL;
		break;
	}
#endif

	glOcclusionQuery* query = new(GLDEV->m_queriesPool) glOcclusionQuery;
	GL(glGenQueriesARB(1, &query->m_handle));
	query->m_type = type;
	return query;
}

void glQuery_Destroy(glOcclusionQuery* query)
{
	GL(glDeleteQueriesARB(1, &query->m_handle));
	GLDEV->m_queriesPool.Free(query);
}

void glCtx_BeginQuery(glCtx* ctx, glOcclusionQuery* query)
{
	GLuint queryType;
	switch (query->m_type)
	{
#ifdef GL_OPENGL_ES
		case glQueryType_OcclusionAnySample:
			queryType = GL_ANY_SAMPLES_PASSED_ARB;
			break;
#else
		case glQueryType_OcclusionAnySample:
		case glQueryType_OcclusionNumSamples:
		case glQueryType_OcclusionPredicate:
			queryType = GL_SAMPLES_PASSED_ARB;
			break;
#endif
		UE_INVALID_CASE(query->m_type);
	}
	GL(glBeginQueryARB(queryType, query->m_handle));
}

void glCtx_EndQuery(glCtx* ctx, glOcclusionQuery* query)
{
	GLuint queryType;
	switch (query->m_type)
	{
#ifdef GL_OPENGL_ES
		case glQueryType_OcclusionAnySample:
			queryType = GL_ANY_SAMPLES_PASSED_ARB;
			break;
#else
		case glQueryType_OcclusionAnySample:
		case glQueryType_OcclusionNumSamples:
		case glQueryType_OcclusionPredicate:
			queryType = GL_SAMPLES_PASSED_ARB;
			break;
#endif
		UE_INVALID_CASE(query->m_type);
	}
	GL(glEndQueryARB(queryType));
}

ueBool glCtx_GetQueryResult(glCtx* ctx, glOcclusionQuery* query, u32& numSamplesRendered, ueBool forceFinish)
{
	if (!forceFinish)
	{
		GLuint isAvailable;
		GL(glGetQueryObjectuivARB(query->m_handle, GL_QUERY_RESULT_AVAILABLE_ARB, &isAvailable));
		if (!isAvailable)
			return UE_FALSE;
	}

	GLuint result;
	GL(glGetQueryObjectuivARB(query->m_handle, GL_QUERY_RESULT_ARB, &result));
	numSamplesRendered = result;

	return UE_TRUE;
}

// Conditional rendering

void glCtx_BeginConditionalRendering(glCtx* ctx, glOcclusionQuery* query)
{
	if (!GLEW_VERSION_3_0)
		return;

#ifndef GL_OPENGL_ES
	GL(glBeginConditionalRender(query->m_handle, GL_QUERY_BY_REGION_NO_WAIT));
#endif
}

void glCtx_EndConditionalRendering(glCtx* ctx)
{
	if (!GLEW_VERSION_3_0)
		return;

#ifndef GL_OPENGL_ES
	GL(glEndConditionalRender());
#endif
}

#else // defined(GL_OPENGL)
	UE_NO_EMPTY_FILE
#endif
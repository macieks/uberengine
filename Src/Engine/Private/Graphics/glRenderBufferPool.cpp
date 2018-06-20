#include "Graphics/glLib_Private.h"

struct glRenderBufferPoolData
{
	ueBool m_isInitialized;

	ueBool m_allowCreateOnDemand;

	u32 m_numSlots;
	u32 m_maxSlots;
	glTextureBuffer** m_slots;

	glRenderBufferPoolData() : m_isInitialized(UE_FALSE) {}
};

static glRenderBufferPoolData s_data;

glTextureBuffer* glRenderBufferPool_GetRenderBuffer(glRenderGroup* group, s32 bufferIndex);

void glRenderBufferPool_Startup()
{
	UE_ASSERT(!s_data.m_isInitialized);

	const glDeviceStartupParams* params = glDevice_GetStartupParams();

	s_data.m_numSlots = 0;
	s_data.m_maxSlots = params->m_maxPoolManagedTextureBuffers;
	s_data.m_slots = (glTextureBuffer**) GL_STACK_ALLOC->Alloc(sizeof(glTextureBuffer*) * s_data.m_maxSlots);

	s_data.m_allowCreateOnDemand = UE_TRUE;

	s_data.m_isInitialized = UE_TRUE;
}

void glRenderBufferPool_Shutdown()
{
	UE_ASSERT(s_data.m_isInitialized);
	glRenderBufferPool_DestroyAllBuffers(UE_TRUE);
	GL_STACK_ALLOC->Free(s_data.m_slots);
	s_data.m_isInitialized = UE_FALSE;
}

glTextureBuffer* glRenderBufferPool_GetTextureBuffer(glTextureBufferDesc& desc, ueBool createIfNotFound, glRenderGroup* group)
{
	UE_ASSERT(s_data.m_isInitialized);

	const ueBool matchByMainFrameBuffer =
#if defined(GL_D3D11)
		!glUtils_IsDepthFormat(desc.m_format);
#else
		UE_TRUE;
#endif

	for (u32 i = 0; i < s_data.m_numSlots; i++)
	{
		glTextureBuffer* tb = s_data.m_slots[i];

		if (tb->m_desc == desc &&
			(!group ||
			!matchByMainFrameBuffer ||
			tb->m_isMainFrameBuffer == group->m_desc.m_isMainFrameBuffer
			))
		{
			s_data.m_slots[i] = s_data.m_slots[--s_data.m_numSlots];
			return tb;
		}
	}

	if (!createIfNotFound)
		return NULL;

	UE_ASSERT_MSG(
		!group ||
		!matchByMainFrameBuffer ||
		!group->m_desc.m_isMainFrameBuffer, "Can't create main frame buffer on demand.");

	ueLogD("Creating new on-demand texture buffer: %ux%u (MSAA x %u; type = %s; format = %s; is texture = %s; mips = %u, ueto = %s)",
		desc.m_width, desc.m_height,
		desc.m_rt.m_MSAALevel,
		glTexType_ToString(desc.m_type),
		glBufferFormat_ToString(desc.m_format),
		(desc.m_flags & glTextureBufferFlags_IsTexture) ? "YES" : "NO",
		desc.m_numLevels, desc.m_enableAutoMips ? "YES" : "NO");

	glTextureBuffer* tb = glTextureBuffer_Create(&desc);
	if (tb)
	{
		char debugName[GL_MAX_TEXTURE_BUFFER_DEBUG_NAME];
		ueStrFormatS(debugName, "render-buffer-0x%x", tb);
		glTextureBuffer_SetDebugName(tb, debugName);
	}
	return tb;
}

void glRenderBufferPool_ReleaseTextureBuffer(glTextureBuffer* tb)
{
	UE_ASSERT(s_data.m_isInitialized);
	UE_ASSERT(s_data.m_numSlots < s_data.m_maxSlots);

	s_data.m_slots[s_data.m_numSlots++] = tb;
}

void glRenderBufferPool_DestroyAllBuffers(ueBool includeAutoManaged)
{
	UE_ASSERT(s_data.m_isInitialized);

	u32 i = 0;
	while (i < s_data.m_numSlots)
	{
		glTextureBuffer* tb = s_data.m_slots[i];

		if (tb->m_isMainFrameBuffer && !includeAutoManaged)
		{
			i++;
			continue;
		}

		glTextureBuffer_Destroy(tb);
		s_data.m_slots[i] = s_data.m_slots[--s_data.m_numSlots];
	}
}

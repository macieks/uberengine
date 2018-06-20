#include "Base/ueBase.h"

#if defined(GL_OPENGL)

#include "Base/Allocators/ueDLMallocAllocator.h"
#include "Graphics/glLib_Private.h"

struct glPBOData
{
	ueBool m_useHardwarePBOs;

	ueList<glPBO> m_usedPBOs;
	ueList<glPBO> m_unusedPBOs;
	ueGenericPool m_pbosPool;

	u32 m_totalSize;
	u32 m_maxTotalSize;

	// PBO emulation

	void* m_memory;
	ueDLMallocAllocator m_allocator;
};

static glPBOData s_data;

void glPBO_Startup()
{
	const glDeviceStartupParams* params = glDevice_GetStartupParams();

#ifdef GL_OPENGL_ES
	s_data.m_useHardwarePBOs = UE_FALSE;
#else
	s_data.m_useHardwarePBOs = params->m_useHardwarePBOs && GLEW_VERSION_2_1;
#endif

	UE_ASSERT_FUNC(s_data.m_pbosPool.Init(GL_STACK_ALLOC, sizeof(glPBO), params->m_maxPBOs));
	s_data.m_totalSize = 0;
	s_data.m_maxTotalSize = params->m_maxTotalPBOSize;

	if (!s_data.m_useHardwarePBOs)
	{
		s_data.m_memory = GL_FREQ_ALLOC->Alloc(s_data.m_maxTotalSize);
		UE_ASSERT(s_data.m_memory);
		s_data.m_allocator.InitMem(s_data.m_memory, s_data.m_maxTotalSize);
	}
}

void glPBO_Shutdown()
{
	UE_ASSERT(s_data.m_usedPBOs.Length() == 0);

	if (s_data.m_useHardwarePBOs)
		while (glPBO* pbo = s_data.m_unusedPBOs.PopFront())
			GL(glDeleteBuffers(1, &pbo->m_handle));
	else
		GL_FREQ_ALLOC->Free(s_data.m_memory);

	s_data.m_pbosPool.Deinit();
}

glPBO* glPBO_Map(GLenum target, GLenum usage, GLenum access, u32 size)
{
	// PBO emulation support

	if (!s_data.m_useHardwarePBOs)
	{
		if (s_data.m_totalSize + size > s_data.m_maxTotalSize)
			return NULL;

		void* data = s_data.m_allocator.Alloc(size);
		if (!data)
		{
			ueLogW("Failed to create (emulated) PBO - not enough memory (max total size = %u)", s_data.m_maxTotalSize);
			return NULL;
		}

		glPBO* pbo = new(s_data.m_pbosPool) glPBO();
		if (!pbo)
		{
			s_data.m_allocator.Free(data);
			ueLogW("Exhausted limit of PBOs (max count = %u)", s_data.m_pbosPool.GetNumBlocks());
			return NULL;
		}
		pbo->m_size = size;
		pbo->m_target = target;
		pbo->m_usage = usage;
		pbo->m_texOpData = pbo->m_lockedData = data;

		s_data.m_totalSize += size;
		s_data.m_usedPBOs.PushBack(pbo);

		return pbo;
	}

#if !defined(GL_OPENGL_ES)

	// Free up enough memory (deleting in LRU order)

	while ((s_data.m_totalSize + size > s_data.m_maxTotalSize || s_data.m_pbosPool.Size() == s_data.m_pbosPool.GetNumBlocks()) &&
		!s_data.m_unusedPBOs.IsEmpty())
	{
		glPBO* pbo = s_data.m_unusedPBOs.PopFront();

		// Check if resizing this PBO would do - if so, resize it if needed, map it and return

		if (s_data.m_totalSize - pbo->m_size + size <= s_data.m_maxTotalSize)
		{
			GL(glBindBuffer(target, pbo->m_handle));
			if (size != pbo->m_size)
				GL(glBufferData(target, size, NULL, usage));
			pbo->m_lockedData = GLC(glMapBuffer(target, access));
			if (!pbo->m_lockedData)
				return NULL;
			pbo->m_texOpData = NULL;

			if (size != pbo->m_size)
			{
				s_data.m_totalSize -= pbo->m_size;
				s_data.m_totalSize += size;
			}

			pbo->m_size = size;
			pbo->m_target = target;
			pbo->m_usage = usage;

			s_data.m_usedPBOs.PushBack(pbo);
			return pbo;
		}

		// Delete PBO

		GL(glDeleteBuffers(1, &pbo->m_handle));

		s_data.m_totalSize -= pbo->m_size;
		s_data.m_pbosPool.Free(pbo);
	}

	// Is there still enough capacity?

	if (s_data.m_totalSize + size > s_data.m_maxTotalSize)
		return NULL;

	// Create new PBO

	glPBO* pbo = new(s_data.m_pbosPool) glPBO();
	if (!pbo)
	{
		ueLogW("Exhausted limit of PBOs (max count = %u)", s_data.m_pbosPool.GetNumBlocks());
		return NULL;
	}
	pbo->m_size = size;
	pbo->m_target = target;
	pbo->m_usage = usage;

	GL(glGenBuffers(1, &pbo->m_handle));
	GL(glBindBuffer(target, pbo->m_handle));
	GL(glBufferData(target, size, NULL, usage));
	pbo->m_lockedData = GLC(glMapBuffer(target, access));
	if (!pbo->m_lockedData)
	{
		s_data.m_unusedPBOs.PushFront(pbo);
		return NULL;
	}
	pbo->m_texOpData = NULL;

	s_data.m_totalSize += size;
	s_data.m_usedPBOs.PushBack(pbo);

	return pbo;

#else
	return NULL;
#endif // !defined(GL_OPENGL_ES)
}

void glPBO_UnmapAndBind(glPBO* pbo)
{
#if !defined(GL_OPENGL_ES)
	if (s_data.m_useHardwarePBOs)
	{
		GL(glBindBuffer(pbo->m_target, pbo->m_handle));
		GL(glUnmapBuffer(pbo->m_target));
	}
#endif
}

void glPBO_Release(glPBO* pbo)
{
	if (s_data.m_useHardwarePBOs)
	{
		GL(glBindBuffer(pbo->m_target, 0));

		s_data.m_usedPBOs.Remove(pbo);
		s_data.m_unusedPBOs.PushBack(pbo);
	}
	else
	{
		s_data.m_totalSize -= pbo->m_size;

		s_data.m_usedPBOs.Remove(pbo);
		s_data.m_allocator.Free(pbo->m_lockedData);
		s_data.m_pbosPool.Free(pbo);
	}
}

#else // defined(GL_OPENGL)
	UE_NO_EMPTY_FILE
#endif
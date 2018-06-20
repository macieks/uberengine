#include "Base/ueBase.h"

#if defined(GL_D3D9)

#include "Graphics/glLib_Private.h"

void glIndexBuffer_CreateInPlace(glIndexBuffer* ib, const void* initData = NULL)
{
	const u32 size = ib->m_desc.m_numIndices * ib->m_desc.m_indexSize;

	UE_ASSERTWinCall( D3DEV->CreateIndexBuffer(
		size,
		((ib->m_desc.m_flags & glBufferFlags_WriteOnly) ? D3DUSAGE_WRITEONLY : 0) | ((ib->m_desc.m_flags & glBufferFlags_Dynamic) ? D3DUSAGE_DYNAMIC : 0),
		(ib->m_desc.m_indexSize == sizeof(u32)) ? D3DFMT_INDEX32 : D3DFMT_INDEX16,
		(ib->m_desc.m_flags & glBufferFlags_Dynamic) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
		&ib->m_handle,
		NULL));

	if (initData)
	{
		void* ptr = glIndexBuffer_Lock(ib);
		UE_ASSERT(ptr);
		ueMemCpy(ptr, initData, size);
		glIndexBuffer_Unlock(ib);
	}
}

void glIndexBuffer_DestroyInPlace(glIndexBuffer* ib)
{
	if (!ib->m_handle)
		return;

	const UINT refCount = ib->m_handle->Release();
	UE_ASSERT(refCount == 0);
	ib->m_handle = NULL;
}

void* glIndexBuffer_Lock(glIndexBuffer* ib, u32 lockFlags, u32 offset, u32 size)
{
	size = size ? size : (ib->m_desc.m_indexSize * ib->m_desc.m_numIndices);
	UE_ASSERT(offset + size <= ib->m_desc.m_numIndices * ib->m_desc.m_indexSize);

	DWORD d3dLockFlags = 0;
	if (lockFlags == glBufferLockFlags_Read)
		d3dLockFlags |= D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK;
	else if (!(lockFlags & glBufferLockFlags_Read) && (lockFlags & glBufferLockFlags_Write))
	{
		if (lockFlags & glBufferLockFlags_NoOverwrite)
			d3dLockFlags |= D3DLOCK_NOOVERWRITE | D3DLOCK_NOSYSLOCK;
		else if (lockFlags & glBufferLockFlags_Discard)
			d3dLockFlags |= D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK;
	}

	void* indexData = NULL;
	if (FAILED(ib->m_handle->Lock(offset, size, &indexData, d3dLockFlags)))
		return NULL;
	return indexData;
}

void glIndexBuffer_Unlock(glIndexBuffer* ib)
{
	ib->m_handle->Unlock();
}

void glIndexBuffer_OnLostDevice(glIndexBuffer* ib)
{
	if (ib->m_handle && (ib->m_desc.m_flags & glBufferFlags_Dynamic))
		glIndexBuffer_DestroyInPlace(ib);
}

void glIndexBuffer_OnResetDevice(glIndexBuffer* ib)
{
	if (!(ib->m_desc.m_flags & glBufferFlags_Dynamic))
		return;

	glIndexBuffer_CreateInPlace(ib);
}

#else // defined(GL_D3D9)
	UE_NO_EMPTY_FILE
#endif
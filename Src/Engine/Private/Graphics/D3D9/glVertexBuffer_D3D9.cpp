#include "Base/ueBase.h"

#if defined(GL_D3D9)

#include "Graphics/glLib_Private.h"

void glVertexBuffer_CreateInPlace(glVertexBuffer* vb, const void* initData = NULL)
{
	UE_ASSERTWinCall(D3DEV->CreateVertexBuffer(
		vb->m_desc.m_size,
		((vb->m_desc.m_flags & glBufferFlags_WriteOnly) ? D3DUSAGE_WRITEONLY : 0) | ((vb->m_desc.m_flags & glBufferFlags_Dynamic) ? D3DUSAGE_DYNAMIC : 0),
		0,
		(vb->m_desc.m_flags & glBufferFlags_Dynamic) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
		&vb->m_handle,
		NULL));

	if (initData)
	{
		void* ptr = glVertexBuffer_Lock(vb);
		UE_ASSERT(ptr);
		ueMemCpy(ptr, initData, vb->m_desc.m_size);
		glVertexBuffer_Unlock(vb);
	}
}

void glVertexBuffer_DestroyInPlace(glVertexBuffer* vb)
{
	if (!vb->m_handle)
		return;

	const UINT refCount = vb->m_handle->Release();
	UE_ASSERT(refCount == 0);
	vb->m_handle = NULL;
}

void* glVertexBuffer_Lock(glVertexBuffer* vb, u32 lockFlags, u32 offset, u32 size)
{
	size = size ? size : vb->m_desc.m_size;
	UE_ASSERT(offset + size <= vb->m_desc.m_size);

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

	void* vertexData = NULL;
	if (FAILED(vb->m_handle->Lock(offset, size, &vertexData, d3dLockFlags)))
		return NULL;
	return vertexData;
}

void glVertexBuffer_Unlock(glVertexBuffer* vb)
{
	vb->m_handle->Unlock();
}

void glVertexBuffer_OnLostDevice(glVertexBuffer* vb)
{
	if (vb->m_desc.m_flags & glBufferFlags_Dynamic)
		glVertexBuffer_DestroyInPlace(vb);
}

void glVertexBuffer_OnResetDevice(glVertexBuffer* vb)
{
	if (!(vb->m_desc.m_flags & glBufferFlags_Dynamic))
		return;

	glVertexBuffer_CreateInPlace(vb);
}

#else // defined(GL_D3D9)
	UE_NO_EMPTY_FILE
#endif
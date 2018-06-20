#include "Base/ueBase.h"

#if defined(GL_D3D11)

#include "Graphics/glLib_Private.h"

void glBuffer_CreateInPlace(glBuffer* buffer, u32 flags, u32 size, u32 bindFlags, const void* initData = NULL)
{
	D3D11_BUFFER_DESC desc;
	ueMemZeroS(desc);
	if (flags & glBufferFlags_Dynamic)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = (flags & glBufferFlags_WriteOnly) ? D3D11_CPU_ACCESS_WRITE : 0;
	}
	else
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
	}
    desc.ByteWidth = size;
    desc.BindFlags = bindFlags;

	D3D11_SUBRESOURCE_DATA subResourceData;
	subResourceData.pSysMem = initData;
	subResourceData.SysMemPitch = 0;
	subResourceData.SysMemSlicePitch = 0;

	UE_ASSERTWinCall(D3DEV->CreateBuffer(&desc, initData ? &subResourceData : NULL, &buffer->m_handle));

	buffer->m_isLocked = UE_FALSE;
}

void glBuffer_DestroyInPlace(glBuffer* buffer)
{
	if (!buffer->m_handle)
		return;

	UE_ASSERT(!buffer->m_isLocked);

	const UINT refCount = buffer->m_handle->Release();
	UE_ASSERT(refCount == 0);
	buffer->m_handle = NULL;
}

void* glBuffer_Lock(glBuffer* buffer, u32 bufferFlags, u32 lockFlags, u32 offset, u32 size, u32 totalSize)
{
	UE_ASSERT(!buffer->m_isLocked);

	size = size ? size : totalSize;
	UE_ASSERT(offset + size <= totalSize);

	// Lock via scratch pad?

	if (!(bufferFlags & glBufferFlags_Dynamic))
	{
		UE_ASSERT(lockFlags & glBufferLockFlags_Write);
		UE_ASSERT(!(lockFlags & glBufferLockFlags_Read));

		buffer->m_scratchPad = GL_FREQ_ALLOC->Alloc(size);
		if (!buffer->m_scratchPad)
			return NULL;

		buffer->m_scratchPadOffset = offset;
		buffer->m_scratchPadSize = size;
		buffer->m_lockFlags = lockFlags;

		buffer->m_isLocked = UE_TRUE;
		return buffer->m_scratchPad;
	}

	// Lock via mapping

	D3D11_MAP mapType;
	if (lockFlags == glBufferLockFlags_Read)
		mapType = D3D11_MAP_READ;
	else if (lockFlags == (glBufferLockFlags_Read | glBufferLockFlags_Write))
		mapType = D3D11_MAP_READ_WRITE;
	else if (!(lockFlags & glBufferLockFlags_Read) && (lockFlags & glBufferLockFlags_Write))
	{
		if (lockFlags & glBufferLockFlags_NoOverwrite)
			mapType = D3D11_MAP_WRITE_NO_OVERWRITE;
		else if (lockFlags & glBufferLockFlags_Discard)
			mapType = D3D11_MAP_WRITE_DISCARD;
		else
			mapType = D3D11_MAP_WRITE;
	}

	D3D11_MAPPED_SUBRESOURCE mappedSubResource;
	if (FAILED(D3DCTX->Map(buffer->m_handle, 0, mapType, 0, &mappedSubResource)))
		return NULL;

	buffer->m_isLocked = UE_TRUE;
	buffer->m_scratchPad = NULL;

	return (u8*) mappedSubResource.pData + offset;
}

void glBuffer_Unlock(glBuffer* buffer)
{
	UE_ASSERT(buffer->m_isLocked);

	if (buffer->m_scratchPad)
	{
		D3D11_BOX dstRegion;
		dstRegion.top = dstRegion.front = 0;
		dstRegion.bottom = dstRegion.back = 1;
		dstRegion.left = buffer->m_scratchPadOffset;
		dstRegion.right = buffer->m_scratchPadOffset + buffer->m_scratchPadSize;

		D3DCTX->UpdateSubresource(buffer->m_handle, 0, &dstRegion, buffer->m_scratchPad, 0, 0);
		GL_FREQ_ALLOC->Free(buffer->m_scratchPad);
	}
	else
		D3DCTX->Unmap(buffer->m_handle, 0);

	buffer->m_isLocked = UE_FALSE;
}

// Index buffer

void glIndexBuffer_CreateInPlace(glIndexBuffer* buffer, const void* initData)
{
	glBuffer_CreateInPlace(buffer, buffer->m_desc.m_flags, buffer->m_desc.m_numIndices * buffer->m_desc.m_indexSize, D3D11_BIND_INDEX_BUFFER, initData);
}

void glIndexBuffer_DestroyInPlace(glIndexBuffer* buffer)
{
	glBuffer_DestroyInPlace(buffer);
}

void* glIndexBuffer_Lock(glIndexBuffer* buffer, u32 lockFlags, u32 offset, u32 size)
{
	return glBuffer_Lock(buffer, buffer->m_desc.m_flags, lockFlags, offset, size, buffer->m_desc.m_indexSize * buffer->m_desc.m_numIndices);
}

void glIndexBuffer_Unlock(glIndexBuffer* buffer)
{
	glBuffer_Unlock(buffer);
}

// Vertex buffer

void glVertexBuffer_CreateInPlace(glVertexBuffer* buffer, const void* initData)
{
	glBuffer_CreateInPlace(buffer, buffer->m_desc.m_flags, buffer->m_desc.m_size, D3D11_BIND_VERTEX_BUFFER, initData);
}

void glVertexBuffer_DestroyInPlace(glVertexBuffer* buffer)
{
	glBuffer_DestroyInPlace(buffer);
}

void* glVertexBuffer_Lock(glVertexBuffer* buffer, u32 lockFlags, u32 offset, u32 size)
{
	return glBuffer_Lock(buffer, buffer->m_desc.m_flags, lockFlags, offset, size, buffer->m_desc.m_size);
}

void glVertexBuffer_Unlock(glVertexBuffer* buffer)
{
	glBuffer_Unlock(buffer);
}

#else // defined(GL_D3D11)
	UE_NO_EMPTY_FILE
#endif
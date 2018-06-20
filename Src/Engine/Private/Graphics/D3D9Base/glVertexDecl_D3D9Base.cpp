#include "Base/ueBase.h"

#if defined(GL_D3D9) || defined(GL_X360)

#include "Graphics/glLib_Private.h"

void glVertexDeclaration_FindIntersection(const glVertexShaderInput* vsi, u32 numFormats, glStreamFormat** formats, u32& numIntersectionFormats, glStreamFormat** intersectionFormats);

glVertexDeclaration* glVertexShaderInput_GetVertexDeclaration(glVertexShaderInput* vsi, u32 numFormats, glStreamFormat** formats)
{
	// Find existing entry

	glVertexDeclarationEntry temp;
	temp.m_vsi = vsi;
	temp.m_numFormats = numFormats;
	ueMemCpy(temp.m_formats, formats, numFormats * sizeof(glStreamFormat*));

	glVertexDeclarationEntry** vdePtr = GLDEV->m_vertexShaderDecls.Find(&temp);
	if (vdePtr)
		return (*vdePtr)->m_decl;

	// Create new entry

	glVertexDeclarationEntry* entry = new(GLDEV->m_vertexShaderDeclsPool) glVertexDeclarationEntry;
	entry->m_vsi = vsi;
	entry->m_numFormats = numFormats;
	ueMemCpy(entry->m_formats, formats, numFormats * sizeof(glStreamFormat*));
	for (u32 i = 0; i < numFormats; i++)
		if (formats[i])
			formats[i]->m_refCount++;
	entry->m_decl = glVertexDeclaration_Create(vsi, numFormats, formats);

	entry->m_next = vsi->m_entries;
	vsi->m_entries = entry;

	GLDEV->m_vertexShaderDecls.Insert(entry);
	return entry->m_decl;
}

// glVertexDeclaration

void glVertexDeclaration_FindIntersection(const glVertexShaderInput* vsi, u32 numFormats, glStreamFormat** formats, u32& numIntersectionFormats, glStreamFormat** intersectionFormats)
{
	// For each stream determine its minimal necessary subset of elements that
	// contains only required attributes (possibly creating new stream formats)

	numIntersectionFormats = 0;

	for (u32 i = 0; i < numFormats; i++)
	{
		glStreamFormat* srcFormat = formats[i];

		// Empty set?

		if (!srcFormat)
		{
			intersectionFormats[i] = NULL;
			continue;
		}

		glVertexElement dstElements[GL_MAX_VERTEX_STREAM_ELEMENTS];
		glStreamFormatDesc dstFormatDesc;
		dstFormatDesc.m_numElements = 0;
		dstFormatDesc.m_elements = dstElements;
		dstFormatDesc.m_stride = formats[i]->m_stride;

		// Determine matching vertex elements

		for (u32 j = 0; j < srcFormat->m_numElements; j++)
			for (u32 k = 0; k < vsi->m_numAttrs; k++)
			{
				const glShaderAttr& vsiAttr = vsi->m_attrs[k];
				const glVertexElement& vfElem = srcFormat->m_elements[j];
				if (vsiAttr.m_semantic == vfElem.m_semantic && vsiAttr.m_usageIndex == vfElem.m_usageIndex)
				{
					// Found! - add next element
					dstElements[dstFormatDesc.m_numElements++] = vfElem;
					break;
				}
			}

		// If not matches were found, skip the stream

		if (dstFormatDesc.m_numElements == 0)
		{
			intersectionFormats[i] = NULL;
			continue;
		}

		numIntersectionFormats = i + 1;

		// Just add reference to existing stream format if all vertex elements match up

		if (srcFormat->m_numElements == dstFormatDesc.m_numElements)
		{
			intersectionFormats[i] = srcFormat;
			srcFormat->m_refCount++;
		}

		// Create new or get "smaller" stream format

		else
			intersectionFormats[i] = glStreamFormat_Create(&dstFormatDesc);
	}
}

glVertexDeclaration* glVertexDeclaration_Create(const glVertexShaderInput* vsi, u32 numFormats, glStreamFormat** formats)
{
	// Find intersection between vertex shader input and stream formats

	glVertexDeclaration temp;
	glVertexDeclaration_FindIntersection(vsi, numFormats, formats, temp.m_numFormats, temp.m_formats);

	// Try to find existing declaration

	glVertexDeclaration** vdPtr = GLDEV->m_vertexDecls.Find(&temp);
	if (vdPtr)
	{
		// Release stream formats we've just added references to

		for (u32 i = 0; i < temp.m_numFormats; i++)
			if (temp.m_formats[i])
				glStreamFormat_Destroy(temp.m_formats[i]);

		// Return found declaration

		(*vdPtr)->m_refCount++;
		return *vdPtr;
	}

	// Create new one

	glVertexDeclaration* decl = new(GLDEV->m_vertexDeclsPool) glVertexDeclaration;
	UE_ASSERT(decl);
	decl->m_refCount = 1;
	decl->m_numFormats = temp.m_numFormats;
	ueMemCpy(decl->m_formats, temp.m_formats, temp.m_numFormats * sizeof(glStreamFormat*));

	D3DVERTEXELEMENT9 d3dElements[GL_MAX_BOUND_VERTEX_STREAMS * GL_MAX_VERTEX_STREAM_ELEMENTS + 1];
	u32 numD3DElements = 0;
	for (u32 i = 0; i < decl->m_numFormats; i++)
	{
		glStreamFormat* format = decl->m_formats[i];
		if (!format)
			continue;

		for (u32 j = 0; j < format->m_numElements; j++)
		{
			const glVertexElement& element = format->m_elements[j];

			D3DVERTEXELEMENT9& d3dElement = d3dElements[numD3DElements++];

			d3dElement.Stream = i;
			d3dElement.Offset = (WORD) element.m_offset;
			d3dElement.Type = glUtils_ToD3DDECLTYPE((ueNumType) element.m_type, element.m_count, element.m_isNormalized);
			d3dElement.Method = D3DDECLMETHOD_DEFAULT;
			d3dElement.Usage = glUtils_ToD3DDECLUSAGE((glSemantic) element.m_semantic);
			d3dElement.UsageIndex = element.m_usageIndex;
		}
	}

	D3DVERTEXELEMENT9 end = D3DDECL_END();
	ueMemCpy(&d3dElements[numD3DElements], &end, sizeof(D3DVERTEXELEMENT9));

#if defined(GL_D3D9)
	const HRESULT hr = D3DEV->CreateVertexDeclaration(d3dElements, &decl->m_handle);
	UE_ASSERT(SUCCEEDED(hr));
#elif defined(GL_X360)
	UE_NOT_IMPLEMENTED();
#endif

	GLDEV->m_vertexDecls.Insert(decl);

	return decl;
}

void glVertexDeclaration_Destroy(glVertexDeclaration* decl)
{
	UE_ASSERT(decl->m_refCount > 0);

	if (--decl->m_refCount)
		return;

	const ULONG refCount = decl->m_handle->Release();
	UE_ASSERT(refCount == 0);

	for (u32 i = 0; i < decl->m_numFormats; i++)
		if (decl->m_formats[i])
			glStreamFormat_Destroy(decl->m_formats[i]);

	GLDEV->m_vertexDecls.Remove(decl);
	GLDEV->m_vertexDeclsPool.Free(decl);
}

#else // defined(GL_D3D9) || defined(GL_X360)
	UE_NO_EMPTY_FILE
#endif
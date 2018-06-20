#include "IO/ioPackageWriter.h"
#include "IO/ioPackageWriterUtils.h"
#include "IO/ioBuffer.h"
#include "ModelCompiler_Common/ueToolCollisionGeometry.h"
#include "Physics/Bullet/phLib_Bullet_Shared.h"

void CookTriMesh(phMeshDesc* desc, ioBuffer* buffer)
{
	// Build BVH triangle mesh shape

	phCollisionMeshInterface mi;
	mi.m_desc = desc;
	btBvhTriangleMeshShape* triMesh = new btBvhTriangleMeshShape(&mi, true, true);
	UE_ASSERT(triMesh);

	btOptimizedBvh* bvh = triMesh->getOptimizedBvh();

	// Allocate memory for serialized BVH

	int numBytes = bvh->calculateSerializeBufferSize();
	void* tempBuffer = malloc(numBytes + 15);
	UE_ASSERT(tempBuffer);
	void* tempBufferAligned = ueAlignPow2(tempBuffer, 16);

	// Serialize to temp buffer

	bool swapEndian = false;
	UE_ASSERT_FUNC( bvh->serialize(tempBufferAligned, numBytes, swapEndian) );

	// Copy to our buffer

	buffer->WriteBytes(tempBufferAligned, numBytes);

	// Clean up

	free(tempBuffer);
	delete triMesh;
}

void CookConvexMesh(phMeshDesc* desc, ioBuffer* buffer)
{
	UE_NOT_IMPLEMENTED();
}

void WritePrimShape(ioSegmentWriter* sw, const ueToolModel::Primitive& prim)
{
	UE_NOT_IMPLEMENTED();
}

void WriteCollisionMeshDesc(ioSegmentWriter* sw, phMeshDesc* desc)
{
	const bool output16BitIndices = desc->m_numVerts < (1 << 16);

	sw->WriteBool(desc->m_isConvex);
	sw->WriteAlignDefault();
	sw->WriteNumber(desc->m_numIndices);
	ioPtr indicesPtr = sw->WritePtr();
	sw->WriteNumber<u32>(output16BitIndices ? sizeof(u16) : sizeof(u32));

	sw->WriteNumber(desc->m_numVerts);

	ioPtr posPtr = sw->WritePtr();
	sw->WriteNumber(desc->m_posStride);

	sw->WriteNullPtr();
	sw->WriteNumber(desc->m_normalStride);

	// Indices

	sw->BeginPtr(indicesPtr);
	UE_ASSERT(desc->m_indexSize == sizeof(u32));
	const u32* indices = (const u32*) desc->m_indexData;
	if (output16BitIndices)
		for (u32 i = 0; i < desc->m_numIndices; i++)
			sw->WriteNumber<u16>(indices[i]);
	else
		for (u32 i = 0; i < desc->m_numIndices; i++)
			sw->WriteNumber<u32>(indices[i]);

	// Positions

	sw->BeginPtr(posPtr);
	const f32* pos = desc->m_pos;
	for (u32 i = 0; i < desc->m_numVerts; i++)
	{
		sw->WriteNumber(pos[0]);
		sw->WriteNumber(pos[1]);
		sw->WriteNumber(pos[2]);

		((u8*&) pos) += desc->m_posStride;
	}
}

void WriteMeshShape(ioSegmentWriter* sw, const ueToolCollisionMesh& mesh)
{
	// Build mesh description

	phMeshDesc meshDesc;
	meshDesc.m_indexData = &mesh.m_indices[0];
	meshDesc.m_indexSize = sizeof(u32);
	meshDesc.m_numIndices = (u32) mesh.m_indices.size();
	meshDesc.m_isConvex = mesh.m_isConvex;
	meshDesc.m_pos = (const f32*) &mesh.m_pos[0];
	meshDesc.m_posStride = sizeof(ueVec3);
	meshDesc.m_numVerts = (u32) mesh.m_pos.size();

	// Cook mesh data

	ioBuffer buffer;
	buffer.Init(ueAllocator::GetGlobal(), ioBuffer::Mode_Write);
	if (meshDesc.m_isConvex)
		CookConvexMesh(&meshDesc, &buffer);
	else
		CookTriMesh(&meshDesc, &buffer);

	// phMeshShapeDesc_Bullet

	sw->WriteNumber<u32>(mesh.m_isConvex ? phShapeType_Convex : phShapeType_Mesh); // Type m_type
	ioPtr meshDescPtr = sw->WritePtr();					// phMeshDesc* m_meshDesc
	WriteVec3(sw, ueVec3::One);							// ueVec3 m_scale
	sw->WriteNumber<u32>(buffer.GetSize());				// u32 m_bvhSize
	ioPtr dataPtr = sw->WritePtr();						// btOptimizedBvh* m_bvh

	// Write mesh description

	sw->BeginPtr(meshDescPtr);
	WriteCollisionMeshDesc(sw, &meshDesc);

	// Write cooked mesh data

	sw->WriteAlign(16);
	sw->BeginPtr(dataPtr);
	sw->WriteData(buffer.GetData(), buffer.GetSize());
	sw->WriteAlign(16);
}

void WriteCompoundShape(ioSegmentWriter* sw, const ueToolShape& shape)
{
	UE_NOT_IMPLEMENTED();
}

extern "C"
{
	__declspec(dllexport) void WriteCollisionGeometry(ioSegmentWriter* sw, ioSegmentWriter* initSw, const ueToolCollisionGeometry* geom);
}

void WriteCollisionGeometry(ioSegmentWriter* sw, ioSegmentWriter* initSw, const ueToolCollisionGeometry* geom)
{
	// Physics library symbol

	const u32 symbol = UE_BE_4BYTE_SYMBOL('b','l','l','t');
	initSw->WriteData(&symbol, sizeof(u32));

	// gxCollisionGeometry

	sw->WriteNumber<u32>((u32) geom->m_shapes.size());		// u32 m_numParts
	ioPtr shapesPtr = sw->WritePtr();						// Part* m_parts

	std::vector<ioPtr> shapePtrs(geom->m_shapes.size());

	sw->BeginPtr(shapesPtr);
	for (u32 i = 0; i < geom->m_shapes.size(); i++)
	{
		const ueToolShape& shape = geom->m_shapes[i];
		
		sw->WriteNumber<u32>(shape.m_nodeIndex);		// u32 m_nodeIndex
		WriteMat44(sw, shape.m_localTransform);			// ueMat44 m_localTransform
		shapePtrs[i] = sw->WritePtr();					// phShape* m_shape
	}

	// Shapes

	for (u32 i = 0; i < geom->m_shapes.size(); i++)
	{
		const ueToolShape& shape = geom->m_shapes[i];

		sw->BeginPtr(shapePtrs[i]);

		// phShape

		const u32 numSubShapes = (u32) (shape.m_prims.size() + shape.m_meshes.size());
		if (numSubShapes > 1)
			WriteCompoundShape(sw, shape);
		else if (shape.m_prims.size() == 1)
			WritePrimShape(sw, shape.m_prims[0]);
		else
			WriteMeshShape(sw, shape.m_meshes[0]);
	}
}
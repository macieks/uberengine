#include "IO/ioPackageWriter.h"
#include "IO/ioPackageWriterUtils.h"
#include "IO/ioBuffer.h"
#include "ModelCompiler_Common/ueToolCollisionGeometry.h"
#include "Physics/phStructs_Shared.h"

#include "NxCooking.h"
#include "NxCapsuleController.h"
#include "NxControllerManager.h"
#include "NxStream.h"
#include "NxPhysics.h"

extern ueBool phShape_CookTriMesh(NxStream& out, phMeshDesc* desc);
extern ueBool phShape_CookConvexMesh(NxStream& out, phMeshDesc* desc);
extern ueBool phShape_CookSoftBodyMesh(NxStream& out, phSoftMeshDesc* desc);

class phNxStream_Resizable : public NxStream
{
public:
	phNxStream_Resizable(ioBuffer* buffer) : m_buffer(buffer) {}

#define READ_FUNC_BODY(type) { return type(); }
#define WRITE_FUNC_BODY(type) { storeBuffer(&value, sizeof(value)); return *this; }

	virtual		NxU8			readByte()								const READ_FUNC_BODY(NxU8)
	virtual		NxU16			readWord()								const READ_FUNC_BODY(NxU16)
	virtual		NxU32			readDword()								const READ_FUNC_BODY(NxU32)
	virtual		float			readFloat()								const READ_FUNC_BODY(float)
	virtual		double			readDouble()							const READ_FUNC_BODY(double)
	virtual		void			readBuffer(void* buffer, NxU32 size)	const {}

	virtual		NxStream&		storeByte(NxU8 value)		WRITE_FUNC_BODY(NxU8)
	virtual		NxStream&		storeWord(NxU16 value)		WRITE_FUNC_BODY(NxU16)
	virtual		NxStream&		storeDword(NxU32 value)		WRITE_FUNC_BODY(NxU32)
	virtual		NxStream&		storeFloat(NxReal value)	WRITE_FUNC_BODY(NxRead)
	virtual		NxStream&		storeDouble(NxF64 value)	WRITE_FUNC_BODY(NxF64)
	virtual		NxStream&		storeBuffer(const void* buffer, NxU32 size)
	{
		m_buffer->WriteBytes(buffer, size);
		return *this;
	}

private:
	ioBuffer* m_buffer;
};

void WritePrimShape(ioSegmentWriter* sw, const ueToolModel::Primitive& prim)
{
	UE_NOT_IMPLEMENTED();
}

void WriteMeshShape(ioSegmentWriter* sw, ioBuffer* initBuffer, const ueToolCollisionMesh& mesh)
{
	// phShape_Mesh

	sw->WriteBool(false);				// ueBool m_ownsMemory
	ioPtr descPtr = sw->WritePtr();		// phShapeDesc* m_desc
	sw->WriteNullPtr();					// NxTriangleMesh* m_triangleMesh

	// phShapeDesc (base only!)

	sw->BeginPtr(descPtr);
	sw->WriteNumber<u32>(mesh.m_isConvex ? phShapeType_Convex : phShapeType_Mesh); // phShapeDesc::Type m_type

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

	phNxStream_Resizable stream(&buffer);
	if (meshDesc.m_isConvex)
		phShape_CookConvexMesh((NxStream&) stream, &meshDesc);
	else
		phShape_CookTriMesh((NxStream&) stream, &meshDesc);

	// Write cooked mesh data into init buffer

	initBuffer->WriteNumber<u32>(buffer.GetSize());
	initBuffer->WriteBytes(buffer.GetData(), buffer.GetSize());
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
	NxInitCooking();

	// gxCollisionGeometry

	sw->WriteNumber<u32>((u32) geom->m_shapes.size());		// u32 m_numParts
	ioPtr shapesPtr = sw->WritePtr();						// gxCollisionGeometry::Part* m_parts

	std::vector<ioPtr> shapePtrs(geom->m_shapes.size());

	sw->BeginPtr(shapesPtr);
	for (u32 i = 0; i < geom->m_shapes.size(); i++)
	{
		const ueToolShape& shape = geom->m_shapes[i];
		
		sw->WriteNumber<u32>(shape.m_nodeIndex);			// u32 m_nodeIndex
		WriteMat44(sw, shape.m_localTransform);				// ueMat44 m_localTransform
		shapePtrs[i] = sw->WritePtr();						// phShape* m_shape
	}

	// Shapes

	ioBuffer initBuffer;
	initBuffer.Init(ueAllocator::GetGlobal(), ioBuffer::Mode_Write);
	initBuffer.SetTargetEndianess(sw->GetConfig().m_endianess);

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
			WriteMeshShape(sw, &initBuffer, shape.m_meshes[0]);
	}

	// gxCollisionGeometry_InitData

	const u32 symbol = UE_BE_4BYTE_SYMBOL('p','h','s','x');
	initSw->WriteData(&symbol, sizeof(u32));			// u32 m_symbol; Avoid endianess swap
	initSw->WriteNumber<u32>(initBuffer.GetSize());		// u32 m_bufferSize
	if (initBuffer.GetSize() > 0)
	{
		initSw->WriteAndBeginPtr();						// void* m_buffer
		initSw->WriteData(initBuffer.GetData(), initBuffer.GetSize());
	}
	else
		initSw->WriteNullPtr();

	NxCloseCooking();
}
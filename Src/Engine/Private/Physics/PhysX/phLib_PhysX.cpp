#include "Base/ueBase.h"

#if defined(PH_PHYSX)

#include "Physics/PhysX/phLib_PhysX_Private.h"
#include "IO/ueBuffer.h"
#include "Base/Containers/ueHashMap.h"

extern phDebugDraw* s_defaultDebugDraw;

phLibData* g_phLibData = NULL;

// Player controller
// ---------------------------

class MyNxBodyControllerCallback : public NxUserControllerHitReport
{
public:
	NxControllerAction onShapeHit(const NxControllerShapeHit& hit)
	{
		if (hit.shape)
		{
			NxCollisionGroup group = hit.shape->getGroup();
			//cBody* body = (cBody*) hit.shape->getActor().userData;
			if (group != phPhysXCollisionGroup_Static)
			{
				NxActor& actor = hit.shape->getActor();
				if (actor.isDynamic())
				{
					// We only allow horizontal pushes. Vertical pushes when we stand on dynamic objects creates
					// useless stress on the solver. It would be possible to enable/disable vertical pushes on
					// particular objects, if the gameplay requires it.
					if (hit.dir.y == 0.0f)
					{
						NxF32 coeff = actor.getMass() * hit.length * 1.0f;
						actor.addForceAtLocalPos(hit.dir*coeff, NxVec3(0,0,0), NX_IMPULSE);
						//						actor.addForceAtPos(hit.dir*coeff, hit.controller->getPosition(), NX_IMPULSE);
						//						actor.addForceAtPos(hit.dir*coeff, hit.worldPos, NX_IMPULSE);
					}
				}
			}
		}

		return NX_ACTION_NONE;
	}

	NxControllerAction onControllerHit(const NxControllersHit& hit)
	{
		return NX_ACTION_PUSH;
	}
};

static MyNxBodyControllerCallback s_nxBodyControllerCallback;

phPlayerController*	phPlayerController_Create(phPlayerControllerDesc* desc)
{
	UE_ASSERT(g_phLibData);

	phPlayerController* controller = new(g_phLibData->m_allocator) phPlayerController();
	controller->m_userData = NULL;

	NxCapsuleControllerDesc nxDesc;
	nxDesc.height = desc->m_height;
	nxDesc.radius = desc->m_radius;
	nxDesc.slopeLimit = cosf(desc->m_maxSlope);
	nxDesc.stepOffset = desc->m_stepHeight;
	nxDesc.upDirection = NX_Y;
	nxDesc.callback = &s_nxBodyControllerCallback;

	controller->m_controller = g_phLibData->m_controllerManager->createController(desc->m_scene->m_scene, nxDesc);
	UE_ASSERT(controller->m_controller);

	return controller;
}

void phPlayerController_Destroy(phPlayerController* pc)
{
	UE_ASSERT(g_phLibData);
	g_phLibData->m_controllerManager->releaseController(*pc->m_controller);
	ueDelete(pc, g_phLibData->m_allocator);
}

void phPlayerController_SetPosition(phPlayerController* pc, const ueVec3& pos)
{
	UE_ASSERT(g_phLibData);
	pc->m_controller->setPosition( NxExtendedVec3(pos[0], pos[1], pos[2]) );
}

void phPlayerController_GetPosition(phPlayerController* pc, ueVec3& pos)
{
	UE_ASSERT(g_phLibData);
	const NxExtendedVec3& vec = pc->m_controller->getPosition();
	pos.Set((f32) vec.x, (f32) vec.y, (f32) vec.z);
}

void phPlayerController_Jump(phPlayerController* pc, f32 v)
{
	UE_ASSERT(g_phLibData);
	pc->m_jump.StartJump(v);
}

void phPlayerController_Move(phPlayerController* pc, const ueVec3& _dir, f32 dt)
{
	UE_ASSERT(g_phLibData);

	pc->m_jump.Update(dt);

	ueVec3 dir = _dir;
	dir[1] = dir[1] + pc->m_jump.GetHeight();
	dir *= dt;

	NxVec3 vec;
	phPhysXConverter::ToPhysXVec3(vec, dir);

	NxU32 collisionFlags = 0;
	const f32 minDistance = 0.000001f;
	pc->m_controller->move(vec, phPhysXCollidableMask, minDistance, collisionFlags, 0.9f);
	if (collisionFlags & NXCC_COLLISION_DOWN)
		pc->m_jump.StopJump();
}

void phPlayerController_UpdateAll()
{
	UE_ASSERT(g_phLibData);
	g_phLibData->m_controllerManager->updateControllers();
}

// Shape
// ---------------------------

class phNxStream_FixedBuffer : public NxStream
{
public:
	phNxStream_FixedBuffer(void* buffer, u32 bufferSize, ueBool read) :
		m_buffer((u8*) buffer),
		m_capacity(bufferSize),
		m_size(read ? bufferSize : 0),
		m_offset(0)
	{}

	u32 GetOffset() const { return m_offset; }
	void SetOffset(u32 offset) { m_offset = offset; }

#define READ_FUNC_BODY(type) { type value; readBuffer(&value, sizeof(value)); return value; }
#define WRITE_FUNC_BODY(type) { storeBuffer(&value, sizeof(value)); return *this; }

	virtual		NxU8			readByte()								const READ_FUNC_BODY(NxU8)
	virtual		NxU16			readWord()								const READ_FUNC_BODY(NxU16)
	virtual		NxU32			readDword()								const READ_FUNC_BODY(NxU32)
	virtual		float			readFloat()								const READ_FUNC_BODY(float)
	virtual		double			readDouble()							const READ_FUNC_BODY(double)
	virtual		void			readBuffer(void* buffer, NxU32 size)	const
	{
		UE_ASSERT(m_offset + size <= m_size);
		ueMemCpy(buffer, m_buffer + m_offset, size);
		m_offset += size;
	}

	virtual		NxStream&		storeByte(NxU8 value)		WRITE_FUNC_BODY(NxU8)
	virtual		NxStream&		storeWord(NxU16 value)		WRITE_FUNC_BODY(NxU16)
	virtual		NxStream&		storeDword(NxU32 value)		WRITE_FUNC_BODY(NxU32)
	virtual		NxStream&		storeFloat(NxReal value)	WRITE_FUNC_BODY(NxRead)
	virtual		NxStream&		storeDouble(NxF64 value)	WRITE_FUNC_BODY(NxF64)
	virtual		NxStream&		storeBuffer(const void* buffer, NxU32 size)
	{
		UE_ASSERT(m_offset + size <= m_capacity);
		ueMemCpy(m_buffer + m_offset, buffer, size);
		m_offset += size;
		m_size += size;
		return *this;
	}

private:
	u8* m_buffer;
	mutable u32 m_offset;
	u32 m_size;
	u32 m_capacity;
};

NxTriangleMesh* phShape_CookTriMesh(phMeshDesc* desc)
{
	phNxStream_FixedBuffer stream(g_phLibData->m_cookingBuffer, g_phLibData->m_cookingBufferSize, UE_FALSE);
	if (!phShape_CookTriMesh((NxStream&) stream, desc))
		return NULL;

	stream.SetOffset(0);
	return g_phLibData->m_sdk->createTriangleMesh(stream);
}

NxTriangleMesh* phShape_LoadTriMesh(ueBuffer* buffer)
{
	u32 dataSize;
	UE_ASSERT_FUNC( buffer->ReadAny<u32>(dataSize) );

	phNxStream_FixedBuffer stream(buffer->GetCurrData(), dataSize, true);
	NxTriangleMesh* triMesh = g_phLibData->m_sdk->createTriangleMesh(stream);
	UE_ASSERT(triMesh);

	buffer->Seek(dataSize);

	return triMesh;
}

NxConvexMesh* phShape_CookConvexMesh(phMeshDesc* desc)
{
	phNxStream_FixedBuffer stream(g_phLibData->m_cookingBuffer, g_phLibData->m_cookingBufferSize, UE_FALSE);
	if (!phShape_CookConvexMesh((NxStream&) stream, desc))
		return NULL;

	stream.SetOffset(0);
	return g_phLibData->m_sdk->createConvexMesh(stream);
}

NxConvexMesh* phShape_LoadConvexMesh(ueBuffer* buffer)
{
	u32 dataSize;
	UE_ASSERT_FUNC( buffer->ReadAny<u32>(dataSize) );

	phNxStream_FixedBuffer stream(buffer->GetCurrData(), dataSize, true);
	NxConvexMesh* convexMesh = g_phLibData->m_sdk->createConvexMesh(stream);
	UE_ASSERT(convexMesh);

	buffer->Seek(dataSize);

	return convexMesh;
}

NxSoftBodyMesh* phShape_CookSoftBodyMesh(phSoftMeshDesc* desc)
{
	phNxStream_FixedBuffer stream(g_phLibData->m_cookingBuffer, g_phLibData->m_cookingBufferSize, UE_FALSE);
	if (!phShape_CookSoftBodyMesh((NxStream&) stream, desc))
		return NULL;

	stream.SetOffset(0);
	return g_phLibData->m_sdk->createSoftBodyMesh(stream);
}

NxSoftBodyMesh* phShape_LoadSoftMesh(ueBuffer* buffer)
{
	u32 dataSize;
	UE_ASSERT_FUNC( buffer->ReadAny<u32>(dataSize) );

	phNxStream_FixedBuffer stream(buffer->GetCurrData(), dataSize, true);
	NxSoftBodyMesh* softMesh = g_phLibData->m_sdk->createSoftBodyMesh(stream);
	UE_ASSERT(softMesh);

	buffer->Seek(dataSize);

	return softMesh;
}

phShape* phShape_Create(phShapeDesc* desc)
{
	UE_ASSERT(g_phLibData);

	switch (desc->m_type)
	{
#define CASE_SIMPLE_SHAPE(name) \
		case phShapeType_##name: \
		{ \
			const ueSize memorySize = (ueSize) sizeof(phShape) + sizeof(ph##name##ShapeDesc); \
			u8* memory = (u8*) g_phLibData->m_allocator->Alloc(memorySize); \
			UE_ASSERT(memory); \
			ph##name##ShapeDesc* _desc = (ph##name##ShapeDesc*) (memory + sizeof(phShape)); \
			*_desc = * (ph##name##ShapeDesc*) desc; \
			return new(memory) phShape(_desc); \
		}

		CASE_SIMPLE_SHAPE(Box)
		CASE_SIMPLE_SHAPE(Sphere)
		CASE_SIMPLE_SHAPE(Cylinder)
		CASE_SIMPLE_SHAPE(Capsule)

		case phShapeType_Convex:
		{
			phConvexShapeDesc* convexShapeDesc = (phConvexShapeDesc*) desc;

			NxConvexMesh* nxConvexMesh = phShape_CookConvexMesh(convexShapeDesc->m_meshDesc);
			UE_ASSERT(nxConvexMesh);

			const ueSize memorySize = (ueSize) sizeof(phShape_Convex) + sizeof(phConvexShapeDesc);
			u8* memory = (u8*) g_phLibData->m_allocator->Alloc(memorySize);
			UE_ASSERT(memory);
			phConvexShapeDesc* _desc = (phConvexShapeDesc*) memory + sizeof(phShape_Convex);
			*_desc = *convexShapeDesc;
			phShape_Convex* shape = new(memory) phShape_Convex(_desc);
			shape->m_convexMesh = nxConvexMesh;

			return shape;
		}

		case phShapeType_Mesh:
		{
			phMeshShapeDesc* meshShapeDesc = (phMeshShapeDesc*) desc;

			NxTriangleMesh* nxTriMesh = phShape_CookTriMesh(meshShapeDesc->m_meshDesc);
			UE_ASSERT(nxTriMesh);

			const ueSize memorySize = (ueSize) sizeof(phShape_Mesh) + sizeof(phMeshShapeDesc);
			u8* memory = (u8*) g_phLibData->m_allocator->Alloc(memorySize);
			UE_ASSERT(memory);
			phMeshShapeDesc* _desc = (phMeshShapeDesc*) memory + sizeof(phShape_Mesh);
			*_desc = *meshShapeDesc;
			phShape_Mesh* shape = new(memory) phShape_Mesh(_desc);
			shape->m_triangleMesh = nxTriMesh;

			return shape;
		}

		case phShapeType_SoftMesh:
		{
			phSoftMeshShapeDesc* meshShapeDesc = (phSoftMeshShapeDesc*) desc;

			NxSoftBodyMesh* nxSoftMesh = phShape_CookSoftBodyMesh(meshShapeDesc->m_meshDesc);
			UE_ASSERT(nxSoftMesh);

			const ueSize memorySize = (ueSize) sizeof(phShape_SoftMesh) + sizeof(phSoftMeshShapeDesc);
			u8* memory = (u8*) g_phLibData->m_allocator->Alloc(memorySize);
			UE_ASSERT(memory);
			phSoftMeshShapeDesc* _desc = (phSoftMeshShapeDesc*) memory + sizeof(phShape_SoftMesh);
			*_desc = *meshShapeDesc;
			phShape_SoftMesh* shape = new(memory) phShape_SoftMesh(_desc);
			shape->m_softMesh = nxSoftMesh;

			return shape;
		}

		case phShapeType_Compound:
		{
			UE_NOT_IMPLEMENTED();
			return NULL;
		}

		default:
			UE_ASSERT(0);
			return NULL;
	}
}

phShape* phShape_CreateInPlace(void* data, ueBuffer* tempBuffer)
{
	UE_ASSERT(g_phLibData);

	phShape* shape = (phShape*) data;
	switch (shape->m_desc->m_type)
	{
		case phShapeType_Box:
		case phShapeType_Sphere:
		case phShapeType_Cylinder:
		case phShapeType_Capsule:
			return shape;

		case phShapeType_Convex:
		{
			phShape_Convex* convexShape = (phShape_Convex*) shape;
			convexShape->m_convexMesh = phShape_LoadConvexMesh(tempBuffer);
			return shape;
		}

		case phShapeType_Mesh:
		{
			phShape_Mesh* meshShape = (phShape_Mesh*) shape;
			meshShape->m_triangleMesh = phShape_LoadTriMesh(tempBuffer);
			return shape;
		}

		case phShapeType_SoftMesh:
		{
			phShape_SoftMesh* meshShape = (phShape_SoftMesh*) shape;
			meshShape->m_softMesh = phShape_LoadSoftMesh(tempBuffer);
			return shape;
		}

		case phShapeType_Compound:
		{
			UE_NOT_IMPLEMENTED();
			return NULL;
		}

		default:
			UE_ASSERT(0);
			return NULL;
	}

	return shape;
}

void phShape_Destroy(phShape* shape)
{
	UE_ASSERT(g_phLibData);

	switch (shape->m_desc->m_type)
	{
		case phShapeType_Convex:
		{
			phShape_Convex* convexShape = (phShape_Convex*) shape;
			g_phLibData->m_sdk->releaseConvexMesh(*convexShape->m_convexMesh);
			break;
		}

		case phShapeType_Mesh:
		{
			phShape_Mesh* meshShape = (phShape_Mesh*) shape;
			g_phLibData->m_sdk->releaseTriangleMesh(*meshShape->m_triangleMesh);
			break;
		}

		case phShapeType_SoftMesh:
		{
			phShape_SoftMesh* meshShape = (phShape_SoftMesh*) shape;
			g_phLibData->m_sdk->releaseSoftBodyMesh(*meshShape->m_softMesh);
			break;
		}

		case phShapeType_Compound:
		{
			phShape_Compound* compoundShape = (phShape_Compound*) shape;
			phCompoundShapeDesc* shapeDesc = (phCompoundShapeDesc*) shape->m_desc;
			for (u32 i = 0; i < shapeDesc->m_numShapes; i++)
				phShape_Destroy(compoundShape->m_shapes[i]);
			break;
		}
	}
	if (shape->m_ownsMemory)
		ueDelete(shape, g_phLibData->m_allocator);
}

// Body
// ---------------------------

void phBody_AddShape(NxActorDesc* actorDesc, ueBool isRigid, phShape* shape, const ueMat44* transform, u8*& tempBuffer, u8* tempBufferEnd)
{
	switch (shape->m_desc->m_type)
	{
		case phShapeType_Box:
		{
			phBoxShapeDesc* shapeDesc = (phBoxShapeDesc*) shape->m_desc;

			NxBoxShapeDesc* nxShapeDesc = new(tempBuffer) NxBoxShapeDesc();
			tempBuffer += sizeof(NxBoxShapeDesc);
			UE_ASSERT(tempBuffer <= tempBufferEnd);
			phPhysXConverter::ToPhysXMat44(nxShapeDesc->localPose, *transform);
			nxShapeDesc->group = isRigid ? phPhysXCollisionGroup_Dynamic : phPhysXCollisionGroup_Static;
			phPhysXConverter::ToPhysXVec3(nxShapeDesc->dimensions, shapeDesc->m_size);
			nxShapeDesc->dimensions *= 0.5f;

			actorDesc->shapes.push_back(nxShapeDesc);
			break;
		}

		case phShapeType_Sphere:
		{
			phSphereShapeDesc* shapeDesc = (phSphereShapeDesc*) shape->m_desc;

			NxSphereShapeDesc* nxShapeDesc = new(tempBuffer) NxSphereShapeDesc();
			tempBuffer += sizeof(NxSphereShapeDesc);
			UE_ASSERT(tempBuffer <= tempBufferEnd);
			phPhysXConverter::ToPhysXMat44(nxShapeDesc->localPose, *transform);
			nxShapeDesc->group = isRigid ? phPhysXCollisionGroup_Dynamic : phPhysXCollisionGroup_Static;
			nxShapeDesc->radius = shapeDesc->m_radius;

			actorDesc->shapes.push_back(nxShapeDesc);
			break;
		}

		case phShapeType_Cylinder:
		{
			phCylinderShapeDesc* shapeDesc = (phCylinderShapeDesc*) shape->m_desc;
			UE_NOT_IMPLEMENTED(); // PhysX doesn't support cylinder shape
			break;
		}

		case phShapeType_Capsule:
		{
			phCapsuleShapeDesc* shapeDesc = (phCapsuleShapeDesc*) shape->m_desc;

			NxCapsuleShapeDesc* nxShapeDesc = new(tempBuffer) NxCapsuleShapeDesc();
			tempBuffer += sizeof(NxCapsuleShapeDesc);
			UE_ASSERT(tempBuffer <= tempBufferEnd);
			phPhysXConverter::ToPhysXMat44(nxShapeDesc->localPose, *transform);
			nxShapeDesc->group = isRigid ? phPhysXCollisionGroup_Dynamic : phPhysXCollisionGroup_Static;
			nxShapeDesc->radius = shapeDesc->m_radius;
			nxShapeDesc->height = shapeDesc->m_height;

			actorDesc->shapes.push_back(nxShapeDesc);
			break;
		}

		case phShapeType_Convex:
		{
			NxConvexShapeDesc* nxShapeDesc = new(tempBuffer) NxConvexShapeDesc();
			tempBuffer += sizeof(NxConvexShapeDesc);
			UE_ASSERT(tempBuffer <= tempBufferEnd);
			phPhysXConverter::ToPhysXMat44(nxShapeDesc->localPose, *transform);
			nxShapeDesc->group = isRigid ? phPhysXCollisionGroup_Dynamic : phPhysXCollisionGroup_Static;
			nxShapeDesc->meshData = ((phShape_Convex*) shape)->m_convexMesh;

			actorDesc->shapes.push_back(nxShapeDesc);
			break;
		}

		case phShapeType_Mesh:
		{
			UE_ASSERT_MSG(!isRigid, "Dynamic mesh shapes not supported by PhysX");

			NxTriangleMeshShapeDesc* nxShapeDesc = new(tempBuffer) NxTriangleMeshShapeDesc();
			tempBuffer += sizeof(NxTriangleMeshShapeDesc);
			UE_ASSERT(tempBuffer <= tempBufferEnd);
			phPhysXConverter::ToPhysXMat44(nxShapeDesc->localPose, *transform);
			nxShapeDesc->group = isRigid ? phPhysXCollisionGroup_Dynamic : phPhysXCollisionGroup_Static;
			nxShapeDesc->meshData = ((phShape_Mesh*) shape)->m_triangleMesh;

			actorDesc->shapes.push_back(nxShapeDesc);
			break;
		}

		case phShapeType_Compound:
		{
			phShape_Compound* compoundShape = (phShape_Compound*) shape;
			phCompoundShapeDesc* shapeDesc = (phCompoundShapeDesc*) shape->m_desc;

			for (u32 i = 0; i < shapeDesc->m_numShapes; i++)
			{
				phCompoundShapeDesc::SubShapeDesc& subShapeDesc = shapeDesc->m_shapes[i];

				ueMat44 subTransform(*transform);
				subTransform.Rotate(subShapeDesc.m_rotation);
				subTransform.Translate(subShapeDesc.m_translation);
				subTransform.Scale(subShapeDesc.m_scale);

				phBody_AddShape(actorDesc, isRigid, compoundShape->m_shapes[i], &subTransform, tempBuffer, tempBufferEnd);
			}
			break;
		}

		default:
			UE_ASSERT(0);
			break;
	}
}

// phSoftBody
// ----------------------------------

phSoftBody*	phSoftBody_Create(phScene* scene, phSoftBodyDesc* desc)
{
	UE_ASSERT(g_phLibData);
	UE_ASSERT(desc->m_shape->m_desc->m_type == phShapeType_SoftMesh);

	phShape_SoftMesh* softMeshShape = (phShape_SoftMesh*) desc->m_shape;
	phSoftMeshShapeDesc* softMeshShapeDesc = (phSoftMeshShapeDesc*) softMeshShape->m_desc;
	phSoftMeshDesc* softMeshDesc = (phSoftMeshDesc*) softMeshShapeDesc->m_meshDesc;

	const u32 memoryRequired = sizeof(phSoftBody) + sizeof(f32) * 3 * desc->m_maxVerts + sizeof(u32) * desc->m_maxIndices;
	u8* memory = (u8*) g_phLibData->m_allocator->Alloc(memoryRequired);
	if (!memory)
		return NULL;

	phSoftBody* softBody = (phSoftBody*) memory;
	memory += sizeof(phSoftBody);

	softBody->m_maxVerts = desc->m_maxVerts;
	softBody->m_numVerts = 0;
	softBody->m_pos = (f32*) memory;
	memory += sizeof(f32) * 3 * desc->m_maxVerts;

	softBody->m_maxIndices = desc->m_maxIndices;
	softBody->m_numIndices = 0;
	softBody->m_indices = (u32*) memory;

	softBody->m_userData = NULL;

	NxSoftBodyDesc nxDesc;
	nxDesc.flags |=
		NX_SBF_COLLISION_TWOWAY |
		(g_phLibData->m_supportDebugVisualization ? NX_SBF_VISUALIZATION : 0);
	phPhysXConverter::ToPhysXMat44(nxDesc.globalPose, *desc->m_transform);
	nxDesc.collisionGroup = phPhysXCollisionGroup_Dynamic;
	nxDesc.softBodyMesh = softMeshShape->m_softMesh;
	nxDesc.volumeStiffness = desc->m_volumeStiffness;
	if (desc->m_volumeStiffness != 0.0f)
		nxDesc.flags |= NX_SBF_VOLUME_CONSERVATION;
	nxDesc.stretchingStiffness = desc->m_stretchingStiffness;

	nxDesc.meshData.numVerticesPtr = &softBody->m_numVerts;
	nxDesc.meshData.maxVertices = desc->m_maxVerts;
	nxDesc.meshData.verticesPosBegin = softBody->m_pos;
	nxDesc.meshData.verticesPosByteStride = sizeof(f32) * 3;
	nxDesc.meshData.numIndicesPtr = &softBody->m_numIndices;
	nxDesc.meshData.maxIndices = desc->m_maxIndices;
	nxDesc.meshData.indicesBegin = softBody->m_indices;
	nxDesc.meshData.indicesByteStride = sizeof(u32);

	softBody->m_body = scene->m_scene->createSoftBody(nxDesc);
	UE_ASSERT(softBody->m_body);

	softBody->m_body->userData = softBody;

	return softBody;
}

void phSoftBody_Destroy(phSoftBody* body)
{
	UE_ASSERT(g_phLibData);
	body->m_body->getScene().releaseSoftBody(*body->m_body);
	g_phLibData->m_allocator->Free(body);
}

void phSoftBody_ApplyImpulseAt(phSoftBody* body, const ueVec3& pos)
{
	UE_ASSERT(g_phLibData);
	NxVec3 vec;
	phPhysXConverter::ToPhysXVec3(vec, pos);
	body->m_body->addForceAtPos(vec, 10.0f, 10.0f);
}

void phSoftBody_SetUserData(phSoftBody* body, void* userData)
{
	UE_ASSERT(g_phLibData);
	body->m_userData = userData;
}

void* phSoftBody_GetUserData(phSoftBody* body)
{
	UE_ASSERT(g_phLibData);
	return body->m_userData;
}

// phBody
// ----------------------------------

phBody*	phBody_Create(phScene* scene, phBodyDesc* desc)
{
	UE_ASSERT(g_phLibData);
	const ueBool isRigid = desc->m_mass != 0.0f;

	// Add shapes to the body

	u8 tempBuffer[8192]; // FIXME
	u8* tempBufferPtr = tempBuffer;

	NxActorDesc actorDesc;
	phPhysXConverter::ToPhysXMat44(actorDesc.globalPose, *desc->m_transform);
	phBody_AddShape(&actorDesc, isRigid, desc->m_shape, &ueMat44::Identity, tempBufferPtr, tempBufferPtr + UE_ARRAY_SIZE(tempBuffer));

	// Set up body

	NxBodyDesc bodyDesc;
	if (isRigid)
	{
		bodyDesc.angularDamping	= 0.5f;
		bodyDesc.mass = desc->m_mass;

		actorDesc.group = phPhysXCollisionGroup_Dynamic;
		actorDesc.body = &bodyDesc;
	}
	else
		actorDesc.group = phPhysXCollisionGroup_Static;

	// Create body / actor

	NxActor* actor = scene->m_scene->createActor(actorDesc);
	return (phBody*) actor;
}

void phBody_Destroy(phBody* body)
{
	UE_ASSERT(g_phLibData);
	NxActor* actor = (NxActor*) body;
	actor->getScene().releaseActor(*actor);
}

ueBool phBody_IsDynamic(phBody* body)
{
	UE_ASSERT(g_phLibData);
	NxActor* actor = (NxActor*) body;
	return actor->isDynamic() ? UE_TRUE : UE_FALSE;
}

void phBody_GetTransform(phBody* body, ueMat44& transform)
{
	UE_ASSERT(g_phLibData);
	phPhysXConverter::FromPhysXMat44(transform, ((NxActor*) body)->getGlobalPose());
}

void phBody_SetTransform(phBody* body, const ueMat44& transform)
{
	UE_ASSERT(g_phLibData);
	NxMat34 nxTransform;
	phPhysXConverter::ToPhysXMat44(nxTransform, transform);
	((NxActor*) body)->setGlobalPose(nxTransform);
}

void phBody_GetLinearVelocity(phBody* body, ueVec3& out)
{
	UE_ASSERT(g_phLibData);
	phPhysXConverter::FromPhysXVec3(out, ((NxActor*) body)->getLinearVelocity());
}

void phBody_SetLinearVelocity(phBody* body, ueVec3 velocity)
{
	UE_ASSERT(g_phLibData);
	NxVec3 vec;
	phPhysXConverter::ToPhysXVec3(vec, velocity);
	((NxActor*) body)->setLinearVelocity(vec);
}

f32 phBody_GetSpeed(phBody* body)
{
	UE_ASSERT(g_phLibData);
	ueVec3 vel;
	phBody_GetLinearVelocity(body, vel);
	return vel.Len();
}

f32 phBody_GetMass(phBody* body)
{
	UE_ASSERT(g_phLibData);
	return ((NxActor*) body)->getMass();
}

void phBody_SetMass(phBody* body, f32 mass)
{
	UE_ASSERT(g_phLibData);
	((NxActor*) body)->setMass(mass);
}

u32 phBody_GetCollisionFlags(phBody* body)
{
	UE_ASSERT(g_phLibData);
	UE_NOT_IMPLEMENTED();
	return 0;
}

void phBody_SetCollisionFlags(phBody* body, u32 flags)
{
	UE_ASSERT(g_phLibData);
	UE_NOT_IMPLEMENTED();
}

void phBody_ApplyTorque(phBody* body, const ueVec3& torque)
{
	UE_ASSERT(g_phLibData);
	NxVec3 vec;
	phPhysXConverter::ToPhysXVec3(vec, torque);
	((NxActor*) body)->addTorque(vec);
}

void phBody_ApplyImpulse(phBody* body, const ueVec3& impulse)
{
	UE_ASSERT(g_phLibData);
	NxVec3 vec;
	phPhysXConverter::ToPhysXVec3(vec, impulse);
	((NxActor*) body)->addForce(vec);
}

void phBody_SetUserData(phBody* body, void* userData)
{
	UE_ASSERT(g_phLibData);
	((NxActor*) body)->userData = userData;
}

void* phBody_GetUserData(phBody* body)
{
	UE_ASSERT(g_phLibData);
	return ((NxActor*) body)->userData;
}

// Scene
// ---------------------------

phScene* phScene_Create(const phSceneDesc* desc)
{
	UE_ASSERT(g_phLibData);

	phScene* scene = new(g_phLibData->m_allocator) phScene;
	UE_ASSERT(scene);

	NxSceneDesc sceneDesc;
	sceneDesc.gravity.set(0, -9.81f, 0);
	scene->m_scene = g_phLibData->m_sdk->createScene(sceneDesc);
	UE_ASSERT(scene->m_scene);

	// Set default material
	NxMaterial* defaultMaterial = scene->m_scene->getMaterialFromIndex(0);
	defaultMaterial->setRestitution(0.0f);
	defaultMaterial->setStaticFriction(0.1f);
	defaultMaterial->setDynamicFriction(0.1f);

	return scene;
}

void phScene_Destroy(phScene* scene)
{
	UE_ASSERT(g_phLibData);
	g_phLibData->m_sdk->releaseScene(*scene->m_scene);
	ueDelete(scene, g_phLibData->m_allocator);
}

void phScene_BeginUpdate(phScene* scene, f32 dt)
{
	UE_ASSERT(g_phLibData);
	scene->m_scene->simulate(dt);
	scene->m_scene->flushStream();
}

void phScene_EndUpdate(phScene* scene)
{
	UE_ASSERT(g_phLibData);
	scene->m_scene->fetchResults(NX_RIGID_BODY_FINISHED, true);
}

void phScene_ProcessCollisions(phScene* scene, phCollisionCallback callback)
{
	UE_ASSERT(g_phLibData);
	UE_NOT_IMPLEMENTED();
}

ueBool phScene_IntersectShape(phScene* scene, const phShapeIntersectionTestDesc* desc, phShapeIntersectionResultSet* result)
{
	UE_ASSERT(g_phLibData);
	return UE_FALSE;
}

ueBool phScene_IntersectRay(phScene* scene, const phRayIntersectionTestDesc* desc, phShapeIntersectionResultSet* result)
{
	UE_ASSERT(g_phLibData);

	// Create ray

	ueVec3 direction;
	ueVec3::Sub(direction, desc->m_end, desc->m_start);
	const f32 length = direction.Len();
	direction /= length; // Normalize

	NxRay ray;
	phPhysXConverter::ToPhysXVec3(ray.orig, desc->m_start);
	phPhysXConverter::ToPhysXVec3(ray.dir, direction);

	// Case 1: closest raycast with up to 1 result

	if (desc->m_findClosestIntersections && desc->m_maxNumResults == 1 && desc->m_numInclBodies == 0)
	{
		NxRaycastHit hit;
		NxShape* closestShape = scene->m_scene->raycastClosestShape(ray, NX_ALL_SHAPES, hit, 0xffffffff, length);
		if (!closestShape)
		{
			result->m_numResults = 0;
			return UE_FALSE;
		}

		result->m_numResults = 1;
		phShapeIntersectionResult* r = &result->m_results[0];
		r->m_body = (phBody*) &closestShape->getActor();
		r->m_distance = hit.distance;
		phPhysXConverter::FromPhysXVec3(r->m_normal, hit.worldNormal);
		phPhysXConverter::FromPhysXVec3(r->m_position, hit.worldImpact);
		r->m_fraction = hit.distance / length;
		r->m_triangleIndex = hit.faceID;
		return UE_TRUE;
	}

	// Case 2: any number of results; not necessarily closest ones

	UE_NOT_IMPLEMENTED();
	return UE_FALSE;
}

void phScene_SetGravity(phScene* scene, const ueVec3& gravity)
{
	UE_ASSERT(g_phLibData);

	NxVec3 vec;
	phPhysXConverter::ToPhysXVec3(vec, gravity);
	scene->m_scene->setGravity(vec);
}

void phScene_GetGravity(ueVec3& out, phScene* scene)
{
	UE_ASSERT(g_phLibData);

	NxVec3 nxVec;
	scene->m_scene->getGravity(nxVec);
	phPhysXConverter::FromPhysXVec3(out, nxVec);
}

void phSoftBody_DebugDraw(NxSoftBody* _sb, phDebugDraw* debugDraw)
{
	phSoftBody* sb = (phSoftBody*) _sb->userData;

	for (u32 i = 0; i < sb->m_numIndices; i += 4)
	{
		const u32 ids[4] =
		{
			sb->m_indices[i],
			sb->m_indices[i + 1],
			sb->m_indices[i + 2],
			sb->m_indices[i + 3]
		};

		const ueVec3 pos[4] =
		{
			*(ueVec3*) &sb->m_pos[ids[0] * 3],
			*(ueVec3*) &sb->m_pos[ids[1] * 3],
			*(ueVec3*) &sb->m_pos[ids[2] * 3],
			*(ueVec3*) &sb->m_pos[ids[3] * 3]
		};

		debugDraw->DrawLine(pos[0], pos[1], ueColor32::White);
		debugDraw->DrawLine(pos[0], pos[2], ueColor32::White);
		debugDraw->DrawLine(pos[0], pos[3], ueColor32::White);
		debugDraw->DrawLine(pos[1], pos[2], ueColor32::White);
		debugDraw->DrawLine(pos[1], pos[3], ueColor32::White);
		debugDraw->DrawLine(pos[2], pos[3], ueColor32::White);
	}
}

void phScene_DebugDraw(phScene* scene, phDebugDraw* debugDraw)
{
	UE_ASSERT(g_phLibData);

	if (!debugDraw)
		debugDraw = s_defaultDebugDraw;

	const NxDebugRenderable* dr = scene->m_scene->getDebugRenderable();

	const u32 numPoints = dr->getNbPoints();
	const NxDebugPoint* points = dr->getPoints();
	for (u32 i = 0; i < numPoints; i++)
	{
		// TODO: Draw points
	}

	const u32 numLines = dr->getNbLines();
	const NxDebugLine* lines = dr->getLines();
	for (u32 i = 0; i < numLines; i++)
	{
		const NxDebugLine& line = lines[i];

		ueVec3 p0, p1;
		phPhysXConverter::FromPhysXVec3(p0, line.p0);
		phPhysXConverter::FromPhysXVec3(p1, line.p1);

		ueColor32 color = ueColor32::FromARGB(line.color);
		color.a = 255;

		debugDraw->DrawLine(p0, p1, color);
	}

	const u32 numTris = dr->getNbTriangles();
	const NxDebugTriangle* tris = dr->getTriangles();
	for (u32 i = 0; i < numTris; i++)
	{
		const NxDebugTriangle& tri = tris[i];

		ueVec3 p0, p1, p2;
		phPhysXConverter::FromPhysXVec3(p0, tri.p0);
		phPhysXConverter::FromPhysXVec3(p1, tri.p1);
		phPhysXConverter::FromPhysXVec3(p2, tri.p2);

		ueColor32 color = ueColor32::FromARGB(tri.color);
		color.a = 255;

		debugDraw->DrawLine(p0, p1, color);
		debugDraw->DrawLine(p1, p2, color);
		debugDraw->DrawLine(p2, p0, color);
	}
}

// Engine
// ---------------------------

class MyNxOutputStream : public NxUserOutputStream
{
public:
	void reportError(NxErrorCode code, const char* message, const char* file, int line)
	{
		if (code == NXE_DB_WARNING)
		{
			ueLogW("PhysX: '%s', file: %s, line: %d", message, file, line);
			return;
		}
		UE_ASSERT_MSGP(0, "PhysX: '%s', file: %s, line: %d", message, file, line);
	}

	NxAssertResponse reportAssertViolation(const char* message, const char* file, int line)
	{
		UE_ASSERT_MSGP(0, "PhysX ASSERTION: '%s', file: %s, line: %d", message, file, line);
		return NX_AR_CONTINUE;
	}

	void print(const char* message)
	{
		ueLogI("PhysX: %s", message);
	}
};

static MyNxOutputStream s_nxOut;

class MyNxAllocator : public NxUserAllocator
{
public:

	void* mallocDEBUG(size_t size, const char* fileName, int line) { return g_phLibData->m_allocator->Alloc(size); }
	void* mallocDEBUG(size_t size, const char* fileName, int line, const char* className, NxMemoryType type) { return g_phLibData->m_allocator->Alloc(size); }
	void* malloc(size_t size)
	{
		void* ptr = g_phLibData->m_allocator->Alloc(size);
#ifdef UE_DEBUG
		if (ptr)
			ueMemZero(ptr, size);
#endif
		return ptr;
	}
	void* malloc(size_t size, NxMemoryType type) { return malloc(size); }
	void* realloc(void* memory, size_t size) { return g_phLibData->m_allocator->Realloc(memory, size); }
	void free(void* memory) { return g_phLibData->m_allocator->Free(memory); }
	void checkDEBUG() { ueLogD("PhysX: checkDEBUG"); }
};

static MyNxAllocator s_nxAllocator;

void phLib_Startup(phStartupParams* params)
{
	UE_ASSERT(!g_phLibData);

	g_phLibData = new(params->m_allocator) phLibData();
	UE_ASSERT(g_phLibData);
	g_phLibData->m_allocator = params->m_allocator;

	// Allocate cooking buffer

	if (params->m_cookingBufferSize > 0)
	{
		g_phLibData->m_cookingBuffer = (u8*) g_phLibData->m_allocator->Alloc(params->m_cookingBufferSize);
		UE_ASSERT(g_phLibData->m_cookingBuffer);

		NxInitCooking(&s_nxAllocator, &s_nxOut);
	}
	else
		g_phLibData->m_cookingBuffer = NULL;
	g_phLibData->m_cookingBufferSize = params->m_cookingBufferSize;

	// Create PhysX SDK

	NxSDKCreateError errorCode;
	g_phLibData->m_sdk = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, &s_nxAllocator, &s_nxOut, NxPhysicsSDKDesc(), &errorCode);
	UE_ASSERT_MSGP(g_phLibData->m_sdk, "Failed to initialize PhysX (error code = %u). Make sure PhysX runtime is installed.", (u32) errorCode);

	// Set debug SDK parameters

	g_phLibData->m_supportDebugVisualization = params->m_supportDebugVisualization;
	if (params->m_supportDebugVisualization)
	{
		g_phLibData->m_sdk->setParameter(NX_VISUALIZATION_SCALE, 1.0f);
		g_phLibData->m_sdk->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
		g_phLibData->m_sdk->setParameter(NX_VISUALIZE_SOFTBODY_MESH, 1);
	}

	g_phLibData->m_controllerManager = NxCreateControllerManager(&s_nxAllocator);
	UE_ASSERT(g_phLibData->m_controllerManager);
}

void phLib_Shutdown()
{
	UE_ASSERT(g_phLibData);

	if (g_phLibData->m_cookingBuffer)
	{
		g_phLibData->m_allocator->Free(g_phLibData->m_cookingBuffer);
		NxCloseCooking();
	}

	NxReleaseControllerManager(g_phLibData->m_controllerManager);

	g_phLibData->m_sdk->release();

	ueDelete(g_phLibData, g_phLibData->m_allocator);
	g_phLibData = NULL;
}

u32 phLib_GetSymbol()
{
	return UE_BE_4BYTE_SYMBOL('p','h','s','x');
}

#else // defined(PH_PHYSX)
	UE_NO_EMPTY_FILE
#endif
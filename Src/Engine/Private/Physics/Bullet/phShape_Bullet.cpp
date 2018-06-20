#include "Base/ueBase.h"

#if defined(PH_BULLET)

#include "Physics/Bullet/phLib_Bullet_Private.h"
#include "Physics/phTetraBuilder.h"

btCollisionShape* phShape_CreateSoftMesh(phSoftMeshShapeDesc* meshShapeDesc)
{
	phSoftMeshDesc* meshDesc = meshShapeDesc->m_meshDesc;
	phShape_SoftMesh* shape = new phShape_SoftMesh();

	// Build tetra mesh

	phTetraMesh in;
	in.m_isTetra = UE_FALSE;
	UE_ASSERT(meshDesc->m_indexSize == sizeof(u32));
	in.m_indices = (u32*) meshDesc->m_indexData;
	in.m_numIndices = meshDesc->m_numIndices;
	UE_ASSERT(meshDesc->m_posStride == sizeof(f32) * 3);
	in.m_pos = const_cast<f32*>(meshDesc->m_pos);
	in.m_numVerts = meshDesc->m_numVerts;

	phTetraMesh out;
	if (!phTetraBuilder_BuildTetraMesh(out, in, meshDesc->m_subdivisionLevel))
		return UE_FALSE;

	// Copy tetra data

	u8* memory = (u8*) s_data.m_allocator->Alloc(
		sizeof(btVector3) * out.m_numVerts +
		sizeof(phTetraLink) * out.m_numLinks +
		sizeof(u16) * 3 * out.m_numOuterFaces);
	UE_ASSERT(memory);

	UE_ASSERT(out.m_numVerts < (1 << 16));
	shape->m_numVerts = out.m_numVerts;
	shape->m_verts = (btVector3*) memory; memory += sizeof(btVector3) * out.m_numVerts;
	f32* currPos = out.m_pos;
	for (u32 i = 0; i < out.m_numVerts; i++)
	{
		shape->m_verts[i].setValue(*currPos, *(currPos + 1), *(currPos + 2));
		currPos += 3;
	}

	shape->m_numLinks = out.m_numLinks;
	shape->m_links = (phTetraLink*) memory; memory += sizeof(phTetraLink) * out.m_numLinks;
	ueMemCpy(shape->m_links, out.m_links, sizeof(phTetraLink) * out.m_numLinks);

	shape->m_numOuterFaces = out.m_numOuterFaces;
	shape->m_outerFaces = (u16*) memory; memory += sizeof(u16) * 3 * out.m_numOuterFaces;
	ueMemCpy(shape->m_outerFaces, out.m_outerFaces, sizeof(u16) * 3 * out.m_numOuterFaces);

	// Release tetra mesh

	phTetraBuilder_ReleaseMesh(out);

	return shape;
}

btCollisionShape* phShape_CreateBulletShape(phShapeDesc* desc, ueBuffer* tempBuffer)
{
	UE_ASSERT(s_data);

	switch (desc->m_type)
	{
		case phShapeType_Box:
		{
			phBoxShapeDesc* boxDesc = (phBoxShapeDesc*) desc;
			return new btBoxShape(phBulletConverter::ToBulletVec3(boxDesc->m_size * 0.5f));
		}
		case phShapeType_Sphere:
		{
			phSphereShapeDesc* sphereDesc = (phSphereShapeDesc*) desc;
			return new btSphereShape(sphereDesc->m_radius);
		}
		case phShapeType_Capsule:
		{
			phCapsuleShapeDesc* capsuleDesc = (phCapsuleShapeDesc*) desc;
			return new btCapsuleShape(capsuleDesc->m_radius, capsuleDesc->m_height);
		}
		case phShapeType_Cylinder:
		{
			phCylinderShapeDesc* cylinderDesc = (phCylinderShapeDesc*) desc;
			return new btCylinderShape(btVector3(cylinderDesc->m_radius, cylinderDesc->m_height * 0.5f, cylinderDesc->m_radius));
		}
		case phShapeType_Convex:
		{
			UE_ASSERT(!tempBuffer); // In-place deserialization for convex not yet implemented
			phConvexShapeDesc* convexShapeDesc = (phConvexShapeDesc*) desc;
			const phMeshDesc* meshDesc = convexShapeDesc->m_meshDesc;
			UE_ASSERT(meshDesc->m_isConvex);
			return new btConvexHullShape(meshDesc->m_pos, meshDesc->m_numVerts, meshDesc->m_posStride);
		}
		case phShapeType_Mesh:
		{
			phMeshShapeDesc* meshShapeDesc = (phMeshShapeDesc*) desc;

			phCollisionMeshInterface* meshInterface = NULL;
			if (tempBuffer)
			{
				meshInterface = PH_NEW(s_data.m_allocator, phCollisionMeshInterface);
				meshInterface->m_desc = meshShapeDesc->m_meshDesc;
			}
			else
			{
				u8* meshInterfaceMemory = (u8*) s_data.m_allocator->Alloc(sizeof(phCollisionMeshInterface) + sizeof(phMeshDesc));
				UE_ASSERT(meshInterfaceMemory);

				meshInterface = new(meshInterfaceMemory) phCollisionMeshInterface();
				meshInterface->m_desc = (phMeshDesc*) (meshInterface + 1);
				*meshInterface->m_desc = *meshShapeDesc->m_meshDesc;
			}

			btBvhTriangleMeshShape* meshShape = new btBvhTriangleMeshShape(meshInterface, true, !tempBuffer);
			meshShape->setUserPointer(meshInterface);

			if (tempBuffer)
			{
				phMeshShapeDesc_Serialized* meshShapeDesc_serialized = (phMeshShapeDesc_Serialized*) desc;

				meshShapeDesc_serialized->m_bvh = btOptimizedBvh::deSerializeInPlace(meshShapeDesc_serialized->m_bvh, meshShapeDesc_serialized->m_bvhSize, false);
				UE_ASSERT(meshShapeDesc_serialized->m_bvh);
				meshShape->setOptimizedBvh(meshShapeDesc_serialized->m_bvh);
			}

			return meshShape;
		}
		case phShapeType_SoftMesh:
		{
			UE_ASSERT(!tempBuffer); // In-place deserialization for soft mesh not yet implemented
			phSoftMeshShapeDesc* meshShapeDesc = (phSoftMeshShapeDesc*) desc;
			return phShape_CreateSoftMesh(meshShapeDesc);
		}
		case phShapeType_Compound:
		{
			phCompoundShapeDesc* compoundShapeDesc = (phCompoundShapeDesc*) desc;
			btCompoundShape* parentShape = new btCompoundShape();
			for (u32 i = 0; i < compoundShapeDesc->m_numShapes; i++)
			{
				btTransform localTransform;
				localTransform.setIdentity();
				localTransform.setOrigin( phBulletConverter::ToBulletVec3(compoundShapeDesc->m_shapes[i].m_translation) );
				localTransform.setRotation( phBulletConverter::ToBulletQuaternion(compoundShapeDesc->m_shapes[i].m_rotation) );

				btCollisionShape* childShape = phShape_CreateBulletShape(compoundShapeDesc->m_shapes[i].m_shapeDesc, tempBuffer);

				parentShape->addChildShape(localTransform, childShape);
			}
			return parentShape;
		}
		default:
			UE_ASSERT_MSGP(0, "Unsupported shape type (type = %d).", (s32) desc->m_type);
			return NULL;
	}
}

phShape* phShape_Create(phShapeDesc* desc)
{
	return (phShape*) phShape_CreateBulletShape(desc, NULL);
}

phShape* phShape_CreateInPlace(void* data, ueBuffer* tempBuffer)
{
	phShapeDesc* desc = (phShapeDesc*) data;
	return (phShape*) phShape_CreateBulletShape(desc, tempBuffer);
}

void phShape_DestroyRec(btCollisionShape* shape)
{
	switch (shape->getShapeType())
	{
		case COMPOUND_SHAPE_PROXYTYPE:
		{
			btCompoundShape* compoundShape = (btCompoundShape*) shape;
			for (s32 i = 0; i < compoundShape->getNumChildShapes(); i++)
				phShape_DestroyRec(compoundShape->getChildShape(i));
			break;
		}
		case TRIANGLE_MESH_SHAPE_PROXYTYPE:
		{
			btBvhTriangleMeshShape* meshShape = (btBvhTriangleMeshShape*) shape;
			phCollisionMeshInterface* meshInterface = (phCollisionMeshInterface*) meshShape->getUserPointer();
			PH_DELETE(s_data.m_allocator, meshInterface, phCollisionMeshInterface);
			break;
		}
	}
}

void phShape_Destroy(phShape* shape)
{
	phShape_DestroyRec((btCollisionShape*) shape);
	delete (btCollisionShape*) shape;
}

#else // defined(PH_BULLET)
	UE_NO_EMPTY_FILE
#endif
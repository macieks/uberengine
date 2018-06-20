#include "Physics/phLib.h"
#include "GraphicsExt/gxCollision.h"
#include "IO/ueBuffer.h"

void gxCollisionGeometry::Init(gxCollisionGeometry_InitData* initData)
{
#if defined(UE_ENABLE_ASSERTION)
	char gotSymbol[5];
	ueMemCpy(gotSymbol, UE_4BYTE_SYMBOL_STR(initData->m_symbol), 5);
	char expectedSymbol[5];
	ueMemCpy(expectedSymbol, UE_4BYTE_SYMBOL_STR(phLib_GetSymbol()), 5);
	UE_ASSERT_MSGP(initData->m_symbol == phLib_GetSymbol(), "Collision geometry was exported using different physics library (got '%s', expected '%s').", gotSymbol, expectedSymbol);
#endif

	ueBuffer initBuffer(initData->m_buffer, initData->m_bufferSize, ueBuffer::Mode_Read);

	for (u32 i = 0; i < m_numParts; i++)
	{
		m_parts[i].m_shape = phShape_CreateInPlace(m_parts[i].m_shape, &initBuffer);
		UE_ASSERT(m_parts[i].m_shape);
	}
}

void gxCollisionGeometry::Deinit()
{
	for (u32 i = 0; i < m_numParts; i++)
		if (m_parts[i].m_shape)
		{
			phShape_Destroy(m_parts[i].m_shape);
			m_parts[i].m_shape = NULL;
		}
}

u32 gxCollisionGeometry::GetInstanceSize() const
{
	return sizeof(gxCollisionGeometryInstance) + sizeof(phBody*) * m_numParts;
}

gxCollisionGeometryInstance* gxCollisionGeometry::CreateInstanceInPlace(void* buffer, u32 bufferSize)
{
	UE_ASSERT(bufferSize == GetInstanceSize());

	gxCollisionGeometryInstance* inst = (gxCollisionGeometryInstance*) buffer;
	inst->m_scene = NULL;
	inst->m_geometry = this;
	inst->m_bodies = (phBody**) (inst + 1);
	return inst;
}

void gxCollisionGeometry::DestroyInstanceInPlace(gxCollisionGeometryInstance* inst)
{
	inst->RemoveFromScene();
}

void gxCollisionGeometryInstance::AddToScene(phScene* scene)
{
	UE_ASSERT(m_geometry);
	UE_ASSERT(!m_scene);

	m_scene = scene;
	for (u32 i = 0; i < m_geometry->m_numParts; i++)
	{
		phBodyDesc bodyDesc;
		bodyDesc.m_shape = m_geometry->m_parts[i].m_shape;
		bodyDesc.m_mass = 0.0f;
		bodyDesc.m_transform = &ueMat44::Identity;

		phBody* body = phBody_Create(m_scene, &bodyDesc);
		UE_ASSERT(body);

		m_bodies[i] = body;
	}
}

void gxCollisionGeometryInstance::RemoveFromScene()
{
	if (!m_scene)
		return;

	for (u32 i = 0; i < m_geometry->m_numParts; i++)
	{
		phBody_Destroy(m_bodies[i]);
		m_bodies[i] = NULL;
	}
	m_scene = NULL;
}
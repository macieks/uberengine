#include "gmObjectSet.h"
#include "gmObject.h"
#include "gmObjectDef.h"
#include "gmRenderer.h"
#include "GameObjectSystem/gmRenderer.h"
#include "GL/glLib.h"
#include "GX/gxDebugWindow.h"
#include "GX/gxSprite.h"
#include "GX/gxShapeDraw.h"
#include "Physics/Box2D/b2SoftBody.h"
#include "Physics/Box2D/phBox2D.h"
#include "Input/inSys.h"
#include "Audio/auLib.h"
#include "IO/ioFile.h"

gmObjectSet::gmObjectSet() :
	m_def(NULL),
	m_world(NULL),
	m_displayMode(DisplayMode_Default),
	m_isDebugDrawing(UE_FALSE),
	m_isPaused(UE_FALSE),
	m_contacts(NULL)
{}

gmObjectSet::~gmObjectSet()
{
	UE_ASSERT(m_objects.IsEmpty());
	UE_ASSERT(!m_world);
}

ueBool gmObjectSet::Init(const char* templateName, const ueRect& aabb, f32 gravity)
{
	UE_ASSERT(!m_def);

	ueStrCpyS(m_templateName, templateName);
	m_aabb = aabb;
	m_gravity = gravity;

	m_time = 0.0f;

	// Initialize particle set

	m_effects.Init(gmObjectSys_GetAllocator(), 64);

	// Get/load object set definition

	if (templateName)
	{
		m_def = gmObjectSys_GetDef(m_templateName);
		if (!m_def)
			return UE_FALSE;
	}
	else
		m_def = NULL;

	// Create physics world

	CreateWorld();

	// Instanciate object set container

	if (m_def)
	{
		gmTransform transform;
		gmObject* levelObject = AddObject(NULL, m_def, transform, UE_FALSE);
		m_objects.PushFront(levelObject);
	}

	// Destroy dead objects (scheduled to delete during initialization)

	DeleteDeadObjects();

	return UE_TRUE;
}

void gmObjectSet::Deinit(ueBool unloadTemplates)
{
	// Objects

	while (gmObject* obj = m_objects.PopFront())
		ueDelete(obj, gmObjectSys_GetAllocator());

	// Templates

	if (unloadTemplates)
	{
		gmObjectSys_UnloadAllDefs();
		m_def = NULL;
	}

	// Physics world

	DestroyWorld();

	// Particles

	m_effects.DestroyAllEffects();
	m_effects.Deinit();
}

void gmObjectSet::Restart()
{
	Deinit(UE_TRUE);
	Init(m_templateName, m_aabb, m_gravity);
}

gmObject* gmObjectSet::AddObject(gmObjectRef* ref, gmObjectDef* def, const gmTransform& transform, ueBool addToList)
{
	UE_ASSERT(m_world);

	gmObject* object = new(gmObjectSys_GetAllocator()) gmObject(this, ref, def, transform);
	UE_ASSERT(object);

	if (addToList)
		m_objects.PushBack(object);

	object->SendEvent(gmEventType_OnCreate, NULL);

	return object;
}

gmObject* gmObjectSet::AddObject(const char* defName, const gmTransform& transform, ueBool addToList)
{
	gmObjectDef* def = gmObjectSys_GetDef(defName);
	UE_ASSERT(def);
	gmObject* obj = AddObject(NULL, def, transform, addToList);
	gmObjectSys_ReleaseDef(def);
	return obj;
}

void gmObjectSet::Update(f32 dt)
{
	// Handle paused state

	if (m_isPaused)
	{
		m_physicsTime = 0;
		m_gameplayUpdateTime = 0;
		return;
	}

	// Update physics

	UpdatePhysics(dt);

	// Update objects / gameplay

	ueTimer gameplayUpdateTimer;

	for (gmObject* obj = m_objects.Front(); obj; obj = obj->Next())
	{
		if (obj->m_toDelete)
			continue;
		obj->Update(dt);
	}

	// Delete dead objects

	DeleteDeadObjects();

	// Update particles

	m_effects.Update(dt);

	m_gameplayUpdateTime = gameplayUpdateTimer.GetSecs();
	m_time += dt;
}

void gmObjectSet::DeleteDeadObjects()
{
	for (gmObject* obj = m_objects.Front(); obj;)
	{
		if (!obj->m_toDelete)
		{
			obj = obj->Next();
			continue;
		}

		gmObject* tmp = obj;
		obj = obj->Next();

		m_objects.Remove(tmp);
		ueDelete(tmp, gmObjectSys_GetAllocator());
	}
}

void gmObjectSet::ToDrawCoords(f32& x, f32& y)
{
#if !defined(UE_MARMALADE)

	ueMat44 globalTransform;
	ueRect drawRect, unrotatedRect;
	gmRenderer_GetDrawTransforms(globalTransform, drawRect, unrotatedRect);

	ueVec3 xyz(x, y, 0.0f);
	globalTransform.TransformCoord(xyz);

	x = xyz[0];
	y = xyz[1];

#endif
}

void gmObjectSet::Draw(glCtx* ctx)
{
	ueTimer timer;

	// Determine global draw transform and draw rectangle

	ueMat44 globalTransform;
	ueRect drawRect, unrotatedRect;
	gmRenderer_GetDrawTransforms(globalTransform, drawRect, unrotatedRect);

	// Draw objects normally

	if (m_displayMode != DisplayMode_Collision)
	{
		gmRenderer_Begin(ctx, &globalTransform, drawRect);

		for (gmObject* obj = m_objects.Front(); obj; obj = obj->Next())
			obj->Draw();

		gmRenderer_SwitchToEffects();
		m_effects.Draw(ctx, globalTransform, drawRect);

		gmRenderer_End();
	}

	// Draw collision

	if (m_displayMode != DisplayMode_Default)
	{
		gxShapeDrawParams drawParams;
		drawParams.m_2DCanvas = drawRect;
		drawParams.m_canvasTransform = &globalTransform;
		gxShapeDraw_SetDrawParams(&drawParams);
		gxShapeDraw_Begin(ctx);

		m_world->SetDebugDraw(phBox2D_GetDefaultDebugDraw());
		phBox2D_SetDebugDrawAlpha(0.5f);
		phBox2D_SetDebugDrawScale(gmObjectSys_GetFromPhysScale());
		m_isDebugDrawing = UE_TRUE;
		m_world->Step(0, 0);
		m_isDebugDrawing = UE_FALSE;
		m_world->SetDebugDraw(NULL);

		gxShapeDraw_End();
	}

	m_renderingTime = timer.GetSecs();
}

void gmObjectSet::ToggleDisplayMode()
{
	m_displayMode = (DisplayMode) ((m_displayMode + 1) % DisplayMode_MAX);
}

void gmObjectSet::TogglePause()
{
	m_isPaused = !m_isPaused;
}
#ifndef GM_OBJECT_SET_H
#define GM_OBJECT_SET_H

#include "gmObject.h"
#include "GX/gxEffect.h"

#if defined(UE_WIN32) && !defined(UE_FINAL)
	#define GM_ENABLE_EDITOR
#endif

class ueAllocator;
struct glCtx;
class b2World;
struct gmObject;
struct gmObjectDef;
struct gmObjectSet;

struct gmContactListener : public b2ContactListener
{
	void Add(const b2ContactPoint* point);
	void Persist(const b2ContactPoint* point);
	void Remove(const b2ContactPoint* point);
	void Result(const b2ContactResult* point);

	gmObjectSet* m_objectSet;
};

struct gmObjectSet
{
	// Optional - template that created this object set

	char m_templateName[256];
	gmObjectDef* m_def;

	// Objects

	ueList<gmObject> m_objects;

	// Physics

	ueRect m_aabb;
	f32 m_gravity;
	b2World* m_world;

	u32 m_maxContacts;
	u32 m_numContacts;
	b2ContactPoint* m_contacts;

	gmContactListener m_contactListener;

	// Rendering

	enum DisplayMode
	{
		DisplayMode_Default = 0,
		DisplayMode_HalfCollision,
		DisplayMode_Collision,
		DisplayMode_MAX
	};

	DisplayMode m_displayMode;

	ueBool m_isDebugDrawing;

	// Updating

	ueBool m_isPaused;
	f32 m_time;

	// Particles

	gxEffectSet m_effects;

	// Statistics

	f32 m_physicsTime;
	f32 m_renderingTime;
	f32 m_gameplayUpdateTime;

	gmObjectSet();
	~gmObjectSet();

	ueBool Init(const char* templateName, const ueRect& aabb, f32 gravity);
	void Deinit(ueBool unloadTemplates = UE_TRUE);
	void Restart();

	void Draw(glCtx* ctx);
	void Update(f32 dt);

	void ToggleDisplayMode();
	void TogglePause();

	// Physics

	void UpdatePhysics(f32 dt);

	void CreateWorld();
	void DestroyWorld();
	gmObject* AddObject(gmObjectRef* ref, gmObjectDef* def, const gmTransform& transform, ueBool addToList = UE_TRUE);
	gmObject* AddObject(const char* defName, const gmTransform& transform, ueBool addToList = UE_TRUE);

	void ProcessCollisions();
	void OnContactPointSolved(const b2ContactResult* point);
	void DeleteDeadObjects();

	void ToDrawCoords(f32& x, f32& y);
};

#endif // GM_OBJECT_SET_H
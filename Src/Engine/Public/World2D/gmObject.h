#ifndef GM_OBJECT_H
#define GM_OBJECT_H

#include "gmObjectDef.h"

class b2Body;
class b2SoftBody;
class b2World;
class b2Shape;
struct b2ShapeDef;
struct b2SoftShapeDef;
struct gxAnimation2D;
struct ueSoundId;

struct gmShapeDef;
struct gmShape;
struct gmObjectDef;
struct gmObject;
struct gmObjectSet;
struct gmElementDef;
struct gmElement;
struct gmActionDef;
struct gmAction;
struct gmSpriteDef;
struct gxSprite;
struct gmParticleEffectDef;
struct gmParticleEffect;

struct gmSprite
{
	gmSpriteDef* m_def;

	gmElement* m_element;
	b2Shape* m_shape;

	ueBool m_draw;

	ueRect m_rect;
	ueVec2 m_tmpScale; // HACK
	gxSprite* m_sprite;
	ueColor32 m_color;
	f32 m_layer;

	f32 m_rotation;
	f32 m_scale;

	gmTransform m_transform;

	gmSprite* m_next;

	gmSprite(gmElement* element, gmSpriteDef* def, const gmTransform& transform);
	~gmSprite();
	void Update(f32 dt);
	void Draw();

	void CalcBoundingRect(ueRect& rect);
};

struct gmParticleEffect
{
	gmParticleEffectDef* m_def;

	gxEffectHandle m_handle;
	gmTransform m_transform;

	gmParticleEffect* m_next;

	gmParticleEffect(gmParticleEffectDef* def, const gmTransform& transform);
	~gmParticleEffect();
	void Update(f32 dt);
	void Draw();
};

struct gmElement
{
	gmObject* m_object;

	gmElementDef* m_def;

	void* m_body; // Use GetBody() or GetSoftBody() to retrieve proper type

	gmSprite* m_sprites;
	gmParticleEffect* m_particleEffects;

	gmElement* m_next;

	gmElement(gmObject* object, gmElementDef* def, const gmTransform& transform);
	~gmElement();

	void InitBody(const gmTransform& transform);
	void DestroyBody();

	void AddPrimitiveShape(gmShapeDef* def, b2Body* body, const ueVec2& scale);
	void Update(f32 dt);
	void Draw();

	void SendEvent(gmEventType eventType, void* data);
	void CalcBoundingRect(ueRect& rect);

	b2Body* GetBody() const;
	b2SoftBody* GetSoftBody() const;

	gmSprite* FindSprite(const char* name);
};

struct gmObject : ueList<gmObject>::Node
{
	gmObjectSet* m_set;

	gmObjectDef* m_def;
	gmObjectRef* m_ref;

	ueBool m_toDelete;

	gmElement* m_elements;
	void* m_userData;

	gmObject(gmObjectSet* set, gmObjectRef* ref, gmObjectDef* def, const gmTransform& transform);
	~gmObject();
	void Update(f32 dt);
	void Draw();

	void ScheduleToDelete();

	void SendEvent(gmEventType eventType, void* data);
	void CalcBoundingRect(ueRect& rect);
	void GetCenter(f32& x, f32& y);
	void GetCenter(ueVec2& xy);
	static gmObject* GetOther(gmObject* obj, b2ContactPoint* point);
	static gmObject* GetOther(gmObject* obj, b2ContactResult* result);

	gmElement* FindElement(const char* name);

	gmPropertyDef* FindProperty(const char* name);
	ueBool HasProperty(const char* name);
	const char* ReadProperty(const char* name);
	ueBool ReadPropertyFloat(const char* name, f32& value);
	ueBool ReadPropertyInt(const char* name, s32& value);
	ueBool ReadPropertyBool(const char* name, ueBool& value);
};

#endif // GM_OBJECT_H
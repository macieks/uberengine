#ifndef GM_OBJECT_DEF_H
#define GM_OBJECT_DEF_H

#include "Base/ueHash.h"
#include "Base/ueShapes.h"
#include "Containers/ueList.h"
#include "GX/gxTexture.h"
#include "GX/gxEffect.h"
#include <Box2D.h>

class b2Body;
class b2SoftBody;
class b2World;
struct b2ShapeDef;
struct b2SoftShapeDef;
struct ioXmlDoc;
struct ioXmlNode;
struct gxSpriteTemplate;
struct ueSoundId;
struct gmObjectDef;
struct gmObject;
struct gmElementDef;
struct gmElement;
struct gmActionDef;
struct gmAction;

struct gmTransform
{
	ueVec2 m_pos;
	f32 m_rot;
	ueVec2 m_scale;

	UE_INLINE gmTransform()
	{
		m_pos.Set(0, 0);
		m_rot = 0.0f;
		m_scale.Set(1, 1);
	}

	void Load(ioXmlNode* node);
	void Save(ioXmlNode* node);

	void Mul(const gmTransform& t);
	void Align();
};

enum gmEventType
{
	gmEventType_OnCreate, // m_data is NULL
	gmEventType_OnDestroy, // m_data is NULL
	gmEventType_OnTick, // m_data is f32 deltaTime
	gmEventType_OnDraw, // m_data is glCtx context
	gmEventType_OnCollide, // m_data b2ContactPoint
	gmEventType_OnCollide2, // m_data is b2ContactResult
	gmEventType_OnClick,

	gmEventType_MAX
};

struct gmActionCallParams
{
	gmObject* m_object;
	gmElement* m_element;
	void* m_data;
};

typedef void (*gmActionCallFunc)(gmActionCallParams* params);

enum gmShapeType
{
	gmShapeType_Circle = 0,
	gmShapeType_Rectangle,
	gmShapeType_Polygon,
	gmShapeType_SoftCircle,

	gmShapeType_MAX
};

struct gmShapeDef
{
	const char* m_name;

	gmShapeType m_type;

	ueBool m_isSensor;

	f32 m_friction;
	f32 m_density; // 0 - static; >0 - dynamic
	f32 m_restitution; // 0 - no bouncing; 1 - max. bouncing; default=0.2

	u32 m_collisionFlag;
	u32 m_collisionMask;

	gmTransform m_transform;

	struct Shape
	{
		struct Circle
		{
			f32 m_radius;

			Circle() :
				m_radius(15.0f)
			{}
		} m_circle;
		
		struct Rectangle
		{
			f32 m_width;
			f32 m_height;

			Rectangle() :
				m_width(20.0f),
				m_height(20.0f)
			{}
		} m_rectangle;

		struct Polygon
		{
			u32 m_numVerts;
			b2Vec2 m_pos[10];

			Polygon() :
				m_numVerts(0)
			{}
		} m_polygon;

		struct SoftCircle
		{
			f32 m_softness;
			f32 m_radius;
			u32 m_numOuterCircles;

			SoftCircle() :
				m_softness(0.2f),
				m_radius(10.0f),
				m_numOuterCircles(5)
			{}
		} m_softCircle;
	} m_shape;

	gmShapeDef* m_next;

	// Load time only
	ioXmlNode* m_node;

	gmShapeDef();
	~gmShapeDef();
	void PostLoad(gmElementDef* elementDef);

	UE_INLINE ueBool IsPrimitive() const { return m_type == gmShapeType_Circle || m_type == gmShapeType_Rectangle || m_type == gmShapeType_Polygon; }
	UE_INLINE ueBool IsSoft() const { return m_type == gmShapeType_SoftCircle; }

	static gmShapeDef* Load(ioXmlNode* node);
};

struct gmSpriteDef
{
	const char* m_name;

	gxSpriteTemplate* m_spriteTemplate;
	ueResourceHandle<gxTexture> m_texture;

	ueBool m_blendAdditive;

	gmShapeDef* m_shapeDef; // Optional shape to align to
	u32 m_shapeIndex; // Index of the m_shapeDef within parent gmElementDef

	ueRect m_rect; // Used when no body is linked (i.e. sprite is purely visual element)

	gmTransform m_transform;
	ueBool m_noRotation;

	f32 m_rotationSpeed;

	ueColor32 m_color;
	f32 m_layer;

	ueBool m_moveOnTilting; // Useful for 2D background parallax

	gmSpriteDef* m_next;

	// Load time only
	ioXmlNode* m_node;

	gmSpriteDef();
	~gmSpriteDef();
	void PostLoad(gmElementDef* elementDef);

	static gmSpriteDef* Load(ioXmlNode* node);
	void Save(ioXmlNode* parent);
};

struct gmParticleEffectDef
{
	ueResourceHandle<gxEffectType> m_type;
	gmTransform m_transform;
	ueColor32 m_color;
	f32 m_layer;

	gmParticleEffectDef* m_next;

	gmParticleEffectDef();
	~gmParticleEffectDef();

	static gmParticleEffectDef* Load(ioXmlNode* node);
	void Save(ioXmlNode* parent);
};

struct gmEventDef
{
	gmEventType m_type;

	gmActionDef* m_actionDefs;

	gmEventDef* m_next;

	gmEventDef();
	~gmEventDef();
	void Fire(gmObject* object, gmElement* element, void* data);

	static gmEventDef* Load(ioXmlNode* node);
};

enum gmActionType
{
	gmActionType_PlaySound = 0,
	gmActionType_PlayAnimation,
	gmActionType_SpawnParticles,
	gmActionType_Call,
	gmActionType_SelectRandom,

	gmActionType_MAX
};

struct gmActionDef
{
	gmActionType m_type;
	f32 m_frequency;

	gmActionDef* m_next;

	gmActionDef(gmActionType type) :
		m_type(type),
		m_frequency(0.0f),
		m_next(NULL)
	{}
	virtual ~gmActionDef() {}

	virtual void Fire(gmObject* object, gmElement* element, void* data) = 0;

	static gmActionDef* Load(gmEventDef* eventDef, ioXmlNode* node);
};

struct gmActionDef_PlaySound : gmActionDef
{
	ueSoundId* m_soundId;

	gmActionDef_PlaySound() :
		gmActionDef(gmActionType_PlaySound),
		m_soundId(NULL)
	{}
	virtual void Fire(gmObject* object, gmElement* element, void* data) {}

	static gmActionDef_PlaySound* Load(ioXmlNode* node);
};

struct gmActionDef_PlayAnimation : gmActionDef
{
	const char* m_animation;
	u32 m_spriteIndex;

	gmActionDef_PlayAnimation() :
		gmActionDef(gmActionType_PlayAnimation),
		m_animation(NULL),
		m_spriteIndex(0)
	{}
	virtual void Fire(gmObject* object, gmElement* element, void* data) { /* TODO: Implement me */ }

	static gmActionDef_PlayAnimation* Load(ioXmlNode* node);
};

struct gmActionDef_SpawnParticles : gmActionDef
{
	gmParticleEffectDef* m_effectDef;

	gmActionDef_SpawnParticles() :
		gmActionDef(gmActionType_SpawnParticles),
		m_effectDef(NULL)
	{}
	virtual void Fire(gmObject* object, gmElement* element, void* data) {}

	static gmActionDef_SpawnParticles* Load(ioXmlNode* node);
};

struct gmActionDef_Call;
typedef void (*gmAction_Call_Register)(gmEventDef* eventDef, gmActionDef_Call* actionDef, const char* functionName);

struct gmActionDef_Call : gmActionDef
{
	gmActionCallFunc m_func;
	void* m_userData;

	gmActionDef_Call() :
		gmActionDef(gmActionType_Call),
		m_func(NULL),
		m_userData(NULL)
	{}
	void Fire(gmObject* object, gmElement* element, void* data);

	static gmAction_Call_Register m_registerFunc;

	static gmActionDef_Call* Load(gmEventDef* eventDef, ioXmlNode* node);
};

struct gmActionDef_SelectRandom : gmActionDef
{
	gmActionDef* m_actionDefs;

	gmActionDef_SelectRandom() :
		gmActionDef(gmActionType_SelectRandom),
		m_actionDefs(NULL)
	{}
	virtual void Fire(gmObject* object, gmElement* element, void* data) {}

	static gmActionDef_SelectRandom* Load(ioXmlNode* node);
};

struct gmLevelInfo
{
	ueRect m_rect;
	f32 m_gravity;
	f32 m_boundary;

	u32 m_tutorialIndex; // 0xFFFFFFFF indicates no tutorial

	gmLevelInfo();

	static gmLevelInfo* Load(ioXmlNode* node);
	void Save(ioXmlNode* node);

	UE_INLINE ueBool HasTutorial() const { return m_tutorialIndex != 0xFFFFFFFF; }
};

struct gmElementDef
{
	const char* m_name;

	gmSpriteDef* m_spriteDefs;
	u32 m_numShapeDefs;
	gmShapeDef* m_shapeDefs;
	gmEventDef* m_eventDefs;
	gmParticleEffectDef* m_particleEffectDefs;

	gmElementDef* m_next;

	gmElementDef();
	~gmElementDef();
	
	gmShapeDef* FindShapeDef(const char* name, u32* index = NULL);
	gmSpriteDef* FindSpriteDef(const char* name);

	UE_INLINE ueBool IsPrimitive() const { UE_ASSERT(m_shapeDefs); return m_shapeDefs->IsPrimitive(); }
	UE_INLINE ueBool IsSoft() const { UE_ASSERT(m_shapeDefs); return m_shapeDefs->IsSoft(); }

	static gmElementDef* Load(ioXmlNode* node);
	void Save(ioXmlNode* parent);
};

enum gmPropertyType
{
	gmPropertyType_Float = 0,
	gmPropertyType_Int,
	gmPropertyType_Bool,
	gmPropertyType_String,

	gmPropertyType_MAX
};

struct gmPropertyDef
{
	const char* m_name;

	gmPropertyType m_type;

	union
	{
		f32 m_float;
		s32 m_int;
		ueBool m_bool;
		char* m_string;
	} m_value;

	gmPropertyDef* m_next;

	gmPropertyDef();
	~gmPropertyDef();

	static gmPropertyDef* Load(ioXmlNode* node);
	void Save(ioXmlNode* node);

	gmPropertyDef* Clone();
};

struct gmObjectRef : ueList<gmObjectRef>::Node
{
	gmObjectDef* m_def;

	gmPropertyDef* m_propertyDefs;

	gmTransform m_transform;

	gmObjectRef();
	~gmObjectRef();

	static gmObjectRef* Load(ioXmlNode* node);
	void Save(ioXmlNode* parent);

	gmObjectRef* Clone();

	gmPropertyDef* FindProperty(const char* name);
};

struct gmObjectDef
{
	const char* m_name;
	u32 m_refCount;

	gmElementDef* m_elementDefs;
	gmEventDef* m_eventDefs;
	gmPropertyDef* m_propertyDefs;
	ueList<gmObjectRef> m_objectRefs;
	gmLevelInfo* m_levelInfo;

	gmObjectDef();
	~gmObjectDef();

	gmPropertyDef* FindProperty(const char* name);

	static gmObjectDef* Load(const char* name);
	void Save(ioXmlDoc* doc);

	struct HashPred
	{
		UE_INLINE u32 operator () (const gmObjectDef* def) const
		{
			return ueCalcHash(def->m_name);
		}
	};

	struct CmpPred
	{
		s32 operator () (const gmObjectDef* a, const gmObjectDef* b) const
		{
			return ueStrCmp(a->m_name, b->m_name);
		}
	};
};

#define GM_REGISTER_ACTION_CALL(name) \
	if (!ueStrCmp(functionName, #name)) \
	{ \
		actionDef->m_func = gmActionCallFunc_##name; \
		return UE_TRUE; \
	}

void			gmObjectSys_Startup(ueAllocator* allocator, gmAction_Call_Register registerFunc = NULL, u32 maxObjectDefs = 128, f32 toPhysScale = 0.01f);
void			gmObjectSys_Shutdown();

gmObjectDef*	gmObjectSys_GetDef(const char* name);
void			gmObjectSys_ReleaseDef(gmObjectDef* def);
void			gmObjectSys_UnloadAllDefs();

ueAllocator*	gmObjectSys_GetAllocator();
f32				gmObjectSys_GetToPhysScale();
f32				gmObjectSys_GetFromPhysScale();

#endif // GM_OBJECT_DEF_H
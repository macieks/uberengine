#include "gmObjectDef.h"
#include "gmObject.h"
#include "gmObjectSet.h"
#include "Containers/ueHashSet.h"
#include "Physics/Box2D/b2SoftBody.h"
#include "Physics/Box2D/phBox2D.h"
#include "GL/glLib.h"
#include "GX/gxSprite.h"
#include "IO/ioXml.h"

#if defined(UE_DEBUG) && 0
	#define GM_LOG_OBJ ueLogI
#else
	#define GM_LOG_OBJ
#endif

template <class TYPE>
void ReverseList(TYPE*& list)
{
	TYPE* rev = NULL;
	while (list)
	{
		TYPE* obj = list;
		list = list->m_next;

		obj->m_next = rev;
		rev = obj;
	}

	list = rev;
}

typedef ueHashSet<gmObjectDef*, gmObjectDef::HashPred, gmObjectDef::CmpPred> gmObjectDefsSet;

struct gmObjectSysData
{
	ueAllocator* m_allocator;
	gmObjectDefsSet m_defs;
	f32 m_toPhysScale;
	f32 m_fromPhysScale;
};

static gmObjectSysData* s_data = NULL;

gmAction_Call_Register gmActionDef_Call::m_registerFunc = NULL;

void gmObjectSys_Startup(ueAllocator* allocator, gmAction_Call_Register registerFunc, u32 maxObjectDefs, f32 toPhysScale)
{
	UE_ASSERT(!s_data);

	s_data = new(allocator) gmObjectSysData();
	UE_ASSERT(s_data);
	s_data->m_allocator = allocator;
	s_data->m_defs.Init(allocator, maxObjectDefs);
	s_data->m_toPhysScale = toPhysScale;
	s_data->m_fromPhysScale = 1.0f / s_data->m_toPhysScale;

	gmActionDef_Call::m_registerFunc = registerFunc;
}

void gmObjectSys_Shutdown()
{
	UE_ASSERT(s_data);
	gmObjectSys_UnloadAllDefs();
	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

ueAllocator* gmObjectSys_GetAllocator()
{
	return s_data->m_allocator;
}

gmObjectDef* gmObjectSys_GetDef(const char* name)
{
	UE_ASSERT(s_data);

	// Find existing

	gmObjectDef key;
	key.m_name = name;
	gmObjectDef** defPtr = s_data->m_defs.Find(&key);
	if (defPtr)
	{
		gmObjectDef* def = *defPtr;
		def->m_refCount++;
		return def;
	}

	// Load new object template

	gmObjectDef* def = gmObjectDef::Load(name);
	if (!def)
		return NULL;

	def->m_refCount++;
	s_data->m_defs.Insert(def);

	return def;
}

void gmObjectSys_ReleaseDef(gmObjectDef* def)
{
	UE_ASSERT(def->m_refCount > 0);
	def->m_refCount--;
	if (!def->m_refCount)
	{
		s_data->m_defs.Remove(def);
		ueDelete(def, gmObjectSys_GetAllocator());
	}
}

void gmObjectSys_UnloadAllDefs()
{
	UE_ASSERT(s_data);
	gmObjectDefsSet::Iterator iter(s_data->m_defs);
	while (gmObjectDef** defPtr = iter.Next())
		ueDelete(*defPtr, gmObjectSys_GetAllocator());
	s_data->m_defs.Clear();
}

f32 gmObjectSys_GetToPhysScale()
{
	return s_data->m_toPhysScale;
}

f32 gmObjectSys_GetFromPhysScale()
{
	return s_data->m_fromPhysScale;
}

// gmTransform

void gmTransform::Load(ioXmlNode* node)
{
	ioXmlNode_GetAttrValueVec2(node, "pos", m_pos);

	if (!ioXmlNode_GetAttrValueVec2(node, "scale", m_scale))
	{
		f32 scale;
		if (ioXmlNode_GetAttrValueF32(node, "scale", scale))
			m_scale.Set(scale, scale);
	}

	ioXmlNode_GetAttrValueF32(node, "rot", m_rot);
	m_rot = m_rot / 180.0f * UE_PI;
}

void gmTransform::Save(ioXmlNode* node)
{
	if (m_pos != ueVec2::Zero)
		ioXmlNode_AddAttrVec2(node, "pos", m_pos);

	if (m_scale != ueVec2::One)
	{
		if (m_scale[0] == m_scale[1])
			ioXmlNode_AddAttrF32(node, "scale", m_scale[0]);
		else
			ioXmlNode_AddAttrVec2(node, "scale", m_scale);
	}

	if (m_rot != 0.0f)
		ioXmlNode_AddAttrF32(node, "rot", ueRadToDeg(m_rot));
}

void gmTransform::Mul(const gmTransform& t)
{
	ueVec2 rotatedPos(t.m_pos);
	rotatedPos.Rotate(m_rot);
	m_pos += rotatedPos;

	m_rot += t.m_rot;

	m_scale *= t.m_scale;
}

void gmTransform::Align()
{
	m_pos.Set(ueFloor(m_pos[0]), ueFloor(m_pos[1]));

	m_scale.Set(ueFloor(m_scale[0] * 100.0f) * 0.01f, ueFloor(m_scale[1] * 100.0f) * 0.01f);

	m_rot = ueDegToRad(ueFloor(ueRadToDeg(m_rot)));
	if (m_rot > UE_2PI)
		m_rot -= UE_2PI;
	else if (m_rot < 0.0f)
		m_rot += UE_2PI;
}

// gmShapeDef

gmShapeDef::gmShapeDef() :
	m_name(NULL),
	m_type(gmShapeType_MAX),
	m_isSensor(UE_FALSE),
	m_friction(0.3f),
	m_density(0.0f),
	m_restitution(0.2f),
	m_collisionFlag(0xFFFFFFFF),
	m_collisionMask(0xFFFFFFFF),
	m_next(NULL)
{}

gmShapeDef::~gmShapeDef()
{
}

gmShapeDef* gmShapeDef::Load(ioXmlNode* node)
{
	const char* name = ioXmlNode_GetAttrValue(node, "name");
	u32 nameSize = ueStrLen(name);
	if (nameSize)
		nameSize++;

	u8* defMemory = (u8*) gmObjectSys_GetAllocator()->Alloc(sizeof(gmShapeDef) + nameSize);
	UE_ASSERT(defMemory);

	gmShapeDef* def = new(defMemory) gmShapeDef();
	defMemory += sizeof(gmShapeDef);
	
	if (nameSize)
	{
		ueMemCpy(defMemory, name, nameSize);
		def->m_name = (const char*) defMemory;
	}

	def->m_node = node;

	return def;
}

void gmShapeDef::PostLoad(gmElementDef* elementDef)
{
	// Get size-referenced sprite

	const char* sizeRef = ioXmlNode_GetAttrValue(m_node, "sizeRef");
	gmSpriteDef* spriteDef = elementDef->FindSpriteDef(sizeRef);

	// Load body physical description

	const char* type = ioXmlNode_GetAttrValue(m_node, "type");

	GM_LOG_OBJ("    Loading gmShapeDef: type = '%s'...", type);

	ioXmlNode_GetAttrValueF32(m_node, "density", m_density);
	ioXmlNode_GetAttrValueF32(m_node, "friction", m_friction);
	ioXmlNode_GetAttrValueF32(m_node, "restitution", m_restitution);

	ioXmlNode_GetAttrValueU32(m_node, "collisionFlag", m_collisionFlag);
	ioXmlNode_GetAttrValueU32(m_node, "collisionMask", m_collisionMask);

	ioXmlNode_GetAttrValueBool(m_node, "isSensor", m_isSensor);

	m_transform.Load(m_node);

	if (!ueStrCmp(type, "circle"))
	{
		m_type = gmShapeType_Circle;
		if (!ioXmlNode_GetAttrValueF32(m_node, "radius", m_shape.m_circle.m_radius))
		{
			UE_ASSERT(spriteDef);
//			if (spriteDef->m_rect.GetWidth() != spriteDef->m_rect.GetHeight())
//				ueLogW("Sprite is linked to circle body but its width != height");
			m_shape.m_circle.m_radius = (spriteDef->m_rect.GetWidth() + spriteDef->m_rect.GetHeight()) * 0.25f;
		}
	}
	else if (!ueStrCmp(type, "rect"))
	{
		m_type = gmShapeType_Rectangle;
		ueVec2 size;
		f32 scalarSize;
		if (ioXmlNode_GetAttrValueVec2(m_node, "size", size))
		{
			m_shape.m_rectangle.m_width = size[0];
			m_shape.m_rectangle.m_height = size[1];
		}
		else if (ioXmlNode_GetAttrValueF32(m_node, "size", scalarSize))
		{
			m_shape.m_rectangle.m_width = scalarSize;
			m_shape.m_rectangle.m_height = scalarSize;
		}
		else
		{
			UE_ASSERT(spriteDef);
			m_shape.m_rectangle.m_width = spriteDef->m_rect.GetWidth();
			m_shape.m_rectangle.m_height = spriteDef->m_rect.GetHeight();
		}
	}
	else if (!ueStrCmp(type, "soft_circle"))
	{
		m_type = gmShapeType_SoftCircle;

		if (!ioXmlNode_GetAttrValueF32(m_node, "radius", m_shape.m_softCircle.m_radius))
		{
			UE_ASSERT(spriteDef);
//			if (spriteDef->m_rect.GetWidth() != spriteDef->m_rect.GetHeight())
//				ueLogW("Sprite is linked to soft circle body but its width != height");
			m_shape.m_softCircle.m_radius = (spriteDef->m_rect.GetWidth() + spriteDef->m_rect.GetHeight()) * 0.25f;
		}
		ioXmlNode_GetAttrValueU32(m_node, "numOuterCircles", m_shape.m_softCircle.m_numOuterCircles);
		ioXmlNode_GetAttrValueF32(m_node, "softness", m_shape.m_softCircle.m_softness);
	}
	else if (!ueStrCmp(type, "poly"))
	{
		m_type = gmShapeType_Polygon;
		
		m_shape.m_polygon.m_numVerts = ioXmlNode_CalcNumNodes(m_node, "vertex");
		UE_ASSERT(3 <= m_shape.m_polygon.m_numVerts && m_shape.m_polygon.m_numVerts <= UE_ARRAY_SIZE(m_shape.m_polygon.m_pos));

		u32 vertexIndex = 0;
		for (ioXmlNode* vertexNode = ioXmlNode_GetFirstNode(m_node, "vertex"); vertexNode; vertexNode = ioXmlNode_GetNext(vertexNode, "vertex"))
		{
			ueVec2 pos;
			UE_ASSERT_FUNC(ioXmlNode_GetAttrValueVec2(vertexNode, "pos", pos));
			phBox2D_ToBtVec(m_shape.m_polygon.m_pos[vertexIndex], pos);
			vertexIndex++;
		}
	}
	else
	{
		UE_ASSERT_MSGP(0, "Unknown body type '%s'", type);
	}

	// Xml node is not needed anymore

	m_node = NULL;
}

// gmSpriteDef

gmSpriteDef::gmSpriteDef() :
	m_name(NULL),
	m_spriteTemplate(NULL),
	m_blendAdditive(UE_FALSE),
	m_shapeDef(NULL),
	m_layer(0.0f),
	m_noRotation(UE_FALSE),
	m_rotationSpeed(0.0f),
	m_moveOnTilting(UE_FALSE),
	m_next(NULL),
	m_node(NULL)
{}

gmSpriteDef::~gmSpriteDef()
{
	if (m_spriteTemplate)
		gxSpriteTemplate_Release(m_spriteTemplate);
}

gmSpriteDef* gmSpriteDef::Load(ioXmlNode* node)
{
	const char* name = ioXmlNode_GetAttrValue(node, "name");
	u32 nameSize = ueStrLen(name);
	if (nameSize)
		nameSize++;

	u8* spriteMemory = (u8*) gmObjectSys_GetAllocator()->Alloc(sizeof(gmSpriteDef) + nameSize);
	UE_ASSERT(spriteMemory);

	gmSpriteDef* def = new(spriteMemory) gmSpriteDef();
	spriteMemory += sizeof(gmSpriteDef);

	if (nameSize)
	{
		ueMemCpy(spriteMemory, name, nameSize);
		def->m_name = (const char*) spriteMemory;
		spriteMemory += nameSize;
	}

	const char* templateName = NULL;
	const char* textureName = NULL;
	if (templateName = ioXmlNode_GetAttrValue(node, "template"))
	{
		def->m_spriteTemplate = gxSpriteTemplate_Get(templateName);
		UE_ASSERT(def->m_spriteTemplate);
	}
	else if (textureName = ioXmlNode_GetAttrValue(node, "texture"))
	{
		def->m_texture.SetByName(textureName);
	}
	else
		UE_ASSERT(!"Neither 'texture' nor 'template' defined for sprite");

	GM_LOG_OBJ("    Loading gmSpriteDef: name = '%s', template = '%s' texture = '%s'...", name, templateName, textureName);

	def->m_transform.Load(node);
	ioXmlNode_GetAttrValueBool(node, "no_rot", def->m_noRotation);
	
	if (const char* blendMode = ioXmlNode_GetAttrValue(node, "blendMode"))
	{
		if (!ueStrCmp(blendMode, "additive"))
			def->m_blendAdditive = UE_TRUE;
	}

	ioXmlNode_GetAttrValueColor32(node, "color", def->m_color);
	ioXmlNode_GetAttrValueF32(node, "layer", def->m_layer);

	ioXmlNode_GetAttrValueF32(node, "rotationSpeed", def->m_rotationSpeed);

	if (!ioXmlNode_GetAttrValueRect(node, "rect", def->m_rect))
	{
		f32 width, height;

		ueVec2 size;
		if (ioXmlNode_GetAttrValueVec2(node, "size", size))
		{
			width = size[0];
			height = size[1];
		}
		else if (def->m_spriteTemplate)
		{
			u32 texWidth, texHeight;
			gxSpriteTemplate_GetSize(def->m_spriteTemplate, texWidth, texHeight);
			width = (f32) texWidth;
			height = (f32) texHeight;
		}
		else
		{
			width = (f32) gxTexture_GetWidth(*def->m_texture);
			height = (f32) gxTexture_GetHeight(*def->m_texture);
		}

		def->m_rect.Set(-width * 0.5f, -height * 0.5f, width * 0.5f, height * 0.5f);
	}

	ioXmlNode_GetAttrValueBool(node, "moveOnTilting", def->m_moveOnTilting);

	def->m_node = node;

	return def;
}

void gmSpriteDef::PostLoad(gmElementDef* elementDef)
{
	if (const char* alignRef = ioXmlNode_GetAttrValue(m_node, "alignRef"))
	{
		m_shapeDef = elementDef->FindShapeDef(alignRef, &m_shapeIndex);
		UE_ASSERT_MSGP(m_shapeDef, "Failed to find shape '%s' align-ref'ed by sprite", alignRef);
	}

	// Xml node is not needed anymore

	m_node = NULL;
}

void gmSpriteDef::Save(ioXmlNode* parent)
{
	ioXmlNode* node = ioXmlNode_AddNode(parent, "sprite");
	if (m_name)
		ioXmlNode_AddAttr(node, "name", m_name);
	if (m_spriteTemplate)
		ioXmlNode_AddAttr(node, "template", gxSpriteTemplate_GetName(m_spriteTemplate));
	else
		ioXmlNode_AddAttr(node, "texture", m_texture.GetName());

	ioXmlNode_AddAttrRect(node, "rect", m_rect);
	m_transform.Save(node);

	ioXmlNode_AddAttrF32(node, "layer", m_layer);

	if (m_moveOnTilting)
		ioXmlNode_AddAttrBool(node, "moveOnTilting", m_moveOnTilting);

	UE_ASSERT(!m_shapeDef);
	UE_ASSERT(m_color == ueColor32::White);
}

// gmParticleEffectDef

gmParticleEffectDef::gmParticleEffectDef() :
	m_layer(0),
	m_next(NULL)
{}

gmParticleEffectDef::~gmParticleEffectDef()
{
}

gmParticleEffectDef* gmParticleEffectDef::Load(ioXmlNode* node)
{
	gmParticleEffectDef* def = new(gmObjectSys_GetAllocator()) gmParticleEffectDef();
	UE_ASSERT(def);

	const char* type = ioXmlNode_GetAttrValue(node, "type");
	UE_ASSERT(type);
	def->m_type.SetByName(type);

	def->m_transform.Load(node);

	ioXmlNode_GetAttrValueF32(node, "layer", def->m_layer);
	ioXmlNode_GetAttrValueColor32(node, "color", def->m_color);

	return def;
}

void gmParticleEffectDef::Save(ioXmlNode* parent)
{
	ioXmlNode* node = ioXmlNode_AddNode(parent, "effect");
	ioXmlNode_AddAttr(node, "type", m_type.GetName());

	m_transform.Save(node);

	ioXmlNode_AddAttrF32(node, "layer", m_layer);
	ioXmlNode_AddAttrColor32(node, "color", m_color);
}

// gmEvent

gmEventDef::gmEventDef() :
	m_type(gmEventType_MAX),
	m_actionDefs(NULL),
	m_next(NULL)
{}

gmEventDef::~gmEventDef()
{
	while (m_actionDefs)
	{
		gmActionDef* def = m_actionDefs;
		m_actionDefs = m_actionDefs->m_next;
		ueDelete(def, gmObjectSys_GetAllocator());
	}
}

gmEventDef* gmEventDef::Load(ioXmlNode* node)
{
	gmEventDef* def = new(gmObjectSys_GetAllocator()) gmEventDef();
	UE_ASSERT(def);

	const char* type = ioXmlNode_GetAttrValue(node, "type");
	UE_ASSERT_MSG(type, "Event must have 'type'");

	GM_LOG_OBJ("    Loading gmEventDef: type = '%s'...", type);

	if (!ueStrCmp(type, "on_tick")) def->m_type = gmEventType_OnTick;
	else if (!ueStrCmp(type, "on_draw")) def->m_type = gmEventType_OnDraw;
	else if (!ueStrCmp(type, "on_create")) def->m_type = gmEventType_OnCreate;
	else if (!ueStrCmp(type, "on_destroy")) def->m_type = gmEventType_OnDestroy;
	else if (!ueStrCmp(type, "on_collide")) def->m_type = gmEventType_OnCollide;
	else if (!ueStrCmp(type, "on_collide2")) def->m_type = gmEventType_OnCollide2;
	else if (!ueStrCmp(type, "on_click")) def->m_type = gmEventType_OnClick;
	else
	{
		UE_ASSERT_MSGP(0, "Unknown event type '%s'", type);
	}

	// Load actions

	for (ioXmlNode* actionNode = ioXmlNode_GetFirstNode(node); actionNode; actionNode = ioXmlNode_GetNext(actionNode))
	{
		gmActionDef* actionDef = gmActionDef::Load(def, actionNode);
		actionDef->m_next = def->m_actionDefs;
		def->m_actionDefs = actionDef;
	}

	return def;
}

void gmEventDef::Fire(gmObject* object, gmElement* element, void* data)
{
	gmActionDef* actionDef = m_actionDefs;
	while (actionDef)
	{
		actionDef->Fire(object, element, data);
		if ((object ? object : element->m_object)->m_toDelete)
			break;
		actionDef = actionDef->m_next;
	}
}

// gmActionDef

gmActionDef_SelectRandom* gmActionDef_SelectRandom::Load(ioXmlNode* node)
{
	UE_NOT_IMPLEMENTED();
	return NULL;
}

gmActionDef_PlayAnimation* gmActionDef_PlayAnimation::Load(ioXmlNode* node)
{
	UE_NOT_IMPLEMENTED();
	return NULL;
}

gmActionDef_PlaySound* gmActionDef_PlaySound::Load(ioXmlNode* node)
{
	UE_NOT_IMPLEMENTED();
	return NULL;
}

gmActionDef_SpawnParticles* gmActionDef_SpawnParticles::Load(ioXmlNode* node)
{
	UE_NOT_IMPLEMENTED();
	return NULL;
}

void gmActionDef_Call::Fire(gmObject* object, gmElement* element, void* data)
{
	gmActionCallParams params;
	params.m_object = object ? object : element->m_object;
	params.m_element = element;
	params.m_data = data;

	m_func(&params);
}

gmActionDef_Call* gmActionDef_Call::Load(gmEventDef* eventDef, ioXmlNode* node)
{
	const char* function = ioXmlNode_GetAttrValue(node, "function");
	UE_ASSERT(function);

	GM_LOG_OBJ("    Loading gmActionDef_Call: function = '%s'...", function);

	gmActionDef_Call* def = new(gmObjectSys_GetAllocator()) gmActionDef_Call();
	UE_ASSERT(def);

	UE_ASSERT(gmActionDef_Call::m_registerFunc);
	gmActionDef_Call::m_registerFunc(eventDef, def, function);
	UE_ASSERT(def->m_func);

	return def;
}

gmActionDef* gmActionDef::Load(gmEventDef* eventDef, ioXmlNode* node)
{
	const char* name = ioXmlNode_GetName(node);

	GM_LOG_OBJ("    Loading gmActionDef: name = '%s'...", name);

	if (!ueStrCmp(name, "select_random")) return gmActionDef_SelectRandom::Load(node);
	if (!ueStrCmp(name, "play_sound")) return gmActionDef_PlaySound::Load(node);
	if (!ueStrCmp(name, "play_animation")) return gmActionDef_PlayAnimation::Load(node);
	if (!ueStrCmp(name, "spawn_particles")) return gmActionDef_SpawnParticles::Load(node);
	if (!ueStrCmp(name, "call")) return gmActionDef_Call::Load(eventDef, node);

	UE_ASSERT_MSGP(0, "Unknown action type '%s'", name);

	return NULL;
}

// gmLevelInfo

gmLevelInfo::gmLevelInfo() :
	m_rect(0, 0, 960.0f, 640.0f),
	m_gravity(10.0f),
	m_boundary(100.0f),
	m_tutorialIndex(0xFFFFFFFF)
{}

gmLevelInfo* gmLevelInfo::Load(ioXmlNode* node)
{
	GM_LOG_OBJ("    Loading gmLevelInfo...");

	u8* memory = (u8*) gmObjectSys_GetAllocator()->Alloc(sizeof(gmLevelInfo));
	UE_ASSERT(memory);

	gmLevelInfo* def = new(memory) gmLevelInfo();
	UE_ASSERT(def);
	memory += sizeof(gmLevelInfo);

	ioXmlNode_GetAttrValueRect(node, "rect", def->m_rect);
	ioXmlNode_GetAttrValueF32(node, "gravity", def->m_gravity);
	ioXmlNode_GetAttrValueF32(node, "boundary", def->m_boundary);
	ioXmlNode_GetAttrValueU32(node, "tutorialIndex", def->m_tutorialIndex);
	return def;
}

void gmLevelInfo::Save(ioXmlNode* parent)
{
	ioXmlNode* node = ioXmlNode_AddNode(parent, "levelInfo");
	ioXmlNode_AddAttrRect(node, "rect", m_rect);
	ioXmlNode_AddAttrF32(node, "boundary", m_boundary);
	ioXmlNode_AddAttrF32(node, "gravity", m_gravity);
	if (m_tutorialIndex != 0xFFFFFFFF)
		ioXmlNode_AddAttrU32(node, "tutorialIndex", m_tutorialIndex);
}

// gmElement

gmElementDef::gmElementDef() :
	m_name(NULL),
	m_spriteDefs(NULL),
	m_numShapeDefs(0),
	m_shapeDefs(NULL),
	m_eventDefs(NULL),
	m_particleEffectDefs(NULL),
	m_next(NULL)
{}

gmElementDef::~gmElementDef()
{
	while (m_shapeDefs)
	{
		gmShapeDef* def = m_shapeDefs;
		m_shapeDefs = m_shapeDefs->m_next;
		ueDelete(def, gmObjectSys_GetAllocator());
	}

	while (m_spriteDefs)
	{
		gmSpriteDef* def = m_spriteDefs;
		m_spriteDefs = m_spriteDefs->m_next;
		ueDelete(def, gmObjectSys_GetAllocator());
	}

	while (m_eventDefs)
	{
		gmEventDef* def = m_eventDefs;
		m_eventDefs = m_eventDefs->m_next;
		ueDelete(def, gmObjectSys_GetAllocator());
	}

	while (m_particleEffectDefs)
	{
		gmParticleEffectDef* def = m_particleEffectDefs;
		m_particleEffectDefs = m_particleEffectDefs->m_next;
		ueDelete(def, gmObjectSys_GetAllocator());
	}
}

gmSpriteDef* gmElementDef::FindSpriteDef(const char* name)
{
	gmSpriteDef* spriteDef = m_spriteDefs;
	while (spriteDef)
	{
		if (!ueStrCmp(spriteDef->m_name, name))
			return spriteDef;
		spriteDef = spriteDef->m_next;
	}
	return NULL;
}

gmShapeDef* gmElementDef::FindShapeDef(const char* name, u32* index)
{
	u32 counter = 0;

	gmShapeDef* shapeDef = m_shapeDefs;
	while (shapeDef)
	{
		if (!ueStrCmp(shapeDef->m_name, name))
		{
			if (index)
				*index = counter;
			return shapeDef;
		}
		shapeDef = shapeDef->m_next;
		counter++;
	}
	return NULL;
}

gmElementDef* gmElementDef::Load(ioXmlNode* node)
{
	const char* name = ioXmlNode_GetAttrValue(node, "name");
	u32 nameSize = ueStrLen(name);
	if (nameSize)
		nameSize++;

	GM_LOG_OBJ("  Loading gmElementDef '%s'...", name);

	u8* defMemory = (u8*) gmObjectSys_GetAllocator()->Alloc(sizeof(gmElementDef) + nameSize);
	UE_ASSERT(defMemory);

	gmElementDef* def = new(defMemory) gmElementDef();
	defMemory += sizeof(gmElementDef);
	
	if (nameSize)
	{
		ueMemCpy(defMemory, name, nameSize);
		def->m_name = (const char*) defMemory;
	}

	// Load shapes

	for (ioXmlNode* shapeNode = ioXmlNode_GetFirstNode(node, "shape"); shapeNode; shapeNode = ioXmlNode_GetNext(shapeNode, "shape"))
	{
		gmShapeDef* shapeDef = gmShapeDef::Load(shapeNode);
		shapeDef->m_next = def->m_shapeDefs;
		def->m_shapeDefs = shapeDef;

		def->m_numShapeDefs++;
	}

	// Load sprites

	for (ioXmlNode* spriteNode = ioXmlNode_GetFirstNode(node, "sprite"); spriteNode; spriteNode = ioXmlNode_GetNext(spriteNode, "sprite"))
	{
		gmSpriteDef* spriteDef = gmSpriteDef::Load(spriteNode);
		spriteDef->m_next = def->m_spriteDefs;
		def->m_spriteDefs = spriteDef;
	}

	// Load events

	for (ioXmlNode* eventNode = ioXmlNode_GetFirstNode(node, "event"); eventNode; eventNode = ioXmlNode_GetNext(eventNode, "event"))
	{
		gmEventDef* eventDef = gmEventDef::Load(eventNode);
		eventDef->m_next = def->m_eventDefs;
		def->m_eventDefs = eventDef;
	}

	// Load particle effects

	for (ioXmlNode* particleEffectNode = ioXmlNode_GetFirstNode(node, "effect"); particleEffectNode; particleEffectNode = ioXmlNode_GetNext(particleEffectNode, "effect"))
	{
		gmParticleEffectDef* particleEffectDef = gmParticleEffectDef::Load(particleEffectNode);
		particleEffectDef->m_next = def->m_particleEffectDefs;
		def->m_particleEffectDefs = particleEffectDef;
	}

	// Post load stage

	gmShapeDef* shapeDef = def->m_shapeDefs;
	while (shapeDef)
	{
		shapeDef->PostLoad(def);
		shapeDef = shapeDef->m_next;
	}

	gmSpriteDef* spriteDef = def->m_spriteDefs;
	while (spriteDef)
	{
		spriteDef->PostLoad(def);
		spriteDef = spriteDef->m_next;
	}

	return def;
}

void gmElementDef::Save(ioXmlNode* parent)
{
	ioXmlNode* node = ioXmlNode_AddNode(parent, "element");
	if (m_name)
		ioXmlNode_AddAttr(node, "name", m_name);

	gmSpriteDef* spriteDef = m_spriteDefs;
	while (spriteDef)
	{
		spriteDef->Save(node);
		spriteDef = spriteDef->m_next;
	}

	gmParticleEffectDef* particleEffectDef = m_particleEffectDefs;
	while (particleEffectDef)
	{
		particleEffectDef->Save(node);
		particleEffectDef = particleEffectDef->m_next;
	}

	UE_ASSERT(!m_shapeDefs);
	UE_ASSERT(!m_eventDefs);
}

// gmObjectRef

gmObjectRef::gmObjectRef() :
	m_def(NULL),
	m_propertyDefs(NULL)
{}

gmObjectRef::~gmObjectRef()
{
	while (m_propertyDefs)
	{
		gmPropertyDef* def = m_propertyDefs;
		m_propertyDefs = m_propertyDefs->m_next;
		ueDelete(def, gmObjectSys_GetAllocator());
	}
	gmObjectSys_ReleaseDef(m_def);
}

gmObjectRef* gmObjectRef::Load(ioXmlNode* node)
{
	gmObjectRef* ref = new(gmObjectSys_GetAllocator()) gmObjectRef();
	UE_ASSERT(ref);

	const char* templateName = ioXmlNode_GetAttrValue(node, "template");
	UE_ASSERT(templateName);

	GM_LOG_OBJ("  Loading gmObjectRef '%s'...", templateName);

	ref->m_def = gmObjectSys_GetDef(templateName);
	UE_ASSERT(ref->m_def);

	ref->m_transform.Load(node);

	// Load properties

	for (ioXmlNode* propertyNode = ioXmlNode_GetFirstNode(node, "property"); propertyNode; propertyNode = ioXmlNode_GetNext(propertyNode, "property"))
	{
		gmPropertyDef* propertyDef = gmPropertyDef::Load(propertyNode);
		propertyDef->m_next = ref->m_propertyDefs;
		ref->m_propertyDefs = propertyDef;
	}
	ReverseList(ref->m_propertyDefs);

	return ref;
}

void gmObjectRef::Save(ioXmlNode* parent)
{
	ioXmlNode* node = ioXmlNode_AddNode(parent, "object");
	ioXmlNode_AddAttr(node, "template", m_def->m_name);
	m_transform.Save(node);

	// Save properties

	gmPropertyDef* propertyDef = m_propertyDefs;
	while (propertyDef)
	{
		propertyDef->Save(node);
		propertyDef = propertyDef->m_next;
	}
}

gmPropertyDef* gmObjectRef::FindProperty(const char* name)
{
	gmPropertyDef* def = m_propertyDefs;
	while (def)
	{
		if (!ueStrCmp(name, def->m_name))
			return def;
		def = def->m_next;
	}
	return NULL;
}

gmObjectRef* gmObjectRef::Clone()
{
	gmObjectRef* ref = new(gmObjectSys_GetAllocator()) gmObjectRef();
	UE_ASSERT(ref);

	ref->m_transform = m_transform;
	ref->m_def = m_def;
	m_def->m_refCount++;

	// Also clone properties

	gmPropertyDef* propertyDef = m_propertyDefs;
	while (propertyDef)
	{
		gmPropertyDef* clonedPropertyDef = propertyDef->Clone();
		clonedPropertyDef->m_next = ref->m_propertyDefs;
		ref->m_propertyDefs = clonedPropertyDef;
		
		propertyDef = propertyDef->m_next;
	}
	ReverseList(ref->m_propertyDefs);

	return ref;
}

// gmPropertyDef

gmPropertyDef::gmPropertyDef() :
	m_name(NULL),
	m_type(gmPropertyType_MAX),
	m_next(NULL)
{
	m_value.m_int = 0;
}

gmPropertyDef::~gmPropertyDef()
{
	if (m_type == gmPropertyType_String)
		ueDelete(m_value.m_string, gmObjectSys_GetAllocator());
}

gmPropertyDef* gmPropertyDef::Load(ioXmlNode* node)
{
	const char* name = ioXmlNode_GetAttrValue(node, "name");
	UE_ASSERT(name);
	const u32 nameSize = ueStrLen(name) + 1;

	GM_LOG_OBJ("  Loading gmPropertyDef '%s'...", name);

	u8* defMemory = (u8*) gmObjectSys_GetAllocator()->Alloc(sizeof(gmPropertyDef) + nameSize);
	UE_ASSERT(defMemory);

	gmPropertyDef* def = new(defMemory) gmPropertyDef();
	defMemory += sizeof(gmPropertyDef);
	
	ueMemCpy(defMemory, name, nameSize);
	def->m_name = (const char*) defMemory;

	const char* typeName = ioXmlNode_GetAttrValue(node, "type");
	if (!typeName) def->m_type = gmPropertyType_Bool;
	else if (!ueStrCmp(typeName, "float")) def->m_type = gmPropertyType_Float;
	else if (!ueStrCmp(typeName, "int")) def->m_type = gmPropertyType_Int;
	else if (!ueStrCmp(typeName, "bool")) def->m_type = gmPropertyType_Bool;
	else if (!ueStrCmp(typeName, "string")) def->m_type = gmPropertyType_String;
	else UE_ASSERT_MSGP(0, "Unsupported property type: '%s'", typeName);

	switch (def->m_type)
	{
		case gmPropertyType_Float:
			UE_ASSERT_FUNC(ioXmlNode_GetAttrValueF32(node, "value", def->m_value.m_float));
			break;
		case gmPropertyType_Int:
			UE_ASSERT_FUNC(ioXmlNode_GetAttrValueS32(node, "value", def->m_value.m_int));
			break;
		case gmPropertyType_Bool:
			ioXmlNode_GetAttrValueBool(node, "value", def->m_value.m_bool);
			break;
		case gmPropertyType_String:
		{
			const char* value = ioXmlNode_GetAttrValue(node, "value");
			UE_ASSERT(value);
			def->m_value.m_string = ueStrDup(gmObjectSys_GetAllocator(), value);
			UE_ASSERT(def->m_value.m_string);
			break;
		}
	}

	return def;
}

void gmPropertyDef::Save(ioXmlNode* parent)
{
	ioXmlNode* node = ioXmlNode_AddNode(parent, "property");
	ioXmlNode_AddAttr(node, "name", m_name);
	switch (m_type)
	{
	case gmPropertyType_Float:
		ioXmlNode_AddAttr(node, "type", "float");
		ioXmlNode_AddAttrF32(node, "value", m_value.m_float);
		break;
	case gmPropertyType_Int:
		ioXmlNode_AddAttr(node, "type", "int");
		ioXmlNode_AddAttrS32(node, "value", m_value.m_int);
		break;
	case gmPropertyType_Bool:
		ioXmlNode_AddAttr(node, "type", "bool");
		ioXmlNode_AddAttrBool(node, "value", m_value.m_bool);
		break;
	case gmPropertyType_String:
		ioXmlNode_AddAttr(node, "type", "string");
		ioXmlNode_AddAttr(node, "value", m_value.m_string);
		break;
	}
}

gmPropertyDef* gmPropertyDef::Clone()
{
	const u32 nameSize = ueStrLen(m_name) + 1;

	u8* defMemory = (u8*) gmObjectSys_GetAllocator()->Alloc(sizeof(gmPropertyDef) + nameSize);
	UE_ASSERT(defMemory);

	gmPropertyDef* def = new(defMemory) gmPropertyDef();
	defMemory += sizeof(gmPropertyDef);
	
	ueMemCpy(defMemory, m_name, nameSize);
	def->m_name = (const char*) defMemory;

	def->m_type = m_type;
	def->m_value = m_value;

	if (m_type == gmPropertyType_String)
	{
		def->m_value.m_string = ueStrDup(gmObjectSys_GetAllocator(), m_value.m_string);
		UE_ASSERT(def->m_value.m_string);
	}

	return def;
}

// gmObjectDef

gmObjectDef::gmObjectDef() :
	m_name(NULL),
	m_refCount(0),
	m_elementDefs(NULL),
	m_eventDefs(NULL),
	m_propertyDefs(NULL),
	m_levelInfo(NULL)
{}

gmObjectDef::~gmObjectDef()
{
	while (m_elementDefs)
	{
		gmElementDef* def = m_elementDefs;
		m_elementDefs = m_elementDefs->m_next;
		ueDelete(def, gmObjectSys_GetAllocator());
	}
	
	while (m_eventDefs)
	{
		gmEventDef* def = m_eventDefs;
		m_eventDefs = m_eventDefs->m_next;
		ueDelete(def, gmObjectSys_GetAllocator());
	}

	while (m_propertyDefs)
	{
		gmPropertyDef* def = m_propertyDefs;
		m_propertyDefs = m_propertyDefs->m_next;
		ueDelete(def, gmObjectSys_GetAllocator());
	}

	while (gmObjectRef* ref = m_objectRefs.PopFront())
		ueDelete(ref, gmObjectSys_GetAllocator());

	if (m_levelInfo)
		ueDelete(m_levelInfo, gmObjectSys_GetAllocator());

	UE_ASSERT(!m_refCount);
}

void gmObjectDef::Save(ioXmlDoc* doc)
{
	ioXmlNode* node = ioXmlDoc_AddNode(doc, "template");

	ioXmlNode_AddCommentNode(node, "level info");

	UE_ASSERT(m_levelInfo);
	m_levelInfo->Save(node);

	ioXmlNode_AddCommentNode(node, "level elements");

	gmElementDef* elementDef = m_elementDefs;
	while (elementDef)
	{
		elementDef->Save(node);
		elementDef = elementDef->m_next;
	}

	ioXmlNode_AddCommentNode(node, "objects");

	for (gmObjectRef* ref = m_objectRefs.Front(); ref; ref = ref->Next())
		ref->Save(node);

	UE_ASSERT(!m_eventDefs);
	UE_ASSERT(!m_propertyDefs);
}

gmObjectDef* gmObjectDef::Load(const char* name)
{
	// Parse Xml

	uePath path;
	ueStrFormatS(path, "%s.template.xml", name);
	ioXmlDoc* doc = ioXmlDoc_Load(gmObjectSys_GetAllocator(), path);
	if (!doc)
		return NULL;
	ioXmlDocScopedDestructor docDestructor(doc);

	// Get main Xml node

	ioXmlNode* templateNode = ioXmlDoc_GetFirstNode(doc, "template");
	UE_ASSERT(templateNode);

	// Create gmObjectDef
	
	GM_LOG_OBJ("Loading gmObjectDef '%s'...", name);

	const u32 nameLength = ueStrLen(name);
	u8* defMemory = (u8*) gmObjectSys_GetAllocator()->Alloc(sizeof(gmObjectDef) + nameLength + 1);
	UE_ASSERT(defMemory);

	gmObjectDef* def = new(defMemory) gmObjectDef();
	defMemory += sizeof(gmObjectDef);
	
	ueMemCpy(defMemory, name, nameLength + 1);
	def->m_name = (const char*) defMemory;

	s_data->m_defs.Insert(def);

	// Load elements

	for (ioXmlNode* elementNode = ioXmlNode_GetFirstNode(templateNode, "element"); elementNode; elementNode = ioXmlNode_GetNext(elementNode, "element"))
	{
		gmElementDef* elementDef = gmElementDef::Load(elementNode);
		elementDef->m_next = def->m_elementDefs;
		def->m_elementDefs = elementDef;
	}
	ReverseList(def->m_elementDefs);

	// Load events

	for (ioXmlNode* eventNode = ioXmlNode_GetFirstNode(templateNode, "event"); eventNode; eventNode = ioXmlNode_GetNext(eventNode, "event"))
	{
		gmEventDef* eventDef = gmEventDef::Load(eventNode);
		eventDef->m_next = def->m_eventDefs;
		def->m_eventDefs = eventDef;
	}
	ReverseList(def->m_eventDefs);

	// Load child object references

	for (ioXmlNode* objectNode = ioXmlNode_GetFirstNode(templateNode, "object"); objectNode; objectNode = ioXmlNode_GetNext(objectNode, "object"))
	{
		gmObjectRef* ref = gmObjectRef::Load(objectNode);
		def->m_objectRefs.PushBack(ref);
	}

	// Load properties

	for (ioXmlNode* propertyNode = ioXmlNode_GetFirstNode(templateNode, "property"); propertyNode; propertyNode = ioXmlNode_GetNext(propertyNode, "property"))
	{
		gmPropertyDef* propertyDef = gmPropertyDef::Load(propertyNode);
		propertyDef->m_next = def->m_propertyDefs;
		def->m_propertyDefs = propertyDef;
	}
	ReverseList(def->m_propertyDefs);

	// Load level info

	if (ioXmlNode* levelInfoNode = ioXmlNode_GetFirstNode(templateNode, "levelInfo"))
		def->m_levelInfo = gmLevelInfo::Load(levelInfoNode);

	return def;
}

gmPropertyDef* gmObjectDef::FindProperty(const char* name)
{
	gmPropertyDef* def = m_propertyDefs;
	while (def)
	{
		if (!ueStrCmp(name, def->m_name))
			return def;
		def = def->m_next;
	}
	return NULL;
}
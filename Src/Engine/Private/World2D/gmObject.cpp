#include "gmObject.h"
#include "gmObjectDef.h"
#include "gmObjectSet.h"
#include "GameObjectSystem/gmRenderer.h"
#include "GX/gxSprite.h"
#include "GL/glLib.h"
#include "Physics/Box2D/b2SoftBody.h"
#include "Physics/Box2D/phBox2D.h"

template <typename DEF_TYPE, typename TYPE>
void INIT_CHILD_ELEMS(DEF_TYPE* defs, TYPE*& elems, const gmTransform& transform)
{
	for (DEF_TYPE* def = defs; def; def = def->m_next)
	{
		TYPE* elem = new(gmObjectSys_GetAllocator()) TYPE(def, transform);
		UE_ASSERT(elem);

		elem->m_next = elems;
		elems = elem;
	}
}

template <typename TYPE>
void DEINIT_CHILD_ELEMS(TYPE*& elems)
{
	while (elems)
	{
		TYPE* elem = elems;
		elems = elems->m_next;

		ueDelete(elem, gmObjectSys_GetAllocator());
	}
}

template <typename TYPE>
void UPDATE_CHILD_ELEMS(TYPE*& elems, f32 dt)
{
	for (TYPE* elem = elems; elem; elem = elem->m_next)
		elem->Update(dt);
}

template <typename TYPE>
void DRAW_CHILD_ELEMS(TYPE*& elems)
{
	for (TYPE* elem = elems; elem; elem = elem->m_next)
		elem->Draw();
}

// gmSprite

gmSprite::gmSprite(gmElement* element, gmSpriteDef* def, const gmTransform& transform) :
	m_def(def),
	m_element(NULL),
	m_shape(NULL),
	m_draw(UE_TRUE),
	m_sprite(NULL),
	m_layer(0.0f),
	m_next(NULL),
	m_rotation(0.0f),
	m_scale(1.0f)
{
	m_transform = def->m_transform;
	m_color = def->m_color;
	m_layer = def->m_layer;

	// Create sprite

	if (def->m_spriteTemplate)
	{
		m_sprite = gxSprite_Create(def->m_spriteTemplate);
		UE_ASSERT(m_sprite);
	}
	else
	{
		m_sprite = gxSprite_CreateFromTexture(*def->m_texture);
		UE_ASSERT(m_sprite);
	}

	// Copy rectangle

	m_rect = m_def->m_rect;

	// Store element

	m_element = element;

	// Store shape and set up transform

	if (m_element->m_body)
	{
		// If aligning to body, only take its scale and offset

		m_rect.ScaleCentered(transform.m_scale);
		m_tmpScale = transform.m_scale;

		if (m_element->m_def->IsPrimitive())
		{
			b2Body* body = m_element->GetBody();
			m_shape = body->GetShapeList();
			u32 shapeIndexOnTheList = m_element->m_def->m_numShapeDefs - m_def->m_shapeIndex - 1;
			while (shapeIndexOnTheList--)
				m_shape = m_shape->GetNext();
		}
	}
	else
		m_transform.Mul(transform); // No body -> combine local and global transform
}

gmSprite::~gmSprite()
{
	gxSprite_Destroy(m_sprite);
}

void gmSprite::CalcBoundingRect(ueRect& rect)
{
	rect.SetCenterAndSize(m_transform.m_pos, ueVec2::Zero);
}

void gmSprite::Update(f32 dt)
{
	gxSprite_Update(m_sprite, dt);
	m_rotation += m_def->m_rotationSpeed * dt;
}

void gmSprite::Draw()
{
	if (!m_draw)
		return;

	static ueRect rect; // XXX: Without rect being static crashes on Windows Release

#define MAX_POLY_VERTS 10
	static b2Vec2 pos[MAX_POLY_VERTS];
	static b2Vec2 tex[MAX_POLY_VERTS];
	gxSpriteDrawParams::Poly poly;

	gxSpriteDrawParams params;
	params.m_color = m_color;
	params.m_layer = m_layer;
	params.m_additiveBlending = m_def->m_blendAdditive;

	// Is this fixed sprite?

	if (!m_element->m_body)
	{
		params.m_rotation = m_transform.m_rot + m_rotation;

		rect = m_rect;
		rect.Translate(m_transform.m_pos);
		rect.ScaleCentered(m_transform.m_scale * m_scale);
#if 0
		if (m_def->m_moveOnTilting)
		{
			const f32 tiltingScale = 10.0f;

			f32 x, y;
			gmTiltingIndicator_GetSmoothedUnnormalizedTiltingVector(x, y);

			x *= -3.0f;
			y *= -3.0f;
			x = ueClamp(x, -1.0f, 1.0f);
			y = ueClamp(y, -1.0f, 1.0f);
			x *= tiltingScale;
			y *= tiltingScale;

			rect.Translate(x, y);
		}
#endif
		params.m_rect = &rect;
	}

	// Is this non-aligned sprite?

	else if (!m_def->m_shapeDef)
	{
		UE_ASSERT(m_element->m_def->IsPrimitive());
		
		b2Body* body = m_element->GetBody();
		params.m_rotation = body->GetAngle() + m_transform.m_rot + m_rotation;
#if 0
		if (m_def->m_noRotation)
			params.m_rotation = -g_level.m_rotation;
#endif
		ueVec2 center;
		phBox2D_FromBtVec(center, body->GetPosition());
		center *= gmObjectSys_GetFromPhysScale();

		ueVec2 rotatedLocalPos(m_transform.m_pos);
		rotatedLocalPos.Rotate(params.m_rotation);
		rotatedLocalPos *= m_tmpScale;

		rect = m_rect;
		rect.Translate(center);
		rect.Translate(rotatedLocalPos);
		rect.ScaleCentered(m_transform.m_scale * m_scale);

		params.m_rect = &rect;
	}

	// This must be shape-aligned sprite!

	else switch (m_def->m_shapeDef->m_type)
	{
		case gmShapeType_Circle:
		{
			b2Body* body = m_element->GetBody();
			b2CircleShape* shape = (b2CircleShape*) m_shape;

			params.m_rotation = body->GetAngle() + m_transform.m_rot + m_rotation;

			const f32 radius = shape->GetRadius() * m_transform.m_scale[0] * gmObjectSys_GetFromPhysScale() * m_scale;

			b2Vec2 shapeCenter = body->GetWorldPoint(shape->GetLocalPosition());
			shapeCenter *= gmObjectSys_GetFromPhysScale();

			b2Vec2 center;
			phBox2D_ToBtVec(center, m_transform.m_pos);
			center += shapeCenter;

			rect.Set(center.x - radius, center.y - radius, center.x + radius, center.y + radius);

			params.m_rect = &rect;
			break;
		}
		case gmShapeType_Rectangle:
		{
			b2Body* body = m_element->GetBody();
			b2PolygonShape* shape = (b2PolygonShape*) m_shape;

			poly.m_numVerts = shape->GetVertexCount();
			poly.m_pos = pos;
			poly.m_tex = tex;

			b2Vec2 scale, translation;
			phBox2D_ToBtVec(scale, m_transform.m_scale * m_scale);
			phBox2D_ToBtVec(translation, m_transform.m_pos);

			const b2Vec2* localPosArray = shape->GetVertices();
			for (u32 i = 0; i < poly.m_numVerts; i++)
			{
				b2Vec2 localPos = localPosArray[i];
				localPos.x *= scale.x;
				localPos.y *= scale.y;
				localPos += translation;
				pos[i] = body->GetWorldPoint(localPos);
				pos[i] *= gmObjectSys_GetFromPhysScale();
			}

			UE_ASSERT(poly.m_numVerts == 4);
			tex[0].Set(0, 0);
			tex[1].Set(1, 0);
			tex[2].Set(1, 1);
			tex[3].Set(0, 1);

			params.m_poly = &poly;
			break;
		}
		case gmShapeType_Polygon:
		{
			UE_NOT_IMPLEMENTED(); // do like with rect

			b2Body* body = m_element->GetBody();
			b2PolygonShape* shape = (b2PolygonShape*) m_shape;

			poly.m_numVerts = shape->GetVertexCount();
			poly.m_pos = pos;
			poly.m_tex = tex;

			b2Vec2 scale, translation;
			phBox2D_ToBtVec(scale, m_transform.m_scale * m_scale);
			phBox2D_ToBtVec(translation, m_transform.m_pos);

			const b2Vec2* localPosArray = shape->GetVertices();
			for (u32 i = 0; i < poly.m_numVerts; i++)
			{
				b2Vec2 localPos = localPosArray[i];
				localPos.x *= scale.x;
				localPos.y *= scale.y;
				localPos += translation;
				pos[i] = body->GetWorldPoint(localPos);
				pos[i] *= gmObjectSys_GetFromPhysScale();
			}

			const f32 texScaleX = 1.0f / 86.0f;
			const f32 texScaleY = 1.0f / 122.0f;

			const gmShapeDef::Shape::Polygon& polyDesc = m_def->m_shapeDef->m_shape.m_polygon;
			for (u32 i = 0; i < poly.m_numVerts; i++)
			{
//				tex[i] = polyDesc.m_tex[i];
				tex[i].x *= texScaleX;
				tex[i].y *= texScaleY;
			}

			params.m_poly = &poly;
			break;
		}
		case gmShapeType_SoftCircle:
		{
			b2SoftBody* softBody = m_element->GetSoftBody();

			int numVerts;
			b2SoftBodyUtil_GetPosAndTex(softBody, pos, tex, &numVerts, MAX_POLY_VERTS);

			poly.m_numVerts = (u32) numVerts;
			poly.m_pos = pos;
			poly.m_tex = tex;

			for (u32 i = 0; i < poly.m_numVerts; i++)
				pos[i] *= gmObjectSys_GetFromPhysScale();

			params.m_poly = &poly;
			break;
		}
		default:
			UE_NOT_IMPLEMENTED();
	}
	
	gmRenderer_Draw(m_sprite, &params);
}

// gmParticleEffect

gmParticleEffect::gmParticleEffect(gmParticleEffectDef* def, const gmTransform& transform) :
	m_def(def),
	m_next(NULL)
{
	m_transform = transform;
	m_transform.Mul(def->m_transform);

	m_handle.Init(*def->m_type);
	m_handle.SetPos2D(m_transform.m_pos);
	m_handle.SetColor(def->m_color);
}

gmParticleEffect::~gmParticleEffect()
{
	m_handle.Destroy(UE_TRUE);
}

void gmParticleEffect::Update(f32 dt)
{
	m_handle.Update(dt);
}

void gmParticleEffect::Draw()
{
	gmRenderer_Draw(&m_handle);
}

// gmElement

gmElement::gmElement(gmObject* object, gmElementDef* def, const gmTransform& transform) :
	m_object(object),
	m_def(def),
	m_sprites(NULL),
	m_body(NULL),
	m_particleEffects(NULL),
	m_next(NULL)
{
	InitBody(transform);

	for (gmSpriteDef* spriteDef = def->m_spriteDefs; spriteDef; spriteDef = spriteDef->m_next)
	{
		gmSprite* sprite = new(gmObjectSys_GetAllocator()) gmSprite(this, spriteDef, transform);
		UE_ASSERT(sprite);

		sprite->m_next = m_sprites;
		m_sprites = sprite;
	}
	INIT_CHILD_ELEMS(def->m_particleEffectDefs, m_particleEffects, transform);
}

gmElement::~gmElement()
{
	DEINIT_CHILD_ELEMS(m_sprites);
	DEINIT_CHILD_ELEMS(m_particleEffects);

	DestroyBody();
}

void gmElement::DestroyBody()
{
	if (m_body)
	{
		if (m_def->IsPrimitive())
			m_object->m_set->m_world->DestroyBody(GetBody());
		else
			b2SoftBody_Destroy(GetSoftBody());
	}
}

void gmElement::InitBody(const gmTransform& transform)
{
	if (!m_def->m_shapeDefs)
		return;

	if (m_def->m_shapeDefs->IsSoft())
	{
		gmShapeDef* def = m_def->m_shapeDefs;

		UE_ASSERT_MSG(!def->m_next, "Only 1 soft shape allowed per element");

		// Create shape and body definitions

		b2SoftShapeDef softBodyDef;
		softBodyDef.friction = def->m_friction;
		softBodyDef.density = def->m_density;
		softBodyDef.restitution = def->m_restitution;
		softBodyDef.softness = def->m_shape.m_softCircle.m_softness;
		softBodyDef.userData = this;
		
		b2CircleSoftBodyShapeDef circleSoftBodyShapeDef;
		softBodyDef.shape = &circleSoftBodyShapeDef;

		circleSoftBodyShapeDef.radius = def->m_shape.m_softCircle.m_radius * transform.m_scale[0] * gmObjectSys_GetToPhysScale();

		phBox2D_ToBtVec(circleSoftBodyShapeDef.center, transform.m_pos);
		b2Vec2 shapeTranslation;
		phBox2D_ToBtVec(shapeTranslation, def->m_transform.m_pos);
		circleSoftBodyShapeDef.center += shapeTranslation;
		circleSoftBodyShapeDef.center *= gmObjectSys_GetToPhysScale();

		circleSoftBodyShapeDef.numOuterCircles = def->m_shape.m_softCircle.m_numOuterCircles;

		// Create body

		m_body = b2SoftBody_Create(m_object->m_set->m_world, &softBodyDef);
		UE_ASSERT(m_body);
	}
	else
	{
		// Create body

		b2BodyDef bodyDef;
		bodyDef.userData = this;
		phBox2D_ToBtVec(bodyDef.position, transform.m_pos);
		bodyDef.position *= gmObjectSys_GetToPhysScale();
		bodyDef.angle = transform.m_rot;

		b2Body* body = m_object->m_set->m_world->CreateBody(&bodyDef);
		UE_ASSERT(body);

		// Add shapes to body

		gmShapeDef* shapeDef = m_def->m_shapeDefs;
		while (shapeDef)
		{
			AddPrimitiveShape(shapeDef, body, transform.m_scale);
			shapeDef = shapeDef->m_next;
		}

		// Adjust body mass

		body->SetMassFromShapes();

		// Set body

		m_body = body;
	}
}

void gmElement::AddPrimitiveShape(gmShapeDef* def, b2Body* body, const ueVec2& scale)
{
	b2Vec2 b2scale;
	phBox2D_ToBtVec(b2scale, scale);

	switch (def->m_type)
	{
		case gmShapeType_Circle:
		{
			b2CircleDef shape;
			shape.radius = def->m_shape.m_circle.m_radius * def->m_transform.m_scale[0] * b2scale.x * gmObjectSys_GetToPhysScale();
			shape.friction = def->m_friction;
			shape.density = def->m_density;
			phBox2D_ToBtVec(shape.localPosition, def->m_transform.m_pos);
			shape.localPosition.x *= b2scale.x;
			shape.localPosition.y *= b2scale.y;
			shape.localPosition *= gmObjectSys_GetToPhysScale();
			shape.isSensor = def->m_isSensor;
			shape.restitution = def->m_restitution;

			shape.filter.categoryBits = def->m_collisionFlag;
			shape.filter.maskBits = def->m_collisionMask;
			shape.filter.groupIndex = 0;

			body->CreateShape(&shape);
			break;
		}
		case gmShapeType_Rectangle:
		{
			const f32 halfWidth = def->m_shape.m_rectangle.m_width * b2scale.x * 0.5f * gmObjectSys_GetToPhysScale();
			const f32 halfHeight = def->m_shape.m_rectangle.m_height * b2scale.y * 0.5f * gmObjectSys_GetToPhysScale();

			b2Vec2 center;
			phBox2D_ToBtVec(center, def->m_transform.m_pos);
			center.x *= b2scale.x;
			center.y *= b2scale.y;
			center *= gmObjectSys_GetToPhysScale();

			b2PolygonDef shape;
			shape.SetAsBox(halfWidth, halfHeight, center, def->m_transform.m_rot);
			shape.friction = def->m_friction;
			shape.density = def->m_density;
			shape.isSensor = def->m_isSensor;

			shape.filter.categoryBits = def->m_collisionFlag;
			shape.filter.maskBits = def->m_collisionMask;
			shape.filter.groupIndex = 0;

			body->CreateShape(&shape);
			break;
		}
		case gmShapeType_Polygon:
		{
			b2Vec2 translation;
			phBox2D_ToBtVec(translation, def->m_transform.m_pos);

			b2PolygonDef shape;
			shape.friction = def->m_friction;
			shape.density = def->m_density;
			shape.isSensor = def->m_isSensor;

			const gmShapeDef::Shape::Polygon& poly = def->m_shape.m_polygon;
			shape.vertexCount = poly.m_numVerts;
			for (u32 i = 0; i < poly.m_numVerts; i++)
			{
				b2Vec2 pos = poly.m_pos[i];
				pos += translation;
				pos.x *= b2scale.x;
				pos.y *= b2scale.y;
				pos *= gmObjectSys_GetToPhysScale();
				shape.vertices[i] = pos;
			}

			shape.filter.categoryBits = def->m_collisionFlag;
			shape.filter.maskBits = def->m_collisionMask;
			shape.filter.groupIndex = 0;

			body->CreateShape(&shape);
			break;
		}
		default:
			UE_NOT_IMPLEMENTED();
	}
}

void gmElement::Update(f32 dt)
{
	UPDATE_CHILD_ELEMS(m_sprites, dt);
	UPDATE_CHILD_ELEMS(m_particleEffects, dt);
}

void gmElement::Draw()
{
	DRAW_CHILD_ELEMS(m_sprites);
	DRAW_CHILD_ELEMS(m_particleEffects);
}

b2Body* gmElement::GetBody() const
{
	UE_ASSERT(m_def->IsPrimitive());
	return (b2Body*) m_body;
}

b2SoftBody* gmElement::GetSoftBody() const
{
	UE_ASSERT(m_def->IsSoft());
	return (b2SoftBody*) m_body;
}

gmSprite* gmElement::FindSprite(const char* name)
{
	gmSprite* sprite = m_sprites;
	while (sprite)
	{
		if (!ueStrCmp(sprite->m_def->m_name, name))
			return sprite;
		sprite = sprite->m_next;
	}
	return NULL;
}

void gmElement::SendEvent(gmEventType eventType, void* data)
{
	gmEventDef* eventDef = m_def->m_eventDefs;
	while (eventDef)
	{
		if (eventDef->m_type == eventType && (!m_object->m_toDelete || eventType == gmEventType_OnDestroy))
			eventDef->Fire(NULL, this, data);

		eventDef = eventDef->m_next;
	}
}

void gmElement::CalcBoundingRect(ueRect& rect)
{
	rect.Reset();

	if (m_body)
	{
		if (m_def->IsPrimitive())
		{
			b2Body* body = GetBody();
			b2Vec2 center = body->GetWorldCenter();
			center *= gmObjectSys_GetFromPhysScale();
			rect.Extend(center.x, center.y);
		}
		else
		{
			b2SoftBody* softBody = GetSoftBody();
			b2Body* body = b2SoftBody_GetBody(softBody, 0);
			b2Vec2 center = body->GetWorldCenter();
			center *= gmObjectSys_GetFromPhysScale();
			rect.Extend(center.x, center.y);
		}
	}
	else for (gmSprite* sprite = m_sprites; sprite; sprite = sprite->m_next)
	{
		ueRect tmp;
		sprite->CalcBoundingRect(tmp);
		rect.Extend(tmp);
	}
}

// gmObject

gmObject::gmObject(gmObjectSet* set, gmObjectRef* ref, gmObjectDef* def, const gmTransform& transform) :
	m_set(set),
	m_ref(ref),
	m_def(def),
	m_toDelete(UE_FALSE),
	m_elements(NULL),
	m_userData(NULL)
{
	UE_ASSERT(set);

	m_def->m_refCount++;

	// Create child elements

	for (gmElementDef* elementDef = def->m_elementDefs; elementDef; elementDef = elementDef->m_next)
	{
		gmElement* elem = new(gmObjectSys_GetAllocator()) gmElement(this, elementDef, transform);
		UE_ASSERT(elem);

		elem->m_next = m_elements;
		m_elements = elem;
	}

	// Instanciate child objects

	for (gmObjectRef* ref = def->m_objectRefs.Front(); ref; ref = ref->Next())
	{
		gmTransform combinedTransform = transform;
		combinedTransform.Mul(ref->m_transform);
		m_set->AddObject(ref, ref->m_def, combinedTransform);
	}
}

gmObject::~gmObject()
{
	SendEvent(gmEventType_OnDestroy, NULL);
	DEINIT_CHILD_ELEMS(m_elements);
	gmObjectSys_ReleaseDef(m_def);
}

void gmObject::Update(f32 dt)
{
	SendEvent(gmEventType_OnTick, &dt);
	UPDATE_CHILD_ELEMS(m_elements, dt);
}

void gmObject::Draw()
{
	DRAW_CHILD_ELEMS(m_elements);
	SendEvent(gmEventType_OnDraw, NULL);
}

void gmObject::SendEvent(gmEventType eventType, void* data)
{
	gmEventDef* eventDef = m_def->m_eventDefs;
	while (eventDef)
	{
		if (eventDef->m_type == eventType && (!m_toDelete || eventType == gmEventType_OnDestroy))
			eventDef->Fire(this, NULL, data);

		eventDef = eventDef->m_next;
	}
}

void gmObject::CalcBoundingRect(ueRect& rect)
{
	rect.Reset();
	for (gmElement* elem = m_elements; elem; elem = elem->m_next)
	{
		ueRect tmp;
		elem->CalcBoundingRect(tmp);
		rect.Extend(tmp);
	}
}

void gmObject::GetCenter(f32& x, f32& y)
{
	ueRect rect;
	CalcBoundingRect(rect);
	x = rect.CenterX();
	y = rect.CenterY();
}

void gmObject::GetCenter(ueVec2& xy)
{
	ueRect rect;
	CalcBoundingRect(rect);
	rect.Center(xy);
}

void gmObject::ScheduleToDelete()
{
	m_toDelete = UE_TRUE;
}

gmElement* gmObject::FindElement(const char* name)
{
	gmElement* element = m_elements;
	while (element)
	{
		if (!ueStrCmp(element->m_def->m_name, name))
			return element;
		element = element->m_next;
	}
	return NULL;
}

gmPropertyDef* gmObject::FindProperty(const char* name)
{
	// Ref properties can override def properties

	gmPropertyDef* prop = m_ref ? m_ref->FindProperty(name) : NULL;
	return prop ? prop : m_def->FindProperty(name);
}

ueBool gmObject::HasProperty(const char* name)
{
	return FindProperty(name) != NULL;
}

const char* gmObject::ReadProperty(const char* name)
{
	gmPropertyDef* def = FindProperty(name);
	if (!def) return NULL;
	if (def->m_type != gmPropertyType_String) return NULL;
	return def->m_value.m_string;
}

ueBool gmObject::ReadPropertyFloat(const char* name, f32& value)
{
	gmPropertyDef* def = FindProperty(name);
	if (!def) return UE_FALSE;
	if (def->m_type != gmPropertyType_Float) return UE_FALSE;
	value = def->m_value.m_float;
	return UE_TRUE;
}

ueBool gmObject::ReadPropertyInt(const char* name, s32& value)
{
	gmPropertyDef* def = FindProperty(name);
	if (!def) return UE_FALSE;
	if (def->m_type != gmPropertyType_Int) return UE_FALSE;
	value = def->m_value.m_int;
	return UE_TRUE;
}

ueBool gmObject::ReadPropertyBool(const char* name, ueBool& value)
{
	gmPropertyDef* def = FindProperty(name);
	if (!def) return UE_FALSE;
	if (def->m_type != gmPropertyType_Bool) return UE_FALSE;
	value = def->m_value.m_bool;
	return UE_TRUE;
}

gmObject* gmObject::GetOther(gmObject* obj, b2ContactPoint* point)
{
	gmElement* elem0 = (gmElement*) point->shape1->GetBody()->GetUserData();
	gmElement* elem1 = (gmElement*) point->shape2->GetBody()->GetUserData();
	gmObject* obj0 = elem0->m_object;
	gmObject* obj1 = elem1->m_object;
	return obj0 != obj ? obj0 : obj1;
}

gmObject* gmObject::GetOther(gmObject* obj, b2ContactResult* result)
{
	gmElement* elem0 = (gmElement*) result->shape1->GetBody()->GetUserData();
	gmElement* elem1 = (gmElement*) result->shape2->GetBody()->GetUserData();
	gmObject* obj0 = elem0->m_object;
	gmObject* obj1 = elem1->m_object;
	return obj0 != obj ? obj0 : obj1;
}
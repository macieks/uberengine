#include "Physics/Box2D/b2SoftBody.h"

class b2SoftBody
{
public:
	int numBodies;
	b2Body** bodies;
	int numJoints;
	b2Joint** joints;
	b2World* world;
	void* userData;

	b2Vec2* drawTexCoords;
	float outerCircleRadius;
};

b2SoftBody* b2SoftBody_Alloc(b2World* world, void* userData, int numBodies, int numJoints, int numDrawVerts)
{
	char* memory = (char*) b2Alloc(
		sizeof(b2SoftBody) +
		sizeof(b2Body*) * numBodies +
		sizeof(b2Joint*) * numJoints +
		sizeof(b2Vec2) * numDrawVerts);
	if (!memory)
		return NULL;

	b2SoftBody* body = (b2SoftBody*) memory;
	memory += sizeof(b2SoftBody);
	body->world = world;
	body->userData = userData;

	body->numBodies = numBodies;
	body->bodies = (b2Body**) memory;
	memory += sizeof(b2Body*) * numBodies;

	body->numJoints = numJoints;
	body->joints = (b2Joint**) memory;
	memory += sizeof(b2Joint*) * numJoints;

	body->drawTexCoords = (b2Vec2*) memory;
	memory += sizeof(b2Vec2) * numDrawVerts;

	return body;
}

b2SoftBody* b2RectangleSoftBody_Create(b2World* world, const b2SoftShapeDef* def)
{
	b2RectangleSoftBodyShapeDef* shape = (b2RectangleSoftBodyShapeDef*) def->shape;
	b2Assert(1 <= shape->numVerticalCircles);
	b2Assert(1 <= shape->numHorizontalCircles);

	// Allocate object

	b2SoftBody* body = b2SoftBody_Alloc(world, def->userData,
		shape->numVerticalCircles * shape->numHorizontalCircles,
		shape->numVerticalCircles * (shape->numHorizontalCircles - 1) + (shape->numVerticalCircles - 1) * shape->numHorizontalCircles,
		0);
	if (!body)
		return NULL;

	// Create bodies

	const float width = shape->max.x - shape->min.x;
	const float height = shape->max.y - shape->min.y;

	const float cellWidth = shape->numHorizontalCircles == 1 ? 0 : width / (float) (shape->numHorizontalCircles - 1);
	const float cellHeight = shape->numVerticalCircles == 1 ? 0 : height / (float) (shape->numVerticalCircles - 1);

	const float cellRadius = (cellWidth < cellHeight ? cellWidth : cellHeight) * 0.5f;

	b2CircleDef sd;
	sd.radius = cellRadius;
	sd.density = def->density;
	sd.friction = def->friction;
	sd.restitution = def->restitution;

	float y = shape->min.y + cellRadius;
	for (int i = 0; i < shape->numVerticalCircles; i++)
	{
		float x = shape->min.x + cellRadius;
		for (int j = 0; j < shape->numHorizontalCircles; j++)
		{
			b2BodyDef bd;
			bd.position.Set(x, y);

			b2Body* tempBody = body->bodies[i * shape->numHorizontalCircles + j] = world->CreateBody(&bd);
			tempBody->CreateShape(&sd);
			tempBody->SetMassFromShapes();

			x += cellWidth;
		}
		y += cellHeight;
	}

	// Create links

	b2DistanceJointDef jointDef;
	jointDef.collideConnected = true;
	jointDef.frequencyHz = def->jointFrequencyHz;
	jointDef.dampingRatio = def->jointDampingRatio;

	int jointIndex = 0;
	for (int i = 0; i < shape->numVerticalCircles; i++)
		for (int j = 0; j < shape->numHorizontalCircles; j++)
		{
			b2Body* bodyA = body->bodies[i * shape->numHorizontalCircles + j];
			b2Body* bodyB = i + 1 < shape->numVerticalCircles ? body->bodies[(i + 1) * shape->numHorizontalCircles + j] : NULL;
			b2Body* bodyC = j + 1 < shape->numHorizontalCircles ? body->bodies[i * shape->numHorizontalCircles + j + 1] : NULL;

			if (bodyB)
			{
				jointDef.Initialize(bodyA, bodyB, bodyA->GetWorldCenter(), bodyB->GetWorldCenter());
				body->joints[jointIndex++] = world->CreateJoint(&jointDef);
			}

			if (bodyC)
			{
				jointDef.Initialize(bodyA, bodyC, bodyA->GetWorldCenter(), bodyC->GetWorldCenter());
				body->joints[jointIndex++] = world->CreateJoint(&jointDef);
			}
		}

	return body;
}

b2SoftBody* b2CircleSoftBody_Create(b2World* world, const b2SoftShapeDef* def)
{
	b2CircleSoftBodyShapeDef* shape = (b2CircleSoftBodyShapeDef*) def->shape;
	b2Assert(3 <= shape->numOuterCircles);

	// Allocate object

	b2SoftBody* body = b2SoftBody_Alloc(world, def->userData, shape->numOuterCircles + 1, 2 * shape->numOuterCircles, shape->numOuterCircles);
	if (!body)
		return NULL;

	// Determine part radius

	const float angleStep = (3.14159265358979323846f * 2.0f) / shape->numOuterCircles;
	const float sinHalfAngle = sinf(angleStep * 0.5f);
	const float subCircleRadius = sinHalfAngle * shape->radius / (1.0f + sinHalfAngle);

	body->outerCircleRadius = subCircleRadius;

	// Create bodies

	b2CircleDef sd;
	sd.radius = subCircleRadius;
	sd.density = def->density;
	sd.friction = def->friction;
	sd.restitution = def->restitution;

	float angle = 0;
	for (int i = 0; i < shape->numOuterCircles; i++)
	{
		b2Vec2 offset(sinf(angle), cosf(angle));

		body->drawTexCoords[i] = offset;
		body->drawTexCoords[i] *= (1.0f + subCircleRadius / shape->radius);
		body->drawTexCoords[i] *= 0.5f;
		body->drawTexCoords[i] += b2Vec2(0.5f, 0.5f);

		offset *= (shape->radius - subCircleRadius);

		b2BodyDef bd;
		bd.userData = def->userData;
		bd.position = (shape->center + offset);

		body->bodies[i] = world->CreateBody(&bd);
		body->bodies[i]->CreateShape(&sd);
		body->bodies[i]->SetMassFromShapes();

		angle += angleStep;
	}

	// Create center body

	b2BodyDef bd;
	bd.userData = def->userData;
	bd.position = shape->center;
	b2Body* center = body->bodies[shape->numOuterCircles] = world->CreateBody(&bd);

	sd.radius = (shape->radius - subCircleRadius * 2.0f) * (1.0f - def->softness);
	center->CreateShape(&sd);
	center->SetMassFromShapes();

	// Create links

	b2DistanceJointDef jointDef;
	jointDef.collideConnected = true;
	jointDef.frequencyHz = def->jointFrequencyHz;
	jointDef.dampingRatio = def->jointDampingRatio;
	jointDef.userData = def->userData;

	for (int i = 0; i < shape->numOuterCircles; i++)
	{
		const int neighborIndex = (i + 1) % shape->numOuterCircles;

		// 1st link between neighbors

		jointDef.Initialize(
			body->bodies[i], body->bodies[neighborIndex],
			body->bodies[i]->GetWorldCenter(), body->bodies[neighborIndex]->GetWorldCenter());
		body->joints[i] = world->CreateJoint(&jointDef);

		// 2nd link with center body

		jointDef.Initialize(body->bodies[i], center, body->bodies[i]->GetWorldCenter(), shape->center);
		body->joints[shape->numOuterCircles + i] = world->CreateJoint(&jointDef);
	}

	return body;
}

b2SoftBody* b2SoftBody_Create(b2World* world, const b2SoftShapeDef* def)
{
	b2Assert(0.0f <= def->softness && def->softness <= 1.0f);
	switch (def->shape->type)
	{
	case b2SoftBodyType_Circle:
		return b2CircleSoftBody_Create(world, def);
	case b2SoftBodyType_Rectangle:
		return b2RectangleSoftBody_Create(world, def);
	default:
		return NULL;
	}
}

void b2SoftBody_Destroy(b2SoftBody* body)
{
	for (int i = 0; i < body->numJoints; i++)
		body->world->DestroyJoint(body->joints[i]);
	for (int i = 0; i < body->numBodies; i++)
		body->world->DestroyBody(body->bodies[i]);
	b2Free(body);
}

int b2SoftBody_GetNumBodies(b2SoftBody* body)
{
	return body->numBodies;
}

b2Body* b2SoftBody_GetBody(b2SoftBody* body, int index)
{
	b2Assert(0 <= index && index < body->numBodies);
	return body->bodies[index];
}

void b2SoftBodyUtil_GetPosAndTex(b2SoftBody* body, b2Vec2* posArray, b2Vec2* texArray, int* numVerts, int vertexArraySize)
{
	b2Body* centerBody = b2SoftBody_GetBody(body, body->numBodies - 1);
	const b2Vec2 centerBodyCenter = centerBody->GetWorldCenter();

	const int _numVerts = body->numBodies - 1;
	for (int i = 0; i < _numVerts; i++)
	{
		b2Body* _body = body->bodies[i];

		b2Vec2 bodyCenter = _body->GetWorldCenter();
		b2Vec2 toBodyVec = bodyCenter - centerBodyCenter;
		toBodyVec.Normalize();
		toBodyVec *= body->outerCircleRadius * 2.0f;

		posArray[i] = bodyCenter + toBodyVec;
		texArray[i] = body->drawTexCoords[i];
	}

	*numVerts = _numVerts;
}
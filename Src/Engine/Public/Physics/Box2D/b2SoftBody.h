/*
 *	Soft body for Box2D by Maciej Sawitus, 2011
 *	Feel free to use for any projects (commercial or non-commercial) without letting me know.
 *	I do not guarantee that this code works.
 */
#ifndef B2_SOFT_BODY_H
#define B2_SOFT_BODY_H

#include "Box2D.h"

//! Soft body shape type
enum b2SoftBodyType
{
	b2SoftBodyType_Circle = 0,
	b2SoftBodyType_Rectangle,

	b2SoftBodyType_MAX
};

//! Base soft body shape definition
struct b2SoftBodyShapeDef
{
	b2SoftBodyType type;

	inline b2SoftBodyShapeDef(b2SoftBodyType _type) : type(_type) {}
};

//! Circle soft body shape definition
struct b2CircleSoftBodyShapeDef : b2SoftBodyShapeDef
{
	b2Vec2 center;			//!< Soft circle center
	float radius;			//!< Soft circle radius
	int numOuterCircles;    //!< Number of outer circles to be created around the soft circle center

	b2CircleSoftBodyShapeDef() :
		b2SoftBodyShapeDef(b2SoftBodyType_Circle),
		center(0.0f, 0.0f),
		radius(5.0f),
		numOuterCircles(6)
	{}
};

//! Rectangle soft body shape definition
struct b2RectangleSoftBodyShapeDef : b2SoftBodyShapeDef
{
	b2Vec2 min;					//!< Min. rectangle coordinate
	b2Vec2 max;					//!< Max. rectangle coordinate
	int numVerticalCircles;		//!< Number of vertical circle rows within rectangle
	int numHorizontalCircles;	//!< Number of horizontal circle columns within rectangle

	b2RectangleSoftBodyShapeDef() :
		b2SoftBodyShapeDef(b2SoftBodyType_Rectangle),
		min(-1.0f, -1.0f),
		max(1.0f, 1.0f),
		numVerticalCircles(3),
		numHorizontalCircles(3)
	{}
};

//! Soft circle description
struct b2SoftShapeDef
{
	b2SoftShapeDef() :
		softness(0.2f),
		density(1.0f),
		friction(0.3f),
		restitution(0.0f),
		jointFrequencyHz(4.0f),
		jointDampingRatio(0.5f),
		userData(NULL)
	{}

	b2SoftBodyShapeDef* shape; //!< Soft body shape definition
	float softness; //!< Softness value within 0..1

	// Sub-body properties (see b2ShapeDef)

	float density;
	float friction;
	float restitution;

	// Distance joints' properties (see b2DistanceJointDef)

	float jointFrequencyHz;
	float jointDampingRatio;

	// Misc

	void* userData;
};

/**
 *  @class b2SoftBody
 *  @brief Soft body consisting of number of rigid bodies linked using joints
 */
class b2SoftBody;

//! Creates complex soft body in the world
b2SoftBody* b2SoftBody_Create(b2World* world, const b2SoftShapeDef* def);
//! Destroy soft body
void b2SoftBody_Destroy(b2SoftBody* body);

//! Gets number of sub-bodies of the soft body
int b2SoftBody_GetNumBodies(b2SoftBody* body);
//! Gets sub-body of the soft body at given index
b2Body* b2SoftBody_GetBody(b2SoftBody* body, int index);

//! Utility function to get position and texture coordinates of the soft body as a triangle list
void b2SoftBodyUtil_GetPosAndTex(b2SoftBody* body, b2Vec2* posArray, b2Vec2* texArray, int* numVerts, int vertexArraySize);

#endif // B2_SOFT_BODY_H
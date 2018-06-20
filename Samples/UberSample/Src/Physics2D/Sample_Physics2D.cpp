#include "SampleApp.h"
#include "Base/ueRand.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "Physics/Box2D/phBox2D.h"
#include "Physics/Box2D/b2SoftBody.h"

#include <Box2D.h>

/**
 *	Demonstrates 2D physics (using open-source Box2D library).
 */
class ueSample_Physics2D : public ueSample
{
public:
	ueBool Init()
	{
		phBox2D_Startup(g_app.GetFreqAllocator());

		// World

		b2AABB aabb;
		aabb.lowerBound.Set(-1000.0f, -1000.0f);
		aabb.upperBound.Set(1000.0f, 1000.0f);
		const b2Vec2 gravity(0.0f, -10.0f);
		const bool doSleep = true;
		m_world = new(g_app.GetStackAllocator()) b2World(aabb, gravity, doSleep);
		m_world->SetDebugDraw(phBox2D_GetDefaultDebugDraw());

		// Ground

		b2BodyDef groundBodyDef;
		groundBodyDef.position.Set(0.0f, -10.0f);
		m_groundBody = m_world->CreateBody(&groundBodyDef);

		b2PolygonDef groundBox;
		groundBox.SetAsBox(50.0f, 10.0f);

		m_groundBody->CreateShape(&groundBox);

		// Dynamic spheres and boxes

		b2PolygonDef boxShape;
		boxShape.SetAsBox(1.0f, 0.7f);
		boxShape.density = 1.0f; // Makes body dynamic
		boxShape.friction = 0.3f;

		b2CircleDef circleShape;
		circleShape.radius = 1.0f;
		circleShape.density = 1.0f; // Makes body dynamic
		circleShape.friction = 0.3f;

		for (u32 i = 0; i < UE_ARRAY_SIZE(m_bodies); i++)
		{
			b2BodyDef bodyDef;
			bodyDef.position.Set((f32) i, 10.0f + (f32) (i % 3) * 3);
			m_bodies[i] = m_world->CreateBody(&bodyDef);
			m_bodies[i]->CreateShape((i & 1) ? (b2ShapeDef*) &boxShape : (b2ShapeDef*) &circleShape);
			m_bodies[i]->SetMassFromShapes();
		}

		// Create soft bodies

		b2SoftShapeDef softBodyDef;
		softBodyDef.softness = 0.4f;

		// Soft circle body

		b2CircleSoftBodyShapeDef circleSoftBodyShapeDef;
		circleSoftBodyShapeDef.numOuterCircles = 10;
		circleSoftBodyShapeDef.radius = 4.0f;
		circleSoftBodyShapeDef.center = b2Vec2(0.0f, 20.0f);

		softBodyDef.shape = &circleSoftBodyShapeDef;
		m_softBodies[0] = b2SoftBody_Create(m_world, &softBodyDef);

		// Soft rectangle body

		b2RectangleSoftBodyShapeDef rectSoftBodyShapeDef;
		rectSoftBodyShapeDef.numVerticalCircles = 4;
		rectSoftBodyShapeDef.numHorizontalCircles = 4;
		rectSoftBodyShapeDef.min = b2Vec2(0.0f, 30.0f);
		rectSoftBodyShapeDef.max = b2Vec2(5.0f, 35.0f);

		softBodyDef.shape = &rectSoftBodyShapeDef;
		m_softBodies[1] = b2SoftBody_Create(m_world, &softBodyDef);

		return UE_TRUE;
	}

	void Deinit()
	{
		for (u32 i = 0; i < UE_ARRAY_SIZE(m_softBodies); i++)
			b2SoftBody_Destroy(m_softBodies[i]);
		ueDelete(m_world, g_app.GetStackAllocator());
		phBox2D_Shutdown();
	}

	void DoFrame(f32 dt)
	{
		glCtx* ctx = g_app.BeginDrawing();
		if (!ctx)
			return;
		UpdateAndDrawScene(dt, ctx);
		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

private:
	void UpdateAndDrawScene(f32 dt, glCtx* ctx)
	{
		UE_PROF_SCOPE("Update and Draw Physics Objects");

		// Begin drawing

		gxShapeDrawParams drawParams;
		drawParams.m_2DCanvas.Set(-40, 40, 50, -10);
		gxShapeDraw_SetDrawParams(&drawParams);
		gxShapeDraw_Begin(ctx);

		// Update (and debug-draw physics objects)

		m_world->Step(dt, 6);

		// End drawing

		gxShapeDraw_End();
	}

	b2World* m_world;
	b2Body* m_groundBody;
	b2Body* m_bodies[20];
	b2SoftBody* m_softBodies[2];
};

UE_DECLARE_SAMPLE(ueSample_Physics2D, "Physics2D")

#include "SampleApp.h"
#include "Input/inSys.h"
#include "Base/ueRand.h"
#include "Base/ueMath.h"
#include "GraphicsExt/gxCamera.h"
#include "Physics/phLib.h"
#include "Physics/phTetraBuilder.h"
#include "GraphicsExt/gxShapeDraw.h"

#define MAX_BODIES_STACKS			8
#define MAX_BODIES_PER_STACK		15
#define MAX_THROWABLE_BODIES		128
#define MAX_BODIES					(1 /* ground */ + MAX_THROWABLE_BODIES + MAX_BODIES_STACKS * MAX_BODIES_PER_STACK)

#define MAX_SOFT_BODIES_STACKS		1
#define MAX_SOFT_BODIES_PER_STACK	1
#define MAX_THROWABLE_SOFT_BODIES	10
#define MAX_SOFT_BODIES				(MAX_THROWABLE_SOFT_BODIES + MAX_SOFT_BODIES_STACKS * MAX_SOFT_BODIES_PER_STACK)

/**
 *	Demonstrates use of 3D physics library (phLib).
 */
class ueSample_Physics : public ueSample, phDebugDraw
{
public:
	ueBool Init()
	{
		// Physics

		m_groundOffset = 5.0f;
		m_spaceBetweenObjects = 0.5f;
		m_rayStart.Set(-15, 0.5f, 15),
		m_rayYAngle = 0;
		m_numBodies = 0;
		m_numSoftBodies = 0;

		phStartupParams phParams;
		phParams.m_allocator = g_app.GetFreqAllocator();
#if defined(PH_PHYSX)
		phParams.m_cookingBufferSize = 1 << 20; // Allow for run-time tri-mesh/convex/soft-mesh cooking
#endif
		phLib_Startup(&phParams);
		phTetraBuilder_Startup();

		phSceneDesc sceneDesc;
#if defined(PH_BULLET)
		sceneDesc.m_discreteWorld = UE_TRUE;
		sceneDesc.m_supportSoftBodies = UE_TRUE;
		sceneDesc.m_enableMultiThreadedSimulation = UE_FALSE;
#endif
		m_scene = phScene_Create(&sceneDesc);
		UE_ASSERT(m_scene);

		// Creare reusable shapes

		m_groundShapeSize.Set(100, 1, 100);
		phBoxShapeDesc groundShapeDesc;
		groundShapeDesc.m_size = m_groundShapeSize;
		m_groundShape = phShape_Create(&groundShapeDesc);

		// Create convex mesh description (distorted box)

		ueBox box(-0.4f, -0.4f, -0.4f, 0.4f, 0.4f, 0.4f);
		const f32 d = 0.2f; // "random" box distortion factor

		static const f32 posData[] =
		{
			box.m_min[0] + d,	box.m_min[1],		box.m_min[2] - d,
			box.m_max[0],		box.m_min[1] + d,	box.m_min[2] + d,
			box.m_min[0],		box.m_max[1] + d,	box.m_min[2],
			box.m_max[0] + d,	box.m_max[1],		box.m_min[2],
			box.m_min[0],		box.m_min[1],		box.m_max[2] - d,
			box.m_max[0],		box.m_min[1] - d,	box.m_max[2] + d,
			box.m_min[0] - d,	box.m_max[1],		box.m_max[2],
			box.m_max[0],		box.m_max[1] + d,	box.m_max[2]
		};

		static const u32 indices[] =
		{
			0,3,1, 0,2,3,
			4,5,7, 4,7,6,
			0,4,2, 2,4,6,
			1,3,5, 5,3,7,
			2,6,3, 6,7,3,
			1,5,4, 1,4,0
		};

		m_meshDesc.m_isConvex = UE_TRUE;

		m_meshDesc.m_numVerts = UE_ARRAY_SIZE(posData) / 3;
		m_meshDesc.m_posStride = sizeof(f32) * 3;
		m_meshDesc.m_pos = posData;

		m_meshDesc.m_numIndices = UE_ARRAY_SIZE(indices);
		m_meshDesc.m_indexSize = sizeof(u32);
		m_meshDesc.m_indexData = indices;

		// Create soft body mesh description

		box.Set(-0.4f, -0.4f, -0.4f, 0.4f, 0.4f, 0.4f);
		box.Scale(3.8f, 6.0f, 2.0f);

		static const f32 softBodyPosData[] =
		{
			box.m_min[0],	box.m_min[1],	box.m_min[2],
			box.m_max[0],	box.m_min[1],	box.m_min[2],
			box.m_min[0],	box.m_max[1],	box.m_min[2],
			box.m_max[0],	box.m_max[1],	box.m_min[2],
			box.m_min[0],	box.m_min[1],	box.m_max[2],
			box.m_max[0],	box.m_min[1],	box.m_max[2],
			box.m_min[0],	box.m_max[1],	box.m_max[2],
			box.m_max[0],	box.m_max[1],	box.m_max[2]
		};

		m_softMeshDesc.m_subdivisionLevel = 20;

		m_softMeshDesc.m_numVerts = UE_ARRAY_SIZE(softBodyPosData) / 3;
		m_softMeshDesc.m_posStride = sizeof(f32) * 3;
		m_softMeshDesc.m_pos = softBodyPosData;

		m_softMeshDesc.m_numIndices = UE_ARRAY_SIZE(indices);
		m_softMeshDesc.m_indexSize = sizeof(u32);
		m_softMeshDesc.m_indexData = indices;

		// Initialize current shape

		m_currentShape = NULL;
		m_currentShapeType = phShapeType_Box;
		RebuildCurrentShape();

		// Create physics objects

		RestartScene();

		// Create camera

		m_camera.SetLookAt(ueVec3(10, 10, 10), ueVec3::Zero);

		// Initialize input events

		m_inputConsumerId = inSys_RegisterConsumer("physics sample", 0.0f);
		m_inputEvents.m_restartScene = inSys_RegisterEvent(m_inputConsumerId, "restart scene", &inBinding(inDev_Keyboard, inKey_R), &inBinding(inDev_Gamepad, inGamepadButton_X));
		m_inputEvents.m_toggleShape = inSys_RegisterEvent(m_inputConsumerId, "toggle shape", &inBinding(inDev_Keyboard, inKey_N), &inBinding(inDev_Gamepad, inGamepadButton_A));
		m_inputEvents.m_throwShape = inSys_RegisterEvent(m_inputConsumerId, "throw shape", &inBinding(inDev_Keyboard, inKey_T), &inBinding(inDev_Gamepad, inGamepadButton_B));

		return UE_TRUE;
	}

	void Deinit()
	{
		// Input

		inSys_UnregisterConsumer(m_inputConsumerId);

		// Physics

		DestroyScene();

		phShape_Destroy(m_currentShape);
		phShape_Destroy(m_groundShape);

		phScene_Destroy(m_scene);

		phTetraBuilder_Shutdown();
		phLib_Shutdown();
	}

	void DoFrame(f32 dt)
	{
		Update(dt);
		Draw();
	}

	void Update(f32 dt)
	{
		if (inSys_IsConsumerActive(m_inputConsumerId))
		{
			m_camera.UpdateFreeCamera(dt);

			if (inSys_WasPressed(m_inputEvents.m_restartScene))
				RestartScene();
			else if (inSys_WasPressed(m_inputEvents.m_toggleShape))
			{
				if (m_currentShapeType == phShapeType_Box) m_currentShapeType = phShapeType_Sphere;
				else if (m_currentShapeType == phShapeType_Sphere) m_currentShapeType = phShapeType_Capsule;
				else if (m_currentShapeType == phShapeType_Capsule) m_currentShapeType = phShapeType_SoftMesh;
#if defined(PH_PHYSX)
				// Note: PhysX doesn't support cylinder shape
				else if (m_currentShapeType == phShapeType_SoftMesh) m_currentShapeType = phShapeType_Convex;
#else
				else if (m_currentShapeType == phShapeType_SoftMesh) m_currentShapeType = phShapeType_Cylinder;
				else if (m_currentShapeType == phShapeType_Cylinder) m_currentShapeType = phShapeType_Convex;
#endif
				else if (m_currentShapeType == phShapeType_Convex) m_currentShapeType = phShapeType_Box;

				DestroyScene();
				RebuildCurrentShape();
				RestartScene();
			}
			else if (inSys_WasPressed(m_inputEvents.m_throwShape))
			{
				const ueVec3& cameraPos = m_camera.GetPosition();
				const ueVec3& cameraViewDir = m_camera.GetViewDir();

				if (m_currentShapeType == phShapeType_SoftMesh)
				{
					phSoftBody* body = CreateSoftBody(cameraPos);
					if (body)
						phSoftBody_ApplyImpulseAt(body, cameraPos);
				}
				else
				{
					phBody* body = CreateRigidBody(cameraPos);
					if (body)
					{
						phBody_ApplyImpulse(body, cameraViewDir * 20.0f);
					}
				}
			}
		}

		// Update physics
		{
			UE_PROF_SCOPE("Update Physics");

			phScene_BeginUpdate(m_scene, dt);
			phScene_EndUpdate(m_scene);
		}

		// Update ray
		{
			UE_PROF_SCOPE("Ray Test");

			m_rayYAngle += dt * 1.0f;
			m_rayDir.Set(ueSin(m_rayYAngle), 0, -ueCos(m_rayYAngle));
			m_rayDir.Normalize();

			phRayIntersectionTestDesc testDesc;
			testDesc.m_findClosestIntersections = UE_TRUE;
			testDesc.m_maxNumResults = 1;
			testDesc.m_start = m_rayStart;
			testDesc.m_end = m_rayStart + m_rayDir * 1000.0f;

			phShapeIntersectionResult result;
			phShapeIntersectionResultSet results;
			results.m_results = &result;
			phScene_IntersectRay(m_scene, &testDesc, &results);

			if (results.m_numResults == 0)
			{
				m_rayHit = UE_FALSE;
				m_rayEnd = m_rayStart + m_rayDir;
			}
			else
			{
				m_rayHit = UE_TRUE;
				m_rayEnd = m_rayStart + m_rayDir * results.m_results[0].m_distance;
			}
		}
	}

	void DestroyScene()
	{
		for (u32 i = 0; i < m_numBodies; i++)
			phBody_Destroy(m_bodies[i]);
		m_numBodies = 0;

		for (u32 i = 0; i < m_numSoftBodies; i++)
			phSoftBody_Destroy(m_softBodies[i]);
		m_numSoftBodies = 0;
	}

	void RestartScene()
	{
		// Destroy existing objects
		
		DestroyScene();

		// Reset ray

		m_rayYAngle = 0;

		// Create ground
		{
			ueMat44 initialTransform;
			initialTransform.SetTranslation(0, -0.5f, 0);

			phBodyDesc bodyDesc;
			bodyDesc.m_shape = m_groundShape;
			bodyDesc.m_transform = &initialTransform;

			m_bodies[m_numBodies++] = phBody_Create(m_scene, &bodyDesc);
		}

		// Create stacks of objects

		ueRand* rand = ueRand_GetGlobal();

		if (m_currentShapeType == phShapeType_SoftMesh)
		{
			const f32 size = 5.0f;
			m_spaceBetweenObjects = 1.0f;

			for (u32 i = 0; i < MAX_SOFT_BODIES_STACKS; i++)
				for (u32 j = 0; j < MAX_SOFT_BODIES_PER_STACK; j++)
				{
					const ueVec3 initialPosition(
						(i - (MAX_SOFT_BODIES_STACKS + 0.5f) * size * 0.5f) * 2.0f,
						m_groundOffset + j * (size + m_spaceBetweenObjects),
						0);

					CreateSoftBody(initialPosition);
				}
		}
		else
			for (u32 i = 0; i < MAX_BODIES_STACKS; i++)
				for (u32 j = 0; j < MAX_BODIES_PER_STACK; j++)
				{
					const ueVec3 initialPosition(
						(i - MAX_BODIES_STACKS * 0.5f + 0.5f) * 2.0f,
						m_groundOffset + j * (1.0f + m_spaceBetweenObjects),
						0);

					CreateRigidBody(initialPosition);
				}
	}

	phBody* CreateRigidBody(const ueVec3 initialPosition)
	{
		if (m_numBodies == UE_ARRAY_SIZE(m_bodies))
			return NULL;

		ueMat44 initialTransform;
		initialTransform.SetTranslation(initialPosition);

		phBodyDesc bodyDesc;
		bodyDesc.m_mass = 1.0f;
		bodyDesc.m_transform = &initialTransform;
		bodyDesc.m_shape = m_currentShape;
		phBody* body = phBody_Create(m_scene, &bodyDesc);
		if (!body)
			return NULL;
		m_bodies[m_numBodies++] = body;
		return body;
	}

	phSoftBody* CreateSoftBody(const ueVec3 initialPosition)
	{
		if (m_numSoftBodies == UE_ARRAY_SIZE(m_softBodies))
			return NULL;

		ueMat44 initialTransform;
		initialTransform.SetTranslation(initialPosition);

		phSoftBodyDesc bodyDesc;
		bodyDesc.m_transform = &initialTransform;
		bodyDesc.m_shape = m_currentShape;
		bodyDesc.m_volumeStiffness = 0.1f;
		bodyDesc.m_stretchingStiffness = 0.1f;

		phSoftBody* body = phSoftBody_Create(m_scene, &bodyDesc);
		if (!body)
			return NULL;
		m_softBodies[m_numSoftBodies++] = body;
		return body;
	}

	void RebuildCurrentShape()
	{
		if (m_currentShape)
		{
			phShape_Destroy(m_currentShape);
			m_currentShape = NULL;
		}

		switch (m_currentShapeType)
		{
			case phShapeType_Box:
			{
				phBoxShapeDesc shapeDesc;
				shapeDesc.m_size = ueVec3::One;
				m_currentShape = phShape_Create(&shapeDesc);
				break;
			}

			case phShapeType_Sphere:
			{
				phSphereShapeDesc shapeDesc;
				shapeDesc.m_radius = 0.5f;
				m_currentShape = phShape_Create(&shapeDesc);
				break;
			}

			case phShapeType_Capsule:
			{
				phCapsuleShapeDesc shapeDesc;
				shapeDesc.m_radius = 0.25f;
				shapeDesc.m_height = 0.5f;
				m_currentShape = phShape_Create(&shapeDesc);
				break;
			}

			case phShapeType_Cylinder:
			{
				phCylinderShapeDesc shapeDesc;
				shapeDesc.m_radius = 0.5f;
				shapeDesc.m_height = 1.0f;
				m_currentShape = phShape_Create(&shapeDesc);
				break;
			}

			case phShapeType_Convex:
			{
				phConvexShapeDesc shapeDesc;
				shapeDesc.m_meshDesc = &m_meshDesc;
				m_currentShape = phShape_Create(&shapeDesc);
				break;
			}

			case phShapeType_SoftMesh:
			{
				phSoftMeshShapeDesc shapeDesc;
				shapeDesc.m_meshDesc = &m_softMeshDesc;
				m_currentShape = phShape_Create(&shapeDesc);
				break;
			}
		}
	}

	void Draw()
	{
		glCtx* ctx = g_app.BeginDrawing();
		if (!ctx)
			return;
		DrawScene(ctx);
		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

	void DrawScene(glCtx* ctx)
	{
		UE_PROF_SCOPE("Draw Physics Objects");

		gxShapeDrawParams drawParams;
		drawParams.m_viewProj = &m_camera.GetViewProj();
		gxShapeDraw_SetDrawParams(&drawParams);
		gxShapeDraw_Begin(ctx);

		phScene_DebugDraw(m_scene, this); // Use this as phDebugDraw callback

		DrawLine(m_rayStart, m_rayEnd, m_rayHit ? ueColor32::Red : ueColor32(100, 100, 100, 255));
		if (m_rayHit)
		{
			const ueVec3 drawRayEnd(0, 5, 0);
			m_rayEnd += drawRayEnd;
			DrawLine(m_rayEnd, drawRayEnd, ueColor32::Red);
		}

		gxShapeDraw_End();
	}

	void DrawLine(const ueVec3& from, const ueVec3& to, ueColor32 color)
	{
		gxShape_Line line;
		line.m_color = color;
		line.m_a = from;
		line.m_b = to;
		gxShapeDraw_DrawLine(line);
	}

	// Physics

	phScene* m_scene;

	u32 m_numBodies;
	phBody* m_bodies[MAX_BODIES];
	u32 m_numSoftBodies;
	phSoftBody* m_softBodies[MAX_SOFT_BODIES];

	phShapeType m_currentShapeType;
	phShape* m_currentShape;

	ueVec3 m_groundShapeSize;
	phShape* m_groundShape;

	phMeshDesc m_meshDesc;
	phSoftMeshDesc m_softMeshDesc;

	f32 m_groundOffset;
	f32 m_spaceBetweenObjects;

	ueVec3 m_rayDir;
	ueVec3 m_rayStart;
	ueVec3 m_rayEnd;
	f32 m_rayYAngle;
	ueBool m_rayHit;

	gxCamera m_camera;

	// Input

	inConsumerId m_inputConsumerId;
	struct InputEvents
	{
		inEventId m_restartScene;
		inEventId m_toggleShape;
		inEventId m_throwShape;
	} m_inputEvents;
};

UE_DECLARE_SAMPLE(ueSample_Physics, "Physics")
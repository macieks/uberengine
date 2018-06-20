#include "SampleApp.h"
#include "Base/ueMath.h"
#include "Base/ueRand.h"
#include "Input/inSys.h"
#include "GraphicsExt/gxCommonConstants.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxDebugModelRenderer.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxSkeleton.h"
#include "GraphicsExt/gxModel.h"
#include "GraphicsExt/gxModelInstance.h"
#include "GraphicsExt/gxCollision.h"
#include "GraphicsExt/gxCamera.h"
#include "Physics/phLib.h"

/**
 *	Demonstrates simple "game" in quake 3 level.
 */
class ueSample_Quake3Level : public ueSample
{
	ueBool Init()
	{
		// Start up physics engine

		phStartupParams phParams;
		phParams.m_allocator = g_app.GetFreqAllocator();
		phLib_Startup(&phParams);

		// Load resources (sync load)

		m_model.SetByName("quake3_level_sample/test_level");
		//m_texture.SetByName("common/default_texture");
		m_texture.SetByName("rts_sample/terrain0");

		m_program.Create("common/model_vs", "common/model_fs");

		// Create physics world representation

		phSceneDesc sceneDesc;
#if defined(PH_BULLET)
		sceneDesc.m_discreteWorld = UE_TRUE;
		sceneDesc.m_enableMultiThreadedSimulation = UE_FALSE;
#endif
		m_scene = phScene_Create(&sceneDesc);
		UE_ASSERT(m_scene);

		phScene_SetGravity(m_scene, ueVec3(0.0f, -29.8f, 0.0f));

		m_showCollision = UE_FALSE;

		// Create model instance and add it to physics scene

		m_modelInstance = gxModel_CreateInstance(*m_model);
		gxModelInstance_AddToScene(m_modelInstance, m_scene);

		// Create player controller

		phPlayerControllerDesc pcDesc;
		pcDesc.m_scene = m_scene;
		pcDesc.m_height = m_playerHeight = 15;
		pcDesc.m_radius = 5;
		pcDesc.m_stepHeight = 6;
		pcDesc.m_maxSlope = UE_PI * 0.25f;
		m_playerController = phPlayerController_Create(&pcDesc);
		UE_ASSERT(m_playerController);

		// Init camera

		m_isFreeCamera = UE_FALSE;
		m_camera.SetFarZ(500.0f);

		// Spawn player somewhere

		Respawn();

		// Input

		m_inputConsumerId = inSys_RegisterConsumer("Quake 3 Level sample", 0.0f);

		m_inputEvents.m_showCollision = inSys_RegisterEvent(m_inputConsumerId, "show collision", &inBinding(inDev_Keyboard, inKey_C), &inBinding(inDev_Gamepad, inGamepadButton_B));
		m_inputEvents.m_switchCamera = inSys_RegisterEvent(m_inputConsumerId, "switch camera", &inBinding(inDev_Keyboard, inKey_Z), &inBinding(inDev_Gamepad, inGamepadButton_Y));

		m_inputEvents.m_respawn = inSys_RegisterEvent(m_inputConsumerId, "respawn", &inBinding(inDev_Keyboard, inKey_X), &inBinding(inDev_Gamepad, inGamepadButton_X));

		m_inputEvents.m_jump = inSys_RegisterEvent(m_inputConsumerId, "jump", &inBinding(inDev_Keyboard, inKey_Space), &inBinding(inDev_Gamepad, inGamepadButton_A));
		m_inputEvents.m_forward = inSys_RegisterEvent(m_inputConsumerId, "forward", &inBinding(inDev_Keyboard, inKey_Up), &inBinding(inDev_Keyboard, inKey_W));
		m_inputEvents.m_back = inSys_RegisterEvent(m_inputConsumerId, "back", &inBinding(inDev_Keyboard, inKey_Down), &inBinding(inDev_Keyboard, inKey_S));
		m_inputEvents.m_strafeRight = inSys_RegisterEvent(m_inputConsumerId, "strafe_right", &inBinding(inDev_Keyboard, inKey_Right), &inBinding(inDev_Keyboard, inKey_D));
		m_inputEvents.m_strafeLeft = inSys_RegisterEvent(m_inputConsumerId, "strafe_left", &inBinding(inDev_Keyboard, inKey_Left), &inBinding(inDev_Keyboard, inKey_A));

		return UE_TRUE;
	}

	void Deinit()
	{
		inSys_UnregisterConsumer(m_inputConsumerId);
		gxModelInstance_Destroy(m_modelInstance);
		phPlayerController_Destroy(m_playerController);
		phScene_Destroy(m_scene);
		m_model.Release(); // Must destroy model before physics library (model keeps reference to collision geometry)
		phLib_Shutdown();
	}

	void DoFrame(f32 dt)
	{
		// Update

		if (inSys_IsConsumerActive(m_inputConsumerId))
		{
			if (inSys_WasPressed(m_inputEvents.m_showCollision))
				m_showCollision = !m_showCollision;

			if (inSys_WasPressed(m_inputEvents.m_switchCamera))
			{
				m_isFreeCamera = !m_isFreeCamera;

				// Place player controller where camera was

				if (!m_isFreeCamera)
					phPlayerController_SetPosition(m_playerController, m_camera.GetPosition());
			}

			if (m_isFreeCamera)
				m_camera.UpdateFreeCamera(dt);
			else
			{
				UE_PROF_SCOPE("Update Player Controller");

				if (inSys_WasPressed(m_inputEvents.m_respawn))
					Respawn();

				if (inSys_WasPressed(m_inputEvents.m_jump))
					phPlayerController_Jump(m_playerController, 150);

				// Get front and right view vectors

				ueVec3 frontVec = m_camera.GetViewDir();
				frontVec[1] = 0.0f;
				frontVec.Normalize();
				
				const ueVec3 rightVec(frontVec[2], 0.0f, -frontVec[0]);

				// Determine desired player velocity

				const f32 movementSpeed = 80.0f;
				const f32 maxSpeed = 1.0f;

				m_playerVelocity.Zeroe();

				if (inSys_IsDown(m_inputEvents.m_forward))
					m_playerVelocity += frontVec * 1.0f;
				else if (inSys_IsDown(m_inputEvents.m_back))
					m_playerVelocity += frontVec * -1.0f;
				if (inSys_IsDown(m_inputEvents.m_strafeRight))
					m_playerVelocity += rightVec * 1.0f;
				else if (inSys_IsDown(m_inputEvents.m_strafeLeft))
					m_playerVelocity += rightVec * -1.0f;

				m_playerVelocity += frontVec * inGamepad_GetAnalog(inGamepadAnalog_LeftStick_Y);
				m_playerVelocity += rightVec * inGamepad_GetAnalog(inGamepadAnalog_LeftStick_X);

				m_playerVelocity.ClampLen(maxSpeed);
				m_playerVelocity *= movementSpeed;

				ueVec3 gravity;
				phScene_GetGravity(gravity, m_scene);
				m_playerVelocity += gravity;

				// Move player

				phPlayerController_Move(m_playerController, m_playerVelocity, dt);

				// Update camera

				f32 rotationX = 0.0f;
				f32 rotationY = 0.0f;

				{
					s32 mouseX = 0;
					s32 mouseY = 0;
					inMouse_GetVector(mouseX, mouseY);

					const f32 mouseSpeed = 0.005f;

					rotationY += (f32) mouseX * mouseSpeed;
					rotationX += (f32) mouseY * mouseSpeed;
				}

				{
					const f32 gamepadSpeed = 4.0f;

					rotationY += inGamepad_GetAnalog(inGamepadAnalog_RightStick_X) * gamepadSpeed * dt;
					rotationX += -inGamepad_GetAnalog(inGamepadAnalog_RightStick_Y) * gamepadSpeed * dt;
				}

				if (rotationX != 0.0f)
					m_camera.RotateX(rotationX);
				if (rotationY != 0.0f)
					m_camera.RotateY(rotationY);

				ueVec3 playerPos;
				phPlayerController_GetPosition(m_playerController, playerPos);

				m_camera.SetLookAt(playerPos, playerPos + m_camera.GetViewDir());
			}
		}

		// Update physics
		{
			UE_PROF_SCOPE("Update Physics");

			if (!m_isFreeCamera)
			{
				phScene_BeginUpdate(m_scene, dt);
				phScene_EndUpdate(m_scene);
				phPlayerController_UpdateAll();
			}
		}

		// Draw

		glCtx* ctx = g_app.BeginDrawing();
		if (!ctx)
			return;
		if (m_showCollision)
			DrawPhysicsScene(ctx);
		else
			DrawModelInstance(ctx, m_modelInstance);
		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

	void DrawPhysicsScene(glCtx* ctx)
	{
		UE_PROF_SCOPE("Draw Physics Objects");

		gxShapeDrawParams drawParams;
		drawParams.m_viewProj = &m_camera.GetViewProj();
		gxShapeDraw_SetDrawParams(&drawParams);
		gxShapeDraw_Begin(ctx);

		phScene_DebugDraw(m_scene); // Use default phDebugDraw callback

		gxShapeDraw_End();
	}

	void DrawModelInstance(glCtx* ctx, gxModelInstance* modelInstance)
	{
		UE_PROF_SCOPE("Draw Model Instance");

		gxModel* model = gxModelInstance_GetModel(modelInstance);
		gxModelLOD* lod = gxModel_GetLOD(model, 0);

		// Set default mesh render states

		glCtx_SetFillMode(ctx, glFillMode_Solid);
		glCtx_SetBlending(ctx, UE_FALSE);
		glCtx_SetCullMode(ctx, glCullMode_CCW);
		glCtx_SetDepthWrite(ctx, UE_TRUE);
		glCtx_SetDepthTest(ctx, UE_TRUE);

		glCtx_SetIndices(ctx, lod->m_IB);

		glCtx_SetProgram(ctx, m_program.GetProgram());

		glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::World, gxModelInstance_GetBaseTransform(modelInstance));
		glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::ViewProj, &m_camera.GetViewProj());

		// Draw all meshes

		for (u32 i = 0; i < lod->m_numMeshes; i++)
		{
			gxModelMesh* mesh = &lod->m_meshes[i];

			glCtx_SetStream(ctx, 0, lod->m_VB, lod->m_vertexFormats[mesh->m_vertexFormatIndex], mesh->m_vertexBufferOffset);
			glCtx_SetSamplerConstant(ctx, gxCommonConstants::ColorMap, gxTexture_GetBuffer(*m_texture));
			glCtx_DrawIndexed(ctx, (glPrimitive) mesh->m_primType, mesh->m_baseVertex, mesh->m_firstVertex, mesh->m_numVerts, mesh->m_firstIndex, mesh->m_numIndices);
		}

		// Draw normals

		gxDebugModelRendererDrawParams debugModelDraw;
		debugModelDraw.m_ctx = ctx;
		debugModelDraw.m_viewProj = &m_camera.GetViewProj();
		debugModelDraw.m_drawModel = UE_FALSE;
		debugModelDraw.m_drawNormals = UE_TRUE;
		debugModelDraw.m_modelInstance = modelInstance;
		gxDebugModelRenderer_Draw(&debugModelDraw);
	}

	void Respawn()
	{
		// Respawn points are stored in bones

		const gxModelLOD* lod = gxModel_GetLOD(*m_model, 0);
		const gxSkeleton* skeleton = lod->m_skeleton;

		UE_ASSERT(skeleton->m_numNodes > 0);

		// Pick random respawn point

		const gxSkeletonNode& spawnPoint = skeleton->m_nodes[ueRandG_U32() % skeleton->m_numNodes];

		// Position player controller at respawn point

		ueVec3 controllerPos = spawnPoint.m_localTransform.GetTranslation();
		controllerPos[1] += m_playerHeight + 0.1f;

		phPlayerController_SetPosition(m_playerController, controllerPos);

		// Position camera at respawn point

		if (m_isFreeCamera)
			m_camera.SetLookAt(
				spawnPoint.m_localTransform.GetTranslation(),
				spawnPoint.m_localTransform.GetTranslation() + spawnPoint.m_localTransform.GetFrontVec());
	}

	// Camera

	gxCamera m_camera;
	ueBool m_isFreeCamera;

	// Rendering

	gxModelInstance* m_modelInstance;

	gxProgram m_program;

	ueResourceHandle<gxTexture> m_texture;
	ueResourceHandle<gxModel> m_model;

	// Physics

	phScene* m_scene;

	phPlayerController* m_playerController;
	ueVec3 m_playerVelocity;
	f32 m_playerHeight;

	ueBool m_showCollision;

	// Input

	inConsumerId m_inputConsumerId;
	struct InputEvents
	{
		inEventId m_showCollision;
		inEventId m_switchCamera;

		inEventId m_respawn;

		inEventId m_jump;
		inEventId m_forward;
		inEventId m_back;
		inEventId m_strafeLeft;
		inEventId m_strafeRight;
	} m_inputEvents;
};

UE_DECLARE_SAMPLE(ueSample_Quake3Level, "Quake3Level")

#include "SampleApp.h"
#include "GraphicsExt/gxCommonConstants.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxModel.h"
#include "GraphicsExt/gxMultiPostprocess.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxCamera.h"
#include "Input/inSys.h"

/**
 *	Demonstrates use of post-processing framework as well as multisampled render target.
 */
class ueSample_Postprocessing : public ueSample
{
public:
	ueBool Init()
	{
		// Get model

		m_model.SetByName("postprocessing_sample/chair_model");
		m_texture.SetByName("postprocessing_sample/chair_tex");

		// Get shaders

		m_colorProgram.Create("common/model_vs", "common/model_fs");
		m_depthProgram.Create("common/depth_view_space_vs", "common/depth_view_space_fs");

		// Initialize post-processing

		gxMultiPostprocessStartupParams ppParams;
		ppParams.m_allocator = g_app.GetStackAllocator();
		gxMultiPostprocess_Startup(&ppParams);

		m_updatePostprocesses = UE_TRUE;

		// Enable some effects at startup

		gxMultiPostprocess_SetEnabled(
			gxMultiPostprocessEffect_CameraMotionBlur |
			gxMultiPostprocessEffect_RainyGlass);

		// Disable stereo at startup

		m_enableStereo = UE_FALSE;
		m_stereoEyeOffset = 0.02f;
		m_leftEyeColorMask = glColorMask_Green;
		m_rightEyeColorMask = glColorMask_Red | glColorMask_Blue;

		// Disable MSAA at startup

		m_MSAAModeIndex = 0;

		// Reset scene rotation

		m_rotation = 0.0f;

		// Create camera

		m_camera.SetLookAt(ueVec3(3, 3, 3), ueVec3::Zero);

		// Initialize input events

		m_inputConsumerId = inSys_RegisterConsumer("post-processing sample", 0.0f);
		m_inputEvents.m_toggleUpdate = inSys_RegisterEvent(m_inputConsumerId, "toggle update", &inBinding(inDev_Keyboard, inKey_0), &inBinding(inDev_Gamepad, inGamepadButton_Start));
		m_inputEvents.m_toggleBloom = inSys_RegisterEvent(m_inputConsumerId, "toggle bloom", &inBinding(inDev_Keyboard, inKey_1), &inBinding(inDev_Gamepad, inGamepadButton_X));
		m_inputEvents.m_toggleOldTV = inSys_RegisterEvent(m_inputConsumerId, "toggle old tv", &inBinding(inDev_Keyboard, inKey_2), &inBinding(inDev_Gamepad, inGamepadButton_A));
		m_inputEvents.m_toggleDepthOfFieldLQ = inSys_RegisterEvent(m_inputConsumerId, "toggle depth of field low quality", &inBinding(inDev_Keyboard, inKey_3), &inBinding(inDev_Gamepad, inGamepadButton_B));
		m_inputEvents.m_toggleDepthOfFieldHQ = inSys_RegisterEvent(m_inputConsumerId, "toggle depth of field high quality", &inBinding(inDev_Keyboard, inKey_4), &inBinding(inDev_Gamepad, inGamepadButton_Y));
		m_inputEvents.m_toggleRainyGlass = inSys_RegisterEvent(m_inputConsumerId, "toggle rainy glass", &inBinding(inDev_Keyboard, inKey_5), &inBinding(inDev_Gamepad, inGamepadButton_DPad_Up));
		m_inputEvents.m_toggleCameraMotionBlur = inSys_RegisterEvent(m_inputConsumerId, "toggle camera motion blur", &inBinding(inDev_Keyboard, inKey_6), &inBinding(inDev_Gamepad, inGamepadButton_DPad_Down));
		m_inputEvents.m_toggleStereo = inSys_RegisterEvent(m_inputConsumerId, "toggle stereo rendering (needs glasses)", &inBinding(inDev_Keyboard, inKey_7), &inBinding(inDev_Gamepad, inGamepadButton_DPad_Left));
		m_inputEvents.m_decreaseStereoEyeDist = inSys_RegisterEvent(m_inputConsumerId, "decrease stereo eye distance", &inBinding(inDev_Keyboard, inKey_Y));
		m_inputEvents.m_increaseStereoEyeDist = inSys_RegisterEvent(m_inputConsumerId, "increase stereo eye distance", &inBinding(inDev_Keyboard, inKey_U));
		m_inputEvents.m_decreaseMSAALevel = inSys_RegisterEvent(m_inputConsumerId, "decrease MSAA level", &inBinding(inDev_Keyboard, inKey_8));
		m_inputEvents.m_increaseMSAALevel = inSys_RegisterEvent(m_inputConsumerId, "increase MSAA level", &inBinding(inDev_Keyboard, inKey_9));

		return UE_TRUE;
	}

	void Deinit()
	{
		inSys_UnregisterConsumer(m_inputConsumerId);
		gxMultiPostprocess_Shutdown();
	}

	void DoFrame(f32 dt)
	{
		Update(dt);
		Draw();
	}

private:

	void Update(f32 dt)
	{
		// Update input & camera

		if (inSys_IsConsumerActive(m_inputConsumerId))
		{
			m_camera.UpdateFreeCamera(dt);

			if (inSys_WasPressed(m_inputEvents.m_toggleUpdate))
			{
				m_updatePostprocesses = !m_updatePostprocesses;
				if (m_updatePostprocesses)
					gxMultiPostprocess_ResetMotionBlur();
			}
			else if (inSys_WasPressed(m_inputEvents.m_toggleBloom))
				gxMultiPostprocess_ToggleOnOff(gxMultiPostprocessEffect_Bloom);
			else if (inSys_WasPressed(m_inputEvents.m_toggleOldTV))
				gxMultiPostprocess_ToggleOnOff(gxMultiPostprocessEffect_OldTV);
			else if (inSys_WasPressed(m_inputEvents.m_toggleDepthOfFieldLQ))
			{
				if (gxMultiPostprocess_ToggleOnOff(gxMultiPostprocessEffect_DepthOfFieldLQ))
					gxMultiPostprocess_Disable(gxMultiPostprocessEffect_DepthOfFieldHQ);
			}
			else if (inSys_WasPressed(m_inputEvents.m_toggleDepthOfFieldHQ))
			{
				if (gxMultiPostprocess_ToggleOnOff(gxMultiPostprocessEffect_DepthOfFieldHQ))
					gxMultiPostprocess_Disable(gxMultiPostprocessEffect_DepthOfFieldLQ);
			}
			else if (inSys_WasPressed(m_inputEvents.m_toggleRainyGlass))
				gxMultiPostprocess_ToggleOnOff(gxMultiPostprocessEffect_RainyGlass);
			else if (inSys_WasPressed(m_inputEvents.m_toggleCameraMotionBlur))
				gxMultiPostprocess_ToggleOnOff(gxMultiPostprocessEffect_CameraMotionBlur);
			else if (inSys_WasPressed(m_inputEvents.m_toggleStereo))
			{
				m_enableStereo = !m_enableStereo;

				// Disable stereo incompatible post-processes

				if (m_enableStereo)
					gxMultiPostprocess_Disable(
						gxMultiPostprocessEffect_DepthOfFieldLQ |
						gxMultiPostprocessEffect_DepthOfFieldHQ |
						gxMultiPostprocessEffect_CameraMotionBlur);
			}
			else if (inSys_IsDown(m_inputEvents.m_increaseStereoEyeDist))
				m_stereoEyeOffset += dt * 0.01f;
			else if (inSys_IsDown(m_inputEvents.m_decreaseStereoEyeDist))
				m_stereoEyeOffset -= dt * 0.01f;
			else if (inSys_WasPressed(m_inputEvents.m_increaseMSAALevel))
			{
				const glCaps* caps = glDevice_GetCaps();
				if (m_MSAAModeIndex < caps->m_numMSAALevels - 1)
					m_MSAAModeIndex++;
			}
			else if (inSys_WasPressed(m_inputEvents.m_decreaseMSAALevel))
			{
				if (m_MSAAModeIndex > 0)
					m_MSAAModeIndex--;
			}
		}

		// Update scene rotation

		m_rotation = ueMod(m_rotation + dt * 0.01f, UE_2PI);

		// Update post-processes

		if (m_updatePostprocesses)
		{
			gxMultiPostprocessUpdateParams ppParams;
			ppParams.m_dt = dt;
			ppParams.m_view = &m_camera.GetView();
			ppParams.m_proj = &m_camera.GetProj();
			gxMultiPostprocess_Update(&ppParams);
		}
	}

	void Draw()
	{
		// Begin

		glCtx* ctx = glDevice_GetDefaultContext();
		if (!glCtx_Begin(ctx))
			return;

		// Draw depth (if needed)

		glTextureBuffer* depth =
			gxMultiPostprocess_NeedsDepth() ?
				DrawScene(ctx, UE_FALSE) :
				NULL;

		// Draw color

		glTextureBuffer* color =
			(gxMultiPostprocess_NeedsColor() || !gxMultiPostprocess_WouldDraw()) ?
				DrawScene(ctx, UE_TRUE) :
				NULL;

		// Draw post-processes

		if (gxMultiPostprocess_WouldDraw())
			color = DrawPostprocesses(ctx, color, depth);

		// End & present

		glCtx_End(ctx);

		glCtx_Present(ctx, color);
		glRenderBufferPool_ReleaseTextureBuffer(color);
	}

	glTextureBuffer* DrawScene(glCtx* ctx, ueBool drawColor = UE_TRUE)
	{
		UE_PROF_SCOPE("Draw Scene");

		// Set up render group

		glRenderGroup* group = NULL;
		if (gxMultiPostprocess_WouldDraw())
		{
			const glRenderGroupDesc* mainGroupDesc = glRenderGroup_GetDesc(glDevice_GetMainRenderGroup());

			glRenderGroupDesc desc;
			desc.m_width = mainGroupDesc->m_width;
			desc.m_height = mainGroupDesc->m_height;
			desc.m_needColorContent = UE_TRUE;
			desc.m_colorDesc[0].m_format = drawColor ? glBufferFormat_Native_R8G8B8A8 : glBufferFormat_R32F;
			desc.m_hasDepthStencil = UE_TRUE;
			desc.m_depthStencilDesc.m_format = glBufferFormat_Depth;
			if (drawColor)
			{
				const glCaps* caps = glDevice_GetCaps();
				desc.m_MSAALevel = caps->m_MSAALevels[m_MSAAModeIndex];
			}

			group = glRenderGroup_Create(&desc);
			UE_ASSERT(group);

			m_camera.SetDrawingToTexture(UE_TRUE);
		}
		else
		{
			group = glDevice_GetMainRenderGroup();
			m_camera.SetDrawingToTexture(UE_FALSE);
		}

		// Begin

		glCtx_SetClearColor(ctx, 0, 0, 0, 0);
		glCtx_SetClearDepthStencil(ctx, 1.0f, 0);

		glRenderGroup_Begin(group, ctx);

		if (!drawColor) // Clear F32 depth
		{
			ueVec4 farZ;
			farZ.Set(m_camera.GetFarZ(), m_camera.GetFarZ(), m_camera.GetFarZ(), m_camera.GetFarZ());
			gxShapeDraw_DrawFullscreenRect(farZ, UE_FALSE);
		}

		// Draw

		if (!m_enableStereo || !drawColor)
			DrawObjects(ctx, m_camera.GetView(), drawColor);

		else // Stereo color
		{
			UE_PROF_SCOPE("Draw Stereo");

			const ueVec3& rightVec = m_camera.GetView().GetRightVec();

			ueMat44 view = m_camera.GetView();

			// Draw left eye

			ueVec3 offset = rightVec * -m_stereoEyeOffset;
			view.Translate(offset);

			glCtx_SetColorWrite(ctx, 0, m_leftEyeColorMask);
			DrawObjects(ctx, view, UE_TRUE);

			// Draw right eye

			offset = rightVec * 2.0f * m_stereoEyeOffset;
			view.Translate(offset);

			glCtx_SetColorWrite(ctx, 0, m_rightEyeColorMask);
			glCtx_Clear(ctx, glClearFlag_Depth | glClearFlag_Stencil);
			DrawObjects(ctx, view, UE_TRUE);

			glCtx_SetColorWrite(ctx, 0, glColorMask_All);
		}

		// Draw app overlay

		if (!gxMultiPostprocess_WouldDraw())
			g_app.DrawAppOverlay();

		// End

		glRenderGroup_EndDrawing(group);
		glTextureBuffer* result = glRenderGroup_AcquireColorOutput(group, 0);
		glRenderGroup_End(group);
		if (gxMultiPostprocess_WouldDraw())
			glRenderGroup_Destroy(group);

		return result;
	}

	void DrawObjects(glCtx* ctx, const ueMat44& view, ueBool drawColor)
	{
		UE_PROF_SCOPE("Draw Objects");

		glCtx_SetFillMode(ctx, glFillMode_Solid);
		glCtx_SetBlending(ctx, UE_FALSE);
		glCtx_SetCullMode(ctx, glCullMode_CCW);
		glCtx_SetDepthWrite(ctx, UE_TRUE);
		glCtx_SetDepthTest(ctx, UE_TRUE);

		if (drawColor)
			glCtx_SetSamplerConstant(ctx, gxCommonConstants::ColorMap, gxTexture_GetBuffer(*m_texture));
		else
			glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::View, &view);

		ueMat44 viewProj;
		ueMat44::Mul(viewProj, view, m_camera.GetProj());
		glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::ViewProj, &viewProj);

		glCtx_SetProgram(ctx, (drawColor ? m_colorProgram : m_depthProgram).GetProgram());

		const u32 lodIndex = 0;
		gxModelLOD* lod = gxModel_GetLOD(*m_model, lodIndex);

		glCtx_SetIndices(ctx, lod->m_IB);

		const f32 numModelsInRow = 5;
		const ueVec3 modelSpacing = ueVec3::One;
		const ueVec3 modelOffset = modelSpacing * numModelsInRow * 0.5f;
		
		const ueVec3 modelScale(0.15f, 0.15f, 0.15f);

		for (f32 x = 1; x <= numModelsInRow; x++)
			for (f32 y = 1; y <= numModelsInRow; y++)
				for (f32 z = 1; z <= numModelsInRow; z++)
				{
					ueVec3 translation(x, y, z);
					translation *= modelSpacing;
					translation -= modelOffset;

					ueMat44 transform;
					transform.SetScale(modelScale);
					transform.Translate(translation);
					transform.Rotate(ueVec3(0, 0, 1), m_rotation);
					transform.Rotate(ueVec3(1, 0, 1), m_rotation * 0.5f);
					glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::World, &transform);

					for (u32 i = 0; i < lod->m_numMeshes; i++)
					{
						gxModelMesh* mesh = &lod->m_meshes[i];
						glCtx_SetStream(ctx, 0, lod->m_VB, lod->m_vertexFormats[mesh->m_vertexFormatIndex], mesh->m_vertexBufferOffset);
						glCtx_DrawIndexed(ctx, (glPrimitive) mesh->m_primType, mesh->m_baseVertex, mesh->m_firstVertex, mesh->m_numVerts, mesh->m_firstIndex, mesh->m_numIndices);
					}
				}
	}

	glTextureBuffer* DrawPostprocesses(glCtx* ctx, glTextureBuffer* color, glTextureBuffer* depth)
	{
		gxMultiPostprocessDrawParams ppParams;

		ppParams.m_ctx = ctx;
		ppParams.m_group = glDevice_GetMainRenderGroup();
		ppParams.m_color = color;
		ppParams.m_depth = depth;

		ppParams.m_dofHQParams.m_nearZ = m_camera.GetNearZ();
		ppParams.m_dofHQParams.m_farZ = m_camera.GetFarZ();

		ppParams.m_cameraMotionBlurParams.m_nearZ = m_camera.GetNearZ();
		ppParams.m_cameraMotionBlurParams.m_farZ = m_camera.GetFarZ();

		// Draw

		gxMultiPostprocess_Draw(&ppParams);

		// Release unnecessary buffers

		if (depth)
			glRenderBufferPool_ReleaseTextureBuffer(depth);

		// Draw debug app overlay

		g_app.DrawAppOverlay();

		// End drawing

		glRenderGroup_EndDrawing(ppParams.m_group);
		color = glRenderGroup_AcquireColorOutput(ppParams.m_group, 0);
		glRenderGroup_End(ppParams.m_group);

		return color;
	}

	// Postprocessing

	ueBool m_updatePostprocesses;

	// Stereo

	ueBool m_enableStereo;
	f32 m_stereoEyeOffset;
	u32 m_leftEyeColorMask;
	u32 m_rightEyeColorMask;

	// MSAA

	u32 m_MSAAModeIndex;

	// Model

	gxProgram m_colorProgram;
	gxProgram m_depthProgram;

	ueResourceHandle<gxTexture> m_texture;
	ueResourceHandle<gxModel> m_model;

	// Scene

	f32 m_rotation;

	// Camera

	gxCamera m_camera;

	// Input

	inConsumerId m_inputConsumerId;
	struct InputEvents
	{
		inEventId m_toggleUpdate;

		inEventId m_toggleBloom;
		inEventId m_toggleCameraMotionBlur;
		inEventId m_toggleDepthOfFieldLQ;
		inEventId m_toggleDepthOfFieldHQ;
		inEventId m_toggleRainyGlass;
		inEventId m_toggleOldTV;

		inEventId m_toggleStereo;
		inEventId m_increaseStereoEyeDist;
		inEventId m_decreaseStereoEyeDist;

		inEventId m_increaseMSAALevel;
		inEventId m_decreaseMSAALevel;

	} m_inputEvents;
};

UE_DECLARE_SAMPLE(ueSample_Postprocessing, "Postprocessing")

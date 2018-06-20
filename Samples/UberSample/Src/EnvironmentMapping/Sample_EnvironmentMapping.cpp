#include "SampleApp.h"
#include "Base/ueMath.h"
#include "Input/inSys.h"
#include "GraphicsExt/gxCommonConstants.h"
#include "GraphicsExt/gxDebugModelRenderer.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxModel.h"
#include "GraphicsExt/gxCamera.h"
#include "GraphicsExt/gxTextRenderer.h"

/**
 *	Demonstrates environment mapping using cube map.
 */
class ueSample_EnvironmentMapping : public ueSample
{
	ueBool Init()
	{
		// Load resources

		m_model.SetByName("common/default_model");
		m_envMap.SetByName("env_mapping_sample/env_cubemap");

		m_program.Create("env_mapping_sample/env_model_vs", "env_mapping_sample/env_model_fs");

		// Initialize model rotation

		m_rotation = -UE_PI * 0.5f;

		// Create camera
		
		m_camera.SetLookAt(ueVec3(1.5f, 0.5f, 0), ueVec3::Zero);

		// Input

		m_inputConsumerId = inSys_RegisterConsumer("Environment Mapping sample", 0.0f);

		return UE_TRUE;
	}

	void Deinit()
	{
		inSys_UnregisterConsumer(m_inputConsumerId);
	}

	void DoFrame(f32 dt)
	{
		// Update camera

		if (inSys_IsConsumerActive(m_inputConsumerId))
		{
			m_camera.UpdateFreeCamera(dt);
			m_rotation = ueMod(m_rotation + dt * 0.1f, UE_2PI);
		}

		// Draw

		glCtx* ctx = g_app.BeginDrawing();
		if (!ctx)
			return;

		{
			UE_PROF_SCOPE("Draw Model");

			ueMat44 world;
			world.SetAxisRotation(ueVec3(0, 1, 0), m_rotation);
			ueMat44 worldView;
			ueMat44::Mul(worldView, world, m_camera.GetView());

			glCtx_SetSamplerConstant(ctx, gxCommonConstants::EnvMap, gxTexture_GetBuffer(*m_envMap), &glSamplerParams::DefaultClamp);
			glCtx_SetFloat4x4Constant(ctx, gxCommonConstants::WorldView, &worldView);

			gxDebugModelRendererDrawParams params;
			params.m_ctx = ctx;
			params.m_world = &world;
			params.m_viewProj = &m_camera.GetViewProj();
			params.m_model = *m_model;
			params.m_rigidProgram = m_program.GetProgram();

			gxDebugModelRenderer_Draw(&params);
		}

		// debug

		const ueMat44& m = m_camera.GetView();
		char buf[512];
		ueStrFormatS(buf,
			"MATRIX\n0: %.3f %.3f %.3f\n1: %.3f %.3f %.3f\n2: %.3f %.3f %.3f\n",
			m[0][0], m[0][1], m[0][2],
			m[1][0], m[1][1], m[1][2],
			m[2][0], m[2][1], m[2][2]);
		gxTextRenderer_Draw(ctx, 10, 10, buf);

		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

	gxCamera m_camera;

	gxProgram m_program;
	ueResourceHandle<gxTexture> m_envMap;

	ueResourceHandle<gxModel> m_model;
	f32 m_rotation;

	inConsumerId m_inputConsumerId;
};

UE_DECLARE_SAMPLE(ueSample_EnvironmentMapping, "EnvironmentMapping")

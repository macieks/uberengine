#include "SampleApp.h"
#include "Base/ueMath.h"
#include "Input/inSys.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxModel.h"
#include "GraphicsExt/gxAnimation.h"
#include "GraphicsExt/gxModelInstance.h"
#include "GraphicsExt/gxCamera.h"
#include "GraphicsExt/gxDebugModelRenderer.h"

/**
 *	Demonstrates 3D animated model with skeletal animation.
 */
class ueSample_Model : public ueSample
{
	ueBool Init()
	{
		// Load resources (sync load)

#if 1 // Soft skinned model
		m_model.SetByName("model_sample/test_model");
		m_animation.SetByName("model_sample/test_anim");
		m_texture.SetByName("model_sample/test_tex");
#else // Rigid skinned model
		m_model.SetByName("rts_sample/soldier");
		m_animation.SetByName("rts_sample/walk_anim");
		m_texture.SetByName("rts_sample/soldiermap0");
#endif

		m_model.SetByName("scenes/designer_home");
		m_texture.SetByName("common/default_texture");

		UE_ASSERT(m_model.IsReady() && m_texture.IsReady() && m_animation.IsReady());

		// Create model instance

		m_modelInstance = gxModel_CreateInstance(*m_model);
		gxModelInstance_PlayAnimation(m_modelInstance, *m_animation);

		// Create camera

		m_camera.SetLookAt(ueVec3(20, 40, 0), ueVec3(0, 20, 0));

		const f32 modelMaxDim = 1000;//ueVec3::Dist(m_model->m_box.m_min, m_model->m_box.m_max);
		m_camera.SetFarZ(ueMax(m_camera.GetFarZ(), modelMaxDim));

		// Input

		m_inputConsumerId = inSys_RegisterConsumer("Model sample", 0.0f);

		return UE_TRUE;
	}

	void Deinit()
	{
		inSys_UnregisterConsumer(m_inputConsumerId);
		gxModelInstance_Destroy(m_modelInstance);
	}

	void DoFrame(f32 dt)
	{
		// Update camera

		if (inSys_IsConsumerActive(m_inputConsumerId))
			m_camera.UpdateFreeCamera(dt);

		// Update model instance
		{
			UE_PROF_SCOPE("UpdateAnimations");
			gxModelInstance_Update(m_modelInstance, dt);
		}

		// Draw

		glCtx* ctx = g_app.BeginDrawing();
		if (!ctx)
			return;

		{
			UE_PROF_SCOPE("Draw Model & Skeleton");

			gxDebugModelRendererDrawParams params;
			params.m_ctx = ctx;
			params.m_viewProj = &m_camera.GetViewProj();
			params.m_modelInstance = m_modelInstance;
			params.m_drawModel = UE_TRUE;
			params.m_drawSkeleton = UE_TRUE;
			params.m_texture = gxTexture_GetBuffer(*m_texture);

			gxDebugModelRenderer_Draw(&params);
		}

		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

	gxCamera m_camera;

	gxModelInstance* m_modelInstance;

	ueResourceHandle<gxTexture> m_texture;
	ueResourceHandle<gxModel> m_model;
	ueResourceHandle<gxAnimation> m_animation;

	inConsumerId m_inputConsumerId;
};

UE_DECLARE_SAMPLE(ueSample_Model, "Model")

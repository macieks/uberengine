#include "GraphicsExt/gxPostprocesses.h"
#include "GraphicsExt/gxProgram.h"
#include "GraphicsExt/gxTexture.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxCommonConstants.h"
#include "Base/ueRand.h"

struct gxPostprocess_RainyGlassData
{
	gxProgram m_evaporationProgram;
	gxProgram m_distortionProgram;

	ueResourceHandle<gxTexture> m_dropletTexture;

	struct ShaderConstants
	{
		glConstantHandle MaxEvaporation;
		glConstantHandle DistortionScale;
	} m_shaderConstants;
};

static gxPostprocess_RainyGlassData s_data;

void gxPostprocess_RainyGlass::Startup(StartupParams* params)
{
	s_data.m_evaporationProgram.Create(params->m_defaultVSName, params->m_evaporationFSName);
	s_data.m_distortionProgram.Create(params->m_defaultVSName, params->m_distortionFSName);

	s_data.m_dropletTexture.SetByName(params->m_dropletTextureName);

	s_data.m_shaderConstants.MaxEvaporation.Init("MaxEvaporation", glConstantType_Float);
	s_data.m_shaderConstants.DistortionScale.Init("DistortionScale", glConstantType_Float);
}

void gxPostprocess_RainyGlass::Shutdown()
{
	s_data.m_evaporationProgram.Destroy();
	s_data.m_distortionProgram.Destroy();
	s_data.m_dropletTexture = NULL;
}

gxPostprocess_RainyGlass::gxPostprocess_RainyGlass() :
	m_dropletBuffer(NULL),
	m_dtAccumulator(0.0f),
	m_evaporationAccumulator(0.0f)
{
}

gxPostprocess_RainyGlass::~gxPostprocess_RainyGlass()
{
	Deinit();
}

void gxPostprocess_RainyGlass::Init(InitParams* params)
{
	UE_ASSERT(!m_initParams.m_allocator);
	m_initParams = *params;

	m_numDroplets = 0;
	m_droplets = (Droplet*) m_initParams.m_allocator->Alloc(m_initParams.m_maxDroplets * sizeof(Droplet));
	UE_ASSERT(m_droplets);
}

void gxPostprocess_RainyGlass::Deinit()
{
	if (m_dropletBuffer)
	{
		glTextureBuffer_Destroy(m_dropletBuffer);
		m_dropletBuffer = NULL;
	}

	if (m_initParams.m_allocator)
	{
		m_initParams.m_allocator->Free(m_droplets);
		m_initParams.m_allocator = NULL;
	}
}

void gxPostprocess_RainyGlass::SetUpdateParams(UpdateParams* params)
{
	m_updateParams = *params;
	m_updateParams.m_dir.Normalize();
}

void gxPostprocess_RainyGlass::Update(f32 dt)
{
	m_dtAccumulator += dt;
	m_evaporationAccumulator += dt;
}

void gxPostprocess_RainyGlass::Draw(DrawParams* params)
{
	UE_PROF_SCOPE("post-process rainy glass");

	// Update droplet buffer (and create if not created before)
	{
		UE_PROF_SCOPE("update droplet buffer");

		// Begin

		glRenderGroupDesc desc;
		desc.m_width = m_initParams.m_rainBufferWidth;
		desc.m_height = m_initParams.m_rainBufferHeight;
		desc.m_colorDesc[0].m_format = glBufferFormat_Native_R8G8B8A8;
		desc.m_needColorContent = UE_TRUE;

		glRenderGroup* group = glRenderGroup_Create(&desc);
		UE_ASSERT(group);

		glCtx_SetClearColor(params->m_ctx, 0.5f, 0.5f, 1.0f, 0.0f); // Clear to normal pointing towards camera + alpha of 0

		// Update droplet buffer in steps

		u32 numIterationsLeft = m_updateParams.m_maxUpdateIterations;
		while (numIterationsLeft-- > 0 && m_dtAccumulator >= m_updateParams.m_minUpdateDT)
		{
			UE_PROF_SCOPE("update step");

			glRenderGroup_Begin(group, params->m_ctx, glClearFlag_Color);

			const f32 dt = ueMin(m_dtAccumulator, m_updateParams.m_maxUpdateDT);
			m_dtAccumulator -= dt;

			// Evaporate (or just copy)

			if (m_dropletBuffer)
			{
				gxShape_FullscreenRect rect;
				rect.m_colorMap = m_dropletBuffer;
				rect.m_colorMapSampler = &glSamplerParams::DefaultPP;

				if (m_evaporationAccumulator >= 1.0f / 60.0f || 1)
				{
					m_evaporationAccumulator -= 1.0f / 60.0f;

					UE_PROF_SCOPE("evaporate");

					const f32 maxEvaporation = 1.0f / 255.0f;
					glCtx_SetFloatConstant(params->m_ctx, s_data.m_shaderConstants.MaxEvaporation, &maxEvaporation);

					rect.m_program = s_data.m_evaporationProgram.GetProgram();
					gxShapeDraw_DrawFullscreenRect(rect);
				}
				else
				{
					UE_PROF_SCOPE("copy droplet buffer");

					gxShapeDraw_DrawFullscreenRect(rect);
				}

				glRenderBufferPool_ReleaseTextureBuffer(m_dropletBuffer);
			}

			// Simulate

			SimulateDroplets(dt);

			// Draw new droplets
			{
				UE_PROF_SCOPE("draw droplets");

				// FIXME: Do not use gxShapeDraw here (inefficient!)

				gxShapeDrawParams shapeDrawParams;
				shapeDrawParams.m_2DCanvas.Set(0, 0, 1, 1);
				gxShapeDraw_SetDrawParams(&shapeDrawParams);
				gxShapeDraw_Begin(params->m_ctx);

				gxShape_TexturedRect rect;
				rect.m_enableBlending = UE_TRUE;
				rect.m_colorMap = gxTexture_GetBuffer(*s_data.m_dropletTexture);
				rect.m_colorMapSampler = &glSamplerParams::DefaultClamp;

				for (u32 i = 0; i < m_numDroplets; i++)
				{
					const Droplet& d = m_droplets[i];
					rect.m_pos.m_left = d.m_pos[0] - d.m_size;
					rect.m_pos.m_right = d.m_pos[0] + d.m_size;
					rect.m_pos.m_top = d.m_pos[1] - d.m_size;
					rect.m_pos.m_bottom = d.m_pos[1] + d.m_size;
					gxShapeDraw_DrawTexturedRect(rect);
				}

				gxShapeDraw_End();
			}

			// End

			glRenderGroup_EndDrawing(group);
			m_dropletBuffer = glRenderGroup_AcquireColorOutput(group, 0);
			glRenderGroup_End(group);
		}

		glRenderGroup_Destroy(group);

		glCtx_SetClearColor(params->m_ctx, 0, 0, 0, 0);
	}

	// Main pass
	{
		UE_PROF_SCOPE("distort scene");

		// Begin

		UE_ASSERT(params->m_srcColor);
		const glTextureBufferDesc* srcDesc = glTextureBuffer_GetDesc(params->m_srcColor);

		glRenderGroup* group = params->m_group;
		if (!group)
		{
			glRenderGroupDesc desc;
			desc.m_width = srcDesc->m_width;
			desc.m_height = srcDesc->m_height;
			desc.m_colorDesc[0].m_format = srcDesc->m_format;
			desc.m_needColorContent = UE_TRUE;

			group = glRenderGroup_Create(&desc);
		}

		glRenderGroup_Begin(group, params->m_ctx, 0);

		// Draw

		const f32 distortionScale = 0.02f;
		glCtx_SetFloatConstant(params->m_ctx, s_data.m_shaderConstants.DistortionScale, &distortionScale);

		glCtx_SetSamplerConstant(params->m_ctx, gxCommonConstants::NormalMap, m_dropletBuffer, &glSamplerParams::DefaultClamp);
		glCtx_SetFloat4Constant(params->m_ctx, gxCommonConstants::Color, &m_initParams.m_dropletColor);

		gxShape_FullscreenRect rect;
		rect.m_program = s_data.m_distortionProgram.GetProgram();
		rect.m_colorMap = params->m_srcColor;
		rect.m_colorMapSampler = &glSamplerParams::DefaultClamp;
		gxShapeDraw_DrawFullscreenRect(rect);

		// End

		if (!params->m_group)
		{
			glRenderGroup_EndDrawing(group);
			params->m_dstColor = glRenderGroup_AcquireColorOutput(group, 0);
			glRenderGroup_End(group);
			glRenderGroup_Destroy(group);
		}
	}
}

void gxPostprocess_RainyGlass::SimulateDroplets(f32 dt)
{
	UE_PROF_SCOPE("simulate droplets");

	const f32 newDirPercentage = 0.2f;
	const f32 minNextChangeTime = 0.05f;
	const f32 maxNextChangeTime = 0.5f;

	// Kill old droplets

	for (u32 i = 0; i < m_numDroplets;)
		if (m_droplets[i].m_pos[0] > 1.0f + m_droplets[i].m_size * 0.5f)
			m_droplets[i] = m_droplets[--m_numDroplets];
		else
			i++;

	// Spawn new droplets

	m_spawnCountAccumulator += dt * m_updateParams.m_spawnFreq;

	while (m_spawnCountAccumulator >= 1.0f)
	{
		if (m_numDroplets == m_initParams.m_maxDroplets)
		{
			m_spawnCountAccumulator = 0.0f;
			break;
		}

		Droplet& d = m_droplets[m_numDroplets++];
		d.m_size = ueRandG_F32(m_updateParams.m_minDropletSize, m_updateParams.m_maxDropletSize);
		d.m_pos.Set(ueRandG_F32(1.0f), -d.m_size);

		d.m_velocity = m_updateParams.m_dir;
		d.m_velocity *= ueRandG_F32(m_updateParams.m_minSpeed, m_updateParams.m_maxSpeed) * (d.m_size / m_updateParams.m_maxDropletSize);
		d.m_timeLeftToVelocityChange = ueRandG_F32(minNextChangeTime, maxNextChangeTime);

		m_spawnCountAccumulator -= 1.0f;
	}

#if 0
	const ueFloat32 mergeDistSqr = ueSqr(0.05f);

	// Merge droplets
	for (ueInt i = 0; i < m_droplets.Size() - 1; i++)
		for (ueInt j = i + 1; j < m_droplets.Size();)
			if (ueVec2::DotF(m_droplets[i].m_pos, m_droplets[j].m_pos) < mergeDistSqr)
			{
				Droplet& a = m_droplets[i];
				Droplet& b = m_droplets[j];

				const ueFloat scale = b.m_size / (a.m_size + b.m_size);

				a.m_velocity = ueVec2::Lerp(a.m_velocity, b.m_velocity, scale);
				a.m_pos = ueVec2::Lerp(a.m_pos, b.m_pos, scale);
				a.m_size = ueMin(dropletMaxSize, a.m_size + b.m_size);

				m_droplets[j] = m_droplets.GetLast();
				m_droplets.PopBack();
			}
			else
				j++;
#endif

	// Update existing droplets

	for (u32 i = 0; i < m_numDroplets; i++)
	{
		Droplet& d = m_droplets[i];
		d.m_timeLeftToVelocityChange -= dt;
		if (d.m_timeLeftToVelocityChange <= 0.0f)
		{
			ueVec2 newDir(ueRandG_F32(-1.0f, 1.0f), ueRandG_F32(0.5f, 1.0f));
			newDir.Normalize();

			d.m_velocity.Normalize();
			ueVec2::Lerp(d.m_velocity, d.m_velocity, newDir, newDirPercentage);
			d.m_velocity.Normalize();
			d.m_velocity *= ueRandG_F32(m_updateParams.m_minSpeed, m_updateParams.m_maxSpeed) * (d.m_size / m_updateParams.m_maxDropletSize);
			d.m_timeLeftToVelocityChange = ueRandG_F32(minNextChangeTime, maxNextChangeTime);
		}
		d.m_pos = d.m_velocity * dt + d.m_pos;
	}
}

#if defined(GL_D3D9) && 0

void gxPostprocess_RainyGlass::OnDeviceReset()
{
	if (m_dropletBuffer)
	{
		glRenderBufferPool_ReleaseTextureBuffer(m_dropletBuffer);
		m_dropletBuffer = NULL;
	}
}

#endif
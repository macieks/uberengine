#ifndef GX_POSTPROCESSES_H
#define GX_POSTPROCESSES_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "Graphics/glLib.h"

// Downscale 2x2

class gxPostprocess_Down2x2
{
public:
	struct StartupParams
	{
		const char* m_VSName;
		const char* m_FSName;

		StartupParams() :
			m_VSName("postprocesses/pp_down2x2_vs"),
			m_FSName("postprocesses/pp_down2x2_fs")
		{}
	};

	struct DrawParams
	{
		enum Type
		{
			Type_2x2Nearest = 0,
			Type_Linear,

			Type_MAX
		};

		glCtx* m_ctx;					//!< Context to draw to

		glRenderGroup* m_group;			//!< Optional render group to use
		glTextureBuffer* m_srcColor;	//!< Source color texture
		glTextureBuffer* m_dstColor;	//!< Destination color buffer

		Type m_type;

		DrawParams() :
			m_ctx(NULL),
			m_group(NULL),
			m_srcColor(NULL),
			m_dstColor(NULL),
			m_type(Type_2x2Nearest)
		{}
	};

	static void Startup(StartupParams* params);
	static void Shutdown();

	static void Draw(DrawParams* params);
};

// Blur

class gxPostprocess_Blur
{
public:
	struct StartupParams
	{
		const char* m_verticalBlurVSName;
		const char* m_verticalBlurFSName;

		const char* m_horizontalBlurVSName;
		const char* m_horizontalBlurFSName;

		const char* m_radialBlurVSName;
		const char* m_radialBlurFSName;

		StartupParams() :
			m_verticalBlurVSName("postprocesses/pp_default_vs"),
			m_verticalBlurFSName("postprocesses/pp_vertical_blur_fs"),
			m_horizontalBlurVSName("postprocesses/pp_default_vs"),
			m_horizontalBlurFSName("postprocesses/pp_horizontal_blur_fs"),
			m_radialBlurVSName("postprocesses/pp_radial_blur_vs"),
			m_radialBlurFSName("postprocesses/pp_radial_blur_fs")
		{}
	};

	struct DrawParams
	{
		enum Type
		{
			Type_Vertical = 0,
			Type_Horizontal,
			Type_Radial,

			Type_MAX
		};

		glCtx* m_ctx;					//!< Context to draw to

		glRenderGroup* m_group;			//!< Optional render group to use
		glTextureBuffer* m_srcColor;	//!< Source color texture
		glTextureBuffer* m_dstColor;	//!< Destination color buffer

		Type m_type;

		f32 m_blurKernel; //!< Measured in src texture pixels
		ueVec2 m_radialBlurCenter;

		DrawParams() :
			m_ctx(NULL),
			m_group(NULL),
			m_srcColor(NULL),
			m_dstColor(NULL),
			m_type(Type_Vertical),
			m_blurKernel(1.0f),
			m_radialBlurCenter(0.5f, 0.5f)
		{}
	};

	static void Startup(StartupParams* params);
	static void Shutdown();

	static void Draw(DrawParams* params);
};

// Bloom

class gxPostprocess_Bloom
{
public:
	struct StartupParams
	{
		const char* m_bloomCompositeVSName;
		const char* m_bloomCompositeFSName;

		StartupParams() :
			m_bloomCompositeVSName("postprocesses/pp_default_vs"),
			m_bloomCompositeFSName("postprocesses/pp_bloom_composite_fs")
		{}
	};

	struct DrawParams
	{
		glCtx* m_ctx;					//!< Context to draw to

		glRenderGroup* m_group;			//!< Optional render group to use
		ueRect* m_dstRect;				//!< Destination rectangle to draw to

		glTextureBuffer* m_srcColor;	//!< Source color texture
		glSamplerParams* m_srcColorSampler; //!< Source color sampler paramsters
		glTextureBuffer* m_srcBlur;		//!< Source blur texture
		glTextureBuffer* m_dstColor;	//!< Destination color buffer

		u32 m_numHalfsizeDownscalesForBlur; //!< Number of half-size downscale steps (each halving width & height) to be done for bluring
		u32 m_numBlurIterations;		//!< Number of vertical & horizontal blurs
		f32 m_blurKernel;				//!< Measured in src texture pixels

		f32 m_blendFactor;				//!< Value within 0..1 used to blend source color with blurred downscaled color (0 - only source color; 1 - only blurred downscaled source color)

		ueBool m_getBlurBuffer;			//!< Indicates whether to get calculated blur buffer
		glTextureBuffer* m_outBlurBuffer;//!< Will be set if m_getBlurBuffer is set

		DrawParams() :
			m_ctx(NULL),
			m_group(NULL),
			m_dstRect(NULL),
			m_srcColor(NULL),
			m_srcColorSampler(&glSamplerParams::DefaultPP),
			m_srcBlur(NULL),
			m_dstColor(NULL),
			m_numHalfsizeDownscalesForBlur(2),
			m_numBlurIterations(1),
			m_blurKernel(1.0f),
			m_blendFactor(0.4f),
			m_getBlurBuffer(UE_FALSE),
			m_outBlurBuffer(NULL)
		{}
	};

	static void Startup(StartupParams* params);
	static void Shutdown();

	static void Draw(DrawParams* params);
};

// Old TV

class gxPostprocess_OldTV
{
public:
	struct StartupParams
	{
		const char* m_VSName;
		const char* m_FSName;

		const char* m_dustMapName;
		const char* m_lineMapName;
		const char* m_tvAndNoiseMapName;

		StartupParams() :
			m_VSName("postprocesses/pp_old_tv_vs"),
			m_FSName("postprocesses/pp_old_tv_fs"),
			m_dustMapName("postprocesses/pp_old_tv_dust_map"),
			m_lineMapName("postprocesses/pp_old_tv_line_map"),
			m_tvAndNoiseMapName("postprocesses/pp_old_tv_tv_and_noise_map")
		{}
	};

	struct DrawParams
	{
		glCtx* m_ctx;					//!< Context to draw to

		glRenderGroup* m_group;			//!< Optional render group to use
		glTextureBuffer* m_srcColor;	//!< Source color texture
		glTextureBuffer* m_dstColor;	//!< Destination color buffer

		f32 m_overExposureAmount;
		f32 m_dustAmount;
		f32 m_frameJitterFrequency;
		f32 m_maxFrameJitter;
		ueVec4 m_filmColor;
		f32 m_grainThicknes;
		f32 m_grainAmount;
		f32 m_scratchesAmount;
		f32 m_scratchesLevel;

		f32 m_time;

		DrawParams() :
			m_ctx(NULL),
			m_group(NULL),
			m_srcColor(NULL),
			m_dstColor(NULL),
			m_overExposureAmount(0.1f),
			m_dustAmount(4.0f),
			m_frameJitterFrequency(3.0f),
			m_maxFrameJitter(1.4f),
			m_grainThicknes(1.0f),
			m_grainAmount(0.8f),
			m_scratchesAmount(3.0f),
			m_scratchesLevel(0.7f),
			m_time(0.0f),
			m_filmColor(1.0f, 0.7559052f, 0.58474624f, 1.0f)
		{}
	};

	static void Startup(StartupParams* params);
	static void Shutdown();

	static void Draw(DrawParams* params);
};

// Rainy Glass

class gxPostprocess_RainyGlass
{
public:
	struct StartupParams
	{
		const char* m_defaultVSName;
		const char* m_evaporationFSName;
		const char* m_distortionFSName;

		const char* m_dropletTextureName;

		StartupParams() :
			m_defaultVSName("postprocesses/pp_default_vs"),
			m_evaporationFSName("postprocesses/pp_rainy_glass_evaporation_fs"),
			m_distortionFSName("postprocesses/pp_rainy_glass_distortion_fs"),
			m_dropletTextureName("postprocesses/pp_rainy_glass_droplet")
		{}
	};

	struct InitParams
	{
		ueAllocator* m_allocator;
		u32 m_rainBufferWidth;
		u32 m_rainBufferHeight;
		u32 m_maxDroplets;
		ueVec4 m_dropletColor;

		InitParams() :
			m_allocator(NULL),
			m_rainBufferWidth(1280 / 4),
			m_rainBufferHeight(720 / 4),
			m_maxDroplets(64)
		{
			m_dropletColor.Set(0.4f, 0.1f, 0.7f, 1.0f);
			m_dropletColor *= 0.35f;
		}
	};

	struct UpdateParams
	{
		f32 m_spawnFreq;		//!< Droplet spawn frequency (droplets per second)
		ueVec2 m_dir;			//!< Rain direction (will be normalized)
		f32 m_minSpeed;			//!< Measured in rain texture height per second
		f32 m_maxSpeed;			//!< Measured in rain texture height per second
		f32 m_minDropletSize;	//!< Measured in rain texture heights
		f32 m_maxDropletSize;	//!< Measured in rain texture heights
		f32 m_minUpdateDT;		//!< Min. update delta time of the rain texture (per single update step)
		f32 m_maxUpdateDT;		//!< Max. update delta time of the rain texture (per single update step)
		u32 m_maxUpdateIterations; //!< Max. number of update iterations per Draw() call
		f32 m_evaporationSpeed;	//!< Droplet evaporation speed

		UpdateParams() :
			m_spawnFreq(10.0f),
			m_minSpeed(0.3f),
			m_maxSpeed(0.5f),
			m_minDropletSize(0.01f),
			m_maxDropletSize(0.02f),
			m_minUpdateDT(1.0f / 120.0f),
			m_maxUpdateDT(1.0f / 120.0f),
			m_maxUpdateIterations(2),
			m_evaporationSpeed(0.5f),
			m_dir(-0.3f, 1.0f)
		{}
	};

	struct DrawParams
	{
		glCtx* m_ctx;					//!< Context to draw to

		glRenderGroup* m_group;			//!< Optional render group to use
		glTextureBuffer* m_srcColor;	//!< Source color texture
		glTextureBuffer* m_dstColor;	//!< Destination color buffer

		DrawParams() :
			m_ctx(NULL),
			m_group(NULL),
			m_srcColor(NULL),
			m_dstColor(NULL)
		{}
	};

	static void Startup(StartupParams* params);
	static void Shutdown();

	gxPostprocess_RainyGlass();
	~gxPostprocess_RainyGlass();
	void Init(InitParams* params);
	void Deinit();
	void SetUpdateParams(UpdateParams* params);
	void Update(f32 dt);
	void Draw(DrawParams* params);

private:
	void SimulateDroplets(f32 dt);

	InitParams m_initParams;
	UpdateParams m_updateParams;

	glTextureBuffer* m_dropletBuffer;

	struct Droplet
	{
		f32 m_size;
		f32 m_timeLeftToVelocityChange;
		ueVec2 m_pos;
		ueVec2 m_velocity;
	};

	u32 m_numDroplets;
	Droplet* m_droplets;

	f32 m_spawnCountAccumulator;
	f32 m_evaporationAccumulator;

	f32 m_dtAccumulator;
};

// Depth Of Field

class gxPostprocess_DepthOfField_Base
{
public:
	struct DrawParams
	{
		glCtx* m_ctx;					//!< Context to draw to

		glRenderGroup* m_group;			//!< Optional render group to use
		glTextureBuffer* m_srcColor;	//!< Source color texture
		glTextureBuffer* m_srcDepth;	//!< Source depth texture
		glTextureBuffer* m_dstColor;	//!< Destination color buffer

		f32 m_nearStart;
		f32 m_nearEnd;
		f32 m_farStart;
		f32 m_farEnd;

		DrawParams() :
			m_ctx(NULL),
			m_group(NULL),
			m_srcColor(NULL),
			m_srcDepth(NULL),
			m_dstColor(NULL),
			m_nearStart(0.5f),
			m_nearEnd(1.0f),
			m_farStart(2.0f),
			m_farEnd(3.0f)
		{}
	};
};

class gxPostprocess_DepthOfField_LowQuality : gxPostprocess_DepthOfField_Base
{
public:
	struct StartupParams
	{
		const char* m_VSName;
		const char* m_FSName;

		StartupParams() :
			m_VSName("postprocesses/pp_default_vs"),
			m_FSName("postprocesses/pp_depth_of_field_fs")
		{}
	};

	struct DrawParams : gxPostprocess_DepthOfField_Base::DrawParams
	{
		glTextureBuffer* m_srcBlur;		//!< Source blur texture

		DrawParams() :
			m_srcBlur(NULL)
		{}
	};

	static void Startup(StartupParams* params);
	static void Shutdown();

	static void Draw(DrawParams* params);
};

class gxPostprocess_DepthOfField_HighQuality : gxPostprocess_DepthOfField_Base
{
public:
	struct StartupParams
	{
		const char* m_downsampleAndCalcCOCVSName;
		const char* m_downsampleAndCalcCOCFSName;

		const char* m_nearCOCVSName;
		const char* m_nearCOCFSName;

		const char* m_smallBlurVSName;
		const char* m_smallBlurFSName;

		const char* m_compositeVSName;
		const char* m_compositeFSName;

		StartupParams() :
			m_downsampleAndCalcCOCVSName("postprocesses/pp_depth_of_field_hq_downsample_vs"),
			m_downsampleAndCalcCOCFSName("postprocesses/pp_depth_of_field_hq_downsample_fs"),
			m_nearCOCVSName("postprocesses/pp_default_vs"),
			m_nearCOCFSName("postprocesses/pp_depth_of_field_hq_near_coc_fs"),
			m_smallBlurVSName("postprocesses/pp_depth_of_field_hq_small_blur_vs"),
			m_smallBlurFSName("postprocesses/pp_depth_of_field_hq_small_blur_fs"),
			m_compositeVSName("postprocesses/pp_default_vs"),
			m_compositeFSName("postprocesses/pp_depth_of_field_hq_composite_fs")
		{}
	};

	struct DrawParams : gxPostprocess_DepthOfField_Base::DrawParams
	{
		f32 m_farBlurRadius;
		f32 m_nearBlurRadius;

		f32 m_nearZ;
		f32 m_farZ;

		DrawParams() :
			m_farBlurRadius(0.8f),
			m_nearBlurRadius(1.6f),
			m_nearZ(0.1f),
			m_farZ(1000.0f)
		{}
	};

	static void Startup(StartupParams* params);
	static void Shutdown();

	static void Draw(DrawParams* params);
};

// Camera Motion Blur

class gxPostprocess_CameraMotionBlur
{
public:
	struct StartupParams
	{
		const char* m_VSName;
		const char* m_FSName;

		StartupParams() :
			m_VSName("postprocesses/pp_default_vs"),
			m_FSName("postprocesses/pp_camera_motion_blur_fs")
		{}
	};

	struct DrawParams
	{
		glCtx* m_ctx;					//!< Context to draw to

		glRenderGroup* m_group;			//!< Optional render group to use
		glTextureBuffer* m_srcColor;	//!< Source color texture
		glTextureBuffer* m_srcDepth;	//!< Source depth texture
		glTextureBuffer* m_dstColor;	//!< Destination color buffer

		f32 m_maxMotionBlur;

		f32 m_nearZ;
		f32 m_farZ;

		const ueMat44* m_viewProjInv;
		const ueMat44* m_prevViewProj;

		DrawParams() :
			m_ctx(NULL),
			m_group(NULL),
			m_srcColor(NULL),
			m_dstColor(NULL),
			m_maxMotionBlur(0.1f),
			m_nearZ(0.01f),
			m_farZ(1000.0f),
			m_viewProjInv(NULL),
			m_prevViewProj(NULL)
		{}
	};

	static void Startup(StartupParams* params);
	static void Shutdown();

	static void Draw(DrawParams* params);
};

// @}

#endif // GX_POSTPROCESSES_H

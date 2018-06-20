#ifndef GX_SHADOW_MAPPER_H
#define GX_SHADOW_MAPPER_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "Graphics/glLib.h"

//!	Shadow mapping helper with support for multiple popular shadow mappign techniques.
class gxShadowMapper
{
public:
	static const u32 MAX_SHADOW_FRUSTUM_SPLITS = 8;

	//! Available light projection modes
	enum LightProjection
	{
		LightProjection_Directional = 0,		//!< Directional light
		LightProjection_Spot,					//!< Spot light
		LightProjection_OmniCube,				//!< Omni light (rendering to cube)
		LightProjection_OmniDualParaboloidal,	//!< Omni light (rendering to 2 textures using dual-paraboloidal projection)

		LightProjection_MAX
	};

	//! Available shadow warp modes
	enum WarpMethod
	{
		WarpMethod_Standard = 0,			//!< Standard; no perspective modifications
		WarpMethod_Perspective,				//!< Perspective shadow mapping
		WarpMethod_LightSpacePerspective,	//!< Light space perspective shadow mapping
		WarpMethod_Trapezoidal,				//!< Trapezoidal shadow mapping

		WarpMethod_MAX
	};

	//! Available orders of the key rendering operations
	enum DrawOrder
	{
		DrawOrder_AllShadowMapsFirst_SingleShadowMaskPass = 0,	//!< Draw all shadow maps first to n buffers; then do separate shadow mask pass per shadow map
		DrawOrder_AllShadowMapsFirst_ShadowMaskPassPerSplit,	//!< Draw all shadow maps first to n buffers; then do the single shadow mask pass (sampling from n shadow maps simultaneously)
		DrawOrder_FullPassPerSplit,								//!< Draw consecutive shadow maps and after each one do the shadow mask pass; this method requires only a single shadow map buffer as opposed to other methods which require n buffers

		DrawOrder_MAX
	};

	//! Information passed to user supplied callback responsible for rendering scene depth
	struct DrawDepthFuncData
	{
		gxShadowMapper* m_shadowMapper;	//!< Shadow mapper
		ueFrustum m_frustum;			//!< Frustum
		ueMat44 m_view;					//!< View matrix
		ueMat44 m_proj;					//!< Projection matrix
		glCullMode m_cullMode;			//!< Cull mode that should be used for rendering the scene depth

		DrawDepthFuncData() :
			m_shadowMapper(NULL)
		{}
	};

	//! Information passed to user supplied callback responsible for rendering scene shadow (for selected splits)
	struct DrawShadowFuncData
	{
		gxShadowMapper* m_shadowMapper;	//!< Shadow mapper
		ueFrustum m_frustum;			//!< Frustum
		ueMat44 m_cameraView;			//!< Camera view matrix
		ueMat44 m_cameraProj;			//!< Camera projection matrix
		glViewportDesc m_viewport;		//!< Viewport

		ueBool m_doSinglePassForAllShadowMaps;	//!< Indicates whether to sample all shadow maps at once or just one (the first in m_shadowMaps array)
		ueMat44 m_cameraLightTransformations[MAX_SHADOW_FRUSTUM_SPLITS];	//!< Light transforms for all splits
		f32 m_splittingDistances[MAX_SHADOW_FRUSTUM_SPLITS + 1];			//!< Splitting distances
		glTextureBuffer* m_shadowMaps[MAX_SHADOW_FRUSTUM_SPLITS];			//!< Shadow maps for all splits

		glTextureBuffer* m_sceneDepthTexture; //!< Optional scene depth texture to be used

		DrawShadowFuncData() :
			m_shadowMapper(NULL),
			m_sceneDepthTexture(NULL)
		{}
	};

	//! Draw depth callback
	typedef void (*DrawDepthFunc)(DrawDepthFuncData& data);
	//! Draw shadow (shadow mask) callback
	typedef void (*DrawShadowFunc)(DrawShadowFuncData& data);

	//! Description of the light source
	struct LightDesc
	{
		LightProjection m_lightProjection;	//!< Desired light projection type (being effectively light type as well)
		ueVec3 m_position;					//!< For point and spot light - light sphere center; unused by directional lights
		ueVec3 m_direction;					//!< Light direction; unused by point light
		f32 m_radius;						//!< For point and spot light - light sphere radius; for directional light - distance to consider from reference position

		f32 m_spotFOV;						//!< Spot light field of view
		f32 m_spotAspect;					//!< Spot light aspect ratio
		f32 m_spotRotation;					//!< Spot light rotation
	};

	//! Description of the perspective camera
	struct CameraDesc
	{
		f32 m_aspect;		//!< Camera aspect ratio
		f32 m_FOV;			//!< Camera field of view
		f32 m_nearPlane;	//!< Camera near plane
		f32 m_farPlane;		//!< Camera far plane
		ueMat44 m_view;		//!< Camera view matrix
	};

	//! Description of desired shadow method to be used
	struct ShadowMethodDesc
	{
		WarpMethod m_warpMethod;			//!< Shadow warping method
		ueBool m_enableVarianceShadows;		//!< Indicates whether to enable variance shadow mapping (requires rendering both front and back faces)

		u32 m_numShadowSplits;				//!< Desired number of shadow map splits; for omni directional lights must be 1; can't exceed MAX_SHADOW_FRUSTUM_SPLITS
		ueBool m_enableAutomaticSplitting;	//!< Indicates whether to enable automatic calculation of splitting distances
		f32 m_automaticSplitLambda;			//!< Lambda value in the range from 0 to 1 used to determine splitting distances
		f32 m_splitDistances[MAX_SHADOW_FRUSTUM_SPLITS + 1]; //!< Splitting distances

		f32 m_fadeOutDistance;				//!< Distance over which the shadow fades out (into no shadow)
		ueBool m_fadeOutBehind;				//!< Indicates whether to apply distant shadow fade out
		ueColor32 m_fadeOutColor;			//!< Color into which the shadow fades out

		DrawOrder m_drawOrder;				//!< An order of key rendering operations

		ueBool m_useSeparateDepthBiasPerSplit;		//!< Indicates whether to use separate depth bias per shadow split; otherwise first depth bias from an array is used for all splits
		f32 m_depthBias[MAX_SHADOW_FRUSTUM_SPLITS];	//!< An array of depth biases for each shadow split; if m_useSeparateDepthBiasPerSplit is not set, then the first bias is used for all splits

		u32 m_shadowMapSize;						//!< Width and height of the shadow map to be used
		ueBool m_packShadowMapsIntoSingleTexture;	//!< Whether to pack all shadow maps into single texture; packing will be done automatically and viewport passed to used via function to draw shadow map will be set appropriately

		u32 m_shadowMaskWidth;				//!< Width of the shadow mask
		u32 m_shadowMaskHeight;				//!< Height of the shadow mask

		ueBool m_sampleSceneDepth;			//!< Indicates whether to sample scene depth rather than render objects; if scene depth texture is supplied it will be used, otherwise depth texture will be generated

		ueBool m_hintUseDepthTexture;		//!< Use depth (shadow) textures if available on hardware (e.g. NVIDIA GPUs, X360, PS3)
		ueBool m_hintStable;				//!< Maintain world-space stability of the shadow if possible

		ShadowMethodDesc() :
			m_warpMethod(WarpMethod_Standard),
			m_enableVarianceShadows(UE_FALSE),
			m_numShadowSplits(3),
			m_enableAutomaticSplitting(UE_TRUE),
			m_automaticSplitLambda(0.8f),
			m_fadeOutBehind(UE_TRUE),
			m_fadeOutColor(ueColor32(127, 127, 127, 255)),
			m_drawOrder(DrawOrder_AllShadowMapsFirst_ShadowMaskPassPerSplit),
			m_useSeparateDepthBiasPerSplit(UE_FALSE),
			m_shadowMapSize(256),
			m_packShadowMapsIntoSingleTexture(UE_FALSE),
			m_shadowMaskWidth(256),
			m_shadowMaskHeight(256),
			m_sampleSceneDepth(UE_FALSE),
			m_hintUseDepthTexture(UE_TRUE),
			m_hintStable(UE_TRUE)
		{
			m_depthBias[0] = 0.000001f;
		}
	};

	//! Extra data supplied by the user - some of which are obligatory, some other - optional
	struct ExtraData
	{
		glTextureBuffer* m_sceneDepthTexture;	//!< Optional scene depth texture to sample while rendering scene color
		DrawDepthFunc m_drawDepthFunction;		//!< Callback function to draw depth; when not specified scene depth buffer must be supplied
		DrawShadowFunc m_drawShadowFunction;	//!< Callback function to draw shadow

		u32 m_skipSplitsBitMask;				//!< Bit mask indicating which shadow splits to skip; by default none is skipped; used for debugging purposes only

		ueBool m_allowShadowMapAccess;			//!< Indicates whether the shadow masks can be retrieved by the user; note: it's up to user to release all shadow maps after rendering; default to UE_FALSE

		glShader* m_computeVarianceShader;		//!< Shader to be used for variance compuation

		void* m_userData;						//!< User data

		ExtraData() :
			m_sceneDepthTexture(NULL),
			m_drawDepthFunction(NULL),
			m_drawShadowFunction(NULL),
			m_skipSplitsBitMask(0),
			m_allowShadowMapAccess(UE_FALSE),
			m_userData(NULL)
		{}
	};

public:
	gxShadowMapper();
	~gxShadowMapper();

	/**
	 *	Draws the shadow mask.
	 *
	 *	@param context context to be used for rendering
	 *	@param lightDesc description of the light source
	 *	@param cameraDesc description of the camera
	 *	@param methodDesc description of desired shadow mapping method
	 *	@param extraData extra data used for rendering shadow mask
	 *
	 *	@return UE_TRUE on success; UE_FALSE otherwise (e.g. desired method is not - yet - supported)
	 */
	ueBool DrawShadowMask(glCtx* context, const LightDesc& lightDesc, const CameraDesc& cameraDesc, const ShadowMethodDesc& methodDesc, ExtraData& extraData);

	//! Returns whether rendering is in progress (inside DrawShadowMask() call)
	UE_INLINE ueBool IsRendering() const { return m_isRendering; }

	/**
	 *	Retrieves texture containing shadow mask.
	 *
	 *	Note 1: Valid only after call to DrawShadowMask() and before next call to DrawShadowMask().
	 *	Note 2: It's up to user to release shadow mask texture when it's no more necessary.
	 */
	UE_INLINE glTextureBuffer* GetShadowMask() const { UE_ASSERT(!m_isRendering); return m_shadowMaskColor; }

	UE_INLINE u32 GetNumShadowMaps() const { UE_ASSERT(!m_isRendering && m_extraData.m_allowShadowMapAccess); return m_numShadowMaps; }
	UE_INLINE glTextureBuffer* GetShadowMap(u32 index) const { UE_ASSERT(!m_isRendering && m_extraData.m_allowShadowMapAccess); return m_shadowMaps[index]; }

	// Accessors for input information (possibly modified)

	UE_INLINE glCtx* GetContext() const { UE_ASSERT(m_isRendering); return m_ctx; }
	UE_INLINE const LightDesc& GetLightDesc() const { return m_light; }
	UE_INLINE const CameraDesc& GetCameraDesc() const { return m_camera; }
	UE_INLINE const ShadowMethodDesc& GetMethodDesc() const { return m_methodDesc; }
	UE_INLINE ExtraData& GetExtraData() { return m_extraData; }

protected:
	struct ShadowMapPassInfo
	{
		ueFrustum m_frustum;
		ueMat44 m_lightView;
		ueMat44 m_lightProj;
	};

	struct ShadowMaskPassInfo
	{
		ueFrustum m_frustum;
		ueMat44 m_cameraView;
		ueMat44 m_cameraProj;
		ueMat44 m_lightTransformation;
		glViewportDesc m_shadowMaskViewport;
	};

	// Input information

	glCtx* m_ctx;
	LightDesc m_light;
	CameraDesc m_camera;
	ShadowMethodDesc m_methodDesc;
	ExtraData m_extraData;

	// State

	ueBool m_isRendering; //!< Whether we're in the middle of rendering shadow mask

	// Information extracted from input

	ueBool m_useDepthTexture; //!< Whether we actually use depth buffer (both requested and supported)
	u32 m_numShadowMapPasses; //!< Number of shadow map rendering passes
	u32 m_numShadowMaps; //!< Number of necessary shadow map textures

	ueVec3 m_cameraPosition; //!< Camera position (extracted from input view matrix)
	ueVec3 m_cameraViewDir; //!< Normalized camera view direction (extracted from input view matrix)
	ueMat44 m_cameraProj; //!< Camera projection matrix (calculated based on input camera description)
	ueMat44 m_cameraViewProj; //!< Camera view projection matrix

	// Information on rendering passes

	ShadowMapPassInfo m_shadowMapPassInfos[MAX_SHADOW_FRUSTUM_SPLITS];
	ShadowMaskPassInfo m_shadowMaskPassInfos[MAX_SHADOW_FRUSTUM_SPLITS];

	// Render target resources

	glRenderGroup* m_shadowMaskGroup;
	glTextureBuffer* m_shadowMaskColor;
	glTextureBuffer* m_shadowMaskDepth;

	glRenderGroup* m_shadowMapGroup;
	glTextureBuffer* m_shadowMaps[MAX_SHADOW_FRUSTUM_SPLITS];

	glRenderGroup* m_varianceShadowMapGroup;
	glTextureBuffer* m_varianceShadowMaps[MAX_SHADOW_FRUSTUM_SPLITS];

	glTextureBuffer* m_sceneDepthTexture; //!< Optional scene depth texture (used when desired, i.e. ShadowMethodDesc::m_sampleSceneDepth is set)

	void ShadowMapPass(u32 passIndex);
	void ShadowMaskPass(u32 passIndex);

	glTextureBuffer* DrawSceneDepth();
	void GenerateVarianceShadowMap(u32 index, glTextureBuffer* shadowMap);

	// Calculation of light transformation for each pass, viewport and other necessary stuff

	ueBool CalculateDrawPassesInfo();
	ueBool CalculateDrawPassesInfo_WarpStandard();
	ueBool CalculateDrawPassesInfo_WarpPerspective();
	ueBool CalculateDrawPassesInfo_WarpLightSpacePerspective();
	ueBool CalculateDrawPassesInfo_WarpTrapezoidal();

	// Helper shadow math functionality

	void CalculateSplittingPlanes();
	void CalculateFrustumCorners(f32 zNear, f32 zFar, f32 scale, ueVec3* corners);
	void CalculateLightTransformation(u32 splitIndex, const ueVec3* corners, ueMat44& lightViewMatrix, ueMat44& lightProjMatrix, ueMat44& lightViewProjMatrix);
};

// @}

#endif // GX_SHADOW_MAPPER_H

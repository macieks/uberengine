#ifndef GX_EFFECT_SHARED_H
#define GX_EFFECT_SHARED_H

#include "Utils/utValueInTime.h"
#include "Graphics/glLib_Shared.h"
#include "GraphicsExt/gxTexture.h"

struct gxEmitterType
{
	// P - based on particle's age, E - based on emitter's age
	enum Param
	{
		Param_MaxAgeE = 0,
		Param_InitialSpawnCountE,
		Param_SpawnCountE,
		Param_AccelerationP,
		Param_AccelerationE,
		Param_InitialVelocityE,
		Param_VelocityP,
		Param_VelocityE,
		Param_ColorP,
		Param_ColorE,
		Param_SizeP,
		Param_SizeE,
		Param_InitialRotationE,
		Param_RotationChangeP,
		Param_RotationChangeE,

		Param_MAX,
	};

	static f32 m_paramsMultiplier[Param_MAX + 3];

	enum SpawnAreaType
	{
		SpawnAreaType_Point = 0,
		SpawnAreaType_Circle,
		SpawnAreaType_FullCircle,
		SpawnAreaType_Rect,
		SpawnAreaType_FullRect,
		SpawnAreaType_Sphere,
		SpawnAreaType_FullSphere,
		SpawnAreaType_Box,
		SpawnAreaType_FullBox,

		SpawnAreaType_MAX
	};

	enum Texture
	{
		Texture_ColorMap = 0,
		Texture_NormalMap,

		Texture_MAX
	};

	enum DrawMethod
	{
		DrawMethod_Quad = 0,
		DrawMethod_Spark,

		DrawMethod_MAX
	};

	struct SpawnArea
	{
		SpawnAreaType m_type : 32;
		union
		{
			struct
			{
				f32 m_radius;
			} m_circle;
			struct
			{
				f32 m_radius;
			} m_sphere;
			struct
			{
				f32 m_width;
				f32 m_height;
			} m_rect;
			struct
			{
				f32 m_width;
				f32 m_height;
				f32 m_depth;
			} m_box;
		};
	};

	ueVec3 m_pos;
	ueQuat m_rot;
	f32 m_scale;

	f32 m_minAge;
	f32 m_maxAge;

	SpawnArea m_spawnArea;

	ueBool m_localSpaceSimulation;

	f32 m_bboxUpdateFreq;
	f32 m_bboxSafeBound;

	u32 m_maxParticles;

	f32 m_spawnTime;

	// Render states

	glBlendingFunc m_srcBlend : 32;
	glBlendingFunc m_dstBlend : 32;

	ueBool m_enableAlphaTest;
	glCmpFunc m_alphaFunc : 32;
	u32 m_alphaRef;

	ueBool m_enableDepthTest;
	ueBool m_enableDepthWrite;

	ueResourceHandle<gxTexture> m_textures[Texture_MAX];

	DrawMethod m_drawMethod : 32;
	ueBool m_drawSoft;

	utValueInTime* m_params[Param_MAX]; // Some may be NULL

	ueBool GetValue(f32 time, Param param, f32 seed, f32* dst, u32 dstCount) const;
};

struct gxEffectTypeData
{
	u32 m_numEmitterTypes;
	gxEmitterType* m_emitterTypes; // Sorted for drawing
	u32 m_instanceSize;
};

#endif // GX_EFFECT_SHARED_H

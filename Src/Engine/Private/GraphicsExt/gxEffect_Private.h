#ifndef GX_EFFECT_PRIVATE_H
#define GX_EFFECT_PRIVATE_H

#include "GraphicsExt/gxEffect.h"
#include "GraphicsExt/gxEffect_Shared.h"
#include "GraphicsExt/gxTexture.h"
#include "Graphics/glLib.h"
#include "Utils/utValueInTime.h"
#include "Base/ueRand.h"
#include "Base/ueResource_Private.h"

struct ueAsync;
struct ioPackage;

struct fxParticleSet
{
	u32 m_capacity;
	u32 m_count;
	gxParticle* m_particles;
};

struct gxEmitter
{
	enum State
	{
		State_WaitingForSpawn = 0,
		State_Running,
		State_BeingDestroyed,
		State_Destroyed
	};
	State m_state;

	gxEffect* m_effect;

	f32 m_age;
	f32 m_oldAge;
	f32 m_maxAge;

	f32 m_seed;

	ueVec3 m_pos;
	ueQuat m_rot;
	f32 m_scale;

	ueColor32 m_color;

	ueVec3 m_oldPos;
	ueQuat m_oldRot;
	f32 m_oldScale;

	f32 m_spawnCountAccumulator;

	f32 m_bboxUpdateTimer;
	ueBox m_bbox;

	gxEmitterType* m_type;
	f32 m_spawnCountScale;

	fxParticleSet m_set;

	fxParticleCallback m_callback;
	void* m_userData;

	void Init(gxEffect* effect, gxEmitterType* type);
	void Deinit();
	void Destroy();
	void Update(f32 dt);
	void Spawn();
	void UpdateTransform();
	void UpdateCurrentTransform();
	void CopyOldTransform();
	void UpdateParticles(f32 dt);
	void UpdateParticle(gxParticle* p, f32 dt, f32 emitterAgeN);
	void SpawnParticles(f32 dt);
	void UpdateBBox(f32 dt);

	void Draw(glCtx* ctx);
};

struct gxEffect
{
	enum State
	{
		State_Running = 0,
		State_BeingDestroyed
	};
	State m_state;
	f32 m_age;

	ueResourceHandle<gxEffectType> m_type;
	gxEmitter* m_emitters;

	ueVec3 m_pos;
	ueQuat m_rot;
	f32 m_scale;
	ueBool m_dirtyTransform;

	ueBox m_bbox;

	gxEffectHandle* m_handle;

	void Init(gxEffectType* type);
	void Destroy(ueBool immediately);
	void Kill();
	void Update(f32 dt);

	void UpdateTransform();
	void ResetOldTransform();
};

//! Particle effect type resource
struct gxEffectType : ueResource
{
	gxEffectType();
	~gxEffectType();

	gxEffectTypeData* m_data;

	// Load time data

	ueAsync* m_job;
	ioPackage* m_package;
};

struct fxSysData
{
	ueAllocator* m_allocator;
	u32 m_numInstances;

	ueRand* m_rand;

	ueResourceTypeDesc m_typeDesc;

	fxSysData() :
		m_allocator(NULL),
		m_numInstances(0)
	{}
};

extern fxSysData* s_fxSysData;

#endif // GX_EFFECT_PRIVATE_H

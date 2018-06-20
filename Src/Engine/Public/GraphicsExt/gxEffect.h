#ifndef GX_EFFECT_H
#define GX_EFFECT_H

#include "Base/ueShapes.h"
#include "Base/ueResource.h"

struct glCtx;
struct gxEffect;

UE_DEFINE_RESOURCE_TYPE(gxEffectType, UE_BE_4BYTE_SYMBOL('e','f','f','x'));

//! Particle
struct gxParticle
{
	ueVec4 m_color;
	ueVec3 m_pos;
	ueVec3 m_size;
	ueVec3 m_vel;
	f32 m_rot;
	f32 m_age;
	f32 m_maxAge;
	f32 m_seed;
};

//! Custom particle callback invoked during effect update; the purpose of the callback is to allow for any custom operations
typedef void (*fxParticleCallback)(gxParticle* particles, u32 count, void* userData);

//! Particle effect handle
class gxEffectHandle : public ueSimpleHandle<gxEffect>
{
public:
	//! Destroys an effect immediately if not done before
	~gxEffectHandle();

	//! Initializes by effect type
	void Init(gxEffectType* effectType);
	//! Initializes by effect type name
	void Init(const char* effectTypeName);

	//! Destroys an effect
	void Destroy(ueBool immediately = UE_TRUE);
	//! Tells whether effect is now being destroyed
	ueBool IsBeingDestroyed();

	//! Updates an effect
	void Update(f32 dt);
	//! Renders an effect
	void Draw(glCtx* ctx);
	//! Spawn particles
	void SpawnParticles(f32 count, u32 emitterIndex = 0);

	//! Sets position
	void SetPos(const ueVec3& pos);
	//! Sets position
	void SetPos2D(const ueVec2& pos);
	//! Sets position
	void SetPos2D(f32 x, f32 y);
	//! Sets rotation
	void SetRot(const ueQuat& rot);
	//! Sets scale
	void SetScale(f32 scale);
	//! Resets old transform preventing lerping between old and current transform while spawning new particles
	void ResetOldTransform();
	//! Sets color
	void SetColor(ueColor32 color);
	//! Sets callback for a specific emitter; emitterIndex set to U32_MAX indicates that the callback shall be set for all emitters
	void SetCallback(fxParticleCallback callback, void* userData, u32 emitterIndex = U32_MAX);

	//! Gets effect gounding box; return UE_TRUE on success (effect is still alive), UE_FALSE otherwise
	ueBool GetBBox(ueBox& box);

private:
	void operator = (const gxEffectHandle& other) {}
};

//! Set of effects
class gxEffectSet
{
public:
	gxEffectSet();
	~gxEffectSet();
	void Init(ueAllocator* allocator, u32 capacity);
	void Deinit();
	gxEffectHandle* SpawnEffect(gxEffectType* effectType, const ueVec3& pos);
	gxEffectHandle* SpawnEffect(gxEffectType* effectType, f32 x, f32 y, f32 z = 0.0f);
	void DestroyAllEffects(ueBool immediately = UE_TRUE);
	void Update(f32 dt);
	//! Assumes gxEffectTypeMgr_BeginDraw was called before
	void Draw(glCtx* ctx);
	void Draw(glCtx* ctx, const ueMat44& view, const ueMat44& proj);
	void Draw(glCtx* ctx, const ueMat44& view, const ueRect& projRect);
private:
	ueAllocator* m_allocator;
	u32 m_capacity;
	gxEffectHandle* m_handles;
};

//! Starts up effect type manager
void gxEffectTypeMgr_Startup(ueAllocator* allocator, const char* config, ueBool supportLocalization = UE_TRUE);
//! Shuts down effect type manager
void gxEffectTypeMgr_Shutdown();

//! Reseeds random number generator used by particles
void gxEffectTypeMgr_RandomReseed(u32 seed);

//! Sets draw transforms for use by particles
void gxEffectTypeMgr_BeginDraw(glCtx* ctx, const ueMat44& view, const ueMat44& proj);
//! Sets draw transforms for use by particles
void gxEffectTypeMgr_BeginDraw(glCtx* ctx, const ueMat44& view, const ueRect& projRect);
//! Ends drawing particles
void gxEffectTypeMgr_EndDraw();

#endif // GX_EFFECT_H

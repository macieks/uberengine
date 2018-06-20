#include "GraphicsExt/gxEffect.h"

gxEffectSet::gxEffectSet() :
	m_capacity(0),
	m_handles(NULL),
	m_allocator(NULL)
{}

gxEffectSet::~gxEffectSet()
{
	Deinit();
}

void gxEffectSet::Init(ueAllocator* allocator, u32 capacity)
{
	Deinit();

	m_allocator = allocator;
	m_capacity = capacity;
	m_handles = ueNewArray<gxEffectHandle>(allocator, capacity);
	UE_ASSERT(m_handles);
}

void gxEffectSet::Deinit()
{
	if (!m_handles)
		return;

	ueDeleteArray<gxEffectHandle>(m_handles, m_allocator, m_capacity);
	m_capacity = 0;
	m_handles = NULL;
	m_allocator = NULL;
}

gxEffectHandle* gxEffectSet::SpawnEffect(gxEffectType* effectType, const ueVec3& pos)
{
	for (u32 i = 0; i < m_capacity; i++)
		if (!m_handles[i].IsValid())
		{
			m_handles[i].Init(effectType);
			m_handles[i].SetPos(pos);
			return &m_handles[i];
		}
	return NULL;
}

gxEffectHandle* gxEffectSet::SpawnEffect(gxEffectType* effectType, f32 x, f32 y, f32 z)
{
	return SpawnEffect(effectType, ueVec3(x, y, z));
}

void gxEffectSet::DestroyAllEffects(ueBool immediately)
{
	for (u32 i = 0; i < m_capacity; i++)
		m_handles[i].Destroy(immediately);
}

void gxEffectSet::Update(f32 dt)
{
	for (u32 i = 0; i < m_capacity; i++)
		m_handles[i].Update(dt);
}

void gxEffectSet::Draw(glCtx* ctx)
{
	for (u32 i = 0; i < m_capacity; i++)
		m_handles[i].Draw(ctx);
}

void gxEffectSet::Draw(glCtx* ctx, const ueMat44& view, const ueMat44& proj)
{
	gxEffectTypeMgr_BeginDraw(ctx, view, proj);
	for (u32 i = 0; i < m_capacity; i++)
		m_handles[i].Draw(ctx);
}

void gxEffectSet::Draw(glCtx* ctx, const ueMat44& view, const ueRect& projRect)
{
	gxEffectTypeMgr_BeginDraw(ctx, view, projRect);
	for (u32 i = 0; i < m_capacity; i++)
		m_handles[i].Draw(ctx);
}
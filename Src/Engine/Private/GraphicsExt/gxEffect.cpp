#include "GraphicsExt/gxEffect_Private.h"
#include "Graphics/glLib.h"

fxSysData* s_fxSysData = NULL;

f32 gxEmitterType::m_paramsMultiplier[Param_MAX + 3] =
{
	543.565456f,
	765.444325f,
	423.675756f,
	967.654566f,
	983.854645f,
	384.454675f,
	876.246856f,
	947.654654f,
	894.927596f,
	210.544367f,
	543.678354f,
	365.546457f,
	774.544655f,

	// Additional 3 for 1,2,3 components

	176.246856f,
	247.654654f,
	394.927596f,
};

// gxEmitter type

void gxEmitterType_Destroy(gxEmitterType* emitterType)
{
	for (u32 i = 0; i < gxEmitterType::Param_MAX; i++)
		if (utValueInTime* param = emitterType->m_params[i])
			ueDelete(param, s_fxSysData->m_allocator);
	for (u32 i = 0; i < gxEmitterType::Texture_MAX; i++)
		emitterType->m_textures[i].Release();
}

ueBool gxEmitterType::GetValue(f32 time, Param p, f32 seed, f32* dst, u32 dstCount) const
{
	const utValueInTime* param = m_params[p];
	if (!param || param->m_numComponents != dstCount)
		return UE_FALSE;

	f32 seedArray[4];
	for (u32 i = 0; i < dstCount; i++)
		seedArray[i] = ueFrac(m_paramsMultiplier[p + i] * seed);

	switch (param->m_type)
	{
		case utValueInTimeType_Constant:
			ueMemCpy(dst, static_cast<const utValueInTime_Constant*>(param)->m_value, sizeof(f32) * dstCount);
			return UE_TRUE;
		case utValueInTimeType_ConstantRange:
			static_cast<const utValueInTime_ConstantRange*>(param)->Val(seedArray, dst);
			return UE_TRUE;
		case utValueInTimeType_TimeLine:
			static_cast<const utValueInTime_TimeLine*>(param)->Val(time, dst);
			return UE_TRUE;
		case utValueInTimeType_TimeLineRange:
			static_cast<const utValueInTime_TimeLineRange*>(param)->Val(time, seedArray, dst);
			return UE_TRUE;
		UE_INVALID_CASE(param->m_type);
	}
	return UE_FALSE;
}

// gxEmitter

void gxEmitter::Update(f32 dt)
{
	if (m_state == State_Destroyed)
		return;

	// Spawn new particles

	if (m_state == State_WaitingForSpawn)
	{
		if (m_effect->m_age < m_type->m_spawnTime)
			return;
		Spawn();
	}

	// Update transform

	UpdateTransform();

	// Update particles

	UpdateParticles(dt);

	if (m_state == State_Running)
		SpawnParticles(dt);
	else if (m_state == State_BeingDestroyed && m_set.m_count == 0)
	{
		m_state = State_Destroyed;
		return;
	}

	if (m_callback && m_set.m_count > 0)
		m_callback(m_set.m_particles, m_set.m_count, m_userData);

	// Update bounding box

	UpdateBBox(dt);

	// Update age

	m_oldAge = m_age;
	m_age += dt;
	if (m_age >= m_maxAge)
		m_state = State_BeingDestroyed;
}

void gxEmitter::Init(gxEffect* effect, gxEmitterType* type)
{
	m_effect = effect;
	m_type = type;
	m_state = State_WaitingForSpawn;

	m_color = ueColor32::White;

	m_bbox.Reset();

	m_callback = NULL;
}

void gxEmitter::Destroy()
{
	switch (m_state)
	{
	case State_WaitingForSpawn:
		m_state = State_Destroyed;
		break;
	case State_Running:
		m_state = m_set.m_count == 0 ? State_Destroyed : State_BeingDestroyed;
		break;
	case State_BeingDestroyed:
	case State_Destroyed:
		return;
	}
}

void gxEmitter::Spawn()
{
	m_state = State_Running;
	m_seed = ueRand_F32(s_fxSysData->m_rand);
	m_age = 0;
	m_maxAge = ueLerp(m_type->m_minAge, m_type->m_maxAge, ueFrac(m_seed));

	m_set.m_count = 0;

	m_bboxUpdateTimer = 0.0f;

	m_spawnCountScale = 1.0f;

	f32 spawnDelta = 0.0f;
	m_type->GetValue(0.0f, gxEmitterType::Param_InitialSpawnCountE, m_seed, &spawnDelta, 1);
	m_spawnCountAccumulator = spawnDelta * m_spawnCountScale;

	UpdateCurrentTransform();
	CopyOldTransform();
}

void gxEmitter::CopyOldTransform()
{
	m_oldPos = m_pos;
	m_oldRot = m_rot;
	m_oldScale = m_scale;
}

void gxEmitter::UpdateCurrentTransform()
{
	ueVec3::Add(m_pos, m_effect->m_pos, m_type->m_pos);
	ueQuat::Mul(m_rot, m_effect->m_rot, m_type->m_rot);
	m_scale = m_effect->m_scale * m_type->m_scale;
}

void gxEmitter::UpdateTransform()
{
	CopyOldTransform();
	UpdateCurrentTransform();
}

void gxParticle_GenerateSpawnPos(gxParticle* p, const gxEmitterType::SpawnArea& area)
{
	switch (area.m_type)
	{
		case gxEmitterType::SpawnAreaType_FullRect:
		{
			p->m_pos +=
				ueVec3(
					ueRand_F32(s_fxSysData->m_rand, area.m_rect.m_width) - area.m_rect.m_width * 0.5f,
					ueRand_F32(s_fxSysData->m_rand, area.m_rect.m_height) - area.m_rect.m_height * 0.5f,
					0.0f);
			break;
		}
	}
}

void gxEmitter::UpdateParticles(f32 dt)
{
	const f32 emitterAgeN = m_maxAge > 0.0f ? (m_age / m_maxAge) : 0.0f;

	// Update age and remove dead particles

	u32 i = 0;
	while (i < m_set.m_count)
	{
		gxParticle* p = &m_set.m_particles[i];

		// Update age

		p->m_age += dt;

		// Remove?

		if (p->m_age >= p->m_maxAge)
		{
			m_set.m_particles[i] = m_set.m_particles[--m_set.m_count];
			continue;
		}

		// Go to next one

		i++;
	}

	// Update alive particles

	for (u32 i = 0; i < m_set.m_count; i++)
	{
		gxParticle* p = &m_set.m_particles[i];
		UpdateParticle(p, dt, emitterAgeN);
	}
}

void gxEmitter::UpdateParticle(gxParticle* p, f32 dt, f32 emitterAgeN)
{
	const f32 particleAgeN = p->m_age / p->m_maxAge;
	const ueVec3 oldPos = p->m_pos;

	ueVec3 velP = ueVec3::One;
	const ueBool hasVelP = m_type->GetValue(particleAgeN, gxEmitterType::Param_VelocityP, p->m_seed, (f32*) &velP, 3);
	ueVec3 velE = ueVec3::One;
	const ueBool hasVelE = m_type->GetValue(emitterAgeN, gxEmitterType::Param_VelocityE, p->m_seed, (f32*) &velE, 3);
	if (hasVelP || hasVelE)
		ueVec3::Mul(p->m_vel, velE, velP);
	else
	{
		ueVec3 accP = ueVec3::One;
		const ueBool hasAccP = m_type->GetValue(particleAgeN, gxEmitterType::Param_AccelerationP, p->m_seed, (f32*) &accP, 3);
		ueVec3 accE = ueVec3::One;
		const ueBool hasAccE = m_type->GetValue(emitterAgeN, gxEmitterType::Param_AccelerationE, p->m_seed, (f32*) &accE, 3);
		if (hasAccP || hasAccE)
		{
			ueVec3 acc;
			ueVec3::Mul(acc, accE, accP);
			acc *= dt;
			p->m_vel += acc;
		}
	}

	p->m_pos += p->m_vel * dt;

	ueVec4 colorP = ueVec4::One;
	const ueBool hasColorP = m_type->GetValue(particleAgeN, gxEmitterType::Param_ColorP, p->m_seed, (f32*) &colorP, 4);
	ueVec4 colorE = ueVec4::One;
	const ueBool hasColorE = m_type->GetValue(emitterAgeN, gxEmitterType::Param_ColorE, p->m_seed, (f32*) &colorE, 4);
	if (hasColorE || hasColorP)
		ueVec4::Mul(p->m_color, colorE, colorP);

	ueVec3 sizeP = ueVec3::One;
	const ueBool hasSizeP = m_type->GetValue(particleAgeN, gxEmitterType::Param_SizeP, p->m_seed, (f32*) &sizeP, 3);
	ueVec3 sizeE = ueVec3::One;
	const ueBool hasSizeE = m_type->GetValue(emitterAgeN, gxEmitterType::Param_SizeE, p->m_seed, (f32*) &sizeE, 3);
	if (hasSizeP || hasSizeE)
		ueVec3::Mul(p->m_size, sizeE, sizeP);

	f32 rotChangeP = 1.0f;
	const ueBool hasRotChangeP = m_type->GetValue(particleAgeN, gxEmitterType::Param_RotationChangeP, p->m_seed, &rotChangeP, 1);
	f32 rotChangeE = 1.0f;
	const ueBool hasRotChangeE = m_type->GetValue(emitterAgeN, gxEmitterType::Param_RotationChangeE, p->m_seed, &rotChangeE, 1);
	if (hasRotChangeP || hasRotChangeE)
		p->m_rot += rotChangeE * rotChangeP * dt;
}

void gxEmitter::SpawnParticles(f32 dt)
{
	const f32 oldfxEmitterAgeN = m_maxAge > 0.0f ? (m_oldAge / m_maxAge) : 0.0f;
	const f32 newfxEmitterAgeN = m_maxAge > 0.0f ? (m_age / m_maxAge) : 0.0f;

	// Determine how many particles to spawn

	f32 spawnDelta = 0.0f;
	m_type->GetValue(newfxEmitterAgeN, gxEmitterType::Param_SpawnCountE, m_seed, &spawnDelta, 1);
	m_spawnCountAccumulator += spawnDelta * m_spawnCountScale * dt;

	u32 numParticlesToSpawn = ueMin((u32) ueFloor(m_spawnCountAccumulator), m_set.m_capacity - m_set.m_count);
	m_spawnCountAccumulator = ueFrac(m_spawnCountAccumulator);

	if (numParticlesToSpawn == 0)
		return;

	// Spawn particles

	f32 emitterAgeN = oldfxEmitterAgeN;
	const f32 emitterAgeNStep = (newfxEmitterAgeN - oldfxEmitterAgeN) / (f32) numParticlesToSpawn;

	ueVec3 pos;
	ueVec3 posStep;
	if (m_type->m_localSpaceSimulation)
	{
		pos.Zeroe();
		posStep.Zeroe();
	}
	else
	{
		pos = m_oldPos;
		ueVec3::Sub(posStep, m_pos, m_oldPos);
		posStep /= (f32) numParticlesToSpawn;
	}

	while (numParticlesToSpawn--)
	{
		emitterAgeN += emitterAgeNStep;
		pos += posStep;

		gxParticle* p = &m_set.m_particles[m_set.m_count++];
		p->m_seed = ueRand_F32(s_fxSysData->m_rand);
		p->m_pos = pos;
		p->m_age = 0.0f;

		// Set up spawn position

		gxParticle_GenerateSpawnPos(p, m_type->m_spawnArea);

		// Set initial properties

		UE_ASSERT_FUNC(m_type->GetValue(emitterAgeN, gxEmitterType::Param_MaxAgeE, p->m_seed, (f32*) &p->m_maxAge, 1));

		p->m_vel.Zeroe();
		if (m_type->GetValue(emitterAgeN, gxEmitterType::Param_InitialVelocityE, p->m_seed, (f32*) &p->m_vel, 3))
		{
			// Fire particles evenly around


		}

		p->m_rot = 0.0f;
		m_type->GetValue(emitterAgeN, gxEmitterType::Param_InitialRotationE, p->m_seed, (f32*) &p->m_rot, 1);

		// Perform single particle update

		UpdateParticle(p, 0.0f, emitterAgeN);
	}
}

void gxEmitter::UpdateBBox(f32 dt)
{
	m_bboxUpdateTimer += dt;
	if (m_bboxUpdateTimer < m_type->m_bboxUpdateFreq)
		return;

	m_bbox.Reset();
	for (u32 i = 0; i < m_set.m_count; i++)
		m_bbox.Extend(m_set.m_particles[i].m_pos);
	m_bboxUpdateTimer = 0.0f;

//	if (m_type->m_localSpaceSimulation)
//		m_bbox.Transform(m_pos, m_rot, m_scale);
}

// gxEffectTypeData

void gxEffectTypeData_Destroy(gxEffectTypeData* data)
{
	for (u32 i = 0; i < data->m_numEmitterTypes; i++)
		gxEmitterType_Destroy(&data->m_emitterTypes[i]);
	ueDelete(data, s_fxSysData->m_allocator);
}

// gxEffect

//! Creates an instance; on success returns UE_TRUE and sets up handle, on failure returns UE_FALSE
ueBool gxEffectType_CreateEffect(gxEffectType* type, gxEffectHandle* handle);

void gxEffect::Init(gxEffectType* type)
{
	m_type = type;
	m_state = State_Running;
	m_age = 0.0f;

	m_pos.Zeroe();
	m_rot.SetIdentity();
	m_scale = 1.0f;
	m_dirtyTransform = UE_TRUE;

	m_bbox.Reset();

	for (u32 i = 0; i < type->m_data->m_numEmitterTypes; i++)
		m_emitters[i].Init(this, &type->m_data->m_emitterTypes[i]);
}

void gxEffect::Destroy(ueBool immediately)
{
	if (immediately)
		Kill();
	else
	{
		ueBool allfxEmittersDestroyed = UE_TRUE;
		for (u32 i = 0; i < m_type->m_data->m_numEmitterTypes; i++)
		{
			gxEmitter* emitter = &m_emitters[i];
			emitter->Destroy();

			if (emitter->m_state != gxEmitter::State_Destroyed)
				allfxEmittersDestroyed = UE_FALSE;
		}

		if (allfxEmittersDestroyed)
			Kill();
		else
			m_state = State_BeingDestroyed;
	}
}

void gxEffect::Kill()
{
	if (m_handle)
		m_handle->_SetHandle(NULL);
	ueDelete(this, s_fxSysData->m_allocator);
	s_fxSysData->m_numInstances--;
}

void gxEffect::UpdateTransform()
{
	m_bbox.Reset();
	for (u32 i = 0; i < m_type->m_data->m_numEmitterTypes; i++)
	{
		gxEmitter* emitter = &m_emitters[i];
		emitter->UpdateCurrentTransform();
		if (emitter->m_state != gxEmitter::State_Destroyed)
			m_bbox.Extend(emitter->m_bbox);
	}

	m_dirtyTransform = UE_FALSE;
}

void gxEffect::ResetOldTransform()
{
	if (m_dirtyTransform)
		UpdateTransform();

	for (u32 i = 0; i < m_type->m_data->m_numEmitterTypes; i++)
		m_emitters[i].CopyOldTransform();
}

void gxEffect::Update(f32 dt)
{
	m_age += dt;

	// Update

	m_bbox.Reset();
	ueBool allfxEmittersDestroyed = UE_TRUE;
	for (u32 i = 0; i < m_type->m_data->m_numEmitterTypes; i++)
	{
		gxEmitter* emitter = &m_emitters[i];

		emitter->Update(dt);

		if (emitter->m_state != gxEmitter::State_Destroyed)
		{
			allfxEmittersDestroyed = UE_FALSE;
			m_bbox.Extend(emitter->m_bbox);
		}
	}

	m_dirtyTransform = UE_FALSE;

	// Destroy?

	if (allfxEmittersDestroyed)
	{
		Kill();
		return;
	}
}

// gxEffect type

gxEffectType::gxEffectType() :
	m_data(NULL),
	m_package(NULL),
	m_job(NULL)
{}

gxEffectType::~gxEffectType()
{
	UE_ASSERT(!m_package);

	if (m_data)
	{
		// Release textures

		for (u32 i = 0; i < m_data->m_numEmitterTypes; i++)
		{
			gxEmitterType* emitterType = &m_data->m_emitterTypes[i];
			for (u32 j = 0; j < gxEmitterType::Texture_MAX; j++)
				emitterType->m_textures[j].Release();
		}

		s_fxSysData->m_allocator->Free(m_data);
		m_data = NULL;
	}
}

ueBool gxEffectType_CreateEffect(gxEffectType* type, gxEffectHandle* handle)
{
	UE_ASSERT(!handle->IsValid());
	UE_ASSERT(ueResource_IsReady(type));

	u8* memory = (u8*) s_fxSysData->m_allocator->Alloc(type->m_data->m_instanceSize);
	if (!memory)
		return UE_FALSE;

	// Assign memory

	gxEffect* effect = new(memory) gxEffect();
	memory += (u32) sizeof(gxEffect);

	effect->m_emitters = (gxEmitter*) memory;
	memory += (u32) sizeof(gxEmitter) * type->m_data->m_numEmitterTypes;

	for (u32 i = 0; i < type->m_data->m_numEmitterTypes; i++)
	{
		gxEmitterType* emitterType = &type->m_data->m_emitterTypes[i];

		gxEmitter& emitter = effect->m_emitters[i];

		emitter.m_set.m_capacity = emitterType->m_maxParticles;
		emitter.m_set.m_particles = (gxParticle*) memory;
		memory += emitterType->m_maxParticles * sizeof(gxParticle);
	}

	// Init

	effect->Init(type);
	handle->_SetHandle(effect);
	effect->m_handle = handle;

	s_fxSysData->m_numInstances++;
	return UE_TRUE;
}

// gxEffect handle

gxEffectHandle::~gxEffectHandle()
{
	Destroy(UE_TRUE);
}

void gxEffectHandle::Init(gxEffectType* effectType)
{
	Destroy(UE_TRUE);
	gxEffectType_CreateEffect(effectType, this);
}

void gxEffectHandle::Init(const char* effectTypeName)
{
	gxEffectType* type = (gxEffectType*) ueResourceMgr_Get(ueResourceType<gxEffectType>::ID, effectTypeName, UE_TRUE);
	if (!type)
		return;

	Init(type);

	ueResourceMgr_Release(type);
}

void gxEffectHandle::Destroy(ueBool immediately)
{
	if (!m_object)
		return;

	m_object->Destroy(immediately);
}

ueBool gxEffectHandle::IsBeingDestroyed()
{
	return m_object && m_object->m_state == gxEffect::State_BeingDestroyed;
}

void gxEffectHandle::Update(f32 dt)
{
	if (!m_object)
		return;

	m_object->Update(dt);
}

void gxEffectHandle::Draw(glCtx* ctx)
{
	if (!m_object)
		return;

	const u32 numEmitters = m_object->m_type->m_data->m_numEmitterTypes;

	if (m_object->m_dirtyTransform)
	{
		for (u32 i = 0; i < numEmitters; i++)
			m_object->m_emitters[i].UpdateTransform();
		m_object->m_dirtyTransform = UE_FALSE;
	}

	for (u32 i = 0; i < numEmitters; i++)
		m_object->m_emitters[i].Draw(ctx);
}

void gxEffectHandle::SpawnParticles(f32 count, u32 emitterIndex)
{
	if (!m_object)
		return;

	UE_ASSERT(emitterIndex < m_object->m_type->m_data->m_numEmitterTypes);

	if (m_object->m_dirtyTransform)
		m_object->UpdateTransform();

	gxEmitter& emitter = m_object->m_emitters[emitterIndex];
	emitter.m_spawnCountAccumulator += count;
	emitter.SpawnParticles(0.0f);
}

void gxEffectHandle::SetPos(const ueVec3& pos)
{
	if (!m_object)
		return;

	m_object->m_pos = pos;
	m_object->m_dirtyTransform = UE_TRUE;
}

void gxEffectHandle::SetPos2D(const ueVec2& pos)
{
	if (!m_object)
		return;

	m_object->m_pos.Set(pos, 0.0f);
	m_object->m_dirtyTransform = UE_TRUE;
}

void gxEffectHandle::SetPos2D(f32 x, f32 y)
{
	SetPos2D(ueVec2(x, y));
}

void gxEffectHandle::SetRot(const ueQuat& rot)
{
	if (!m_object)
		return;

	m_object->m_rot = rot;
	m_object->m_dirtyTransform = UE_TRUE;
}

void gxEffectHandle::SetScale(f32 scale)
{
	if (!m_object)
		return;

	m_object->m_scale = scale;
	m_object->m_dirtyTransform = UE_TRUE;
}

void gxEffectHandle::ResetOldTransform()
{
	if (!m_object)
		return;

	m_object->ResetOldTransform();
}

void gxEffectHandle::SetColor(ueColor32 color)
{
	if (!m_object)
		return;

	const u32 numEmitters = m_object->m_type->m_data->m_numEmitterTypes;
	for (u32 i = 0; i < numEmitters; i++)
	{
		gxEmitter* emitter = &m_object->m_emitters[i];
		emitter->m_color = color;
	}
}

void gxEffectHandle::SetCallback(fxParticleCallback callback, void* userData, u32 emitterIndex)
{
	if (!m_object)
		return;

	const u32 numEmitters = m_object->m_type->m_data->m_numEmitterTypes;
	if (emitterIndex == U32_MAX)
	{
		for (u32 i = 0; i < numEmitters; i++)
		{
			gxEmitter* emitter = &m_object->m_emitters[i];
			emitter->m_callback = callback;
			emitter->m_userData = userData;
		}
	}
	else
	{
		UE_ASSERT(emitterIndex < numEmitters);
		gxEmitter* emitter = &m_object->m_emitters[emitterIndex];
		emitter->m_callback = callback;
		emitter->m_userData = userData;
	}
}

ueBool gxEffectHandle::GetBBox(ueBox& box)
{
	if (!m_object)
		return UE_FALSE;
	box = m_object->m_bbox;
	return UE_TRUE;
}

// gxEffectTypeMgr

extern void gxEffectTypeMgr_Startup_SKU();
extern void gxEffectTypeMgr_Shutdown_SKU();

extern ueResource* gxEffectType_CreateFunc(const char* name);
extern void gxEffectType_InitFunc(gxEffectType* r, ueBool isSyncInit);
extern void gxEffectType_DestroyFunc(gxEffectType* r);

void gxEffectTypeMgr_Startup(ueAllocator* allocator, const char* config, ueBool supportLocalization)
{
	UE_ASSERT(!s_fxSysData);

	s_fxSysData = new(allocator) fxSysData();
	UE_ASSERT(s_fxSysData);
	s_fxSysData->m_allocator = allocator;

	s_fxSysData->m_typeDesc.m_id = ueResourceType<gxEffectType>::ID;
	s_fxSysData->m_typeDesc.m_name = "gxEffectType";
	s_fxSysData->m_typeDesc.m_createFunc = gxEffectType_CreateFunc;
	s_fxSysData->m_typeDesc.m_initFunc = (ueResourceInitFunc) gxEffectType_InitFunc;
	s_fxSysData->m_typeDesc.m_destroyFunc = (ueResourceDestroyFunc) gxEffectType_DestroyFunc;
	s_fxSysData->m_typeDesc.m_needsSyncDeinit = UE_FALSE;
	s_fxSysData->m_typeDesc.m_config = config;
	s_fxSysData->m_typeDesc.m_assetPathType = supportLocalization ? ueAssetPath_LocalizedFirst : ueAssetPath_NonlocalizedOnly;
	ueResourceMgr_RegisterType(&s_fxSysData->m_typeDesc);

	s_fxSysData->m_rand = ueRand_Create();
	UE_ASSERT(s_fxSysData->m_rand);

	gxEffectTypeMgr_Startup_SKU();
}

void gxEffectTypeMgr_Shutdown()
{
	UE_ASSERT(s_fxSysData);
	UE_ASSERT(s_fxSysData->m_numInstances == 0);

	ueRand_Destroy(s_fxSysData->m_rand);

	gxEffectTypeMgr_Shutdown_SKU();

	ueResourceMgr_UnregisterType(&s_fxSysData->m_typeDesc);

	ueDelete(s_fxSysData, s_fxSysData->m_allocator);
	s_fxSysData = NULL;
}

void gxEffectTypeMgr_RandomReseed(u32 seed)
{
	UE_ASSERT(s_fxSysData);
	ueRand_Seed(s_fxSysData->m_rand, seed);
}
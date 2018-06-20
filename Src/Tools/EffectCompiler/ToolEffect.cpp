#include "GX/gxEffect_Private.h"
#include "Base/ueResource_Private.h"
#include "IO/ioXml.h"
#include "Threading/thJobSys.h"

void gxEmitterType_Destroy(gxEmitterType* emitterType);

ueBool gxEffectType_CreateFromXml(gxEffectType* r, ioXmlDoc* doc);
void gxEffectTypeData_Destroy(gxEffectTypeData* data);

ueResource* gxEffectType_CreateFunc(const char* name)
{
	UE_NEW_RESOURCE_RET(gxEffectType, r, s_fxSysData->m_allocator, name);
	r->m_state = ueResourceState_WaitingForAsyncInit;
	return r;
}

void gxEffectType_LoadFunc(gxEffectType* r, ueBool isSyncInit)
{
	ioPath path;
	ueApp_GetAssetPath(path, ueResource_GetName(r), "fx.xml", NULL, ueAssetPath_NonlocalizedOnly);

	// Load Xml doc into memory

	ioXmlDoc* doc = ioXmlDoc_Load(s_fxSysData->m_allocator, path);
	ioXmlDocScopedDestructor docDestructor(doc);
	if (!doc)
	{
		ueLogW("Failed to load Particle Effect Xml '%s', reason: can't load file '%s'", ueResource_GetName(r), path);
		ueResourceMgr_OnInitStageDone(r, ueResourceState_FailedToInitialize, UE_FALSE);
		return;
	}

	// Parse Xml

	if (!gxEffectType_CreateFromXml(r, doc))
	{
		ueLogW("Failed to load Particle Effect Xml '%s', reason: '%s' Xml description invalid", ueResource_GetName(r), path);
		ueResourceMgr_OnInitStageDone(r, ueResourceState_FailedToInitialize, UE_FALSE);
	}
	else
		ueResourceMgr_OnInitStageDone(r, ueResourceState_Ready, UE_FALSE);
}

void gxEffectType_AsyncLoadFunc(thAsync* job, void* userData)
{
	gxEffectType_LoadFunc((gxEffectType*) userData, UE_FALSE);
}

void gxEffectType_InitFunc(gxEffectType* r, ueBool isSyncInit)
{
	r->m_state = ueResourceState_Initializing;

	if (isSyncInit)
	{
		gxEffectType_LoadFunc(r, UE_TRUE);
		return;
	}

	thJobDesc desc;
	desc.m_enableAutoRelease = UE_TRUE;
	desc.m_workFunc = gxEffectType_AsyncLoadFunc;
	desc.m_userData = r;
	r->m_job = thGlobalJobSys_StartJob(&desc);
	if (!r->m_job)
		ueResourceMgr_OnInitStageDone(r, ueResourceState_FailedToInitialize, UE_FALSE);
}

void gxEffectType_DestroyFunc(gxEffectType* r)
{
	if (r->m_data)
	{
		gxEffectTypeData_Destroy(r->m_data);
		r->m_data = NULL;
	}

	ueDelete(r, s_fxSysData->m_allocator);
}

// Loading from Xml

ueBool gxEmitterType_GetParamEnum(const char* name, ueBool ageRefIsParticle, gxEmitterType::Param& param)
{
	struct Entry
	{
		gxEmitterType::Param m_param;
		const char* m_name;
		ueBool m_ageRefIsParticle;
	};

	static Entry entries[] =
	{
		{gxEmitterType::Param_MaxAgeE, "maxAge", UE_FALSE},
		{gxEmitterType::Param_InitialSpawnCountE, "initialSpawnCount", UE_FALSE},
		{gxEmitterType::Param_SpawnCountE, "spawnCount", UE_FALSE},
		{gxEmitterType::Param_AccelerationP, "acceleration", UE_TRUE},
		{gxEmitterType::Param_AccelerationE, "acceleration", UE_FALSE},
		{gxEmitterType::Param_InitialVelocityE, "initialVelocity", UE_FALSE},
		{gxEmitterType::Param_VelocityP, "velocity", UE_TRUE},
		{gxEmitterType::Param_VelocityE, "velocity", UE_FALSE},
		{gxEmitterType::Param_ColorP, "color", UE_TRUE},
		{gxEmitterType::Param_ColorE, "color", UE_FALSE},
		{gxEmitterType::Param_SizeP, "size", UE_TRUE},
		{gxEmitterType::Param_SizeE, "size", UE_FALSE},
		{gxEmitterType::Param_InitialRotationE, "initialRotation", UE_FALSE},
		{gxEmitterType::Param_RotationChangeP, "rotationChange", UE_TRUE},
		{gxEmitterType::Param_RotationChangeE, "rotationChange", UE_FALSE}
	};

	for (u32 i = 0; i < UE_ARRAY_SIZE(entries); i++)
	{
		const Entry& entry = entries[i];
		if (!ueStrCmp(entry.m_name, name) && entry.m_ageRefIsParticle == ageRefIsParticle)
		{
			param = entry.m_param;
			return UE_TRUE;
		}
	}

	ueLogE("Unsupported emitter parameter (name = '%s', age-ref-is-particle = %s)", name, ageRefIsParticle ? "true" : "false");
	return UE_FALSE;
}

ueBool gxEmitterType_ReadBlendFunc(ioXmlNode* node, const char* name, glBlendingFunc& func)
{
	const char* value = ioXmlNode_GetAttrValue(node, name);
	if (!value)
	{
		ueLogE("Missing blend function param (name = '%s')", name);
		return UE_FALSE;
	}

	if (!ueStrCmp(value, "one")) func = glBlendingFunc_One;
	else if (!ueStrCmp(value, "zero")) func = glBlendingFunc_Zero;
	else if (!ueStrCmp(value, "srcAlpha")) func = glBlendingFunc_SrcAlpha;
	else if (!ueStrCmp(value, "invSrcAlpha")) func = glBlendingFunc_InvSrcAlpha;
	else if (!ueStrCmp(value, "srcColor")) func = glBlendingFunc_SrcColor;
	else if (!ueStrCmp(value, "invSrcColor")) func = glBlendingFunc_InvSrcColor;
	else if (!ueStrCmp(value, "dstAlpha")) func = glBlendingFunc_DstAlpha;
	else if (!ueStrCmp(value, "invDstAlpha")) func = glBlendingFunc_InvDstAlpha;
	else if (!ueStrCmp(value, "dstColor")) func = glBlendingFunc_DstColor;
	else if (!ueStrCmp(value, "invDstColor")) func = glBlendingFunc_InvDstColor;
	else
	{
		ueLogE("Unsupported blend function param '%s': %s", value);
		return UE_FALSE;
	}

	return UE_TRUE;
}

ueBool gxEmitterType_CreateFromXml(gxEmitterType* et, ioXmlNode* node)
{
	// Invoke constructor (to set up texture handles etc.)

	new(et) gxEmitterType();

	// Misc.

	ioXmlNode_GetAttrValueVec3(node, "pos", et->m_pos, ueVec3_Zero);
	ioXmlNode_GetAttrValueQuat(node, "quat", et->m_rot, ueQuat_Identity);
	ioXmlNode_GetAttrValueF32(node, "scale", et->m_scale, 1.0f);

	ioXmlNode_GetAttrValueBool(node, "localSpaceSimulation", et->m_localSpaceSimulation, UE_FALSE);

	ioXmlNode_GetAttrValueF32(node, "bboxUpdateFreq", et->m_bboxUpdateFreq, 1.0f);
	ioXmlNode_GetAttrValueF32(node, "bboxSafeBound", et->m_bboxSafeBound, 1.0f);

	ioXmlNode_GetAttrValueU32(node, "maxParticles", et->m_maxParticles, 50);

	ioXmlNode_GetAttrValueF32(node, "spawnTime", et->m_spawnTime, 0.0f);

	ioXmlNode_GetAttrValueF32(node, "minAge", et->m_minAge, 10.0f);
	ioXmlNode_GetAttrValueF32(node, "maxAge", et->m_maxAge, 10.0f);

	// Draw method

	if (const char* drawMethod = ioXmlNode_GetAttrValue(node, "drawMethod"))
	{
		if (!ueStrCmp(drawMethod, "quad")) et->m_drawMethod = gxEmitterType::DrawMethod_Quad;
		else if (!ueStrCmp(drawMethod, "spark")) et->m_drawMethod = gxEmitterType::DrawMethod_Spark;
		else
		{
			ueLogE("Unsupported draw method (name = '%s')", drawMethod);
			return UE_FALSE;
		}
	}

	ioXmlNode_GetAttrValueBool(node, "drawSoft", et->m_drawSoft, UE_FALSE);

	// Textures

	if (const char* colorMap = ioXmlNode_GetAttrValue(node, "colorMap"))
	{
		et->m_textures[gxEmitterType::Texture_ColorMap].SetByName(colorMap, UE_TRUE);
		ueAssert(et->m_textures[gxEmitterType::Texture_ColorMap].IsReady());
	}
	else
	{
		ueLogE("Missing color map");
		return UE_FALSE;
	}

	if (const char* normalMap = ioXmlNode_GetAttrValue(node, "normalMap"))
	{
		et->m_textures[gxEmitterType::Texture_NormalMap].SetByName(normalMap, UE_TRUE);
		ueAssert(et->m_textures[gxEmitterType::Texture_NormalMap].IsReady());
	}

	// Render states

	glBlendingFunc srcBlend, dstBlend;
	if (!gxEmitterType_ReadBlendFunc(node, "srcBlend", srcBlend))
		return UE_FALSE;
	if (!gxEmitterType_ReadBlendFunc(node, "dstBlend", dstBlend))
		return UE_FALSE;
	et->m_srcBlend = srcBlend;
	et->m_dstBlend = dstBlend;

	ioXmlNode_GetAttrValueBool(node, "enableAlphaTest", et->m_enableAlphaTest, UE_FALSE);
	et->m_alphaFunc = glCmpFunc_Greater;
	ueAssert(!ioXmlNode_GetFirstAttr(node, "alphaFunc"));
	ioXmlNode_GetAttrValueU32(node, "enableRef", et->m_alphaRef, 127);

	ioXmlNode_GetAttrValueBool(node, "enableDepthTest", et->m_enableDepthTest, UE_FALSE);
	ioXmlNode_GetAttrValueBool(node, "enableDepthWrite", et->m_enableDepthWrite, UE_FALSE);

	// Params

	ueMemZeroS(et->m_params);

	ioXmlNode* paramsNode = ioXmlNode_GetFirstNode(node, "params");
	ueAssert(paramsNode);

	for (ioXmlNode* paramNode = ioXmlNode_GetFirstNode(paramsNode, "param"); paramNode; paramNode = ioXmlNode_GetNext(paramNode, "param"))
	{
		const char* paramNodeName = ioXmlNode_GetName(paramNode);
		const char* name = ioXmlNode_GetAttrValue(paramNode, "name");

		const char* ageRef = ioXmlNode_GetAttrValue(paramNode, "ageRef");
		ueBool ageRefIsParticle = UE_TRUE;
		if (!ueStrCmp(ageRef, "particle")) ageRefIsParticle = UE_TRUE;
		else if (!ueStrCmp(ageRef, "emitter")) ageRefIsParticle = UE_FALSE;
		else if (ageRef)
		{
			ueLogE("Invalid param age ref: '%s'", ageRef);
			// TODO: Destroy params already loaded
			return UE_FALSE;
		}

		utValueInTime* param = utValueInTime::Create(s_fxSysData->m_allocator, paramNode);
		if (!param)
		{
			ueLogE("Failed to read param (name = '%s')", name);
			// TODO: Destroy params already loaded
			return UE_FALSE;
		}

		gxEmitterType::Param paramEnum;
		if (!gxEmitterType_GetParamEnum(name, ageRefIsParticle, paramEnum))
		{
			// TODO: Destroy params already loaded
			return UE_FALSE;
		}

		et->m_params[paramEnum] = param;
	}

	// Spawn point

	if (ioXmlNode* spawnPointNode = ioXmlNode_GetFirstNode(node, "spawnArea"))
	{
		const char* type = ioXmlNode_GetAttrValue(spawnPointNode, "type");
		if (!ueStrCmp(type, "point")) et->m_spawnArea.m_type = gxEmitterType::SpawnAreaType_Point;
		else if (!ueStrCmp(type, "circle"))
		{
			et->m_spawnArea.m_type = gxEmitterType::SpawnAreaType_Circle;
			ioXmlNode_GetAttrValueF32(spawnPointNode, "radius", et->m_spawnArea.m_circle.m_radius, 1.0f);
		}
		else if (!ueStrCmp(type, "fullCircle"))
		{
			et->m_spawnArea.m_type = gxEmitterType::SpawnAreaType_FullCircle;
			ioXmlNode_GetAttrValueF32(spawnPointNode, "radius", et->m_spawnArea.m_circle.m_radius, 1.0f);
		}
		else if (!ueStrCmp(type, "rect"))
		{
			et->m_spawnArea.m_type = gxEmitterType::SpawnAreaType_Rect;
			ioXmlNode_GetAttrValueF32(spawnPointNode, "width", et->m_spawnArea.m_rect.m_width, 1.0f);
			ioXmlNode_GetAttrValueF32(spawnPointNode, "height", et->m_spawnArea.m_rect.m_height, 1.0f);
		}
		else if (!ueStrCmp(type, "fullRect"))
		{
			et->m_spawnArea.m_type = gxEmitterType::SpawnAreaType_FullRect;
			ioXmlNode_GetAttrValueF32(spawnPointNode, "width", et->m_spawnArea.m_rect.m_width, 1.0f);
			ioXmlNode_GetAttrValueF32(spawnPointNode, "height", et->m_spawnArea.m_rect.m_height, 1.0f);
		}
		else if (!ueStrCmp(type, "sphere"))
		{
			et->m_spawnArea.m_type = gxEmitterType::SpawnAreaType_Sphere;
			ioXmlNode_GetAttrValueF32(spawnPointNode, "radius", et->m_spawnArea.m_sphere.m_radius, 1.0f);
		}
		else if (!ueStrCmp(type, "fullSphere"))
		{
			et->m_spawnArea.m_type = gxEmitterType::SpawnAreaType_FullSphere;
			ioXmlNode_GetAttrValueF32(spawnPointNode, "radius", et->m_spawnArea.m_sphere.m_radius, 1.0f);
		}
		else if (!ueStrCmp(type, "box"))
		{
			et->m_spawnArea.m_type = gxEmitterType::SpawnAreaType_Box;
			ioXmlNode_GetAttrValueF32(spawnPointNode, "width", et->m_spawnArea.m_box.m_width, 1.0f);
			ioXmlNode_GetAttrValueF32(spawnPointNode, "height", et->m_spawnArea.m_box.m_height, 1.0f);
			ioXmlNode_GetAttrValueF32(spawnPointNode, "depth", et->m_spawnArea.m_box.m_depth, 1.0f);
		}
		else if (!ueStrCmp(type, "fullBox"))
		{
			et->m_spawnArea.m_type = gxEmitterType::SpawnAreaType_FullBox;
			ioXmlNode_GetAttrValueF32(spawnPointNode, "width", et->m_spawnArea.m_box.m_width, 1.0f);
			ioXmlNode_GetAttrValueF32(spawnPointNode, "height", et->m_spawnArea.m_box.m_height, 1.0f);
			ioXmlNode_GetAttrValueF32(spawnPointNode, "depth", et->m_spawnArea.m_box.m_depth, 1.0f);
		}
	}
	else
		et->m_spawnArea.m_type = gxEmitterType::SpawnAreaType_Point;

	return UE_TRUE;
}

u32 gxEffectType_CalcInstanceSize(gxEffectTypeData* data)
{
	u32 size = ueAlignPow2((u32) sizeof(gxEffect), (u32) UE_MATH_ALIGNMENT) + ueAlignPow2((u32) sizeof(gxEmitter) * data->m_numEmitterTypes, (u32) UE_MATH_ALIGNMENT);
	for (u32 i = 0; i < data->m_numEmitterTypes; i++)
	{
		gxEmitterType* et = &data->m_emitterTypes[i];
		size += et->m_maxParticles * sizeof(gxParticle);
	}
	return size;
}

ueBool gxEffectType_CreateFromXml(gxEffectType* r, ioXmlDoc* doc)
{
	// Get some root nodes

	ioXmlNode* effectNode = ioXmlDoc_GetFirstNode(doc, "effect");
	if (!effectNode)
	{
		ueLogE("Missing root 'effect' node.");
		return UE_FALSE;
	}

	ioXmlNode* emittersNode = ioXmlNode_GetFirstNode(effectNode, "emitters");
	if (!emittersNode)
	{
		ueLogE("Missing 'emitters' node.");
		return UE_FALSE;
	}

	const u32 numEmitters = ioXmlNode_CalcNumNodes(emittersNode, "emitter");
	if (numEmitters == 0)
	{
		ueLogE("No 'emitter' nodes found.");
		return UE_FALSE;
	}

	// Allocate memory

	ueAssert(sizeof(gxEmitterType) % UE_MATH_ALIGNMENT == 0);

	const u32 alignedEffectTypeDataSize = ueAlignPow2((u32) sizeof(gxEffectTypeData), (u32) UE_MATH_ALIGNMENT);
	const u32 memorySize = alignedEffectTypeDataSize + numEmitters * sizeof(gxEmitterType);
	u8* memory = (u8*) s_fxSysData->m_allocator->Alloc(memorySize, UE_MATH_ALIGNMENT);

	// Create effect type data

	gxEffectTypeData* data = (gxEffectTypeData*) memory;
	ueAssert(data);
	memory += alignedEffectTypeDataSize;
	
	data->m_numEmitterTypes = numEmitters;
	data->m_emitterTypes = (gxEmitterType*) memory;

	ioXmlNode* emitterNode = ioXmlNode_GetFirstNode(emittersNode, "emitter");
	for (u32 i = 0; i < numEmitters; i++, emitterNode = ioXmlNode_GetNext(emitterNode, "emitter"))
		if (!gxEmitterType_CreateFromXml(&data->m_emitterTypes[i], emitterNode))
		{
			// Destroy all

			for (u32 j = 0; j < i; j++)
				gxEmitterType_Destroy(&data->m_emitterTypes[i]);
			ueDelete(data, s_fxSysData->m_allocator);
			return UE_FALSE;
		}

	data->m_instanceSize = gxEffectType_CalcInstanceSize(data);

	// Assign data to effect type

	r->m_data = data;

	return UE_TRUE;
}
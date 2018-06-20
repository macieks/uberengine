#include "ueToolEffect.h"
#include "IO/ioXml.h"
#include "IO/ioPackageWriter.h"
#include "IO/ioPackageWriterUtils.h"
#include "GX/gxEffect_Shared.h"

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
			return true;
		}
	}

	ueLogE("Unsupported emitter parameter (name = '%s', age-ref-is-particle = %s)", name, ageRefIsParticle ? "true" : "false");
	return false;
}

ueBool gxEmitterType_ReadBlendFunc(ioXmlNode* node, const char* name, glBlendingFunc& func)
{
	const char* value = ioXmlNode_GetAttrValue(node, name);
	if (!value)
	{
		ueLogE("Missing blend function param (name = '%s')", name);
		return false;
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
		return false;
	}

	return true;
}

ueBool gxEmitterType_CreateFromXml(ueToolEffect& effect, u32 emitterIndex, gxEmitterType* et, ioXmlNode* node)
{
	// Invoke constructor (to set up texture handles etc.)

	new(et) gxEmitterType();

	// Misc.

	ioXmlNode_GetAttrValueVec3(node, "pos", et->m_pos, ueVec3::Zero);
	ioXmlNode_GetAttrValueQuat(node, "quat", et->m_rot, ueQuat::Identity);
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
			return false;
		}
	}

	ioXmlNode_GetAttrValueBool(node, "drawSoft", et->m_drawSoft, UE_FALSE);

	// Textures

	if (const char* colorMap = ioXmlNode_GetAttrValue(node, "colorMap"))
		effect.m_textureNames[emitterIndex][gxEmitterType::Texture_ColorMap] = colorMap;
	else
	{
		ueLogE("Missing color map");
		return false;
	}

	if (const char* normalMap = ioXmlNode_GetAttrValue(node, "normalMap"))
		effect.m_textureNames[emitterIndex][gxEmitterType::Texture_NormalMap] = normalMap;

	// Render states

	glBlendingFunc srcBlend, dstBlend;
	if (!gxEmitterType_ReadBlendFunc(node, "srcBlend", srcBlend))
		return false;
	if (!gxEmitterType_ReadBlendFunc(node, "dstBlend", dstBlend))
		return false;
	et->m_srcBlend = srcBlend;
	et->m_dstBlend = dstBlend;

	ioXmlNode_GetAttrValueBool(node, "enableAlphaTest", et->m_enableAlphaTest, UE_FALSE);
	et->m_alphaFunc = glCmpFunc_Greater;
	UE_ASSERT(!ioXmlNode_GetFirstAttr(node, "alphaFunc"));
	ioXmlNode_GetAttrValueU32(node, "enableRef", et->m_alphaRef, 127);

	ioXmlNode_GetAttrValueBool(node, "enableDepthTest", et->m_enableDepthTest, UE_FALSE);
	ioXmlNode_GetAttrValueBool(node, "enableDepthWrite", et->m_enableDepthWrite, UE_FALSE);

	// Params

	ueMemZeroS(et->m_params);

	ioXmlNode* paramsNode = ioXmlNode_GetFirstNode(node, "params");
	UE_ASSERT(paramsNode);

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
			return false;
		}

		utValueInTime* param = utValueInTime_CreateFromXmlNode(ueAllocator::GetGlobal(), paramNode);
		if (!param)
		{
			ueLogE("Failed to read param (name = '%s')", name);
			return false;
		}

		gxEmitterType::Param paramEnum;
		if (!gxEmitterType_GetParamEnum(name, ageRefIsParticle, paramEnum))
			return false;

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

	return true;
}

ueBool gxEffectType_CreateFromXml(ueToolEffect& effect, gxEffectTypeData*& data, ioXmlDoc* doc)
{
	// Get some root nodes

	ioXmlNode* effectNode = ioXmlDoc_GetFirstNode(doc, "effect");
	if (!effectNode)
	{
		ueLogE("Missing root 'effect' Xml node.");
		return false;
	}

	ioXmlNode* emittersNode = ioXmlNode_GetFirstNode(effectNode, "emitters");
	if (!emittersNode)
	{
		ueLogE("Missing 'emitters' Xml node.");
		return false;
	}

	const u32 numEmitters = ioXmlNode_CalcNumNodes(emittersNode, "emitter");
	if (numEmitters == 0)
	{
		ueLogE("No 'emitter' Xml nodes found.");
		return false;
	}

	// Allocate memory

	const u32 alignedEffectTypeDataSize = ueAlignPow2((u32) sizeof(gxEffectTypeData), (u32) UE_DEFAULT_ALIGNMENT);
	const u32 memorySize = alignedEffectTypeDataSize + numEmitters * sizeof(gxEmitterType);
	u8* memory = (u8*) ueAllocator::GetGlobal()->Alloc(memorySize, UE_DEFAULT_ALIGNMENT);

	// Create effect type data

	data = (gxEffectTypeData*) memory;
	UE_ASSERT(data);
	memory += alignedEffectTypeDataSize;
	
	data->m_numEmitterTypes = numEmitters;
	data->m_emitterTypes = (gxEmitterType*) memory;

	ioXmlNode* emitterNode = ioXmlNode_GetFirstNode(emittersNode, "emitter");
	for (u32 i = 0; i < numEmitters; i++, emitterNode = ioXmlNode_GetNext(emitterNode, "emitter"))
	{
		effect.m_textureNames.resize(effect.m_textureNames.size() + 1);
		effect.m_textureNames[i].resize(gxEmitterType::Texture_MAX);

		if (!gxEmitterType_CreateFromXml(effect, i, &data->m_emitterTypes[i], emitterNode))
			return false;
	}

	return true;
}

bool ueToolEffect::Serialize(ioPackageWriter* pw)
{
	ioSegmentParams segmentParams;
	segmentParams.m_symbol = UE_BE_4BYTE_SYMBOL('e','f','c','t');
	ioSegmentWriter sw;
	pw->BeginSegment(sw, segmentParams);

	// glTextureFileDesc

	sw.WriteNumber<u32>(m_data->m_numEmitterTypes);
	ioPtr emitterTypesPtr = sw.WritePtr();
	sw.WriteNumber<u32>(0);			// m_instanceSize: This will be generated at runtime (taking into account alignments, vec4 sizes etc.)

	// array of gxEmitterType

	std::vector< std::vector<ioPtr> > paramPtrs(m_data->m_numEmitterTypes);

	sw.BeginPtr(emitterTypesPtr);
	for (u32 i = 0; i < m_data->m_numEmitterTypes; i++)
	{
		const gxEmitterType& et = m_data->m_emitterTypes[i];

		WriteVec3(&sw, et.m_pos);
		WriteQuat(&sw, et.m_rot);
		sw.WriteNumber<f32>(et.m_scale);

		sw.WriteNumber<f32>(et.m_minAge);
		sw.WriteNumber<f32>(et.m_maxAge);

		// gxEmitterType::SpawnArea

		sw.WriteNumber<u32>(et.m_spawnArea.m_type);
		switch (et.m_spawnArea.m_type)
		{
		case gxEmitterType::SpawnAreaType_Point:
			sw.WriteNumber<f32>(0);
			sw.WriteNumber<f32>(0);
			sw.WriteNumber<f32>(0);
			break;
		case gxEmitterType::SpawnAreaType_Circle:
		case gxEmitterType::SpawnAreaType_FullCircle:
		case gxEmitterType::SpawnAreaType_Sphere:
		case gxEmitterType::SpawnAreaType_FullSphere:
			sw.WriteNumber<f32>(et.m_spawnArea.m_sphere.m_radius);
			sw.WriteNumber<f32>(0);
			sw.WriteNumber<f32>(0);
			break;
		case gxEmitterType::SpawnAreaType_Rect:
		case gxEmitterType::SpawnAreaType_FullRect:
			sw.WriteNumber<f32>(et.m_spawnArea.m_rect.m_width);
			sw.WriteNumber<f32>(et.m_spawnArea.m_rect.m_height);
			sw.WriteNumber<f32>(0);
			break;
		case gxEmitterType::SpawnAreaType_Box:
		case gxEmitterType::SpawnAreaType_FullBox:
			sw.WriteNumber<f32>(et.m_spawnArea.m_box.m_width);
			sw.WriteNumber<f32>(et.m_spawnArea.m_box.m_height);
			sw.WriteNumber<f32>(et.m_spawnArea.m_box.m_depth);
			break;
		}

		sw.WriteBool(et.m_localSpaceSimulation);

		sw.WriteNumber<f32>(et.m_bboxUpdateFreq);
		sw.WriteNumber<f32>(et.m_bboxSafeBound);

		sw.WriteNumber<u32>(et.m_maxParticles);

		sw.WriteNumber<f32>(et.m_spawnTime);

		sw.WriteNumber<u32>(et.m_srcBlend);
		sw.WriteNumber<u32>(et.m_dstBlend);

		sw.WriteBool(et.m_enableAlphaTest);
		sw.WriteNumber<u32>(et.m_alphaFunc);
		sw.WriteNumber<u32>(et.m_alphaRef);

		sw.WriteBool(et.m_enableDepthTest);
		sw.WriteBool(et.m_enableDepthWrite);

		const u32 offset = UE_OFFSET_OF(gxEmitterType, m_textures);
		for (u32 j = 0; j < gxEmitterType::Texture_MAX; j++)
			sw.WriteResourceHandle(gxTexture_TYPE_ID, m_textureNames[i][j].c_str());

		sw.WriteNumber<u32>(et.m_drawMethod);
		sw.WriteBool(et.m_drawSoft);

		// Params

		for (u32 j = 0; j < gxEmitterType::Param_MAX; j++)
		{
			utValueInTime* params = et.m_params[j];
			if (params)
				paramPtrs[i].push_back(sw.WritePtr());
			else
			{
				sw.WriteNullPtr();
				paramPtrs[i].push_back(ioPtr());
			}
		}
	}

	// Emitter params

	for (u32 i = 0; i < m_data->m_numEmitterTypes; i++)
	{
		const gxEmitterType& et = m_data->m_emitterTypes[i];
		for (u32 j = 0; j < gxEmitterType::Param_MAX; j++)
		{
			utValueInTime* params = et.m_params[j];
			if (params)
			{
				sw.BeginPtr(paramPtrs[i][j]);
				WriteValueInTime(&sw, params);
			}
		}
	}

	return true;
}

void ueToolEffect::DumpSettings()
{
	DumpSettingHeader();
	DumpSettingString("source", 0, "Source particle effect Xml file relative to root source directory (\"root_src\"), e.g. 'my_dir/my_effect.fx.xml'");
}

bool ueToolEffect::ParseSettings(LoadSettings& settings, const ueAssetParams& params)
{
	settings.m_sourceFileName = params.GetParam("source");
	return true;
}

bool CompileEffect(ueToolEffect& effect, const ueToolEffect::LoadSettings& loadSettings)
{
	// Load Xml doc into memory

	ioXmlDoc* doc = ioXmlDoc_Load(ueAllocator::GetGlobal(), loadSettings.m_sourceFileName.c_str());
	ioXmlDocScopedDestructor docDestructor(doc);
	if (!doc)
	{
		ueLogW("Failed to load Particle Effect Xml file (path = '%s')", loadSettings.m_sourceFileName.c_str());
		return false;
	}

	// Parse Xml

	if (!gxEffectType_CreateFromXml(effect, effect.m_data, doc))
	{
		ueLogW("Invalid Particle Effect Xml file (path = '%s')", loadSettings.m_sourceFileName.c_str());
		return false;
	}

	return true;
}
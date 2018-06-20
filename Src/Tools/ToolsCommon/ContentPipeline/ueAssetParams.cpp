#include "ContentPipeline/ueContentPipeline.h"
#include "IO/ioXml.h"

const char* s_requiredParamValue = "<required>";

bool ueAssetParams::GetPlatformForConfig(uePlatform& platform, const char* config)
{
	if (string_starts_with(config, "win32")) { platform = uePlatform_Win32; return true; }
	if (string_starts_with(config, "linux")) { platform = uePlatform_Linux; return true; }
	if (string_starts_with(config, "mac")) { platform = uePlatform_Mac; return true; }
	if (string_starts_with(config, "x360")) { platform = uePlatform_X360; return true; }
	if (string_starts_with(config, "ps3")) { platform = uePlatform_PS3; return true; }
	if (string_starts_with(config, "marmalade")) { platform = uePlatform_Marmalade; return true; }

	return false;
}

bool ueAssetParams::Load(const char* asset, const char* config, bool verbose)
{
	m_params.clear();
	m_config = config;

	// Get platform

	if (!GetPlatformForConfig(m_platform, config))
	{
		ueLogE("Failed to deduce platform based on config name '%s'", config);
		return false;
	}

	// Load recursively

	if (!LoadDesc(asset))
	{
		ueLogE("Failed to load asset description (name = '%s')", asset);
		m_params.clear();
		return false;
	}

	// Dump all parameters

	if (verbose)
		Dump();

	// Verify all params present

	std::map<std::string, std::string>::iterator iter;
	for (iter = m_params.begin(); iter != m_params.end(); iter++)
		if (!ueStrCmp(iter->second.c_str(), s_requiredParamValue))
		{
			ueLogE("Missing value for required parameter '%s'", iter->first.c_str());
			return false;
		}

	return true;
}

void ueAssetParams::GetAssetParamsPath(char* buffer, u32 bufferSize, const char* assetName)
{
	ueStrFormat(buffer, bufferSize, "%s.asset.xml", assetName);
}

bool ueAssetParams::LoadDesc(const char* asset)
{
	char path[512];
	GetAssetParamsPath(path, UE_ARRAY_SIZE(path), asset);

	ioXmlDoc* doc = ioXmlDoc_Load(ueAllocator::GetGlobal(), path);
	if (!doc)
	{
		ueLogE("Failed to open asset description file (path = '%s')", path);
		return false;
	}
	ioXmlDocScopedDestructor docDestructor(doc);

	m_assetTypeHierarchy.push_back(path);

	ioXmlNode* assetNode = ioXmlDoc_GetFirstNode(doc, "asset");
	if (!assetNode)
		return false;
	{
		// Get & load parent

		const char* parentAsset = ioXmlNode_GetAttrValue(assetNode, "parent");
		if (ueStrLen(parentAsset) > 0 && !LoadDesc(parentAsset))
			return false;

		// Get type & load asset config based on it
		
		const char* type = ioXmlNode_GetAttrValue(assetNode, "type");
		if (ueStrLen(type) > 0)
		{
			if (m_type.length() > 0)
			{
				if (m_type != type)
				{
					ueLogE("Second conflicting asset type definition in Xml asset description (first = '%s'; now = '%s')", m_type.c_str(), type);
					return false;
				}
			}

			m_type = type;
		}

		// Read params

		std::map<std::string, std::string> params_all;
		std::map<std::string, std::string> params_config;

		for (ioXmlNode* groupNode = ioXmlNode_GetFirstNode(assetNode, "paramGroup"); groupNode; groupNode = ioXmlNode_GetNext(groupNode, "paramGroup"))
		{
			const char* paramsGroupName = ioXmlNode_GetAttrValue(groupNode, "name");

			const ueBool isAll = !ueStrCmp(paramsGroupName, "all");
			const ueBool isConfig = !isAll && m_config == paramsGroupName;

			if (isAll || isConfig)
				for (ioXmlNode* paramNode = ioXmlNode_GetFirstNode(groupNode, "param"); paramNode; paramNode = ioXmlNode_GetNext(paramNode, "param"))
				{
					const char* paramName = ioXmlNode_GetAttrValue(paramNode, "name");
					const char* paramValue = ioXmlNode_GetAttrValue(paramNode, "value");
					if (!paramValue)
						paramValue = "<none>";
					(isAll ? params_all : params_config)[paramName] = paramValue;
				}
		}

		// Process read params (first 'all' category, then 'config' category')

		std::map<std::string, std::string>::iterator iter;

		for (iter = params_all.begin(); iter != params_all.end(); iter++)
			SetParam(iter->first, iter->second);
		for (iter = params_config.begin(); iter != params_config.end(); iter++)
			SetParam(iter->first, iter->second);
	}

	return true;
}

void ueAssetParams::SetParam(const std::string& name, std::string& value)
{
	static const char* baseStr = "${base}";
	static const u32 baseStrLen = ueStrLen(baseStr);

#if 0 // WTF???
	const size_t base = value.find_first_of(baseStr);
	if (base != std::string::npos)
#else
	const char* foundPtr = ueStrStr(value.c_str(), baseStr);
	const size_t base = foundPtr ? foundPtr - value.c_str() : 0;
	if (foundPtr)
#endif
	{
		std::string oldValue;
		std::map<std::string, std::string>::iterator iter = m_params.find(name);
		if (iter != m_params.end())
			oldValue = iter->second;

		value.replace(base, baseStrLen, oldValue.c_str());
	}

	m_params[name] = value;
}

bool ueAssetParams::GetParam(const char* name, const char*& value) const
{
	std::map<std::string, std::string>::const_iterator iter = m_params.find(name);
	if (iter == m_params.end())
		return false;
	value = iter->second.c_str();
	return true;
}

bool ueAssetParams::GetBoolParam(const char* name, ueBool& value) const
{
	const char* param = NULL;
	if (GetParam(name, param))
	{
		UE_ASSERT_FUNC( ueStrToBool(param, value) );
		return true;
	}
	return false;
}

bool ueAssetParams::GetS32Param(const char* name, s32& value) const
{
	const char* param = NULL;
	if (GetParam(name, param))
	{
		UE_ASSERT_FUNC( ueStrToS32(param, value) );
		return true;
	}
	return false;
}

bool ueAssetParams::GetU32Param(const char* name, u32& value) const
{
	const char* param = NULL;
	if (GetParam(name, param))
	{
		UE_ASSERT_FUNC( ueStrToU32(param, value) );
		return true;
	}
	return false;
}

bool ueAssetParams::GetF32Param(const char* name, f32& value) const
{
	const char* param = NULL;
	if (GetParam(name, param))
	{
		UE_ASSERT_FUNC( ueStrToF32(param, value) );
		return true;
	}
	return false;
}

bool ueAssetParams::GetVec2Param(const char* name, ueVec2& value) const
{
	const char* param = NULL;
	if (GetParam(name, param))
	{
		f32 x, y;
		if (ueStrScanf(name, "%f %f", &x, &y) != 2)
			return false;
		value.Set(x, y);
		return true;
	}
	return false;
}

bool ueAssetParams::GetVec3Param(const char* name, ueVec3& value) const
{
	const char* param = NULL;
	if (GetParam(name, param))
	{
		f32 x, y, z;
		if (ueStrScanf(name, "%f %f %f", &x, &y, &z) != 3)
			return false;
		value.Set(x, y, z);
		return true;
	}
	return false;
}

bool ueAssetParams::GetVec4Param(const char* name, ueVec4& value) const
{
	const char* param = NULL;
	if (GetParam(name, param))
	{
		f32 x, y, z, w;
		if (ueStrScanf(name, "%f %f %f %f", &x, &y, &z, &w) != 4)
			return false;
		value.Set(x, y, z, w);
		return true;
	}
	return false;
}

bool ueAssetParams::GetParam(const char* name, std::string& value) const
{
	const char* param = NULL;
	if (GetParam(name, param))
	{
		value = param;
		return true;
	}
	return false;
}

const char* ueAssetParams::GetParam(const char* name) const
{
	const char* value = NULL;
	const bool result = GetParam(name, value);
	if (!result)
	{
		ueLogE("Missing obligatory asset param '%s'", name);
		exit(1);
	}
	return value;
}

void ueAssetParams::Dump()
{
	ueLogD("Asset compilation parameters (asset type = '%s', asset config = '%s') [%u]:",
		m_type.c_str(),
		m_config.c_str(),
		(u32) m_params.size());

	u32 index = 0;
	for (std::map<std::string, std::string>::iterator iter = m_params.begin(); iter != m_params.end(); iter++)
		ueLogD("  [%u] %s -> '%s'", index++, iter->first.c_str(), iter->second.c_str());
}
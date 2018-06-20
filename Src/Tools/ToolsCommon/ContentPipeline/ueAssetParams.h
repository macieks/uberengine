#pragma once

#include "Math/ueMath.h"

//! Asset parameters (used by asset compiler)
class ueAssetParams
{
public:
	bool Load(const char* asset, const char* config, bool verbose);

	const char* GetType() const { return m_type.c_str(); }
	const char* GetConfig() const { return m_config.c_str(); }
	uePlatform GetPlatform() const { return m_platform; }

	bool GetParam(const char* name, const char*& value) const;
	bool GetParam(const char* name, std::string& value) const;
	const char* GetParam(const char* name) const;
	bool GetBoolParam(const char* name, ueBool& value) const;
	bool GetS32Param(const char* name, s32& value) const;
	bool GetU32Param(const char* name, u32& value) const;
	bool GetF32Param(const char* name, f32& value) const;
	bool GetVec2Param(const char* name, ueVec2& value) const;
	bool GetVec3Param(const char* name, ueVec3& value) const;
	bool GetVec4Param(const char* name, ueVec4& value) const;
	template <typename ENUM_TYPE>
	bool GetEnumParam(const char* name, ENUM_TYPE& value) const;

	void Dump();

	static void GetAssetParamsPath(char* buffer, u32 bufferSize, const char* assetName);

private:
	void SetParam(const std::string& name, std::string& value);
	bool LoadDesc(const char* asset);
	static bool GetPlatformForConfig(uePlatform& platform, const char* config);

	uePlatform m_platform;
	std::string m_config;
	std::string m_type;
	std::vector<std::string> m_assetTypeHierarchy;
	std::map<std::string, std::string> m_params;

	friend class ueBuildDependency;
};

template <typename ENUM_TYPE>
bool ueAssetParams::GetEnumParam(const char* name, ENUM_TYPE& value) const
{
	const char* stringValue = GetParam(name);
	if (!stringValue)
		return false;
	return ueEnumFromStringSafe(glBufferFormat, stringValue, value);
}
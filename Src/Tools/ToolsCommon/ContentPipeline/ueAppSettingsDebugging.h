#pragma once

enum Setting
{
	Setting_Optional = UE_POW2(0),
	Setting_Array = UE_POW2(1),

	Setting_MAX
};

UE_INLINE void DumpSettingHeader()
{
	fprintf(stdout, "Available settings:\n");
}

UE_INLINE void DumpSetting(const char* typeName, const char* name, u32 settingsFlags, const char* description)
{
	std::string info = " * ";
	info += name;
	info += " [";
	info += typeName;
	info += "] ";
	if (settingsFlags & Setting_Optional)
		info += "[optional] ";
	if (settingsFlags & Setting_Array)
		info += "[array] ";
	info += description;
	info += "\n";

	fprintf(stdout, info.c_str());
}

UE_INLINE void DumpSettingString(const char* name, u32 settingsFlags, const char* description)
{
	DumpSetting("string", name, settingsFlags, description);
}

UE_INLINE void DumpSettingU32(const char* name, u32 settingsFlags, const char* description)
{
	DumpSetting("int", name, settingsFlags, description);
}

UE_INLINE void DumpSettingF32(const char* name, u32 settingsFlags, const char* description)
{
	DumpSetting("float", name, settingsFlags, description);
}

UE_INLINE void DumpSettingBool(const char* name, u32 settingsFlags, const char* description)
{
	DumpSetting("bool", name, settingsFlags, description);
}

UE_INLINE void DumpSettingVec3(const char* name, u32 settingsFlags, const char* description)
{
	DumpSetting("vec3", name, settingsFlags, description);
}

UE_INLINE void DumpSettingEnum_Private(const char* name, u32 settingsFlags, const ueEnumEntry* entries, const char* description)
{
	std::string fullDescription = description;
	fullDescription += "\n\tAvailable values:\n";
	while (entries->m_name)
	{
		fullDescription += "\t";
		fullDescription += entries->m_name;
		entries++;
		if (entries->m_name)
			fullDescription += "\n";
	}
	DumpSetting("enum", name, settingsFlags, fullDescription.c_str());
}

#define DumpSettingEnum(name, settingsFlags, enumName, description) DumpSettingEnum_Private(name, settingsFlags, g_enumMap_##enumName, description)

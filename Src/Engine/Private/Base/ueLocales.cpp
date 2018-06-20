#include "Base/ueLocales.h"
#include "Base/Containers/ueHashMap.h"
#include "IO/ioXml.h"

struct ueLocaleSet
{
	static const u32 TYPE = UE_BE_4BYTE_SYMBOL('l','o','c','a');
	typedef ueHashMap<const char*, const char*> MapType;

	char* m_name;
	MapType m_strings;
};

struct ueLocaleMgrData
{
	ueAllocator* m_allocator;

	uePath m_rootDir;
	ueHashMap<const char*, ueLocaleSet*> m_sets;

	ueLocaleMgrData() :
		m_allocator(NULL)
	{}
};

static ueLocaleMgrData* s_data = NULL;

void ueLocaleMgr_Startup(ueAllocator* allocator, u32 maxSets)
{
	UE_ASSERT(!s_data);

	s_data = new(allocator) ueLocaleMgrData();
	UE_ASSERT(s_data);
	UE_ASSERT_FUNC(s_data->m_sets.Init(allocator, maxSets));
	s_data->m_rootDir[0] = 0;
	s_data->m_allocator = allocator;
}

void ueLocaleMgr_Shutdown()
{
	UE_ASSERT(s_data);

	while (1)
	{
		// TODO: Improve this - it's quite inefficient
		// NOTE: Can't just iterate over and free because name (hashmap key) points into freed memory block
		ueHashMap<const char*, ueLocaleSet*>::Iterator iter(s_data->m_sets);
		if (!iter.Next())
			break;
		ueLocaleSet* set = *iter.Value();
		s_data->m_sets.Remove(set->m_name);
		s_data->m_allocator->Free(set);
	}
	s_data->m_sets.Deinit();

	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

void ueLocaleMgr_SetRootDir(const char* rootDir)
{
	ueStrCpyS(s_data->m_rootDir, rootDir);
}

ueBool ueLocaleMgr_LoadSet(const char* setName)
{
	UE_ASSERT(s_data);

	// Open localization set file

	uePath fullSetName;
	ueStrFormatS(fullSetName, "%s/%s", s_data->m_rootDir, setName);

	uePath path;
	ueAssets_GetAssetPath(path, fullSetName, "loc.xml");

	ioXmlDoc* doc = ioXmlDoc_Load(s_data->m_allocator, path);
	if (!doc)
	{
		ueLogE("Failed to load locale set '%s' (path = '%s')", setName, path);
		return UE_FALSE;
	}
	ioXmlDocScopedDestructor docDestructor(doc);

	// Calculate memory required

	const u32 alignedLocaleSetSize = ueAlignPow2((u32) sizeof(ueLocaleSet), (u32) UE_DEFAULT_ALIGNMENT);

	const u32 setNameLength = ueStrLen(setName);
	u32 memorySize = alignedLocaleSetSize + setNameLength + 1;
	u32 numEntries = 0;

	ioXmlNode* setNode = ioXmlDoc_GetFirstNode(doc, "translationSet");
	if (!setNode)
		return UE_FALSE;

	for (ioXmlNode* groupNode = ioXmlNode_GetFirstNode(setNode, "group"); groupNode; groupNode = ioXmlNode_GetNext(groupNode, "group"))
	{
		const char* groupName = ioXmlNode_GetAttrValue(groupNode, "name");
		u32 groupNameSize = ueStrLen(groupName);
		if (groupNameSize > 0)
			groupNameSize++; // Add '.' character between group name and text name

		for (ioXmlNode* translationNode = ioXmlNode_GetFirstNode(groupNode, "translation"); translationNode; translationNode = ioXmlNode_GetNext(translationNode, "translation"))
		{
			numEntries++;

			const char* name = ioXmlNode_GetAttrValue(translationNode, "name");
			const char* value = ioXmlNode_GetAttrValue(translationNode, "value");
			if (!name || !value)
			{
				ueLogE("Failed to load locale set '%s' (path = '%s'), reason: contains translation without valid name and value.", setName, path);
				return UE_FALSE;
			}

			memorySize += groupNameSize + ueStrLen(name) + 1;
			memorySize += ueStrLen(value) + 1;
		}
	}

	const ueSize hashMapSize = ueLocaleSet::MapType::CalcMemReq(numEntries);
	memorySize += (u32) hashMapSize;

	// Create locale set

	u8* memory = (u8*) s_data->m_allocator->Alloc(memorySize);
	UE_ASSERT(memory);

	ueLocaleSet* set = new(memory) ueLocaleSet();
	memory += alignedLocaleSetSize;

	set->m_strings.InitMem(memory, hashMapSize, numEntries);
	memory += hashMapSize;

	set->m_name = (char*) memory;
	memory += setNameLength + 1;
	ueMemCpy(set->m_name, setName, setNameLength + 1);

	// Build final hashmap entries

	for (ioXmlNode* groupNode = ioXmlNode_GetFirstNode(setNode, "group"); groupNode; groupNode = ioXmlNode_GetNext(groupNode, "group"))
	{
		const char* groupName = ioXmlNode_GetAttrValue(groupNode, "name");
		u32 groupNameSize = ueStrLen(groupName);
		if (groupNameSize > 0)
			groupNameSize++; // Add '.' character between group name and text name

		for (ioXmlNode* translationNode = ioXmlNode_GetFirstNode(groupNode, "translation"); translationNode; translationNode = ioXmlNode_GetNext(translationNode, "translation"))
		{
			const char* name = ioXmlNode_GetAttrValue(translationNode, "name");
			const u32 nameLength = ueStrLen(name);

			char* namePtr = (char*) memory;
			memory += groupNameSize;
			if (groupNameSize > 0)
			{
				ueMemCpy(namePtr, groupName, groupNameSize - 1);
				namePtr[groupNameSize - 1] = '.';
			}

			memory += nameLength + 1;
			ueMemCpy(namePtr + groupNameSize, name, nameLength + 1);

			const char* value = ioXmlNode_GetAttrValue(translationNode, "value");
			const u32 valueLength = ueStrLen(value);
			char* valuePtr = (char*) memory;
			memory += valueLength + 1;
			ueMemCpy(valuePtr, value, valueLength + 1);

#if !defined(UE_FINAL)
			const char** existingValuePtr = set->m_strings.Find(namePtr);
			if (existingValuePtr)
				ueLogW("Locale set '%s' contains duplicate entry '%s'; previously set to '%s', now to '%s'\n", setName, name, *existingValuePtr, value);
#endif
			set->m_strings.Insert(namePtr, valuePtr);
		}
	}

	// Insert set

	s_data->m_sets.Insert(setName, set);

	return UE_TRUE;
}

void ueLocaleMgr_UnloadSet(const char* setName)
{
	UE_ASSERT(s_data);

	ueLocaleSet** setPtr = s_data->m_sets.Find(setName);
	if (!setPtr)
	{
		ueLogE("Failed to unload locale set '%s', reason: set doesn't exist.", setName);
		return;
	}
	s_data->m_sets.Remove((*setPtr)->m_name);
	s_data->m_allocator->Free(*setPtr);
}

const char* ueLocaleMgr_Localize(const char* name, u32 numParams, const ueLocaleParam* params)
{
	UE_ASSERT(s_data);

	// Extract set name

	char setName[256];
	const s32 dotIndex = ueStrFirstIndexOf(name, '.');
	if (dotIndex == -1)
	{
		ueLogE("Failed to localize string '%s', reason: failed to extract set name (no '.' character found).", name);
		return name;
	}
	ueStrNCpy(setName, UE_ARRAY_SIZE(setName), name, dotIndex);

	name += dotIndex + 1;

	// Find set

	ueLocaleSet** setPtr = s_data->m_sets.Find(setName);
	if (!setPtr)
	{
		ueLogE("Failed to localize string '%s', reason: set '%s' doesn't exist.", name, setName);
		return name;
	}

	// Find value

	const char** valuePtr = (*setPtr)->m_strings.Find(name);
	if (!valuePtr)
	{
		ueLogE("Failed to localize string '%s' from set '%s', reason: string doesn't exist.", name, setName);
		return name;
	}

	// Return immediately if no parameters

	if (numParams == 0)
		return *valuePtr;

	// Fill in the parameters

	static char buffer[1 << 12];
	char* dst = (char*) buffer;
	u32 dstLen = 0;

	const char* src = *valuePtr;

	while (*src)
	{
		if (*src == '[')
		{
			src++;

			// Parse parameter name

			const char* paramStart = src;
			while (*src && *src != ']')
				src++;
			if (*src != ']')
			{
				ueLogE("Failed to localize string '%s' from set '%s', reason: invalid parameter.", name, setName);
				return name;
			}

			// Look up parameter in user supplied array

			const ueLocaleParam* param = NULL;
			for (u32 i = 0; i < numParams; i++)
				if (!ueStrNCmp(params[i].m_name, paramStart, (u32) (ueSize) (src - paramStart)))
				{
					param = &params[i];
					break;
				}
			
			src++; // Skip ']'

			// Fill in parameter value

			if (!param)
				ueLogE("Failed to localize string '%s' from set '%s', reason: parameter not found.", name, setName);
			else
			{
				const char* paramValue = param->m_value;
				char intAsStringBuffer[32];
				if (!paramValue)
				{
					ueStrFromS32(intAsStringBuffer, UE_ARRAY_SIZE(intAsStringBuffer), param->m_valueInt);
					paramValue = intAsStringBuffer;
				}
				while (*paramValue)
					*(dst++) = *(paramValue++);
			}
		}
		else
			*(dst++) = *(src++);
	}

	*dst = 0;

	return buffer;
}
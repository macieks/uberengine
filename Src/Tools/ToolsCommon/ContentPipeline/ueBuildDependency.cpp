#include "ContentPipeline/ueContentPipeline.h"
#include "IO/ioXml.h"
#include "IO/ioFile.h"
#include "ContentPipeline/ueAssetParams.h"

#define UE_BUILD_DEP_DATE_FORMAT "yyyy-MM-dd hh:mm:ss"

void ueBuildDependency::AddCurrentExecutable()
{
    Add(io_get_exe_path().c_str());
}

void ueBuildDependency::AddExecutable(const char* fileName)
{
	ioPath exePath;
	ioPath_ExtractDirectoryName(io_get_exe_path().c_str(), exePath, UE_ARRAY_SIZE(exePath));
	ueStrCatS(exePath, "\\");
	ueStrCatS(exePath, fileName);
	Add(exePath);
}

void ueBuildDependency::AddAssetParamFiles(const ueAssetParams& params)
{
	for (u32 i = 0; i < params.m_assetTypeHierarchy.size(); i++)
		Add(params.m_assetTypeHierarchy[i].c_str());
}

void ueBuildDependency::Add(const char* path)
{
	Dependency* dep = NULL;
	for (u32 i = 0; i < m_deps.size(); i++)
		if (m_deps[i].m_path == path)
		{
			dep = &m_deps[i];
			break;
		}
	if (!dep)
		dep = &vector_push(m_deps);

	// Get absolute path

	if (!io_get_full_path(dep->m_path, path))
		dep->m_path = path; // Fallback to relative path

	// Get file modification date & time

	ioFileAttrs attrs;
	attrs.m_flags = ioFileAttrFlags_ModificationDate;
	if (!ioFile_GetAttrs(path, attrs))
	{
		ueDateTime::GetCurrent(dep->m_date);
		return;
	}

	dep->m_date = attrs.m_modificationDate;
}

bool ueBuildDependency::Save(const char* rootDepDir, const char* name, const char* config)
{
	ioXmlDoc* doc = ioXmlDoc_Create(ueAllocator::GetGlobal());
	ioXmlDocScopedDestructor docDestructor(doc);

	ioXmlNode* dependenciesNode = ioXmlDoc_AddNode(doc, "dependencies");
	for (u32 i = 0; i < m_deps.size(); i++)
	{
		Dependency* dep = &m_deps[i];

		char dateString[64];
		dep->m_date.ToString(dateString, UE_ARRAY_SIZE(dateString), UE_BUILD_DEP_DATE_FORMAT);

		ioXmlNode* dependencyNode = ioXmlNode_AddNode(dependenciesNode, "dependency");
		ioXmlNode_AddAttr(dependencyNode, "path", dep->m_path.c_str());
		ioXmlNode_AddAttr(dependencyNode, "date", dateString);
	}

	char path[512];
	ueStrFormatS(path, "%s/%s/%s.deps.xml", rootDepDir, config, name);
	return io_create_dir_chain(path) && ioXmlDoc_Save(doc, path);
}

#include "ueMaterialMapping.h"
#include "ModelCompiler_Common/ueToolModel.h"
#include "IO/ioXml.h"

void ueMaterialMapping::Create(ueToolModel* model, const char* defaultMaterialName)
{
	m_materialToMaterialMappings.clear();
	m_meshToMaterialMappings.clear();
	m_modelName = model->m_name;

	Update(model, defaultMaterialName);
}

void ueMaterialMapping::Update(ueToolModel* model, const char* defaultMaterialName)
{
	for (u32 i = 0; i < model->m_lods.size(); i++)
	{
		const ueToolModel::LOD& lod = model->m_lods[i];
		for (u32 j = 0; j < lod.m_materials.size(); j++)
		{
			const ueToolModel::Material& srcMat = lod.m_materials[j];
			DstMaterial& dstMat = m_materialToMaterialMappings[srcMat.m_name];
			dstMat.m_name = defaultMaterialName;
			dstMat.m_ignoreSettings = false;
		}
	}
}

void ueMaterialMapping::Apply(ueToolModel* model)
{
	for (u32 i = 0; i < model->m_lods.size(); i++)
	{
		ueToolModel::LOD& lod = model->m_lods[i];

		// Remap materials used by meshes (and determine used material indices)

		std::set<u32> usedMaterials;

		for (u32 j = 0; j < lod.m_meshes.size(); j++)
		{
			ueToolModel::Mesh& srcMesh = lod.m_meshes[j];
			const DstMaterial* existingMapping = map_find(m_meshToMaterialMappings, srcMesh.m_name);
			if (!existingMapping)
			{
				if (srcMesh.m_materialIndex != U32_MAX)
					usedMaterials.insert(srcMesh.m_materialIndex);
				continue;
			}

			ueToolModel::Material mat;
			mat.m_name = existingMapping->m_name;
			srcMesh.m_materialIndex = lod.GetAddMaterial(mat);

			usedMaterials.insert(srcMesh.m_materialIndex);
		}

		// Remove unused materials

		std::map<u32, u32> oldToNewMaterial;

		for (u32 j = 0; j < lod.m_materials.size();)
			if (!map_find(oldToNewMaterial, j))
			{
				vector_remove_at(lod.m_materials, j);
				oldToNewMaterial[(u32) lod.m_materials.size()] = j;
			}
			else
				j++;

		// Reindex references to materials from meshes

		for (u32 j = 0; j < lod.m_meshes.size(); j++)
		{
			ueToolModel::Mesh& srcMesh = lod.m_meshes[j];
			const u32* newMaterialIndex = map_find(oldToNewMaterial, srcMesh.m_materialIndex);
			if (newMaterialIndex)
				srcMesh.m_materialIndex = *newMaterialIndex;
		}
	}
}

bool ueMaterialMapping::Load(const char* rootSrcDir, const char* modelName)
{
	m_materialToMaterialMappings.clear();
	m_meshToMaterialMappings.clear();
	m_modelName = modelName;

	// Load Xml doc

	char path[512];
	ueStrFormatS(path, "%s/%s.mats-mapping.xml", rootSrcDir, modelName);

	ioXmlDoc* doc = ioXmlDoc_Load(ueAllocator::GetGlobal(), path);
	if (!doc)
	{
		ueLogI("Model material mapping doesn't exist (model name = '%s'; mapping path = '%s')", modelName, path);
		return false;
	}
	ioXmlDocScopedDestructor docDestructor(doc);

	ioXmlNode* materialMappingNode = ioXmlDoc_GetFirstNode(doc, "materialMapping");
	if (!materialMappingNode)
	{
		ueLogW("Invalid material mapping: missing 'materialMapping' Xml element.");
		return false;
	}

	// Get material -> material mappings

	ioXmlNode* materialToMaterialMappingsNode = ioXmlNode_GetFirstNode(materialMappingNode, "materialToMaterialMappings");
	if (!materialToMaterialMappingsNode)
	{
		ueLogW("Invalid material mapping: missing 'materialToMaterialMappingsNode' Xml element.");
		return false;
	}
	for (ioXmlNode* mappingNode = ioXmlNode_GetFirstNode(materialToMaterialMappingsNode, "mapping"); mappingNode; mappingNode = ioXmlNode_GetNext(mappingNode, "mapping"))
	{
		const char* src = ioXmlNode_GetAttrValue(mappingNode, "src");
		const char* dst = ioXmlNode_GetAttrValue(mappingNode, "dst");
		const char* ignoreSettings = ioXmlNode_GetAttrValue(mappingNode, "ignoreSettings");
		if (!src || !dst || !ignoreSettings)
		{
			ueLogW("Invalid mapping from source material (name = '%s') to destination material (name = '%s') with ignore settings value = '%s' in material mapping file (path = '%s')", src, dst, ignoreSettings, path);
			continue;
		}

		DstMaterial& dstMat = m_materialToMaterialMappings[src];
		dstMat.m_name = dst;
		ueStrToBool(ignoreSettings, dstMat.m_ignoreSettings);
	}

	// Get mesh -> material mappings

	ioXmlNode* meshToMaterialMappingsNode = ioXmlNode_GetFirstNode(materialMappingNode, "meshToMaterialMappings");
	if (!meshToMaterialMappingsNode)
	{
		ueLogW("Invalid material mapping: missing 'meshToMaterialMappingsNode' Xml element.");
		return false;
	}
	for (ioXmlNode* mappingNode = ioXmlNode_GetFirstNode(meshToMaterialMappingsNode, "mapping"); mappingNode; mappingNode = ioXmlNode_GetNext(mappingNode, "mapping"))
	{
		const char* src = ioXmlNode_GetAttrValue(mappingNode, "src");
		const char* dst = ioXmlNode_GetAttrValue(mappingNode, "dst");
		const char* ignoreSettings = ioXmlNode_GetAttrValue(mappingNode, "ignoreSettings");
		if (!src || !dst || !ignoreSettings)
		{
			ueLogW("Invalid mapping from source mesh (name = '%s') to destination material (name = '%s') with ignore settings value = '%s' in material mapping file (path = '%s')", src, dst, ignoreSettings, path);
			continue;
		}

		DstMaterial& dstMat = m_meshToMaterialMappings[src];
		dstMat.m_name = dst;
		ueStrToBool(ignoreSettings, dstMat.m_ignoreSettings);
	}

	return true;
}

bool ueMaterialMapping::Save(const char* rootSrcDir)
{
	// Create Xml doc

	ioXmlDoc* doc = ioXmlDoc_Create(ueAllocator::GetGlobal());
	UE_ASSERT(doc);
	ioXmlDocScopedDestructor docDestructor(doc);

	ioXmlNode* materialMappingNode = ioXmlDoc_AddNode(doc, "materialMapping");

	// Create material -> material mappings

	ioXmlNode* materialToMaterialMappingsNode = ioXmlNode_AddNode(materialMappingNode, "materialToMaterialMappings");
	for (std::map<std::string, DstMaterial>::iterator i = m_materialToMaterialMappings.begin(); i != m_materialToMaterialMappings.end(); i++)
	{
		ioXmlNode* mappingNode = ioXmlNode_AddNode(materialToMaterialMappingsNode, "mapping");
		ioXmlNode_AddAttr(mappingNode, "src", i->first.c_str());
		ioXmlNode_AddAttr(mappingNode, "dst", i->second.m_name.c_str());
		ioXmlNode_AddAttr(mappingNode, "ignoreSettings", ueStrFromBool(i->second.m_ignoreSettings));
	}

	// Create mesh -> material mappings

	ioXmlNode* meshToMaterialMappingsNode = ioXmlNode_AddNode(materialMappingNode, "meshToMaterialMappings");
	for (std::map<std::string, DstMaterial>::iterator i = m_meshToMaterialMappings.begin(); i != m_meshToMaterialMappings.end(); i++)
	{
		ioXmlNode* mappingNode = ioXmlNode_AddNode(meshToMaterialMappingsNode, "mapping");
		ioXmlNode_AddAttr(mappingNode, "src", i->first.c_str());
		ioXmlNode_AddAttr(mappingNode, "dst", i->second.m_name.c_str());
		ioXmlNode_AddAttr(mappingNode, "ignoreSettings", ueStrFromBool(i->second.m_ignoreSettings));
	}

	// Save

	char path[512];
	ueStrFormatS(path, "%s/%s.mats-mapping.xml", rootSrcDir, m_modelName.c_str());

	if (!ioXmlDoc_Save(doc, path))
	{
		ueLogI("Failed to save model material mapping (model name = '%s'; mapping path = '%s')", m_modelName.c_str(), path);
		return UE_FALSE;
	}
	return UE_TRUE;
}

void ueMaterialMapping::AddDependecies(const char* rootSrcDir, ueBuildDependency* dep)
{
	char path[512];
	ueStrFormatS(path, "%s/%s.mats-mapping.xml", rootSrcDir, m_modelName.c_str());
	dep->Add(path);
}

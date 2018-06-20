#pragma once

#include "ContentPipeline/ueContentPipeline.h"

struct ueToolModel;

//! Mapping between model and materials
struct ueMaterialMapping
{
	struct DstMaterial
	{
		std::string m_name;
		bool m_ignoreSettings;
	};

	std::string m_modelName;

	std::map<std::string, DstMaterial> m_materialToMaterialMappings;
	std::map<std::string, DstMaterial> m_meshToMaterialMappings;

	void Create(ueToolModel* model, const char* defaultMaterialName = "common/default_material");
	void Update(ueToolModel* model, const char* defaultMaterialName = "common/default_material");
	void Apply(ueToolModel* model);
	bool Load(const char* rootSrcDir, const char* modelName);
	bool Save(const char* rootSrcDir);

	void AddDependecies(const char* rootSrcDir, ueBuildDependency* dep);
};
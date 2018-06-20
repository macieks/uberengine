#pragma once

//! List of files with timestamps on which some build depends
class ueBuildDependency
{
public:
	void AddCurrentExecutable();
	void AddExecutable(const char* fileName);
	void AddAssetParamFiles(const ueAssetParams& params);
	void Add(const char* path);
	bool Save(const char* rootDepDir, const char* name, const char* config);

private:
	struct Dependency
	{
		std::string m_path;
		ueDateTime m_date;
	};
	std::vector<Dependency> m_deps;
};
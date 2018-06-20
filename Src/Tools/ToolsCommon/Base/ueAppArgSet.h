#pragma once

class ueAppArgSet
{
public:
	bool Init(u32 count, char** args);
	bool GetArg(const char* name, const char*& value);
	bool HasArg(const char* name);
	u32 GetSize() const { return (u32) m_args.size(); }

private:
	std::vector<std::string> m_values;
	std::map<std::string, u32> m_args;
};
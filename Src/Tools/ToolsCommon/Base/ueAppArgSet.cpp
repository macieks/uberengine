#include "Base/ueToolsBase.h"

bool ueAppArgSet::Init(u32 count, char** args)
{
	m_args.clear();
	m_values.clear();

	u32 i = 0;
    while (i < count)
    {
        const char* name = args[i];
        if (name[0] != '-')
		{
            ueLogE("Unexpected parameter '%s'", name);
			return false;
		}

        i++;
        const u32 firstValueIndex = (u32) m_values.size();
        while (i < count && args[i][0] != '-')
        {
            m_values.push_back(args[i]);
            i++;
        }

        m_args[name + 1] = firstValueIndex;
    }
	return true;
}

bool ueAppArgSet::GetArg(const char* name, const char*& value)
{
	std::map<std::string, u32>::iterator iter = m_args.find(name);
	if (iter == m_args.end())
		return false;
	value = m_values[iter->second].c_str();
	return true;
}

bool ueAppArgSet::HasArg(const char* name)
{
	return m_args.find(name) != m_args.end();
}
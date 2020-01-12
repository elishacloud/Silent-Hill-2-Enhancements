#include <string>
#include <string_view>
#include <Windows.h>
#include <sstream>
#include <fstream>
#include "wsfExternal.h"

class CIniReader
{
private:
    std::string m_szFileName;

public:
    CIniReader() {}
    CIniReader(std::string_view) {}
    CIniReader(std::stringstream&) {}

	const std::string& GetIniPath()
	{
		if (!m_szFileName.size())
		{
			// Get config file path
			char configpath[MAX_PATH];
			GetModuleFileNameA(m_hModule, configpath, MAX_PATH);
			strcpy_s(strrchr(configpath, '.'), MAX_PATH - strlen(configpath), ".ini");
			m_szFileName.assign(configpath);
		}
		return m_szFileName;
	}

	bool ReadBoolean(std::string_view szSection, std::string_view szKey, bool bolDefaultValue)
	{
		return (ReadInteger(szSection, szKey, bolDefaultValue) != 0);
	}

	int ReadInteger(std::string_view szSection, std::string_view szKey, int iDefaultValue);

	std::string ReadString(std::string_view, std::string_view, std::string_view)
	{
		return m_szFileName;
	}

	void WriteBoolean(std::string_view, std::string_view, bool, bool useparser = false)
	{
		UNREFERENCED_PARAMETER(useparser);
	}

	void WriteInteger(std::string_view, std::string_view, int, bool useparser = false)
	{
		UNREFERENCED_PARAMETER(useparser);
	}

	void WriteString(std::string_view, std::string_view, std::string_view, bool useparser = false)
	{
		UNREFERENCED_PARAMETER(useparser);
	}
};

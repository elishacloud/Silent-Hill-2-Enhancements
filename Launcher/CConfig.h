#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <string>
#include <algorithm>
#include "External\tinyxml2\tinyxml2.h"
#include "External\xxHash\xxh3.h"
#include "Common\Unicode.h"

struct EXTRAOPTIONS
{
	std::string Name;
	std::string Value;
};

struct Strings
{
	const char* name;
	const char* def;
};

extern std::vector<EXTRAOPTIONS> ExtraOptions;

extern bool ShowSandboxWarning;
extern bool DisableTabOverloadWarning;
extern DWORD DefaultLang;

class CConfig;

using namespace tinyxml2;

static XXH64_hash_t Hash64(std::string name)
{
	// Ensure string is at least 64 characters long
	std::string str(name + "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz");
	return XXH64(str.c_str(), max(name.size(), 64), 0);
}

class CHashString
{
public:
	operator const char* () { return str.c_str(); }
	operator std::string() { return str; }
	operator std::string& () { return str; }

	void operator = (const char* s)
	{
		if (s == nullptr)
			str = std::string("");
		else str = std::string(s);

		GenerateHash();
	}

	void operator = (std::string s)
	{
		str = s;
		GenerateHash();
	}

	// simulate strcmp
	int compare(std::string s)
	{
		if (Hash64(str.c_str()) == hash)
			return 0;

		return 1;
	}

	int compare(const char* s)
	{
		if (Hash64(s) == hash)
			return 0;

		return 1;
	}

private:
	void GenerateHash()
	{
		hash = Hash64(str.c_str());
	}

	std::string str;
	XXH64_hash_t hash;
};

class CConfigValue
{
public:
	void Parse(XMLElement& xml, CConfig& cfg);

	std::string name;		// default visualized entry name
	std::string id;			// string id
	std::string val;		// value in the ini
	bool is_default;		// if this is true, this index is the default value
};

class CConfigOption
{
public:
	void Parse(XMLElement& xml, CConfig& cfg);
	void SetValueFromName(const char* vname)
	{
		if (type == TYPE_TEXT)
		{
			value[cur_val].val.assign(vname);
			return;
		}
		for (size_t i = 0, si = value.size(); i < si; i++)
		{
			if (value[i].val.compare(vname) == 0)
			{
				cur_val = (int)i;
				return;
			}
		}

		// nothing found, load default
		SetValueDefault();
	}
	void SetValueDefault()
	{
		// nothing found, load default
		for (size_t i = 0, si = value.size(); i < si; i++)
		{
			if (value[i].is_default)
			{
				if (type == TYPE_TEXT)
				{
					value[cur_val].val = value[i].val;
				}
				else
				{
					cur_val = (int)i;
				}
				return;
			}
		}
	}
	std::string GetDefaultValue()
	{
		// nothing found, load default
		for (auto& item : value)
		{
			if (item.is_default)
			{
				return item.val;
			}
		}
		return std::string("");
	}

	std::string name;		// option name
	std::string id, desc;	// string references
	UINT type;				// uses TYPE table to determine the control to use

	enum TYPE
	{
		TYPE_UNK,	// undefined behavior
		TYPE_CHECK,	// checkbox with true or false
		TYPE_LIST,	// list of options
		TYPE_TEXT,	// textbox
		TYPE_HIDE,	// hidden options
	};

	std::vector<CConfigValue> value;	// values for this option
	int cur_val;	// current value index
};

class CConfigSection
{
public:
	void Parse(XMLElement& xml, CConfig& cfg);
	void SetValueFromName(const char* section, const char* value)
	{
		for (auto& item : option)
		{
			if (item.name.compare(section) == 0)
			{
				item.SetValueFromName(value);
				return;
			}
		}
	}

	std::string name;	// section name
	std::vector<CConfigOption> option;	// options for this section
	std::string id;		// string reference
	bool extra;	// extra options flag
};

// string pool
class CConfigStrings
{
public:
	class CConfigString
	{
	public:
		XXH64_hash_t hash = NULL;
		std::string str;
	};

	void PushString(const char* multis, const char* id)
	{
		CConfigString s;
		s.str = SAFESTR(multis);
		s.hash = Hash64(id);

		str.push_back(s);
	}

	static bool sort(CConfigString& a, CConfigString& b)
	{
		return a.hash < b.hash;
	}

	int quickfind(XXH64_hash_t hash)
	{
		int first = 0,
			last = (int)str.size() - 1,
			middle = last / 2;

		while (first <= last)
		{
			if (str[middle].hash < hash)
				first = middle + 1;
			else if (str[middle].hash == hash)
				return middle;
			else last = middle - 1;

			middle = (first + last) / 2;
		}

		return -1;
	}

	void Sort()
	{
		std::sort(str.begin(), str.end(), sort);
	}

	std::string Find(XXH64_hash_t id)
	{
		auto f = quickfind(id);
		if (f >= 0) return str[f].str;

		return std::string("");
	}

	std::string Find(std::string id)
	{
		auto hash = Hash64(id.c_str());

		auto f = quickfind(hash);
		if (f >= 0) return str[f].str;

		return std::string("");
	}

	std::vector<CConfigString> str;
};

// group manager
class CConfigGroup
{
public:
	class CConfigSubOpt
	{
	public:
		void Set(std::string section, std::string option)
		{
			sec = Hash64(section.c_str());
			op = Hash64(option.c_str());
		}

		XXH64_hash_t sec, op;		// section and option from the <Sections> table
									// uses hashes to make this easier to search
	};
	class CConfigSub
	{
	public:
		void Parse(XMLElement& xml);
		void ParseTab(XMLElement& xml);
		std::vector<CConfigSubOpt> opt;
		std::string id;
	};
	void Parse(XMLElement& xml);
	void ParseTab(XMLElement& xml, CConfig& cfg);

	std::string id;					// string reference id
	std::vector<CConfigSub> sub;	// list of sub options for this group
};

class CConfig
{
public:
	bool ParseXml();
	void SetDefault();
	const char* SetIDString(const char* id, const char* name);
	void BuildCacheP();
	void SetFromIni(LPCWSTR lpName);
	void SaveIni(LPCWSTR lpName, LPCWSTR error_mes, LPCWSTR error_caption);
	bool IsSettingInXml(std::string setting);
	std::string GetDefaultSetting(std::string name);
	bool IsVisibleSetting(std::string name);
	bool IsHiddenSetting(std::string name);
	void CheckAllXmlSettings(LPCWSTR error_caption);

	std::vector<CConfigSection> section;	// ini hierarchy used to parse and rebuild d3d8.ini
	std::vector<CConfigGroup> group;		// ini groups, represented as tabs on interface (it's only for grouping, doesn't influence ini structure)
	CConfigStrings string;					// unicode string pool
	std::string Preface;

	bool FindSectionAndOption(XXH64_hash_t ss, XXH64_hash_t sh, int &found_sec, int &found_opt)
	{
		for (size_t i = 0, si = section.size(); i < si; i++)
		{
			auto xs = Hash64(section[i].name.c_str());
			for (size_t j = 0, sj = section[i].option.size(); j < sj; j++)
			{
				auto xh = Hash64(section[i].option[j].name.c_str());
				if (xs == ss && xh == sh)
				{
					found_sec = (int)i;
					found_opt = (int)j;
					return true;
				}
			}
		}
		found_sec = 0;
		found_opt = 0;
		return false;
	}
	int FindAndGetValue(std::string name)
	{
		for (size_t i = 0, si = section.size(); i < si; i++)
		{
			for (size_t j = 0, sj = section[i].option.size(); j < sj; j++)
			{
				if (name == section[i].option[j].name)
				{
					std::string val = section[i].option[j].value[section[i].option[j].cur_val].val;
					if (!val.empty() && std::all_of(val.begin(), val.end(), ::isdigit))
					{
						return atoi(val.c_str());
					}
				}
			}
		}
		return 0;
	}

	std::wstring GetSectionString(int sec);
	std::wstring GetOptionString(int sec, int opt);
	std::wstring GetOptionDesc(int sec, int opt);
	std::wstring GetValueString(int sec, int opt, int val);

	std::wstring GetGroupString(int sec);
	std::wstring GetGroupLabel(int sec, int sub);

	std::string GetString(const char* name);
};

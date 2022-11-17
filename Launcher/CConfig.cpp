/**
* Copyright (C) 2022 Gemini
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*
* Code taken from: https://github.com/Gemini-Loboto3/SH2config
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <sstream>
#include <fstream>
#include <psapi.h>
#include <regex>
#include "CConfig.h"
#include "Resource.h"
#include "Common\AutoUpdate.h"
#include "Common\Settings.h"

struct DUALSTRINGS
{
	std::string name;
	std::string val;
};

#define DECLARE_ALL_SETTINGS(name, val) \
	{ std::string(#name), std::string(#val) },

#define DECLARE_HIDDEN_SETTINGS(name) \
	std::string(#name),

DUALSTRINGS AllValues[] = { VISIT_ALL_SETTING(DECLARE_ALL_SETTINGS) };
std::string HiddenValues[] = { VISIT_HIDDEN_SETTING(DECLARE_HIDDEN_SETTINGS) };

extern bool bIsCompiling;

bool DisableMissingSettingsWarning = false;
bool DisableExtraSettingsWarning = false;
bool DisableDefaultValueWarning = false;
bool DisableXMLErrorWarning = false;
bool DisableTabOverloadWarning = false;

struct cb_parse
{
	std::vector<CConfigOption*> list;
	std::string error;
};

cb_parse p;

/////////////////////////////////////////////////
// Various helpers
std::string SAFESTR(const char *X)
{
	if (!X)
		return std::string("");

	return std::string(X);
}

// create a wide string from utf8
wchar_t* MultiToWide(const char* multi)
{
	// gather size of the new string and create a buffer
	int size = MultiByteToWideChar(CP_UTF8, 0, multi, -1, NULL, 0);
	wchar_t* wide = new wchar_t[size];
	// fill allocated string with converted data
	MultiByteToWideChar(CP_UTF8, 0, multi, -1, wide, size);

	return wide;
}

// create an std::wstring from utf8
std::wstring MultiToWide_s(const char* multi)
{
	if (!multi)
		return std::wstring(L"");

	wchar_t* wide = MultiToWide(multi);
	std::wstring wstr = wide;
	delete[] wide;

	return wstr;
}

// crate an std::wstring from utf8 str::string
std::wstring MultiToWide_s(std::string multi)
{
	return MultiToWide_s(multi.c_str());
}

// gets the name or tip for a value
inline const char* GetNameValue(const char* name, const char* tip)
{
	return name ? name : tip;
}

inline bool CheckOptValue(CConfigOption &opt)
{
	if ((opt.type == CConfigOption::TYPE::TYPE_CHECK && opt.value.size() == 2) ||
		(opt.type == CConfigOption::TYPE::TYPE_LIST && opt.value.size() > 1) ||
		(opt.type == CConfigOption::TYPE::TYPE_TEXT && opt.value.size() == 2) ||
		opt.type == CConfigOption::TYPE::TYPE_HIDE)
	{
		return true;
	}

	return false;
}

// get xml from resource
bool GetXMLfromResoruce(XMLDocument &xml)
{
	HRSRC hResource = FindResource(m_hModule, MAKEINTRESOURCE(DefaultLang), RT_RCDATA);
	if (hResource)
	{
		HGLOBAL hLoadedResource = LoadResource(m_hModule, hResource);
		if (hLoadedResource)
		{
			LPVOID pLockedResource = LockResource(hLoadedResource);
			if (pLockedResource)
			{
				DWORD dwResourceSize = SizeofResource(m_hModule, hResource);
				if (dwResourceSize)
				{
					if (!xml.Parse((char*)pLockedResource, dwResourceSize))
					{
						return false;
					}
				}
			}
		}
	}
	return true;
}

std::string GetProcessNameXml()
{
	char path[MAX_PATH] = { '\0' };
	bool ret = (GetProcessImageFileNameA(GetCurrentProcess(), path, MAX_PATH) != 0);
	char* pdest = strrchr(path, '.');
	if (ret && pdest)
	{
		strcpy_s(pdest, MAX_PATH - strlen(path), ".xml");
		pdest = strrchr(path, '\\');
		if (pdest)
		{
			return std::string(pdest + 1);
		}
	}

	return std::string("");
}

/////////////////////////////////////////////////
// Actual configuration
bool CConfig::ParseXml()
{
	XMLDocument xml;
	if (xml.LoadFile(GetProcessNameXml().c_str()) && GetXMLfromResoruce(xml))
		return true;

	auto root = xml.RootElement();

	auto e = root->FirstChildElement("Error");
	if (e)
	{
		// Sample of xml for this
		// <Error DisableMissingSettingsWarning="1" DisableExtraSettingsWarning="1" DisableDefaultValueWarning="1" DisableXMLErrorWarning="1" DisableTabOverloadWarning="1" />
		DisableMissingSettingsWarning = SetValue(e->Attribute("DisableMissingSettingsWarning"));
		DisableExtraSettingsWarning = SetValue(e->Attribute("DisableExtraSettingsWarning"));
		DisableDefaultValueWarning = SetValue(e->Attribute("DisableDefaultValueWarning"));
		DisableXMLErrorWarning = SetValue(e->Attribute("DisableXMLErrorWarning"));
		DisableTabOverloadWarning = SetValue(e->Attribute("DisableTabOverloadWarning"));
	}

	auto i = root->FirstChildElement("Ini");
	if (i)
	{
		auto pre = i->FirstChildElement("Preface");
		if (pre)
		{
			Preface.assign(pre->GetText());
		}
	}

	auto t = root->FirstChildElement("Tab");
	while (t)
	{
		auto sec = t->FirstChildElement("Section");

		CConfigGroup gg;
		gg.ParseTab(*t, *this);
		group.push_back(gg);

		while (sec)
		{
			CConfigSection ssec;
			ssec.Parse(*sec, *this);
			section.push_back(ssec);

			sec = sec->NextSiblingElement("Section");
		}
		t = t->NextSiblingElement("Tab");
	}

	auto s = root->FirstChildElement("Sections");
	if (s)
	{
		auto sec = s->FirstChildElement("Section");
		while (sec)
		{
			CConfigSection ssec;
			ssec.Parse(*sec, *this);
			section.push_back(ssec);

			sec = sec->NextSiblingElement("Section");
		}
	}

	auto g = root->FirstChildElement("Groups");
	if (g)
	{
		auto gp = g->FirstChildElement("Group");
		while (gp)
		{
			CConfigGroup gg;
			gg.Parse(*gp);
			group.push_back(gg);

			gp = gp->NextSiblingElement("Group");
		}
	}

	s = root->FirstChildElement("Strings");
	if (s)
	{
		auto sec = s->FirstChildElement("S");
		while (sec)
		{
			auto str = SAFESTR(sec->GetText());
			auto id = SAFESTR(sec->Attribute("id"));

			string.PushString(str.c_str(), id.c_str());

			sec = sec->NextSiblingElement("S");
		}
	}
	string.Sort();
	return false;
}

void CConfig::SetDefault()
{
	for (auto &sec: section)
		for (auto &opt : sec.option)
			opt.SetValueDefault();
}

const char* CConfig::SetIDString(const char* id, const char* name)
{
	if (id)
		return id;

	if (name)
		string.PushString(name, name);

	return name;
}

void CConfig::BuildCacheP()
{
	for (auto &sec : section)
		for (auto &opt : sec.option)
			p.list.push_back(&opt);
}

void __stdcall ParseIniCallback(char* lpName, char* lpValue, void *lpParam)
{
	// Check for valid entries
	if (!IsValidSettings(lpName, lpValue)) return;

	auto cb = reinterpret_cast<cb_parse*>(lpParam);
	for (auto &item : cb->list)
	{
		if (item->name.compare(lpName) == 0)
		{
			item->SetValueFromName(lpValue);
			return;
		}
	}
}

void CConfig::SetFromIni(LPCWSTR lpName)
{
	// attempt to load the ini
	auto ini = Read(lpName);
	if (ini == nullptr) return;

	// do the parsing (can be slightly slow)
	Parse(ini, ParseIniCallback, (void*)&p);

	// done, disengage!
	free(ini);
}

std::string UpdateDescription(std::string desc)
{
	size_t size = 0;
	do {
		size = desc.size();
		desc = std::regex_replace(desc, std::regex("\n\n"), "\n");
	} while (size != desc.size());
	desc = std::regex_replace(desc, std::regex("\n"), " ");

	// Add line breaks to the description
	//const size_t LineSize = 150;
	//size_t pos = 0;
	//while (false)
	//{
	//	size_t rpos = desc.find("\n", pos);
	//	pos = desc.find(" ", pos + LineSize);
	//	while (rpos < pos && pos != std::string::npos)
	//	{
	//		if (rpos < desc.find(" ", pos + LineSize))
	//			pos = desc.find(" ", rpos + LineSize);
	//		else
	//			break;
	//		rpos = desc.find("\n", pos);
	//	}
	//	if (pos == std::string::npos)
	//		break;
	//	desc.insert(pos, "\n;");
	//}
	return desc;
}

void CConfig::SaveIni(LPCWSTR lpName, LPCWSTR error_mes, LPCWSTR error_caption)
{
	// Read current ini file
	std::wstring name(lpName);
	std::stringstream s_currentini(ReadFileContents(name));

	// New ini file contents
	std::string ini;

	// Add ini preface
	if (Preface.size())
	{
		ini.append(Preface + "\n\n");
	}

	// Write out the rest of the new ini file
	for (auto &sec : section)
	{
		// current section
		ini.append("[" + sec.name + "]\n");
		// write all options
		for (auto &opt : sec.option)
		{
			ini.append("; " + UpdateDescription(opt.desc) + "\n");
			ini.append(opt.name + " = " + opt.value[opt.cur_val].val + "\n\n");
		}
	}

	// Read new ini file
	std::stringstream s_ini(ini);

	// Merge ini files
	std::string newini;
	if (bIsCompiling)
	{
		newini.assign(s_ini.str());
	}
	else
	{
		newini = MergeiniFile(s_currentini, s_ini, true);
	}

	// Write updated ini file
	std::ofstream out(name);
	if (!out)
	{
		MessageBoxW(nullptr, error_mes, error_caption, MB_OK);
		return;
	}
	out << newini;
	out.close();
}

bool CConfig::IsSettingInXml(std::string lpName)
{
	for (auto &item : p.list)
		if (item->name.compare(lpName) == 0)
			return true;

	return false;
}

std::string CConfig::GetDefaultSetting(std::string name)
{
	for (auto &item : AllValues)
		if (item.name.compare(name) == 0)
			return item.val;

	return std::string("");
}

bool CConfig::IsVisibleSetting(std::string name)
{
	for (auto &item : AllValues)
		if (item.name.compare(name) == 0)
			if (!IsHiddenSetting(name))
				return true;

	return false;
}

bool CConfig::IsHiddenSetting(std::string name)
{
	for (auto &item : HiddenValues)
		if (item.compare(name) == 0)
			return true;

	return false;
}

bool CompareSettings(std::string name, std::string setting, std::string xml)
{
	if (setting.compare("0xFFFF") == 0)
		return SetValue(xml.c_str()); // Special handling is needed for 0xFFFF, but for now all the settings default to 'true' or '1'. May need to change this later!
	else if (setting.compare("true") == 0)
		return SetValue(xml.c_str());
	else if (setting.compare("false") == 0)
		return !SetValue(xml.c_str());
	else
		return (setting.compare(xml) == 0);
}

void CConfig::CheckAllXmlSettings(LPCWSTR error_caption)
{
	// Check if XML errors were found during parsing of the file
	if (!DisableXMLErrorWarning)
	{
		// This needs to be at the beginning, before the other checks
		if (!p.error.empty())
		{
			MessageBoxW(nullptr, MultiToWide_s(std::string("XML error found settings:\n\n") + p.error).c_str(), error_caption, MB_OK);
			p.error.clear();
		}

		bool found = false;

		for (size_t s = 0; s < group.size(); s++)
		{
			for (size_t i = 0, si = group[s].sub.size(); i < si; i++)
			{
				if (found || GetGroupLabel(s, (int)i).size() == 0)
				{
					found = true;
					break;
				}

				for (size_t j = 0, sj = group[s].sub[i].opt.size(); j < sj; j++)
				{
					int sec, opt;
					if (!FindSectionAndOption(group[s].sub[i].opt[j].sec, group[s].sub[i].opt[j].op, sec, opt))
					{
						found = true;
						break;
					}
				}
			}

			if (found)
			{
				MessageBoxW(nullptr, L"Error: orphaned XML found!", error_caption, MB_OK);
			}
		}
	}

	// Check for missing settings in xml file
	if (!DisableMissingSettingsWarning)
	{
		for (auto &item : AllValues)
		{
			if (!IsHiddenSetting(item.name) && !IsSettingInXml(item.name))
			{
				p.error += "\"";
				p.error += item.name;
				p.error += "\"\n";
			}
		}

		if (!p.error.empty())
		{
			MessageBoxW(nullptr, MultiToWide_s(std::string("Missing settings:\n\n") + p.error).c_str(), error_caption, MB_OK);
			p.error.clear();
		}
	}

	// Check for extra settings in xml file
	if (!DisableExtraSettingsWarning)
	{
		for (auto &item : p.list)
		{
			if (!IsVisibleSetting(item->name))
			{
				p.error += "\"";
				p.error += item->name;
				p.error += "\"\n";
			}
		}

		if (!p.error.empty())
		{
			MessageBoxW(nullptr, MultiToWide_s(std::string("Extra settings:\n\n") + p.error).c_str(), error_caption, MB_OK);
			p.error.clear();
		}
	}

	// Check default value of settings in xml file
	if (!DisableDefaultValueWarning)
	{
		for (auto &item : p.list)
		{
			std::string defval = GetDefaultSetting(item->name);
			std::string xmldefval = item->GetDefaultValue();
			if (defval.size() && !CompareSettings(item->name, defval, xmldefval))
			{
				p.error += "\"";
				p.error += item->name;
				p.error += "\" ";
				p.error += defval;
				p.error += " > ";
				p.error += xmldefval;
				p.error += "\n";
			}
		}

		if (!p.error.empty())
		{
			MessageBoxW(nullptr, MultiToWide_s(std::string("Default settings mismatch:\n\n") + p.error).c_str(), error_caption, MB_OK);
			p.error.clear();
		}
	}
}

std::wstring CConfig::GetSectionString(int sec)
{
	auto id = string.Find(section[sec].id);
	if (id.size() == 0)
		return MultiToWide_s(section[sec].name);

	return id;
}

std::wstring CConfig::GetGroupString(int sec)
{
	auto id = string.Find(group[sec].id);
	if (id.size() == 0)
		return MultiToWide_s(group[sec].id);

	return id;
}

std::wstring CConfig::GetGroupLabel(int sec, int sub)
{
	auto id = string.Find(group[sec].sub[sub].id);
	if (id.size() == 0)
		return MultiToWide_s(group[sec].sub[sub].id);

	return id;
}

std::wstring CConfig::GetOptionString(int sec, int opt)
{
	auto id = string.Find(section[sec].option[opt].id);
	if (id.size() == 0)
		return MultiToWide_s(section[sec].option[opt].name);

	return id;
}

std::wstring CConfig::GetOptionDesc(int sec, int opt)
{
	auto id = string.Find(section[sec].option[opt].desc);
	if (id.size() == 0)
		return MultiToWide_s(section[sec].option[opt].desc);

	return id;
}

std::wstring CConfig::GetValueString(int sec, int opt, int val)
{
	auto id = string.Find(section[sec].option[opt].value[val].id);
	if (id.size() == 0)
		return MultiToWide_s(section[sec].option[opt].value[val].name);

	return id;
}

std::wstring CConfig::GetString(const char* name)
{
	return string.Find(name);
}

/////////////////////////////////////////////////
void CConfigSection::Parse(XMLElement& xml, CConfig& cfg)
{
	name = SAFESTR(xml.Attribute("name"));
	id = SAFESTR(cfg.SetIDString(xml.Attribute("id"), xml.Attribute("name")));

	for (auto &element : {"Option", "Feature"})
	{
		auto s = xml.FirstChildElement(element);
		while (s)
		{
			CConfigOption opt;
			opt.Parse(*s, cfg);
			if (CheckOptValue(opt))
			{
				option.push_back(opt);
			}
			else
			{
				p.error += "\"";
				p.error += name;
				p.error += "\"\n";
			}

			s = s->NextSiblingElement(element);
		}
	}
}

/////////////////////////////////////////////////
void CConfigOption::Parse(XMLElement& xml, CConfig& cfg)
{
	name = SAFESTR(xml.Attribute("name"));

	// Check for <Title> otherwise use id
	auto d = xml.FirstChildElement("Title");
	id = SAFESTR(d ? cfg.SetIDString(nullptr, d->GetText()) : cfg.SetIDString(xml.Attribute("id"), xml.Attribute("name")));

	// Check for <Description> otherwise use desc
	d = xml.FirstChildElement("Description");
	desc = SAFESTR(d ? cfg.SetIDString(nullptr, d->GetText()) : xml.Attribute("desc"));

	d = xml.FirstChildElement("Choices");
	if (!d)
		d = &xml;

	auto t = d->Attribute("type");
	type = TYPE_UNK;
	if (t)
	{
		if (strcmp(t, "check") == 0)
			type = TYPE_CHECK;
		else if (strcmp(t, "list") == 0)
			type = TYPE_LIST;
		else if (strcmp(t, "text") == 0)
			type = TYPE_TEXT;
		else if (strcmp(t, "invisible") == 0)
			type = TYPE_HIDE;
	}

	CConfigValue defval;

	int i = 0;
	auto s = d->FirstChildElement("Value");
	while (s)
	{
		CConfigValue val;
		val.Parse(*s, cfg);
		// if this is the default value, flag is as the active selection
		if (val.is_default)
		{
			cur_val = i;
			defval = val;
		}

		if (type != TYPE_TEXT)
		{
			value.push_back(val);
		}

		i++;
		s = s->NextSiblingElement("Value");
	}

	if (type == TYPE_TEXT)
	{
		// default text
		defval.is_default = true;
		value.push_back(defval);

		// current text
		cur_val = 1;
		defval.is_default = false;
		value.push_back(defval);
	}
}

/////////////////////////////////////////////////
void CConfigValue::Parse(XMLElement& xml, CConfig& cfg)
{
	name = SAFESTR(GetNameValue(xml.Attribute("name"), xml.Attribute("tip")));
	id = SAFESTR(cfg.SetIDString(xml.Attribute("id"), xml.Attribute("name")));
	is_default = SetValue(xml.Attribute("default"));
	val = SAFESTR(xml.GetText());
}

/////////////////////////////////////////////////
void CConfigGroup::Parse(XMLElement& xml)
{
	id = SAFESTR(xml.Attribute("id"));

	auto o = xml.FirstChildElement("Sub");
	while (o)
	{
		CConfigSub s;
		s.Parse(*o);
		sub.push_back(s);
		o = o->NextSiblingElement("Sub");
	}
}

void CConfigGroup::ParseTab(XMLElement& xml, CConfig& cfg)
{
	id = SAFESTR(cfg.SetIDString(nullptr, xml.Attribute("name")));

	auto o = xml.FirstChildElement("Section");
	while (o)
	{
		CConfigSub s;
		s.ParseTab(*o);
		sub.push_back(s);
		o = o->NextSiblingElement("Section");
	}
}

/////////////////////////////////////////////////
void CConfigGroup::CConfigSub::Parse(XMLElement& xml)
{
	id = SAFESTR(xml.Attribute("id"));

	auto o = xml.FirstChildElement("Opt");
	while (o)
	{
		CConfigSubOpt op;
		std::string xs = SAFESTR(o->Attribute("sec"));
		std::string xo = SAFESTR(o->Attribute("op"));
		op.Set(xs, xo);
		opt.push_back(op);
		o = o->NextSiblingElement("Opt");
	}
}

void CConfigGroup::CConfigSub::ParseTab(XMLElement& xml)
{
	id = SAFESTR(xml.Attribute("name"));

	auto o = xml.FirstChildElement("Feature");
	while (o)
	{
		CConfigSubOpt op;
		std::string xs = SAFESTR(xml.Attribute("name"));
		std::string xo = SAFESTR(o->Attribute("name"));
		op.Set(xs, xo);
		opt.push_back(op);
		o = o->NextSiblingElement("Feature");
	}
}

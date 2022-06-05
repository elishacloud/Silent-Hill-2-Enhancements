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
#include <psapi.h>
#include "CConfig.h"
#include "Launcher.h"
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

bool DisableMissingSettingsWarning = false;
bool DisableExtraSettingsWarning = false;
bool DisableDefaultValueWarning = false;

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

// get xml from resource
bool GetXMLfromResoruce(XMLDocument &xml)
{
	HRSRC hResource = FindResource(m_hModule, MAKEINTRESOURCE(IDR_CONFIG_XML), RT_RCDATA);
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
	if (GetProcessImageFileNameA(GetCurrentProcess(), path, MAX_PATH) && strrchr(path, '\\') && strrchr(path, '.'))
	{
		strcpy_s(strrchr(path, '.'), MAX_PATH - strlen(path), ".xml");
		return std::string(strrchr(path, '\\') + 1);
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
		// <Error DisableMissingSettingsWarning="1" DisableExtraSettingsWarning="1" DisableDefaultValueWarning="1" />
		DisableMissingSettingsWarning = SetValue(e->Attribute("DisableMissingSettingsWarning"));
		DisableExtraSettingsWarning = SetValue(e->Attribute("DisableExtraSettingsWarning"));
		DisableDefaultValueWarning = SetValue(e->Attribute("DisableDefaultValueWarning"));
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
	for (auto sec : section)
		for (auto opt : sec.option)
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
	for (size_t i = 0, si = section.size(); i < si; i++)
		for (size_t j = 0, sj = section[i].option.size(); j < sj; j++)
			p.list.push_back(&section[i].option[j]);
}

void __stdcall ParseIniCallback(char* lpName, char* lpValue, void *lpParam)
{
	// Check for valid entries
	if (!IsValidSettings(lpName, lpValue)) return;

	auto p = reinterpret_cast<cb_parse*>(lpParam);
	for (auto item : p->list)
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

void CConfig::SaveIni(LPCWSTR lpName, LPCWSTR error_mes, LPCWSTR error_caption)
{
	UNREFERENCED_PARAMETER(error_mes);
	UNREFERENCED_PARAMETER(error_caption);

	FILE* fp = nullptr;
	_wfopen_s(&fp, lpName, L"wt");
	if (fp == nullptr)
		return;

	for (auto sec : section)
	{
		// current section
		fwprintf(fp, L"[%hs]\n", sec.name.c_str());
		// write all options
		for (auto opt : sec.option)
			fwprintf(fp, L"%hs = %hs\n", opt.name.c_str(), opt.value[opt.cur_val].val.c_str());
		// tail
		fwprintf(fp, L"\n");
	}
	fclose(fp);
}

bool CConfig::IsSettingInXml(std::string lpName)
{
	for (auto item : p.list)
		if (item->name.compare(lpName) == 0)
			return true;

	return false;
}

std::string CConfig::GetDefaultSetting(std::string name)
{
	for (auto item : AllValues)
		if (item.name.compare(name) == 0)
			return item.val;

	return std::string("");
}

bool CConfig::IsVisibleSetting(std::string name)
{
	for (auto item : AllValues)
		if (item.name.compare(name) == 0)
			if (!IsHiddenSetting(name))
				return true;

	return false;
}

bool CConfig::IsHiddenSetting(std::string name)
{
	for (auto item : HiddenValues)
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
	// Check for missing settings in xml file
	if (!DisableMissingSettingsWarning)
	{
		for (auto item : AllValues)
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
		for (auto item : p.list)
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
		for (auto item : p.list)
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

	for (auto element : {"Option", "Feature"})
	{
		auto s = xml.FirstChildElement(element);
		while (s)
		{
			CConfigOption opt;
			opt.Parse(*s, cfg);
			option.push_back(opt);

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
		if (strcmp(t, "list") == 0)
			type = TYPE_LIST;
		else if (strcmp(t, "check") == 0)
			type = TYPE_CHECK;
		else if (strcmp(t, "invisible") == 0)
			type = TYPE_TEXT;
		else if (strcmp(t, "pad") == 0)
			type = TYPE_PAD;
	}

	auto s = d->FirstChildElement("Value");
	int i = 0;
	while (s)
	{
		CConfigValue val;
		val.Parse(*s, cfg);
		// if this is the default value, flag is as the active selection
		if (val.is_default)
			cur_val = i;
		value.push_back(val);

		s = s->NextSiblingElement("Value");
		i++;
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

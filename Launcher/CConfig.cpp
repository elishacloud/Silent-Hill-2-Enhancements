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

#include "CConfig.h"
#include "Launcher.h"
#include "Common\Settings.h"

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

/////////////////////////////////////////////////
// Actual configuration
bool CConfig::ParseXml()
{
	XMLDocument xml;
	do {
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
					if (dwResourceSize != 0)
					{
						if (!xml.Parse((char*)pLockedResource, dwResourceSize))
						{
							break;
						}
					}
				}
			}
		}
		return true;
	} while (false);

	auto root = xml.RootElement();
	auto s = root->FirstChildElement("Sections");
	auto sec = s->FirstChildElement("Section");
	while (sec)
	{
		auto name = SAFESTR(sec->Attribute("name"));
		CConfigSection ssec;
		ssec.name = name;
		ssec.Parse(*sec);
		section.push_back(ssec);

		sec = sec->NextSiblingElement();
	}

	auto g = root->FirstChildElement("Groups");
	auto gp = g->FirstChildElement("Group");
	while (gp)
	{
		CConfigGroup gg;
		gg.Parse(*gp);
		group.push_back(gg);

		gp = gp->NextSiblingElement("Group");
	}

	s = root->FirstChildElement("Strings");
	sec = s->FirstChildElement("S");
	while (sec)
	{
		auto str = SAFESTR(sec->GetText());
		auto id = SAFESTR(sec->Attribute("id"));

		string.PushString(str.c_str(), id.c_str());
		//string.push_back(MultiToWide_s(str));

		sec = sec->NextSiblingElement();
	}
	string.Sort();
	return false;
}

void CConfig::SetDefault()
{
	for (size_t i = 0, si = section.size(); i < si; i++)
		for (size_t j = 0, sj = section[i].option.size(); j < sj; j++)
			section[i].option[j].SetValueDefault();
}

struct cb_parse
{
	std::vector<CConfigOption*> list;
	std::string error;
};

void __stdcall ParseCallback_Config(char* lpName, char* lpValue, void *lpParam)
{
	// Check for valid entries
	if (!IsValidSettings(lpName, lpValue)) return;

	auto p = reinterpret_cast<cb_parse*>(lpParam);
	for (size_t i = 0, si = p->list.size(); i < si; i++)
	{
		if (p->list[i]->name.compare(lpName) == 0)
		{
			p->list[i]->SetValueFromName(lpValue);
			return;
		}
	}

	p->error += "\"";
	p->error += lpName;
	p->error += "\" > \"";
	p->error += lpValue;
	p->error += "\"\n";
}

void CConfig::SetFromIni(LPCWSTR lpName, LPCWSTR error_caption)
{
	// attempt to load the ini
	auto ini = Read(lpName);
	if (ini == nullptr) return;

	cb_parse p;

	// build reference to all options for quicker callback parsing
	for (size_t i = 0, si = section.size(); i < si; i++)
		for (size_t j = 0, sj = section[i].option.size(); j < sj; j++)
			p.list.push_back(&section[i].option[j]);

	// do the parsing (can be slightly slow)
	Parse(ini, ParseCallback_Config, (void*)&p);

	// done, disengage!
	free(ini);

	// print orphaned
	if(!p.error.empty())
		MessageBoxW(nullptr, MultiToWide_s(std::string("Orphan sections:\n") + p.error).c_str(), error_caption, MB_OK);
}

void CConfig::SaveIni(LPCWSTR lpName, LPCWSTR error_mes, LPCWSTR error_caption)
{
	FILE* fp = nullptr;
	_wfopen_s(&fp, lpName, L"wt");
	if (fp == nullptr)
		return;

	for (size_t i = 0, si = section.size(); i < si; i++)
	{
		// current section
		fwprintf(fp, L"[%hs]\n", section[i].name.c_str());
		// write all options
		for (size_t j = 0, sj = section[i].option.size(); j < sj; j++)
			fwprintf(fp, L"%hs = %hs\n", section[i].option[j].name.c_str(), section[i].option[j].value[section[i].option[j].cur_val].val.c_str());
		// tail
		fwprintf(fp, L"\n");
	}
	fclose(fp);
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
void CConfigSection::Parse(XMLElement& xml)
{
	name = SAFESTR(xml.Attribute("name"));
	id = SAFESTR(xml.Attribute("id"));

	auto s = xml.FirstChildElement("Option");
	while (s)
	{
		CConfigOption opt;
		opt.Parse(*s);
		option.push_back(opt);

		s = s->NextSiblingElement("Option");
	}
}

/////////////////////////////////////////////////
void CConfigOption::Parse(XMLElement& xml)
{
	name = SAFESTR(xml.Attribute("name"));
	auto t = xml.Attribute("type");
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

	id = SAFESTR(xml.Attribute("id"));
	desc = SAFESTR(xml.Attribute("desc"));

	auto s = xml.FirstChildElement("Value");
	int i = 0;
	while (s)
	{
		CConfigValue val;
		val.Parse(*s);
		// if this is the default value, flag is as the active selection
		if (val.is_default)
			cur_val = i;
		value.push_back(val);

		s = s->NextSiblingElement("Value");
		i++;
	}
}

/////////////////////////////////////////////////
void CConfigValue::Parse(XMLElement& xml)
{
	name = SAFESTR(xml.Attribute("tip"));
	id = SAFESTR(xml.Attribute("id"));
	is_default = xml.IntAttribute("default", 0);
	val = SAFESTR(xml.GetText());
}

/////////////////////////////////////////////////
void CConfigGroup::Parse(XMLElement& xml)
{
	//auto gp = xml.FirstChildElement("Groups");
	id = SAFESTR(xml.Attribute("id"));

	auto o = xml.FirstChildElement("Sub");
	while (o)
	{
		CConfigSub s;
		s.Parse(*o);
		s.id = SAFESTR(o->Attribute("id"));
		sub.push_back(s);
		o = o->NextSiblingElement("Sub");
	}
}

/////////////////////////////////////////////////
void CConfigGroup::CConfigSub::Parse(XMLElement& xml)
{
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

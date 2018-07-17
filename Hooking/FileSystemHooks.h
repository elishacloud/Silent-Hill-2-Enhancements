#pragma once

extern bool FileEnabled;
extern bool EnableWndMode;
extern bool Nemesis2000FogFix;
extern wchar_t ConfigName[MAX_PATH];

struct MODULECONFIG
{
	wchar_t *ConfigFileList;		// Module config name
	bool *Enabled;					// Is module enabled
};

// List of hardcoded config file names from memory modules
static MODULECONFIG ConfigList[] =
{
	{ ConfigName, &FileEnabled },
	{ L"sh2fog.ini", &Nemesis2000FogFix },
	{ L"wndmode.ini", &EnableWndMode }
};

void InstallFileSystemHooks(HMODULE hModule, wchar_t *ConfigPath);

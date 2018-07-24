#pragma once

#include "Common\Settings.h"

extern bool FileEnabled;
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
	{ L"sh2fog.ini", &Nemesis2000FogFix }
};

void InstallFileSystemHooks(HMODULE hModule, wchar_t *ConfigPath);

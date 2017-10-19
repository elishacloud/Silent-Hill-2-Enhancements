/**
* Copyright (C) 2017 Elisha Riedlinger
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
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "SFX\sfx.h"
#include "ScreenRes\ScreenRes.h"
#include "Common\Settings.h"
#include "Common\Logging.h"

std::ofstream Log::LOG;
char LogPath[MAX_PATH];

bool EnableSFXAddrHack = true;
bool ResetScreenRes = true;

// Set config from string (file)
void __stdcall ParseCallback(char* name, char* value)
{
	// Check for valid entries
	if (!IsValidSettings(name, value)) return;

	// Check settings
	if (!_strcmpi(name, "EnableSFXAddrHack")) EnableSFXAddrHack = SetValue(value);
	if (!_strcmpi(name, "ResetScreenRes")) ResetScreenRes = SetValue(value);
}

// Dll main function
bool APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Set thread priority a trick to reduce concurrency problems at program startup
		HANDLE hCurrentThread = GetCurrentThread();
		int dwPriorityClass = GetThreadPriority(hCurrentThread);
		dwPriorityClass = (GetLastError() == THREAD_PRIORITY_ERROR_RETURN) ? THREAD_PRIORITY_NORMAL : dwPriorityClass;
		SetThreadPriority(hCurrentThread, THREAD_PRIORITY_HIGHEST);

		// Get log file path and open log file
		char LogPath[MAX_PATH];
		GetModuleFileNameA(hModule, LogPath, MAX_PATH);
		strcpy_s(strrchr(LogPath, '.'), MAX_PATH - strlen(LogPath), ".log");
		Log::LOG.open(LogPath);

		// Starting
		Log() << "Starting Silent Hill 2 Enhancement ASI!";

		// Get config file path
		char configname[MAX_PATH];
		GetModuleFileNameA(hModule, configname, MAX_PATH);
		strcpy_s(strrchr(configname, '.'), MAX_PATH - strlen(configname), ".ini");

		// Read config file
		Log() << "Reading config file:  " << configname;
		char* szCfg = Read(configname);

		// Parce config file
		if (szCfg)
		{
			Parse(szCfg, ParseCallback);
			free(szCfg);
		}
		else
		{
			Log() << "Config file not found, using defaults";
		}

		// Update SFX addresses
		if (EnableSFXAddrHack) UpdateSFXAddr();

		// Resetting thread priority
		SetThreadPriority(hCurrentThread, dwPriorityClass);

		// Closing handle
		CloseHandle(hCurrentThread);
	}
	break;
	case DLL_THREAD_ATTACH:

		// Check and store screen resolution
		if (ResetScreenRes) CheckCurrentScreenRes();

		break;
	case DLL_PROCESS_DETACH:

		// Reset screen back to original Windows settings to fix some display errors on exit
		if (ResetScreenRes) ResetScreen();

		// Quiting
		Log() << "Unloading ASI!";

		break;
	}

	return true;
}

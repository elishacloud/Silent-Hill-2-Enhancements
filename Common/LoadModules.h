#pragma once

void LoadASIPlugins(bool LoadFromScriptsOnly);
void LoadModuleFromResource(HMODULE hModule, DWORD ResID, LPCWSTR lpName);
void LoadModuleFromResourceToFile(HMODULE hModule, DWORD ResID, LPCWSTR lpName, LPCWSTR lpFilepath);
void LoadModUpdater(HMODULE hModule, DWORD ResID);
void LoadWndMode(HMODULE hModule, DWORD ResID);
void UnloadResourceModules();

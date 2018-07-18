#pragma once

void LoadASIPlugins(bool LoadFromScriptsOnly);
void UnloadResourceModules();
void LoadModuleFromResource(HMODULE hModule, DWORD ResID, LPCWSTR lpName);
void LoadModuleFromResourceToFile(HMODULE hModule, DWORD ResID, LPCWSTR lpName, LPCWSTR lpFilepath);

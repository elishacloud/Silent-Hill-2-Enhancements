#pragma once

#include "External\MemoryModule\MemoryModule.h"

void LoadASIPlugins(bool LoadFromScriptsOnly);
HMEMORYMODULE LoadModuleFromResource(HMODULE hModule, DWORD ResID, LPCWSTR lpName);
HMODULE LoadModuleFromResourceToFile(HMODULE hModule, DWORD ResID, LPCWSTR lpName, LPCWSTR lpFilepath);
void InitializeASI(HMODULE hModule);
void InitializeASI(HMEMORYMODULE hModule);
HRESULT DeleteAllfiles(LPCWSTR lpFolder);
HMODULE LoadModuleFromFile(HMODULE hModule, DWORD ResID, LPCWSTR lpConfigName, LPCWSTR lpConfigPath, LPCWSTR lpName, bool CopyDatFile);
void RemoveTempFolders();
void UnloadResourceModules();

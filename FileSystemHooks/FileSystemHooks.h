#pragma once

extern bool LoadingMemoryModule;

void InstallFileSystemHooks(HMODULE hModule, wchar_t *ConfigPath);

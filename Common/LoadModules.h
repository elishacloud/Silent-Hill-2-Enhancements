#pragma once

void LoadASIPlugins(bool LoadFromScriptsOnly);
void InitializeASI(HMODULE hModule);
void ExtractD3DX9Tools();
HRESULT DeleteAllfiles(LPCWSTR lpFolder);

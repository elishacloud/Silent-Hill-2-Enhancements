#pragma once

std::string ReadFileContents(std::wstring &path);
std::string MergeiniFile(std::stringstream &s_currentini, std::stringstream &s_ini, bool OverWriteCurrent = false);
DWORD WINAPI CheckForUpdate(LPVOID lpName);

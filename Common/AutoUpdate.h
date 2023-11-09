#pragma once

std::string ReadFileContents(std::wstring &path);
DWORD MatchCount(std::string& path1, std::string& path2);
std::string MergeiniFile(std::stringstream &s_currentini, std::stringstream &s_ini, bool OverWriteCurrent = false);
DWORD WINAPI CheckForUpdate(LPVOID lpName);

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <psapi.h>
#include <shlwapi.h>
#include <algorithm>

inline char* nullstr(char*) { return "\0"; }
inline wchar_t* nullstr(wchar_t*) { return L"\0"; }

inline errno_t WINAPI strcpy_s(wchar_t* dest, size_t size, LPCWSTR src)
{
	return wcscpy_s(dest, size, src);
}

inline errno_t WINAPI strcat_s(wchar_t* dest, size_t size, LPCWSTR src)
{
	return wcscat_s(dest, size, src);
}

inline wchar_t* WINAPI strrchr(wchar_t* _String, wchar_t _Ch)
{
	return wcsrchr(_String, _Ch);
}

inline size_t WINAPI strlen(wchar_t* _Str)
{
	return wcslen(_Str);
}

inline int WINAPI tolower(const char chr)
{
	return tolower((unsigned char)chr);
}

inline wint_t WINAPI tolower(const wchar_t chr)
{
	return towlower(chr);
}

inline std::string tostring(std::string name)
{
	return name;
}

inline std::wstring tostring(std::wstring name)
{
	return name;
}

inline std::string TransformLower(char name[])
{
	std::string l_name(name);
	std::transform(l_name.begin(), l_name.end(), l_name.begin(), [](char c) { return (char)towlower(c); });
	return l_name;
}

inline std::wstring TransformLower(wchar_t name[])
{
	std::wstring l_name(name);
	std::transform(l_name.begin(), l_name.end(), l_name.begin(), [](wchar_t c) { return (wchar_t)towlower(c); });
	return l_name;
}

inline BOOL WINAPI PathFileExistsW(LPCSTR str)
{
	return PathFileExistsA(str);
}

inline BOOL WINAPI PathFileExistsA(LPCWSTR str)
{
	return PathFileExistsW(str);
}

inline DWORD WINAPI GetModuleFileNameW(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	return GetModuleFileNameA(hModule, lpFilename, nSize);
}

inline DWORD WINAPI GetModuleFileNameA(HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
{
	return GetModuleFileNameW(hModule, lpFilename, nSize);
}

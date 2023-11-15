#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <algorithm>

#ifndef LWSTDAPI_
#define LWSTDAPI_(type)   EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
LWSTDAPI_(BOOL)     PathFileExistsA(__in LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathFileExistsW(__in LPCWSTR pszPath);
#undef LWSTDAPI_
#else
LWSTDAPI_(BOOL)     PathFileExistsA(__in LPCSTR pszPath);
LWSTDAPI_(BOOL)     PathFileExistsW(__in LPCWSTR pszPath);
#endif

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

inline size_t WINAPI strlen(const wchar_t* _Str)
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

inline std::string TransformLower(char* name)
{
	std::string l_name(name);
	std::transform(l_name.begin(), l_name.end(), l_name.begin(), [](char c) { return (char)towlower(c); });
	return l_name;
}

inline std::wstring TransformLower(wchar_t* name)
{
	std::wstring l_name(name);
	std::transform(l_name.begin(), l_name.end(), l_name.begin(), [](wchar_t c) { return (wchar_t)towlower(c); });
	return l_name;
}

inline std::string SAFESTR(const char* X)
{
	if (!X)
		return std::string("");

	return std::string(X);
}

// create a wide string from utf8
inline std::wstring MultiToWide_s(const char* multi)
{
	if (!multi)
		return std::wstring(L"");

	std::wstring wstr;
	// gather size of the new string and create a buffer
	int size = MultiByteToWideChar(CP_UTF8, 0, multi, -1, NULL, 0);
	wchar_t* wide = new wchar_t[size];
	if (wide)
	{
		// fill allocated string with converted data
		MultiByteToWideChar(CP_UTF8, 0, multi, -1, wide, size);
		// convert to std::wstring
		wstr.assign(wide);
		// delete memory
		delete[] wide;
	}
	return wstr;
}

// crate an std::wstring from utf8 str::string
inline std::wstring MultiToWide_s(std::string multi)
{
	return MultiToWide_s(multi.c_str());
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

inline int CharCount(std::string s, char ch)
{
	int count = 0;

	for (int i = 0; (i = s.find(ch, i)) != std::string::npos; i++)
	{
		count++;
	}

	return count;
}

inline int CharCount(std::wstring s, wchar_t ch)
{
	int count = 0;

	for (int i = 0; (i = s.find(ch, i)) != std::wstring::npos; i++)
	{
		count++;
	}

	return count;
}

constexpr char removechars[] = " \t\n\r";
inline void trim(std::string& str, const char chars[] = removechars)
{
	str.erase(0, str.find_first_not_of(chars));
	str.erase(str.find_last_not_of(chars) + 1);
}
inline std::string trim(const std::string& str, const char chars[] = removechars)
{
	std::string res(str);
	trim(res, chars);
	return res;
}

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

inline std::string SAFESTR(const char* str)
{
	if (!str)
	{
		return std::string();
	}

	return std::string(str);
}

inline std::wstring SAFESTR(const wchar_t* str)
{
	if (!str)
	{
		return std::wstring();
	}

	return std::wstring(str);
}

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

inline std::string TransformLower(const char* name)
{
	std::string l_name(name);
	std::transform(l_name.begin(), l_name.end(), l_name.begin(), [](char c) { return (char)towlower(c); });
	return l_name;
}

inline std::wstring TransformLower(const wchar_t* name)
{
	std::wstring l_name(name);
	std::transform(l_name.begin(), l_name.end(), l_name.begin(), [](wchar_t c) { return (wchar_t)towlower(c); });
	return l_name;
}

inline std::wstring MultiToWide_s(std::string multi)
{
	std::wstring wide;

	// Get the size of the required buffer
	int size = MultiByteToWideChar(CP_UTF8, 0, multi.c_str(), -1, NULL, 0);

	if (size > 0)
	{
		// Allocate buffer
		std::vector<wchar_t> buffer(size);

		// Convert multi-byte string to wide string
		if (MultiByteToWideChar(CP_UTF8, 0, multi.c_str(), -1, buffer.data(), size) > 0)
		{
			// Assign to std::wstring
			wide.assign(buffer.data());
		}
	}

	return wide;
}

inline std::wstring MultiToWide_s(const char* multi)
{
	return MultiToWide_s(SAFESTR(multi));
}

inline std::string WideToMulti_s(std::wstring wide)
{
	std::string multi;

	// Get the size of the required buffer
	int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, NULL, 0, NULL, NULL);

	if (size > 0)
	{
		// Allocate buffer
		std::vector<char> buffer(size);

		// Convert wide string to multi-byte string
		if (WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, buffer.data(), size, NULL, NULL) > 0)
		{
			// Assign to std::string
			multi.assign(buffer.data());
		}
	}

	return multi;
}

inline std::string WideToMulti_s(const wchar_t* multi)
{
	return WideToMulti_s(SAFESTR(multi));
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

inline int CharCount(const char* s, char ch)
{
	return CharCount(SAFESTR(s), ch);
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

inline int CharCount(const wchar_t* s, wchar_t ch)
{
	return CharCount(SAFESTR(s), ch);
}

constexpr char charstoremove[] = " \t\n\r\v\f";
constexpr wchar_t charstoremovew[] = L" \t\n\r\v\f";

inline std::string trim(std::string str, const char chars[] = charstoremove)
{
	std::string res(str);

	// Remove specified characters
	res.erase(0, res.find_first_not_of(chars));
	res.erase(res.find_last_not_of(chars) + 1);

	// Remove null characters
	res.erase(std::remove(res.begin(), res.end(), '\0'), res.end());

	return res;
}

inline std::string trim(const char* str, const char chars[] = charstoremove)
{
	return trim(SAFESTR(str), chars);
}

inline std::wstring trim(std::wstring str, const wchar_t chars[] = charstoremovew)
{
	std::wstring res(str);

	// Remove specified characters
	res.erase(0, res.find_first_not_of(chars));
	res.erase(res.find_last_not_of(chars) + 1);

	// Remove null characters
	res.erase(std::remove(res.begin(), res.end(), '\0'), res.end());

	return res;
}

inline std::wstring trim(const wchar_t* str, const wchar_t chars[] = charstoremovew)
{
	return trim(SAFESTR(str), chars);
}

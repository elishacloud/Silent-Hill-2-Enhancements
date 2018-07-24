#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <fstream>

char *ConstStr(LPCSTR str);
wchar_t *ConstStr(LPCWSTR str);
size_t length(LPCSTR str);
size_t length(LPCWSTR str);
std::wstring toLower(std::wstring str);
std::wstring toWString(LPCSTR str);
std::wstring toWString(LPCWSTR str);
char *GetStringType(LPCSTR, char *strA, wchar_t *strW);
wchar_t *GetStringType(LPCWSTR, char *strA, wchar_t *strW);

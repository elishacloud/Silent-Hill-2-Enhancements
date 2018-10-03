#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <fstream>

size_t length(LPCSTR str);
size_t length(LPCWSTR str);
void CopyString(char *strA, size_t size, LPCSTR str);
void CopyString(wchar_t *strW, size_t size, LPCWSTR str);
char *GetStringType(LPCSTR, char *strA, wchar_t *strW);
wchar_t *GetStringType(LPCWSTR, char *strA, wchar_t *strW);
bool IsInString(LPCSTR strCheck, LPCSTR strA, LPCWSTR);
bool IsInString(LPCWSTR strCheck, LPCSTR, LPCWSTR strW);

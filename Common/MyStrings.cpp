/**
* Copyright (C) 2018 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <fstream>

char *ConstStr(LPCSTR str)
{
	return (char*)str;
}

wchar_t *ConstStr(LPCWSTR str)
{
	return (wchar_t*)str;
}

size_t length(LPCSTR str)
{
	return strlen(str);
}

size_t length(LPCWSTR str)
{
	return wcslen(str);
}

std::wstring toWString(LPCSTR str)
{
	std::string mystring(str);
	std::wstring wstring(mystring.begin(), mystring.end());
	return wstring;
}

std::wstring toWString(LPCWSTR str)
{
	std::wstring wstring(str);
	return wstring;
}

char *GetStringType(LPCSTR, char *strA, wchar_t *)
{
	return strA;
}

wchar_t *GetStringType(LPCWSTR, char *, wchar_t *strW)
{
	return strW;
}

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
*
* Code in EraseCppComments, Read and Parse functions taken from source code found in Aqrit's ddwrapper
* http://bitpatch.com/ddwrapper.html
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include "Settings.h"

// Configurable setting defaults
#define SET_BOOL_DEFAULTS(name, value) \
	bool name = value;

VISIT_BOOL_SETTINGS(SET_BOOL_DEFAULTS);

// Forward declarations
bool SetValue(char* name);
bool IsValidSettings(char* name, char* value);

// Get config settings from string (file)
void __stdcall ParseCallback(char* lpName, char* lpValue)
{
	// Check for valid entries
	if (!IsValidSettings(lpName, lpValue)) return;

	// Check settings
#define GET_BOOL_VALUES(name, unused) \
	if (!_strcmpi(lpName, #name)) name = SetValue(lpValue);

	VISIT_BOOL_SETTINGS(GET_BOOL_VALUES);
}

// Set booloean value from string (file)
bool SetValue(char* value)
{
	return (atoi(value) > 0 ||
		_strcmpi("on", value) == 0 ||
		_strcmpi("yes", value) == 0 ||
		_strcmpi("true", value) == 0 ||
		_strcmpi("enabled", value) == 0);
}

// Check if the values are valid
bool IsValidSettings(char* name, char* value)
{
	if (!name || !value)
	{
		return false;
	}
	if (strlen(name) == 0 || strlen(value) == 0 ||
		strlen(name) == ((size_t)(-1)) || strlen(value) == ((size_t)(-1)) ||
		name[0] == '\0' || value[0] == '\0' ||
		!_strcmpi(value, "AUTO"))
	{
		return false;
	}
	return true;
}

// Reads szFileName from disk
char* Read(wchar_t* szFileName)
{
	HANDLE hFile;
	DWORD dwBytesToRead;
	DWORD dwBytesRead;

	char* szCfg = nullptr;
	hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		dwBytesToRead = GetFileSize(hFile, nullptr);
		if ((dwBytesToRead != 0) && (dwBytesToRead != 0xFFFFFFFF))
		{
			szCfg = (char*)malloc(dwBytesToRead + 1); // +1 so a NULL terminator can be added
			if (szCfg)
			{
				if (ReadFile(hFile, szCfg, dwBytesToRead, &dwBytesRead, nullptr))
				{
					if (dwBytesRead != 0)
					{
						szCfg[dwBytesRead] = '\0'; // make txt file easy to deal with 
					}
				}
				else
				{
					free(szCfg);
					szCfg = nullptr;
				}
			}
		}
		CloseHandle(hFile);
	}
	return szCfg;
}

// Commented text is replaced with a space character
void EraseCppComments(char* str)
{
	while ((str = strchr(str, '/')) != 0)
	{
		if (str[1] == '/')
		{
			for (; ((*str != '\0') && (*str != '\n')); str++)
			{
				*str = '\x20';
			}
		}
		else if (str[1] == '*')
		{
			for (; ((*str != '\0') && ((str[0] != '*') || (str[1] != '/'))); str++)
			{
				*str = '\x20';
			}
			if (*str)
			{
				*str++ = '\x20';
				*str = '\x20';
			}
		}
		if (*str)
		{
			str++;
		}
		else
		{
			break;
		}
	}
}

// [sections] are ignored
// escape characters NOT support 
// double quotes NOT suppoted
// Name/value delimiter is an equal sign or colon 
// whitespace is removed from before and after both the name and value
// characters considered to be whitespace:
//  0x20 - space
//	0x09 - horizontal tab
//	0x0D - carriage return
void Parse(char* str, NV NameValueCallback)
{
	char *next_token = nullptr;
	EraseCppComments(str);
	for (str = strtok_s(str, "\n", &next_token); str; str = strtok_s(0, "\n", &next_token))
	{
		if (*str == ';' || *str == '#')
		{
			continue; // skip INI style comments ( must be at start of line )
		}
		char* rvalue = strchr(str, '=');
		if (!rvalue)
		{
			rvalue = strchr(str, ':');
		}
		if (rvalue)
		{
			*rvalue++ = '\0'; // split left and right values

			rvalue = &rvalue[strspn(rvalue, "\x20\t\r")]; // skip beginning whitespace
			for (char* end = strchr(rvalue, '\0'); (--end >= rvalue) && (*end == '\x20' || *end == '\t' || *end == '\r');)
			{
				*end = '\0';  // truncate ending whitespace
			}

			char* lvalue = &str[strspn(str, "\x20\t\r")]; // skip beginning whitespace
			for (char* end = strchr(lvalue, '\0'); (--end >= lvalue) && (*end == '\x20' || *end == '\t' || *end == '\r');)
			{
				*end = '\0';  // truncate ending whitespace
			}

			if (*lvalue && *rvalue)
			{
				NameValueCallback(lvalue, rvalue);
			}
		}
	}
}

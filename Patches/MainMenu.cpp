/**
* Copyright (C) 2020 Elisha Riedlinger
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
#include "Patches.h"
#include "Common\FileSystemHooks.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"
#include "Common\Settings.h"
#include <string>

using namespace std;

void *MainMenuFixRetAddr;

__declspec(naked) void __stdcall MainMenuFixASM()
{
	__asm
	{
		mov		edi, 12Ch
		mov		esi, 16h
		jmp		MainMenuFixRetAddr
	}
}

void UpdateMainMenuFix()
{
	// Check for game versions that don't need to be patched
	if (GameVersion == SH2V_10 || GameVersion == SH2V_DC)
	{
		return;
	}

	constexpr BYTE MenuSearchBytesA[] = { 0x24, 0x14, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x6D, 0xFF, 0x24, 0x85, 0x3C, 0x5D, 0x49, 0x00, 0xBF };
	void *DMenuAddrA = CheckMultiMemoryAddress(0x00000000, (void*)0x00495AA0, 0x00000000, (void*)MenuSearchBytesA, sizeof(MenuSearchBytesA));

	// Checking address pointer
	if (!DMenuAddrA)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	Logging::Log() << "Enabling Main Menu fix...";

	BYTE BreakOffset = 0x55;
	UpdateMemoryAddress((void *)((BYTE*)DMenuAddrA + 0x1F), (void *)&BreakOffset, 1);
	MainMenuFixRetAddr = (void *)((BYTE*)DMenuAddrA + 0x14);
	WriteJMPtoMemory((BYTE*)DMenuAddrA + 0x0F, *MainMenuFixASM, 5);

	int UpdateVal = 0x0133;
	UpdateMemoryAddress((void *)((BYTE*)DMenuAddrA + 0x21), (void *)&UpdateVal, 4);
	UpdateVal = 0x1E;
	UpdateMemoryAddress((void *)((BYTE*)DMenuAddrA + 0x26), (void *)&UpdateVal, 4);
	UpdateVal = 0x014A;
	UpdateMemoryAddress((void *)((BYTE*)DMenuAddrA + 0x3E), (void *)&UpdateVal, 4);
	UpdateVal = 0x014A;
	UpdateMemoryAddress((void *)((BYTE*)DMenuAddrA + 0x62), (void *)&UpdateVal, 4);
	UpdateVal = 0x12;
	UpdateMemoryAddress((void *)((BYTE*)DMenuAddrA + 0x71), (void *)&UpdateVal, 4);
}

typedef struct
{
	char *pathPtr;
	int size;
	int unk2;
	int unk3;
} TexPath;

constexpr BYTE MenuSearchBytesB[] = { 0x00, 0x00, 0x68, 0xC4, 0xD5, 0x94, 0x00, 0xE8, 0xD0, 0x7F, 0x00, 0x00, 0x33, 0xF6, 0x83, 0xC8, 0xFF };
constexpr BYTE MenuSearchBytesC[] = { 0x00, 0x00, 0x68, 0xC4, 0x11, 0x95, 0x00, 0xE8, 0xD6, 0x7F, 0x00, 0x00, 0x33, 0xF6, 0x83, 0xC8, 0xFF };
constexpr BYTE MenuSearchBytesD[] = { 0x07, 0x68, 0x4C, 0xFF, 0xFF, 0xFF, 0xBF, 0x13, 0x00, 0x00, 0x00, 0xBB, 0x3B, 0x00, 0x00, 0x00, 0xE8 };
constexpr BYTE MenuSearchBytesE[] = { 0x8B, 0x4C, 0x24, 0x04, 0x83, 0xF9, 0x0D, 0xB8, 0xFE, 0xFF, 0xFF, 0xFF, 0x0F, 0x87, 0x41, 0x03, 0x00, 0x00 };

extern BYTE *gLangID;
extern void ScaleStart01Texture(wchar_t* TexName);

TexPath start00Offset;
TexPath start01Offset;

char start00CustomPath[MAX_PATH];
char start01CustomPath[MAX_PATH];

void UpdateTitlePath()
{
	char lang;
	switch (*gLangID) 
	{
		case 0: lang = 'j'; break;
		case 1: lang = 'e'; break;
		case 2: lang = 'f'; break;
		case 3: lang = 'g'; break;
		case 4: lang = 'i'; break;
		case 5: lang = 's'; break;
		default: lang = 'e'; break;
	}
#if 0
	sprintf_s((char *)start00CustomPath, sizeof(start00CustomPath), "data\\pic\\etc\\start00%c.tex", lang);
	start00Offset.unk1 = -1;
	start00Offset.unk3 = 0;
	ifstream file(start00CustomPath);
	if (!file.is_open())
	{
		start00Offset.pathPtr = "data\\pic\\etc\\start00.tex";
	}
	else
	{
		start00Offset.pathPtr = (char *)start00CustomPath;
		file.close();
	}
#endif
	sprintf_s((char *)start01CustomPath, sizeof(start01CustomPath), "data\\pic\\etc\\start01%c.tex", lang);
	start01Offset.size = -1;
	start01Offset.unk3 = 0;
	ifstream file(start01CustomPath);
	if (!file.is_open())
	{
		start01Offset.pathPtr = "data\\pic\\etc\\start01.tex";
	}
	else
	{
		start01Offset.pathPtr = (char *)start01CustomPath;
		file.close();
	}

	if (EnableTexAddrHack)
	{
		size_t outsize;
		wchar_t st01path[MAX_PATH];
		mbstowcs_s(&outsize, st01path, start01Offset.pathPtr, MAX_PATH);
		ScaleStart01Texture(st01path);
	}
}

void *MainMenuTitleRetAddr;
DWORD (*LoadMes)(int pos);

__declspec(naked) void __stdcall MainMenuTitleASM()
{
	__asm
	{
		call	UpdateTitlePath
		push	0
		call	LoadMes
		add		esp, 4
		xor		esi, esi
		or		eax, 0FFFFFFFFh
		jmp		MainMenuTitleRetAddr
	}
}

void UpdateMainMenuTitlePerLang()
{
	// Check for game versions that don't need to be patched
	if (GameVersion == SH2V_DC)
	{
		return;
	}

	void *DMenuAddrB = CheckMultiMemoryAddress((void*)0x00496F24, 0x00000000, 0x00000000, (void*)MenuSearchBytesB, sizeof(MenuSearchBytesB));

	// Checking address pointer
	if (!DMenuAddrB)
	{
		DMenuAddrB = CheckMultiMemoryAddress(0x00000000, (void*)0x004971CE, 0x00000000, (void*)MenuSearchBytesC, sizeof(MenuSearchBytesC));
		if (!DMenuAddrB)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}
	}

	start00Offset.pathPtr = (char *)start00CustomPath;
	start00Offset.size = -1;
	start00Offset.unk2 = 0;
	start00Offset.unk3 = 1;
	
	start01Offset.pathPtr = (char *)start01CustomPath;
	start01Offset.size = -1;
	start01Offset.unk2 = 0;
	start01Offset.unk3 = 1;

	int UpdateVal;

	if (DMenuAddrB && UseCustomExeStr)
	{
		Logging::Log() << "Enabling Main Menu title selection by language...";
		MainMenuTitleRetAddr = (void *)((BYTE*)DMenuAddrB + 0x11);
		WriteJMPtoMemory((BYTE*)DMenuAddrB + 0x0C, *MainMenuTitleASM, 5);
#if 0
		UpdateVal = (int)&start00Offset;
		UpdateMemoryAddress((void *)((BYTE*)DMenuAddrB + 0x89), (void *)&UpdateVal, 4);
		UpdateMemoryAddress((void *)((BYTE*)DMenuAddrB + 0xA5), (void *)&UpdateVal, 4);
#endif
		UpdateVal = (int)&start01Offset;
		UpdateMemoryAddress((void *)((BYTE*)DMenuAddrB + 0x0131), (void *)&UpdateVal, 4);
		UpdateMemoryAddress((void *)((BYTE*)DMenuAddrB + 0x014E), (void *)&UpdateVal, 4);
	}

	//Make "Born From a Wish" horizontal position the same as "Letter From Silent Heaven"
	void *DMenuAddrC = (void*)SearchAndGetAddresses(0x00497A08, 0x00497CB8, 0x00000000, MenuSearchBytesD, sizeof(MenuSearchBytesD), 0x00);
	if (DMenuAddrC)
	{
		UpdateVal = -244;
		UpdateMemoryAddress((void *)((BYTE*)DMenuAddrC - 0x25), (void *)&UpdateVal, 4);
		UpdateVal = 508;
		UpdateMemoryAddress((void *)((BYTE*)DMenuAddrC - 0x1A), (void *)&UpdateVal, 2);
		UpdateVal = -180;
		UpdateMemoryAddress((void *)((BYTE*)DMenuAddrC + 0x16), (void *)&UpdateVal, 4);
	}

	//Function to update "Now loading" text in Main Menu
	void *DMenuAddrD = CheckMultiMemoryAddress((void*)0x004457C0, 0x00000000, 0x00000000, (void*)MenuSearchBytesE, sizeof(MenuSearchBytesE));
	if (DMenuAddrD)
	{
		LoadMes = (DWORD(*)(int pos))DMenuAddrD;
	}
	else
	{
		LoadMes = (DWORD(*)(int pos))(((BYTE*)DMenuAddrB - 0x21) + *(int *)((BYTE*)DMenuAddrB - 0x25));
	}
}

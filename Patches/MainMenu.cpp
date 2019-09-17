/**
* Copyright (C) 2019 Elisha Riedlinger
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
#include "Common\FileSystemHooks.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"
#include "Common\Settings.h"
#include <string>

using namespace std;

constexpr BYTE MenuSearchBytesA[] = { 0x24, 0x14, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x6D, 0xFF, 0x24, 0x85, 0x3C, 0x5D, 0x49, 0x00, 0xBF };

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
	void *DMenuAddrA = GetAddressOfData(MenuSearchBytesA, sizeof(MenuSearchBytesA), 1, 0x00495A90, 1800); //00495AA0

	// Checking address pointer
	if (!DMenuAddrA) {
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

typedef struct {
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

TexPath start00Offset;
TexPath start01Offset;

char start00CustomPath[255];
char start01CustomPath[255];

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
	sprintf_s((char *)start00CustomPath, sizeof(start00CustomPath), "data/pic/etc/start00%c.tex", lang);
	start00Offset.unk1 = -1;
	start00Offset.unk3 = 0;
	ifstream file(std::string(std::string(ModPathA) + "\\pic\\etc\\start00" + lang + ".tex").c_str());
	if (!file.is_open()) {
		start00Offset.pathPtr = "data/pic/etc/start00.tex";
	}
	else {
		start00Offset.pathPtr = (char *)start00CustomPath;
		file.close();
	}
#endif
	sprintf_s((char *)start01CustomPath, sizeof(start01CustomPath), "data/pic/etc/start01%c.tex", lang);
	start01Offset.size = -1;
	start01Offset.unk3 = 0;
	ifstream file(std::string(std::string(ModPathA) + "\\pic\\etc\\start01" + lang + ".tex").c_str());
	if (!file.is_open()) {
		start01Offset.pathPtr = "data/pic/etc/start01.tex";
	} else {
		start01Offset.pathPtr = (char *)start01CustomPath;
		file.close();
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
	void *DMenuAddrB = GetAddressOfData(MenuSearchBytesB, sizeof(MenuSearchBytesB), 1, 0x00496F00, 1800); //00496F24

	// Checking address pointer
	if (!DMenuAddrB) {
		DMenuAddrB = GetAddressOfData(MenuSearchBytesC, sizeof(MenuSearchBytesC), 1, 0x004971C0, 1800); //004971CE
		if (!DMenuAddrB) {
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

	if (DMenuAddrB && UseCustomExeStr) {
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
	void *DMenuAddrC = GetAddressOfData(MenuSearchBytesD, sizeof(MenuSearchBytesD), 1, 0x00497A00, 1800); //00497A08 00497CB8
	if (DMenuAddrC) {
		UpdateVal = -244;
		UpdateMemoryAddress((void *)((BYTE*)DMenuAddrC - 0x25), (void *)&UpdateVal, 4);
		UpdateVal = 508;
		UpdateMemoryAddress((void *)((BYTE*)DMenuAddrC - 0x1A), (void *)&UpdateVal, 2);
		UpdateVal = -180;
		UpdateMemoryAddress((void *)((BYTE*)DMenuAddrC + 0x16), (void *)&UpdateVal, 4);
	}

	//Function to update "Now loading" text in Main Menu
	void *DMenuAddrD = GetAddressOfData(MenuSearchBytesE, sizeof(MenuSearchBytesE), 1, 0x004457B0, 1800); //004457C0
	if (DMenuAddrD) {
		LoadMes = (DWORD(*)(int pos))DMenuAddrD;
	} else {
		LoadMes = (DWORD(*)(int pos))(((BYTE*)DMenuAddrB - 0x21) + *(int *)((BYTE*)DMenuAddrB - 0x25));
	}
}

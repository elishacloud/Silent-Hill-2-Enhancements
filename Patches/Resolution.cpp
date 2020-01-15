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
#include "Common\Utils.h"
#include "Logging\Logging.h"
#include "Common\Settings.h"
#include <string>

using namespace std;

unsigned int gWidth;
unsigned int gHeight;

DWORD (*prepText)(char *str);
DWORD (*printTextPos)(char *str, int x, int y);

char resStrBuf[64];
char *resStrPtr;

int printResStr(unsigned short, unsigned char, int x, int y)
{
	sprintf_s((char *)resStrBuf, sizeof(resStrBuf), "\\h%d x %d", gWidth, gHeight);
	resStrPtr = (char *)prepText(resStrBuf);
	return printTextPos(resStrPtr, x, y);
}

extern char *getResolutionDescStr();

int printResDescStr(unsigned short, unsigned char, int x, int y)
{
	char *ptr = (char *)prepText(getResolutionDescStr());
	return printTextPos(ptr, x, y);
}

void *ResSelectStrRetAddr;

__declspec(naked) void __stdcall ResSelectStrASM()
{
	__asm
	{
		call printResStr
		add	esp, 30h
		jmp ResSelectStrRetAddr
	}
}

void *ResArrowRetAddr;

__declspec(naked) void __stdcall ResArrowASM()
{
	__asm
	{
		mov eax, resStrPtr
		push eax
		jmp ResArrowRetAddr
	}
}

void UpdateResolutionLock(DWORD Width, DWORD Height)
{
	constexpr BYTE ResSearchBytesA[] = { 0x94, 0x00, 0x68, 0xB3, 0x00, 0x00, 0x00, 0x05, 0xB0, 0x00, 0x00, 0x00, 0xE9, 0xCD, 0x02, 0x00, 0x00, 0xA0, 0x1C };
	void *DResAddrA = (void*)SearchAndGetAddresses(0x0046565C, 0x004658F8, 0x00465B08, ResSearchBytesA, sizeof(ResSearchBytesA), 0x00);
	constexpr BYTE ResSearchBytesB[] = { 0x8B, 0x08, 0x83, 0xC4, 0x10, 0x68, 0xA4, 0x00, 0x00, 0x00, 0x68, 0x00, 0x01, 0x00, 0x00, 0x51, 0xE8 };
	void *DResAddrB = (void*)SearchAndGetAddresses(0x00407368, 0x00407368, 0x00407378, ResSearchBytesB, sizeof(ResSearchBytesB), 0x00);

	// Checking address pointer
	if (!DResAddrA || !DResAddrB)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	gWidth = Width;
	gHeight = Height;

	unsigned int *ResSelectorAddr = (unsigned int *)((BYTE*)DResAddrA + 0x879);
	int exitOffset = 0x2ED;
	int prtStrOffset = 0x3B0;
	int arrowOffset = 0x35F;

	if (*ResSelectorAddr != 0x00046855)
	{
		ResSelectorAddr = (unsigned int *)((BYTE*)DResAddrA + 0x87F);
		if (*ResSelectorAddr != 0x00046855)
		{
			return;
		}
		exitOffset = 0x2EB;
		prtStrOffset = 0x3B6;
		arrowOffset = 0x365;
	}

	Logging::Log() << "Enabling Resolution Lock...";

	// Get functions
	prepText = (DWORD(*)(char *str))(((BYTE*)DResAddrB + 0x15) + *(int *)((BYTE*)DResAddrB + 0x11));
	printTextPos = (DWORD(*)(char *str, int x, int y))(((BYTE*)DResAddrB + 0x1E) + *(int *)((BYTE*)DResAddrB + 0x1A));

	// Lock resolution
	void *ResSelectorAddrExit = (void *)((BYTE*)ResSelectorAddr + exitOffset);
	WriteJMPtoMemory((BYTE*)ResSelectorAddr, ResSelectorAddrExit, 5);
	
	// Update resolution strings
	ResSelectStrRetAddr = (DWORD *)(((BYTE*)DResAddrA + 0x92) + *(int *)((BYTE*)DResAddrA + 0x8E) + 8);
	WriteJMPtoMemory(((BYTE*)DResAddrA + 0x8D), *ResSelectStrASM, 5);
	WriteCalltoMemory(((BYTE*)DResAddrA - prtStrOffset), *printResStr, 5);

	// Update arrow position
	BYTE codeA[] = { 0x31, 0xC0, 0x90, 0x90, 0x90, 0x90, 0x90 };
	UpdateMemoryAddress((void *)((BYTE*)DResAddrA + arrowOffset), (void *)&codeA, sizeof(codeA));
	ResArrowRetAddr = (DWORD *)(((BYTE*)DResAddrA + arrowOffset + 0x27) + *(int *)((BYTE*)DResAddrA + arrowOffset + 0x23) + 6);
	WriteJMPtoMemory(((BYTE*)DResAddrA + arrowOffset + 0x22), *ResArrowASM, 5);

	// Update resolution description string
	if (UseCustomExeStr)
	{
		WriteCalltoMemory(((BYTE*)DResAddrA + 0x55), *printResDescStr, 5);
	}
}

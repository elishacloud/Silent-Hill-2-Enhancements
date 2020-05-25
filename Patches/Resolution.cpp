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
#include "Patches.h"
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

void SetResolutionLock(DWORD Width, DWORD Height)
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

static DWORD* gFogOn;
static DWORD* gShadowsOn;
static DWORD* gLensOn;
static DWORD* gLowResTexOn;
static DWORD* gMotionBlurOn;
static DWORD* gDepthFieldOn;
static DWORD* gScreenXPos;
static DWORD* gScreenYPos;

static void AdvOptionsSets()
{
	*gFogOn = 1;
	*gShadowsOn = 1;
	*gLensOn = 1;
	*gLowResTexOn = 0;
	*gMotionBlurOn = 1;
	*gDepthFieldOn = 1;
	*gScreenXPos = 0;
	*gScreenYPos = 0;
}

void PatchBestGraphics()
{
	constexpr BYTE OptSearchBytesA[] = { 0x83, 0xEC, 0x20, 0x53, 0x56, 0x57, 0x33, 0xC0, 0xBE, 0x01, 0x00, 0x00, 0x00, 0xB9, 0x08, 0x00 };
	void* DOptAddrA = (void*)SearchAndGetAddresses(0x004F6F70, 0x004F7220, 0x004F6AE0, OptSearchBytesA, sizeof(OptSearchBytesA), 0x00);
	constexpr BYTE OptSearchBytesB[] = { 0x53, 0x55, 0x8B, 0x6C, 0x24, 0x0C, 0x56, 0x33, 0xF6, 0x3B, 0xEE, 0x57, 0xBB, 0x01, 0x00, 0x00 };
	void* DOptAddrB = (void*)SearchAndGetAddresses(0x004F70E0, 0x004F7410, 0x004F6D30, OptSearchBytesB, sizeof(OptSearchBytesB), 0x00);

	// Checking address pointer
	if (!DOptAddrA || !DOptAddrB)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	Logging::Log() << "Enabling Best Graphics settings...";

	gFogOn = (DWORD*)*(DWORD*)((BYTE*)DOptAddrA + 0x2B);
	gShadowsOn = (DWORD*)*(DWORD*)((BYTE*)DOptAddrA + 0x31);
	gLensOn = (DWORD*)*(DWORD*)((BYTE*)DOptAddrA + 0x37);
	gMotionBlurOn = (DWORD*)*(DWORD*)((BYTE*)DOptAddrA + 0x3D);
	gDepthFieldOn = (DWORD*)*(DWORD*)((BYTE*)DOptAddrA + 0x43);
	gScreenXPos = (DWORD*)*(DWORD*)((BYTE*)DOptAddrA + 0x53);
	gScreenYPos = (DWORD*)*(DWORD*)((BYTE*)DOptAddrA + 0x59);
	gLowResTexOn = (DWORD*)*(DWORD*)((BYTE*)DOptAddrA + 0x95);

	if (GameVersion == SH2V_DC)
		WriteCalltoMemory(((BYTE*)DOptAddrB + 0x77), AdvOptionsSets, 11);
	else
		WriteCalltoMemory(((BYTE*)DOptAddrB + 0x78), AdvOptionsSets, 11);
}

static void* LabelA;
static WORD* MenuPos;
static void* LockScreenPosRetAddr;

__declspec(naked) void __stdcall LockScreenPosASM()
{
	__asm
	{
		cmp		ax, 1
		jnz		_LabelB
		lea     eax, [ecx + 1]
		mov		edx, MenuPos
		mov     [edx], ax
	_LabelB:
		cmp		ax, 7
		jnz		_LabelA
		jmp		LockScreenPosRetAddr
	_LabelA:
		jmp		LabelA
	}
}

static DWORD (*setFontColor)(BYTE r, BYTE g, BYTE b, BYTE alpha);
static DWORD (*printTexFromMes)(short *buf, short idx, int x, int y);
static void* PrintScreenPosOptionRetAddr;

__declspec(naked) void __stdcall PrintScreenPosOptionASM()
{
	__asm
	{
		push	96
		push	51
		push	51
		push	51
		call	setFontColor
		add     esp, 16
		call	printTexFromMes
		push	96
		push	63
		push	63
		push	63
		call	setFontColor
		add     esp, 16
		jmp		PrintScreenPosOptionRetAddr
	}
}

void PatchLockScreenPosition()
{
	constexpr BYTE PosSearchBytesA[] = { 0x94, 0x00, 0x68, 0xB3, 0x00, 0x00, 0x00, 0x05, 0xB0, 0x00, 0x00, 0x00, 0xE9, 0xCD, 0x02, 0x00, 0x00, 0xA0, 0x1C };
	void* DPosAddrA = (void*)SearchAndGetAddresses(0x0046565C, 0x004658F8, 0x00465B08, PosSearchBytesA, sizeof(PosSearchBytesA), 0x00);
	constexpr BYTE PosSearchBytesB[] = { 0x94, 0x00, 0x01, 0x00, 0x00, 0x00, 0x6A, 0x00, 0x68, 0x01, 0x00, 0x00, 0x04, 0x6A, 0x00, 0xE8 };
	void* DPosAddrB = (void*)SearchAndGetAddresses(0x0045FBB8, 0x0045FE18, 0x0045FE18, PosSearchBytesB, sizeof(PosSearchBytesB), 0x00);

	// Checking address pointer
	if (!DPosAddrA || !DPosAddrB)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	Logging::Log() << "Enabling Screen Position option lock...";

	unsigned int* ResSelectorAddr = (unsigned int*)((BYTE*)DPosAddrA + 0x879);
	int exitOffset = 0x2ED;
	if (*ResSelectorAddr != 0x00046855)
	{
		ResSelectorAddr = (unsigned int*)((BYTE*)DPosAddrA + 0x87F);
		if (*ResSelectorAddr != 0x00046855)
		{
			return;
		}
		exitOffset = 0x2EB;
	}
	void* PosAddrExit = (void*)((BYTE*)ResSelectorAddr + exitOffset);

	// Lock entering to the screen position menu
	unsigned int* ScreenPosAddr = (unsigned int*)((BYTE*)DPosAddrA + 0x742);
	if (*ScreenPosAddr != 0x55106A55)
	{
		ScreenPosAddr = (unsigned int*)((BYTE*)DPosAddrA + 0x748);
		if (*ScreenPosAddr != 0x55106A55)
		{
			return;
		}
	}
	WriteJMPtoMemory((BYTE*)ScreenPosAddr, PosAddrExit, 5);

	// Set text colour to grey
	void* OptDefault;
	if (GameVersion == SH2V_10)
	{
		setFontColor = (DWORD(*)(BYTE r, BYTE g, BYTE b, BYTE alpha))(((BYTE*)DPosAddrA - 0x66C) + *(int*)((BYTE*)DPosAddrA - 0x670));
		printTexFromMes = (DWORD(*)(short* buf, short idx, int x, int y))(((BYTE*)DPosAddrA - 0x63A) + *(int*)((BYTE*)DPosAddrA - 0x63E));
		PrintScreenPosOptionRetAddr = (void*)((BYTE*)DPosAddrA - 0x63A);
		WriteJMPtoMemory((BYTE*)DPosAddrA - 0x63F, *PrintScreenPosOptionASM, 5);

		OptDefault = (void*)(((BYTE*)DPosAddrA - 0xFA) + *(short*)((BYTE*)DPosAddrA - 0xFE));
		WriteJMPtoMemory((BYTE*)DPosAddrA - 0x136, OptDefault, 5);
	}
	else
	{
		setFontColor = (DWORD(*)(BYTE r, BYTE g, BYTE b, BYTE alpha))(((BYTE*)DPosAddrA - 0x678) + *(int*)((BYTE*)DPosAddrA - 0x67C));
		printTexFromMes = (DWORD(*)(short* buf, short idx, int x, int y))(((BYTE*)DPosAddrA - 0x646) + *(int*)((BYTE*)DPosAddrA - 0x64A));
		PrintScreenPosOptionRetAddr = (void*)((BYTE*)DPosAddrA - 0x646);
		WriteJMPtoMemory((BYTE*)DPosAddrA - 0x64B, *PrintScreenPosOptionASM, 5);

		OptDefault = (void*)(((BYTE*)DPosAddrA - 0x100) + *(short*)((BYTE*)DPosAddrA - 0x104));
		WriteJMPtoMemory((BYTE*)DPosAddrA - 0x13C, OptDefault, 5);
	}

	// Disable option
	LabelA = (DWORD*)(((BYTE*)DPosAddrB + 0x89) + *(BYTE*)((BYTE*)DPosAddrB + 0x88));
	MenuPos = (WORD*)*(DWORD*)((BYTE*)DPosAddrB + 0xD2);
	LockScreenPosRetAddr = (void*)((BYTE*)DPosAddrB + 0x89);
	WriteJMPtoMemory((BYTE*)DPosAddrB + 0x83, *LockScreenPosASM, 6);
}

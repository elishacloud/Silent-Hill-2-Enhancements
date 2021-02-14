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
#include <vector>
#include "Common\Utils.h"
#include "WidescreenFixesPack\WidescreenFixesPack.h"
#include "Wrappers\d3d8\d3d8wrapper.h"
#include "Logging\Logging.h"
#include "Common\Settings.h"
#include "Patches.h"
#include <string>

using namespace std;

struct RESOLUTONLIST
{
	DWORD Width;
	DWORD Height;
};

BYTE *ResolutionIndex = nullptr;
BYTE *TextResIndex = nullptr;
RESOLUTONLIST *ResolutionArray = nullptr;
std::vector<RESOLUTONLIST> ResolutionVector;

DWORD (*prepText)(char *str);
DWORD (*printTextPos)(char *str, int x, int y);

char resStrBuf[64];
char *resStrPtr;

void GetCurrentResolution(int &Width, int &Height)
{
	Width = ResolutionArray[*ResolutionIndex].Width;
	Height = ResolutionArray[*ResolutionIndex].Height;
}

void GetTextResolution(int &Width, int &Height)
{
	Width = ResolutionArray[*TextResIndex].Width;
	Height = ResolutionArray[*TextResIndex].Height;
}

int printResStr(unsigned short, unsigned char, int x, int y)
{
	int gWidth, gHeight;
	GetTextResolution(gWidth, gHeight);

	char* text = "\\h%dx%d";
	if (abs((float)gWidth / 3 - (float)gHeight / 2) < 1.0f)
		text = "\\h%dx%d (3:2)";
	else if (abs((float)gWidth / 4 - (float)gHeight / 3) < 1.0f)
		text = "\\h%dx%d (4:3)";
	else if (abs((float)gWidth / 5 - (float)gHeight / 3) < 1.0f)
		text = "\\h%dx%d (5:3)";
	else if (abs((float)gWidth / 5 - (float)gHeight / 4) < 1.0f)
		text = "\\h%dx%d (5:4)";
	else if (abs((float)gWidth / 7 - (float)gHeight / 5) < 1.0f)
		text = "\\h%dx%d (7:5)";
	else if (abs((float)gWidth / 9 - (float)gHeight / 5) < 1.0f)
		text = "\\h%dx%d (9:5)";
	else if (abs((float)gWidth / 16 - (float)gHeight / 5) < 1.0f)
		text = "\\h%dx%d (16:5)";
	else if (abs((float)gWidth / 16 - (float)gHeight / 9) < 1.0f)
		text = "\\h%dx%d (16:9)";
	else if (abs((float)gWidth / 16 - (float)gHeight / 10) < 1.0f)
		text = "\\h%dx%d (16:10)";
	else if (abs((float)gWidth / 17 - (float)gHeight / 9) < 1.0f)
		text = "\\h%dx%d (17:9)";
	else if (abs((float)gWidth / 21 - (float)gHeight / 9) < 1.0f)
		text = "\\h%dx%d (21:9)";
	else if (abs((float)gWidth / 25 - (float)gHeight / 16) < 1.0f)
		text = "\\h%dx%d (25:16)";
	else if (abs((float)gWidth / 48 - (float)gHeight / 9) < 1.0f)
		text = "\\h%dx%d (48:9)";

	sprintf_s((char *)resStrBuf, sizeof(resStrBuf), text, gWidth, gHeight);
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

void WSFDynamicStartup()
{
	GetCurrentResolution(ResX, ResY);
	WSFInit();
}

__declspec(naked) void __stdcall StartupResASM()
{
	__asm
	{
		pushf
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		mov eax, dword ptr ds : [ResolutionIndex]
		mov dword ptr ds : [eax], ebx		// ebx stores the currnetly used resolution index here
		call WSFDynamicStartup
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		popf
		retn
	}
}

void WSFDynamicChange()
{
	GetTextResolution(ResX, ResY);
	WSFInit();
}

__declspec(naked) void __stdcall ChangeResASM()
{
	__asm
	{
		pushf
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		call WSFDynamicChange
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		popf
		mov dword ptr ds : [ecx], eax
		retn
	}
}

void GetCustomResolutions()
{
	RESOLUTONLIST Resolution;
	IDirect3D8 *pDirect3D = Direct3DCreate8Wrapper(D3D_SDK_VERSION);

	if (!pDirect3D)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to create Direct3D8!";
		return;
	}

	// Setup display mode
	D3DDISPLAYMODE d3ddispmode;

	// Enumerate modes for format XRGB
	UINT modeCount = pDirect3D->GetAdapterModeCount(D3DADAPTER_DEFAULT);

	// Loop through each mode
	for (UINT i = 0; i < modeCount; i++)
	{
		if (FAILED(pDirect3D->EnumAdapterModes(D3DADAPTER_DEFAULT, i, &d3ddispmode)))
		{
			break;
		}
		bool found = false;
		for (auto res : ResolutionVector)
		{
			if (res.Width == d3ddispmode.Width && res.Height == d3ddispmode.Height)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			Resolution.Width = d3ddispmode.Width;
			Resolution.Height = d3ddispmode.Height;
			ResolutionVector.push_back(Resolution);
		}
		if (ResolutionVector.size() >= 0xFF)
		{
			break;
		}
	}

	// Check if any resolutions were found
	if (ResolutionVector.empty())
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get resolution list!";
		return;
	}

	// Release Direct3D8
	pDirect3D->Release();
}

void SetResolutionLock()
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

	unsigned int *ResSelectorAddr = (unsigned int *)((BYTE*)DResAddrA + 0x879);
	int exitOffset = 0x2ED;
	int prtStrOffset = 0x3B0;
	int arrowOffset = 0x35F;

	if (*ResSelectorAddr != 0x00046855)
	{
		ResSelectorAddr = (unsigned int *)((BYTE*)DResAddrA + 0x87F);
		if (*ResSelectorAddr != 0x00046855)
		{
			Logging::Log() << __FUNCTION__ << " Error: wrong memory address!";
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

	// Get text resolution index
	constexpr BYTE TextResIndexSearchBytes[] = { 0x68, 0x8B, 0x01, 0x00, 0x00, 0x6A, 0x46, 0x68, 0xD1, 0x00, 0x00, 0x00, 0x52, 0xE8 };
	TextResIndex = (BYTE*)ReadSearchedAddresses(0x00465631, 0x004658CD, 0x00465ADD, TextResIndexSearchBytes, sizeof(TextResIndexSearchBytes), 0x29);
	constexpr BYTE ResolutionIndexSearchBytes[] = { 0x6A, 0x01, 0x6A, 0x00, 0x50, 0xFF, 0x51, 0x34, 0x6A, 0x00, 0xE8 };
	ResolutionIndex = (BYTE*)ReadSearchedAddresses(0x004F633F, 0x004F65EF, 0x004F5EAF, ResolutionIndexSearchBytes, sizeof(ResolutionIndexSearchBytes), 0x10);
	constexpr BYTE ResolutionArraySearchBytes[] = { 0x10, 0x51, 0x56, 0x6A, 0x00, 0x50, 0xFF, 0x52, 0x1C, 0x8B, 0x54, 0x24, 0x10 };
	ResolutionArray = (RESOLUTONLIST*)ReadSearchedAddresses(0x004F5DEB, 0x004F609B, 0x004F595B, ResolutionArraySearchBytes, sizeof(ResolutionArraySearchBytes), 0xF);
	BYTE *ResStartupAddr = (BYTE*)SearchAndGetAddresses(0x004F5DEB, 0x004F609B, 0x004F595B, ResolutionArraySearchBytes, sizeof(ResolutionArraySearchBytes), 0x4B);
	constexpr BYTE ResSizeSearchBytes[] = { 0x3C, 0x05, 0x73, 0x04, 0xFE, 0xC0, 0xEB, 0x02, 0x32, 0xC0, 0x0F, 0xB6, 0xC8, 0x51, 0xA2 };
	BYTE *ResSizeAddr = (BYTE*)SearchAndGetAddresses(0x00465F2A, 0x004661CC, 0x004663DC, ResSizeSearchBytes, sizeof(ResSizeSearchBytes), 0x00);
	if (!TextResIndex || !ResolutionIndex || !ResolutionArray || !ResStartupAddr || !ResSizeAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory addresses!";
		return;
	}
	TextResIndex += 1;

	// Dynamic resolution
	if (WidescreenFix && DynamicResolution)
	{
		GetCustomResolutions();

		if (ResolutionVector.size())
		{
			ResolutionArray = &ResolutionVector[0];

			BYTE *ArrayWidth = (BYTE*)ResolutionArray;
			BYTE *ArrayHeight = (BYTE*)ResolutionArray + 4;

			UpdateMemoryAddress((void *)(ResStartupAddr - 0x3E + 2), &ArrayWidth, sizeof(DWORD));
			UpdateMemoryAddress((void *)(ResStartupAddr - 0x32 + 2), &ArrayHeight, sizeof(DWORD));

			UpdateMemoryAddress((void *)(ResStartupAddr + 0x3E + 3), &ArrayWidth, sizeof(DWORD));
			UpdateMemoryAddress((void *)(ResStartupAddr + 0x4B + 3), &ArrayHeight, sizeof(DWORD));

			UpdateMemoryAddress((void *)(ResStartupAddr + 0x51D + 3), &ArrayWidth, sizeof(DWORD));
			UpdateMemoryAddress((void *)(ResStartupAddr + 0x529 + 3), &ArrayHeight, sizeof(DWORD));

			// Set the number of resolutions in the list
			if (ResolutionVector.size() != 6)
			{
				BYTE ListSize = (BYTE)ResolutionVector.size();
				UpdateMemoryAddress((void *)(ResStartupAddr + 0x12 + 2), &ListSize, sizeof(BYTE));
				--ListSize;
				UpdateMemoryAddress((void *)(ResSizeAddr + 1), &ListSize, sizeof(BYTE));
				DWORD delta = (GameVersion == SH2V_10) ? 0x33 : 0x31;
				UpdateMemoryAddress((void *)(ResSizeAddr + delta + 1), &ListSize, sizeof(BYTE));
			}
		}

		WriteJMPtoMemory(ResStartupAddr, *StartupResASM);
		UpdateMemoryAddress((ResStartupAddr + 0x56), "\xEB\x1B\x90", 3);		// Jump near to 0x1B
		WriteJMPtoMemory((ResStartupAddr + 0x56 + 0x02 + 0x1B), *ChangeResASM);
	}

	// Check if resolution is locked
	if (*(DWORD*)((BYTE*)ResolutionArray) == *(DWORD*)((BYTE*)ResolutionArray + 8) &&
		*(DWORD*)((BYTE*)ResolutionArray + 4) == *(DWORD*)((BYTE*)ResolutionArray + 12))
	{
		// Lock resolution
		void *ResSelectorAddrExit = (void *)((BYTE*)ResSelectorAddr + exitOffset);
		WriteJMPtoMemory((BYTE*)ResSelectorAddr, ResSelectorAddrExit, 5);

		// Update resolution description string
		if (UseCustomExeStr)
		{
			WriteCalltoMemory(((BYTE*)DResAddrA + 0x55), *printResDescStr, 5);
		}
	}

	// Update resolution strings
	ResSelectStrRetAddr = (DWORD *)(((BYTE*)DResAddrA + 0x92) + *(int *)((BYTE*)DResAddrA + 0x8E) + 8);
	WriteJMPtoMemory(((BYTE*)DResAddrA + 0x8D), *ResSelectStrASM, 5);
	WriteCalltoMemory(((BYTE*)DResAddrA - prtStrOffset), *printResStr, 5);

	// Update arrow position
	BYTE codeA[] = { 0x31, 0xC0, 0x90, 0x90, 0x90, 0x90, 0x90 };
	UpdateMemoryAddress((void *)((BYTE*)DResAddrA + arrowOffset), (void *)&codeA, sizeof(codeA));
	ResArrowRetAddr = (DWORD *)(((BYTE*)DResAddrA + arrowOffset + 0x27) + *(int *)((BYTE*)DResAddrA + arrowOffset + 0x23) + 6);
	WriteJMPtoMemory(((BYTE*)DResAddrA + arrowOffset + 0x22), *ResArrowASM, 5);
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

extern char* getSpeakerConfigDescStr();

int printSpkDescStr(unsigned short, unsigned char, int x, int y)
{
	char* ptr = (char*)prepText(getSpeakerConfigDescStr());
	return printTextPos(ptr, x, y);
}

void PatchSpeakerConfigLock()
{
	constexpr BYTE SpkSearchBytesA[] = { 0x94, 0x00, 0x68, 0x12, 0x27, 0x00, 0x00, 0xF3, 0xA5, 0xE8 };
	void* DSpkAddrA = (void*)SearchAndGetAddresses(0x00463165, 0x004633D5, 0x004633E4, SpkSearchBytesA, sizeof(SpkSearchBytesA), 0x00);
	constexpr BYTE SpkSearchBytesB[] = { 0x93, 0x00, 0x83, 0xC4, 0x10, 0x68, 0x1F, 0x01, 0x00, 0x00, 0x68, 0x0C, 0x01, 0x00, 0x00, 0x05, 0xC2, 0x00, 0x00, 0x00, 0x50, 0x51, 0xE8 };
	void* DSpkAddrB = (void*)SearchAndGetAddresses(0x00461B37, 0x00461DA9, 0x00461DA9, SpkSearchBytesB, sizeof(SpkSearchBytesB), 0x00);

	if (!LockResolution)
	{
		constexpr BYTE SpkSearchBytesC[] = { 0x8B, 0x08, 0x83, 0xC4, 0x10, 0x68, 0xA4, 0x00, 0x00, 0x00, 0x68, 0x00, 0x01, 0x00, 0x00, 0x51, 0xE8 };
		void* DSpkAddrC = (void*)SearchAndGetAddresses(0x00407368, 0x00407368, 0x00407378, SpkSearchBytesC, sizeof(SpkSearchBytesC), 0x00);

		// Checking address pointer
		if (!DSpkAddrC)
		{
			Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
			return;
		}

		// Get functions
		prepText = (DWORD(*)(char* str))(((BYTE*)DSpkAddrC + 0x15) + *(int*)((BYTE*)DSpkAddrC + 0x11));
		printTextPos = (DWORD(*)(char* str, int x, int y))(((BYTE*)DSpkAddrC + 0x1E) + *(int*)((BYTE*)DSpkAddrC + 0x1A));
	}

	// Checking address pointer
	if (!DSpkAddrA || !DSpkAddrB)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	void* SpkSelectorAddr;
	int exitOffset = 0x299;
	if (GameVersion == SH2V_10)
	{
		SpkSelectorAddr = (void*)((BYTE*)DSpkAddrA + 0xC90);
	}
	else if (GameVersion == SH2V_11)
	{
		SpkSelectorAddr = (void*)((BYTE*)DSpkAddrA + 0xC99);
	}
	else if (GameVersion == SH2V_DC)
	{
		SpkSelectorAddr = (void*)((BYTE*)DSpkAddrA + 0xE6D);
		exitOffset = 0x2B1;
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error: unknown game version!";
		return;
	}

	Logging::Log() << "Enabling Speaker Config Lock...";

	// Lock speaker config
	void* SpkSelectorAddrExit = (void*)((BYTE*)SpkSelectorAddr + exitOffset);
	WriteJMPtoMemory((BYTE*)SpkSelectorAddr, SpkSelectorAddrExit, 5);

	// Update speaker config description string
	if (UseCustomExeStr)
	{
		WriteCalltoMemory(((BYTE*)DSpkAddrB + 0x125), *printSpkDescStr, 5);
	}
}

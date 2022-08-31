/**
* Copyright (C) 2022 Elisha Riedlinger
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
#include "Wrappers\d3d8\d3d8wrapper.h"

typedef struct {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwRGBAlphaBitMask;
} DDPIXELFORMAT;

typedef struct {
	DWORD dwCaps1;
	DWORD dwCaps2;
	DWORD Reserved[2];
} DDCAPS2;

typedef struct {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwHeight;
	DWORD dwWidth;
	DWORD dwPitchOrLinearSize;
	DWORD dwDepth;
	DWORD dwMipMapCount;
	DWORD dwReserved1[11];
	DDPIXELFORMAT ddpfPixelFormat;
	DDCAPS2 ddsCaps;
	DWORD dwReserved2;
} DDS_HEADER;

using namespace std;

// Predefined code bytes
constexpr BYTE DFontFuncBlockA[] = { 0x00, 0x83, 0xEC, 0x24, 0x85, 0xC0, 0x56, 0x57, 0x0F, 0x85, 0x12, 0x01 };
constexpr BYTE DFontFuncBlockB[] = { 0x19, 0x00, 0x00, 0x00, 0xF7, 0xFE, 0xC1, 0xE7, 0x04, 0x66, 0x89, 0x79, 0x02, 0xC1, 0xE3, 0x04 };
constexpr BYTE DFontFuncBlockC[] = { 0xBE, 0x19, 0x00, 0x00, 0x00, 0xF7, 0xFE, 0x83, 0xC4, 0x04, 0x6B, 0xD2, 0x16, 0x66, 0x89, 0x51, 0x08, 0xC1, 0xE0, 0x05 };
constexpr BYTE DFontFuncBlockD[] = { 0x00, 0x5F, 0x7F, 0x9F, 0xBF, 0xDF, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00 };
constexpr BYTE DFontFuncBlockE[] = { 0x8B, 0x44, 0x24, 0x08, 0x81, 0xEC, 0x80, 0x00, 0x00, 0x00, 0x53, 0x55, 0x56, 0x57, 0x33, 0xFF, 0x83, 0xC9, 0xFF };
constexpr BYTE DFontFuncBlockF[] = { 0x83, 0xEC, 0x0C, 0x55, 0x8B, 0x6C, 0x24, 0x18, 0x33, 0xD2, 0x3B, 0xEA, 0x75, 0x07, 0x33, 0xC0, 0x5D, 0x83, 0xC4 };

static unsigned int fontTexWidth = 550;
static unsigned int fontTexHeight = 544;
static float fontTexScaleW = 1.0f;
static float fontTexScaleH = 1.0f;
static BYTE charW = 22;
static BYTE charH = 32;
static BYTE charIX = 100;
static BYTE charIY = 150;
static WORD nFontW = 20;
static WORD nFontH = 30;
static WORD sFontW = 16;
static WORD sFontH = 24;
static BYTE letterSpc = 2;
static BYTE spaceSize = 7;

static D3DFORMAT fontFormat = D3DFMT_A8R8G8B8;
static BYTE *fontData;
static DWORD fontDataSize;

static BOOL LoadDDSFont(void);

static void UpdateFontTextureData(BYTE *ptr, int size)
{
	Logging::LogDebug() << __FUNCTION__ << " - address: " << (DWORD)ptr << ", size: " << size;

	if (fontData)
	{
		memcpy(ptr, fontData, fontDataSize);
	}

	return;
}

static int ReturnFontIdx(int fontSize, WORD charID)
{
	int charMax = (charIX * charIY) / 2;

	if (charID > charMax)
	{
		return 0;
	}

	return charID + fontSize * charMax;
}

static void* CreateFontTextureRetAddr;

__declspec(naked) void __stdcall CreateFontTextureASM()
{
	__asm
	{
		push	1
		mov		edi, fontFormat
		push	edi
		push	0
		push	1
		jmp		CreateFontTextureRetAddr
	}
}

static void* SpaceSizeRetAddr;

__declspec(naked) void __stdcall SpaceSizeASM()
{
	__asm
	{
		push	esi
		movsx   esi, spaceSize
		imul	eax, esi
		pop		esi
		cdq
		idiv    ecx
		jmp		SpaceSizeRetAddr
	}
}

static void* SpaceSizeRetAddr2;

__declspec(naked) void __stdcall SpaceSizeASM2()
{
	__asm
	{
		push	esi
		movsx   esi, spaceSize
		imul	eax, esi
		pop		esi
		cdq
		idiv    ecx
		jmp		SpaceSizeRetAddr2
	}
}

static void* SpaceSizeRetAddr3;

__declspec(naked) void __stdcall SpaceSizeASM3()
{
	__asm
	{
		push	esi
		movsx   esi, spaceSize
		imul	eax, esi
		pop		esi
		cdq
		idiv    esi
		jmp		SpaceSizeRetAddr3
	}
}

void PatchCustomFonts()
{
	// Find address for font decode function
	void *DFontAddrA = (void*)SearchAndGetAddresses(0x004809F4, 0x00480C94, 0x00480EA4, DFontFuncBlockA, sizeof(DFontFuncBlockA), 0x00);
	void *DFontAddrB = (void*)SearchAndGetAddresses(0x0047E270, 0x0047E510, 0x0047E720, DFontFuncBlockB, sizeof(DFontFuncBlockB), 0x00);

	// Address found
	if (!DFontAddrA || !DFontAddrB)
	{
		Logging::Log() << __FUNCTION__ << " Error: Could not find font decode function address in memory!";
		return;
	}

	charIX = (BYTE)CustomFontCol;
	charIY = (BYTE)CustomFontRow;
	charW = (BYTE)CustomFontCharWidth;
	charH = (BYTE)CustomFontCharHeight;
	nFontW = (WORD)NormalFontWidth;
	nFontH = (WORD)NormalFontHeight;
	sFontW = (WORD)SmallFontWidth;
	sFontH = (WORD)SmallFontHeight;
	letterSpc = (BYTE)LetterSpacing;
	spaceSize = (BYTE)SpaceSize;

	if (!LoadDDSFont())
	{
		Logging::Log() << __FUNCTION__ << " Error: Could not find font file";
		return;
	}

	Logging::Log() << "Enabling Custom Fonts...";
	fontTexScaleW = 550.0f / (float)(charIX * charW);
	fontTexScaleH = 544.0f / (float)(charIY * charH);

	UpdateMemoryAddress((void *)((BYTE*)DFontAddrA + 0x17), (void *)&fontTexWidth, 4);
	UpdateMemoryAddress((void *)((BYTE*)DFontAddrA + 0x1C), (void *)&fontTexHeight, 4);
	UpdateMemoryAddress((void *)((BYTE*)DFontAddrA + 0x31), (void *)&fontTexScaleW, 4);
	UpdateMemoryAddress((void *)((BYTE*)DFontAddrA + 0x3B), (void *)&fontTexScaleH, 4);

	CreateFontTextureRetAddr = (void*)((BYTE*)DFontAddrA + 0xB4);
	WriteJMPtoMemory((BYTE*)DFontAddrA + 0xAC, *CreateFontTextureASM, 8);

	BYTE codeA[] = { 0x51, 0x57, 0xBA, 0xEF, 0xBE, 0xAD, 0xDE, 0xFF, 0xD2, 0x83, 0xc4, 0x08, 0x90, 0x90, 0x90, 0x90 };
	codeA[3] = (BYTE)((DWORD)*UpdateFontTextureData & 0xFF);
	codeA[4] = (BYTE)((DWORD)*UpdateFontTextureData >> 8);
	codeA[5] = (BYTE)((DWORD)*UpdateFontTextureData >> 16);
	codeA[6] = (BYTE)((DWORD)*UpdateFontTextureData >> 24);
	UpdateMemoryAddress((void *)((BYTE*)DFontAddrA + 0xFE), (void *)&codeA, sizeof(codeA));

	BYTE codeB[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x8B, 0x54, 0x24, 0x30, 0x52, 0xBA, 0xEF, 0xBE, 0xAD, 0xDE };
	codeB[19] = (BYTE)((DWORD)*ReturnFontIdx & 0xFF);
	codeB[20] = (BYTE)((DWORD)*ReturnFontIdx >> 8);
	codeB[21] = (BYTE)((DWORD)*ReturnFontIdx >> 16);
	codeB[22] = (BYTE)((DWORD)*ReturnFontIdx >> 24);
	UpdateMemoryAddress((void *)((BYTE*)DFontAddrA + 0x120), (void *)&codeB, sizeof(codeB));

	BYTE codeC[] = { 0x50, 0xFF, 0xD2, 0x83, 0xC4, 0x08, 0x5F, 0x5E, 0x83, 0xC4, 0x24, 0xC3 };
	UpdateMemoryAddress((void *)((BYTE*)DFontAddrA + 0x120 + sizeof(codeB) + 5), (void *)&codeC, sizeof(codeC));
	
	UpdateMemoryAddress((void *)((BYTE*)DFontAddrB), (void *)&charIX, 1);
	BYTE codeD[] = { 0x6B, 0xC0, 0x05 };
	if (DisableEnlargedText)
	{
		codeD[2] = 0;
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrB + 0x15), (void *)&codeD[2], 1);
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrB + 0x16), (void *)&codeD, sizeof(codeD));
	}
	else
	{
		codeD[2] = (BYTE)charH;
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrB + 0x15), (void *)&charW, 1);
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrB + 0x16), (void *)&codeD, sizeof(codeD));
	}
	codeD[2] = (BYTE)charH;

	void *DFontAddrC = (void*)SearchAndGetAddresses(0x0047E048, 0x0047E2E8, 0x0047E4F8, DFontFuncBlockC, sizeof(DFontFuncBlockC), 0x00);

	if (DFontAddrC)
	{
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrC + 1), (void *)&charIX, 1);
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrC + 12), (void *)&charW, 1);
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrC + 17), (void *)&codeD, sizeof(codeD));

		void *DFontAddrD = (void*)SearchAndGetAddresses(0x0047E11F, 0x0047E3BF, 0x0047E5CF, DFontFuncBlockC, sizeof(DFontFuncBlockC), 0x00);

		if (DFontAddrD)
		{
			UpdateMemoryAddress((void *)((BYTE*)DFontAddrD + 1), (void *)&charIX, 1);
			UpdateMemoryAddress((void *)((BYTE*)DFontAddrD + 12), (void *)&charW, 1);
			UpdateMemoryAddress((void *)((BYTE*)DFontAddrD + 17), (void *)&codeD, sizeof(codeD));
		}
	}

	void *DFontAddrE = (void*)SearchAndGetAddresses(0x008038A8, 0x00807490, 0x00806490, DFontFuncBlockD, sizeof(DFontFuncBlockD), 0x00);

	if (DFontAddrE)
	{
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrE - 0x10), (void *)&nFontW, 2);
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrE - 0x10 + 2), (void *)&nFontH, 2);
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrE - 0x10 + 4), (void *)&sFontW, 2);
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrE - 0x10 + 6), (void *)&sFontH, 2);
			
		ifstream wfile("data\\font\\fontwdata.bin", ios::in | ios::binary | ios::ate);

		if (wfile.is_open())
		{
			BYTE *fwidth = new BYTE[0xE0];
			wfile.seekg(0, ios::beg);
			wfile.read((char *)fwidth, 0xE0);
			UpdateMemoryAddress((void *)(*(DWORD *)((BYTE*)DFontAddrE - 8) + 16), (void *)fwidth, 0xE0);
			wfile.read((char *)fwidth, 0xE0);
			UpdateMemoryAddress((void *)(*(DWORD *)((BYTE*)DFontAddrE - 4) + 16), (void *)fwidth, 0xE0);
			wfile.close();
			delete[] fwidth;
		}
		else
		{
			Logging::Log() << __FUNCTION__ << " Could not find font width data file";
		}
	}

	void *DFontAddrF = (void*)SearchAndGetAddresses(0x0047FAA0, 0x0047FD40, 0x0047FF50, DFontFuncBlockE, sizeof(DFontFuncBlockE), 0x00);
	void *DFontAddrG = (void*)SearchAndGetAddresses(0x0047E5A0, 0x0047E840, 0x0047EA50, DFontFuncBlockF, sizeof(DFontFuncBlockF), 0x00);

	if (DFontAddrF && DFontAddrG)
	{
		UpdateMemoryAddress((void*)((BYTE*)DFontAddrF + 0x0155), (void*)& letterSpc, 1);
		UpdateMemoryAddress((void*)((BYTE*)DFontAddrF + 0x0183), (void*)& letterSpc, 1);
		UpdateMemoryAddress((void*)((BYTE*)DFontAddrF + 0x0521), (void*)& letterSpc, 1);
		UpdateMemoryAddress((void*)((BYTE*)DFontAddrF + 0x0552), (void*)& letterSpc, 1);
		UpdateMemoryAddress((void*)((BYTE*)DFontAddrF + 0x06C2), (void*)& letterSpc, 1);
		UpdateMemoryAddress((void*)((BYTE*)DFontAddrG + 0x00A1), (void*)& letterSpc, 1);
		UpdateMemoryAddress((void*)((BYTE*)DFontAddrG + 0x00CF), (void*)& letterSpc, 1);

		if (spaceSize < 2)
		{
			letterSpc = 0;
		}
		else
		{
			letterSpc = 2;
			spaceSize -= 2;
		}

		UpdateMemoryAddress((void*)((BYTE*)DFontAddrF + 0x0142), (void*)& letterSpc, 1);
		UpdateMemoryAddress((void*)((BYTE*)DFontAddrF + 0x04FB), (void*)& letterSpc, 1);
		UpdateMemoryAddress((void*)((BYTE*)DFontAddrG + 0x008E), (void*)& letterSpc, 1);

		SpaceSizeRetAddr = (void*)((BYTE*)DFontAddrF + 0x013F);
		SpaceSizeRetAddr2 = (void*)((BYTE*)DFontAddrF + 0x04F8);
		SpaceSizeRetAddr3 = (void*)((BYTE*)DFontAddrG + 0x8B);

		WriteJMPtoMemory((BYTE*)DFontAddrF + 0x0139, *SpaceSizeASM, 6);
		WriteJMPtoMemory((BYTE*)DFontAddrF + 0x04F2, *SpaceSizeASM2, 6);
		WriteJMPtoMemory((BYTE*)DFontAddrG + 0x85, *SpaceSizeASM3, 6);
	}
}

static BOOL LoadDDSFont(void) {
	DDS_HEADER dds;
	ifstream file("data\\font\\font000.dds", ios::in | ios::binary | ios::ate);
	if (file.is_open())
	{
		char type[4];
		file.seekg(0, ios::beg);
		file.read(type, 4);

		if (strncmp(type, "DDS ", 4) != 0) {
			Logging::Log() << __FUNCTION__ << " No dds file type found, read type = " << type;
			return false;
		}

		file.read((char*)&dds, sizeof(DDS_HEADER));
		fontTexWidth = dds.dwWidth;
		fontTexHeight = dds.dwHeight;

		if (dds.ddpfPixelFormat.dwFourCC != 0) {
			fontFormat = (D3DFORMAT)dds.ddpfPixelFormat.dwFourCC;
			fontDataSize = dds.dwPitchOrLinearSize;
		}
		else {
			fontDataSize = dds.dwPitchOrLinearSize * fontTexHeight;
		}

		fontData = new BYTE[fontDataSize];
		file.read((char*)fontData, fontDataSize);
		file.close();
		return true;
	}
	return false;
}

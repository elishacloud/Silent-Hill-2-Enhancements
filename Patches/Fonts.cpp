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
#include "stb_image.h"
#include "stb_image_dds.h"

typedef enum {
	FONT_NORMAL = 0,
	FONT_SMALL
} FontType;

using namespace std;

// Predefined code bytes
constexpr BYTE DFontFuncBlockA[] = { 0x00, 0x83, 0xEC, 0x24, 0x85, 0xC0, 0x56, 0x57, 0x0F, 0x85, 0x12, 0x01 };
constexpr BYTE DFontFuncBlockB[] = { 0x19, 0x00, 0x00, 0x00, 0xF7, 0xFE, 0xC1, 0xE7, 0x04, 0x66, 0x89, 0x79, 0x02, 0xC1, 0xE3, 0x04 };
constexpr BYTE DFontFuncBlockC[] = { 0xBE, 0x19, 0x00, 0x00, 0x00, 0xF7, 0xFE, 0x83, 0xC4, 0x04, 0x6B, 0xD2, 0x16, 0x66, 0x89, 0x51, 0x08, 0xC1, 0xE0, 0x05 };
constexpr BYTE DFontFuncBlockD[] = { 0x00, 0x5F, 0x7F, 0x9F, 0xBF, 0xDF, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00 };
constexpr BYTE DFontFuncBlockE[] = { 0x8B, 0x44, 0x24, 0x08, 0x81, 0xEC, 0x80, 0x00, 0x00, 0x00, 0x53, 0x55, 0x56, 0x57, 0x33, 0xFF, 0x83, 0xC9, 0xFF };
constexpr BYTE DFontFuncBlockF[] = { 0x83, 0xEC, 0x0C, 0x55, 0x8B, 0x6C, 0x24, 0x18, 0x33, 0xD2, 0x3B, 0xEA, 0x75, 0x07, 0x33, 0xC0, 0x5D, 0x83, 0xC4 };

//games font parameters and data pointers
static unsigned int fontTexWidth = 550;
static unsigned int fontTexHeight = 544;
static float fontTexScaleW = 1.0f;
static float fontTexScaleH = 1.0f;
static BYTE charW = 22;
static BYTE charH = 32;
static BYTE charIX = 25;
static BYTE charIY = 17;
static WORD nFontW = 20;
static WORD nFontH = 30;
static WORD sFontW = 16;
static WORD sFontH = 24;
static BYTE letterSpc = 2;
static BYTE spaceSize = 7;
static IDirect3DTexture8 **fontTexture;
static DWORD *updateFlags;
static BYTE *fontWidthTable[2];

//external font texture
static BYTE *fontData = NULL;
static BYTE *fontWidthData = NULL;
static DWORD fontWidth;
static DWORD fontHeight;
static DWORD fontColumnNumber;
static DWORD fontRowNumber;

static BOOL loadExternalFontData(void);
static inline void copyFontData(BYTE *out, int pitch, SHORT index, WORD charId, FontType type);

static int UpdateFontTexture(D3DLOCKED_RECT *rect, SHORT index, FontType type, WORD charId) {
	int charMax = (fontColumnNumber * fontRowNumber) / 2;
	WORD id = charId;
	if (id >= charMax) {
		Logging::LogDebug() << __FUNCTION__ << "Error updating font texture: charId " << charId << " type " << type << " index " << index;
		id = 0;
	}
	if (id >= 0xE0 || fontWidthTable[type][id] != 0) {
		*updateFlags |= 4;
	}
	copyFontData((BYTE *)rect->pBits, rect->Pitch, index, id, type);
	(*fontTexture)->UnlockRect(0);
	return index;
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

	if (!loadExternalFontData())
	{
		Logging::Log() << __FUNCTION__ << " Error: Could not load font file";
		return;
	}

	Logging::Log() << "Enabling Custom Fonts...";

	fontTexScaleW = 550.0f / (float)(charIX * charW);
	fontTexScaleH = 544.0f / (float)(charIY * charH);
	fontTexWidth = fontWidth / fontColumnNumber * charIX;
	fontTexHeight = fontHeight / fontRowNumber * charIY;

	UpdateMemoryAddress((void *)((BYTE*)DFontAddrA + 0x17), (void *)&fontTexWidth, 4);
	UpdateMemoryAddress((void *)((BYTE*)DFontAddrA + 0x1C), (void *)&fontTexHeight, 4);
	UpdateMemoryAddress((void *)((BYTE*)DFontAddrA + 0x31), (void *)&fontTexScaleW, 4);
	UpdateMemoryAddress((void *)((BYTE*)DFontAddrA + 0x3B), (void *)&fontTexScaleH, 4);

	fontTexture = (IDirect3DTexture8**)*(DWORD*)((BYTE*)DFontAddrA - 0x03);
	updateFlags = (DWORD*)*(DWORD*)((BYTE*)DFontAddrA + 0x4DC);
	
	BYTE codeA[] = { 0x8B, 0x74, 0x24, 0x38, 0x56, 0x50, 0x52, 0x8D, 0x54, 0x24, 0x38, 0x52, 0xBA, 0xEF, 0xBE, 0xAD, 0xDE, 0xFF, 0xD2, 0x83, 0xC4, 0x10, 0x5D, 0x5B, 0x5F, 0x5E, 0x83, 0xC4, 0x24, 0xC3 };
	codeA[13] = (BYTE)((DWORD)*UpdateFontTexture & 0xFF);
	codeA[14] = (BYTE)((DWORD)*UpdateFontTexture >> 8);
	codeA[15] = (BYTE)((DWORD)*UpdateFontTexture >> 16);
	codeA[16] = (BYTE)((DWORD)*UpdateFontTexture >> 24);
	UpdateMemoryAddress((void*)((BYTE*)DFontAddrA + 0x298), (void*)&codeA, sizeof(codeA));

	if (DisableEnlargedText)
	{
		BYTE codeB[] = { 0x90, 0x90, 0x31, 0xD2 };
		UpdateMemoryAddress((void*)((BYTE*)DFontAddrB - 0xB7), (void*)&codeB, sizeof(codeB));
	}

	UpdateMemoryAddress((void *)((BYTE*)DFontAddrB), (void *)&charIX, 1);
	BYTE codeD[] = { 0x6B, 0xC0, 0x05 };
	codeD[2] = (BYTE)charH;
	UpdateMemoryAddress((void*)((BYTE*)DFontAddrB + 0x15), (void*)&charW, 1);
	UpdateMemoryAddress((void*)((BYTE*)DFontAddrB + 0x16), (void*)&codeD, sizeof(codeD));

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

		fontWidthTable[0] = (BYTE*)(*(DWORD*)((BYTE*)DFontAddrE - 8) + 16);
		fontWidthTable[1] = (BYTE*)(*(DWORD*)((BYTE*)DFontAddrE - 4) + 16);

		if (fontWidthData)
		{
			UpdateMemoryAddress((void *)fontWidthTable[0], (void *)fontWidthData, 0xE0);
			UpdateMemoryAddress((void *)fontWidthTable[1], (void *)(fontWidthData + 0xE0), 0xE0);
			delete[] fontWidthData;
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

static BOOL loadExternalFontData(void)
{
	char Filename[MAX_PATH];
	const bool IsUsingModPath = (GetFileModPath("data\\font", Filename) == Filename) ? true : false;

	const char* DataPath = "data";
	for (auto &IntPath : { "lang", GetModPath(DataPath), DataPath })
	{
		if (IsUsingModPath || IntPath == DataPath)
		{
			for (int j : { 4, 3, 2, 1, 0 })
			{
				for (auto format : { "png", "jpg", "tga", "dds" })
				{
					char path[32];
					snprintf(path, 32, "%s\\font\\font%03d.%s", IntPath, j, format);
					ifstream file(path, ios::in | ios::binary | ios::ate);
					if (file.is_open())
					{
						file.seekg(0, ios::end);
						DWORD size = (DWORD)file.tellg();
						file.seekg(0, ios::beg);

						BYTE* data = new BYTE[size];
						file.read((char*)data, size);
						file.close();

						int channels;
						if (strncmp(format, "dds", 3) == 0)
						{
							fontData = stbi_dds_load_from_memory(data, size, (int*)&fontWidth, (int*)&fontHeight, &channels, 0);
						}
						else
						{
							fontData = stbi_load_from_memory(data, size, (int*)&fontWidth, (int*)&fontHeight, &channels, 0);
						}
						if (!fontData)
						{
							delete[] data;
							continue;
						}

						BYTE* extra = (BYTE*)(data + size - 16);
						if (strncmp((char*)extra, "SH2EEFNT", 8) == 0)
						{
							fontColumnNumber = extra[8] | extra[9] << 8 | extra[10] << 16 | extra[11] << 24;
							fontRowNumber = extra[12] | extra[13] << 8 | extra[14] << 16 | extra[15] << 24;
						}
						else
						{
							fontColumnNumber = fontWidth / 44;
							fontRowNumber = fontHeight / 64;
						}
						Logging::Log() << __FUNCTION__ << ": Using font file: " << path << " fontColumnNumber " << fontColumnNumber << " fontRowNumber " << fontRowNumber;
						delete[] data;

						char wpath[32];
						snprintf(wpath, 32, "%s\\font\\fontwdata%03d.bin", IntPath, j);
						ifstream wfile(wpath, ios::in | ios::binary | ios::ate);
						if (!wfile.is_open())
						{
							snprintf(wpath, 32, "%s\\font\\fontwdata.bin", IntPath);
							wfile.open(wpath, ios::in | ios::binary | ios::ate); //try default
						}
						if (wfile.is_open())
						{
							fontWidthData = new BYTE[0xE0 * 2];
							wfile.seekg(0, ios::beg);
							wfile.read((char*)fontWidthData, 0xE0 * 2);
						}
						return (fontData != NULL);
					}
				}
			}
		}
	}
	return (fontData != NULL);
}

static inline void copyFontData(BYTE *out, int pitch, SHORT index, WORD charId, FontType type)
{
	int charWidth = fontWidth / fontColumnNumber;
	int charHeight = fontHeight / fontRowNumber;
	int fontStart = index / charIX * pitch * charHeight + index % charIX * charWidth * 4;
	int customFontStart = type * fontWidth * fontHeight * 4 / 2 + charId / fontColumnNumber * fontWidth * charHeight * 4 + charId % fontColumnNumber * charWidth * 4;
	for (int y = 0; y < charHeight; y++) {
		memcpy(out + fontStart + pitch * y, fontData + customFontStart + fontWidth * 4 * y, charWidth * 4);
	}
}

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

enum TGA_image_type {
	CMI = 1,		// colour map image
	RGBA,			// RGB(A) uncompressed
	GREY,			// greyscale uncompressed
	GREY_RLE = 9,	// greyscale RLE (compressed)
	RGBA_RLE,		// RGB(A) RLE (compressed)
};

#pragma pack(push, 1)
typedef struct {
	BYTE id;				// id
	BYTE color_map_type;	// colour map type
	BYTE image_type;		// image type
	WORD cm_first_entry;	// colour map first entry
	WORD cm_length;			// colour map length
	BYTE map_entry_size;	// map entry size, colour map depth (16, 24 , 32)
	WORD h_origin;			// horizontal origin
	WORD v_origin;			// vertical origin
	WORD width;				// width
	WORD height;			// height
	BYTE pixel_depth;		// pixel depth
	BYTE image_desc;		// image descriptor
} TGA_FILEHEADER;
#pragma pack(pop)

using namespace std;

// Predefined code bytes
constexpr BYTE DFontFuncBlockA[] = { 0x00, 0x83, 0xEC, 0x24, 0x85, 0xC0, 0x56, 0x57, 0x0F, 0x85, 0x12, 0x01 };
constexpr BYTE DFontFuncBlockB[] = { 0x19, 0x00, 0x00, 0x00, 0xF7, 0xFE, 0xC1, 0xE7, 0x04, 0x66, 0x89, 0x79, 0x02, 0xC1, 0xE3, 0x04 };
constexpr BYTE DFontFuncBlockC[] = { 0xBE, 0x19, 0x00, 0x00, 0x00, 0xF7, 0xFE, 0x83, 0xC4, 0x04, 0x6B, 0xD2, 0x16, 0x66, 0x89, 0x51, 0x08, 0xC1, 0xE0, 0x05 };
constexpr BYTE DFontFuncBlockD[] = { 0x00, 0x5F, 0x7F, 0x9F, 0xBF, 0xDF, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00 };
constexpr BYTE DFontFuncBlockE[] = { 0x8B, 0x44, 0x24, 0x08, 0x81, 0xEC, 0x80, 0x00, 0x00, 0x00, 0x53, 0x55, 0x56, 0x57, 0x33, 0xFF, 0x83, 0xC9, 0xFF };
constexpr BYTE DFontFuncBlockF[] = { 0x83, 0xEC, 0x0C, 0x55, 0x8B, 0x6C, 0x24, 0x18, 0x33, 0xD2, 0x3B, 0xEA, 0x75, 0x07, 0x33, 0xC0, 0x5D, 0x83, 0xC4 };

unsigned int fontTexWidth = 550;
unsigned int fontTexHeight = 544;
float fontTexScaleW = 1.0f;
float fontTexScaleH = 1.0f;
BYTE charW = 22;
BYTE charH = 32;
BYTE charIX = 100;
BYTE charIY = 150;
WORD nFontW = 20;
WORD nFontH = 30;
WORD sFontW = 16;
WORD sFontH = 24;
BYTE letterSpc = 2;

TGA_FILEHEADER tga;
BYTE *fontData;

void UpdateFontTex(BYTE *ptr, int size)
{
	Logging::LogDebug() << __FUNCTION__ << " - address: " << (DWORD)ptr << ", size: " << size;

	if (fontData)
		memcpy(ptr, fontData, size);

	return;
}

int ReturnFontIdx(int fontSize, WORD charID)
{
	int charMax = (charIX * charIY) / 2;

	if (charID > charMax)
		return 0;

	return charID + fontSize * charMax;
}

void UpdateCustomFonts()
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

	ifstream file(std::string(std::string(ModPathA) + "\\font\\font000.tga").c_str(), ios::in | ios::binary | ios::ate);

	if (file.is_open())
	{
		file.seekg(0, ios::beg);
		file.read((char *)&tga, sizeof(TGA_FILEHEADER));

		if (tga.image_type != RGBA || tga.pixel_depth != 32 || tga.image_desc != 40)
		{
			Logging::Log() << __FUNCTION__ << " Error: Unsupported tga format!";
			file.close();
			return;
		}

		fontTexWidth = tga.width;
		fontTexHeight = tga.height;
		int texSize = fontTexWidth * fontTexHeight * 4;
		fontData = new BYTE[texSize];

		file.seekg(sizeof(TGA_FILEHEADER) + tga.cm_length * 4, ios::beg);
		file.read((char *)fontData, texSize);
		file.close();
	}
	else
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

	BYTE codeA[] = { 0x51, 0x57, 0xBA, 0xEF, 0xBE, 0xAD, 0xDE, 0xFF, 0xD2, 0x83, 0xc4, 0x08, 0x90, 0x90, 0x90, 0x90 };
	codeA[3] = (BYTE)((DWORD)*UpdateFontTex & 0xFF);
	codeA[4] = (BYTE)((DWORD)*UpdateFontTex >> 8);
	codeA[5] = (BYTE)((DWORD)*UpdateFontTex >> 16);
	codeA[6] = (BYTE)((DWORD)*UpdateFontTex >> 24);
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
			
		ifstream wfile(std::string(std::string(ModPathA) + "\\font\\fontwdata.bin").c_str(), ios::in | ios::binary | ios::ate);

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
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrF + 0x0142), (void *)&letterSpc, 1);
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrF + 0x0155), (void *)&letterSpc, 1);
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrF + 0x0183), (void *)&letterSpc, 1);
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrF + 0x04FB), (void *)&letterSpc, 1);
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrF + 0x0521), (void *)&letterSpc, 1);
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrF + 0x0552), (void *)&letterSpc, 1);
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrF + 0x06C2), (void *)&letterSpc, 1);
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrG + 0x008E), (void *)&letterSpc, 1);
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrG + 0x00A1), (void *)&letterSpc, 1);
		UpdateMemoryAddress((void *)((BYTE*)DFontAddrG + 0x00CF), (void *)&letterSpc, 1);
	}
}

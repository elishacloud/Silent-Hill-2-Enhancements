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
#include <Shlwapi.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "Patches.h"
#include "Common\Utils.h"
#include "Common\FileSystemHooks.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

namespace fs = std::filesystem;

// Variables for ASM
WORD Start01TempASM;
float Start01XScale;
float Start01YScale;
DWORD Start01Addr1 = 0;
DWORD Start01Addr2 = 0;
DWORD Start01Addr3 = 0;
void *jmpStart01X1Addr;
void *jmpStart01X2Addr;
void *jmpStart01Y1Addr;
void *jmpStart01Y2Addr;

// ASM functions for Start01 Scale X-1
__declspec(naked) void __stdcall Start01ScaleX1ASM()
{
	__asm
	{
		mov edx, dword ptr ds : [Start01Addr2]
		sub edx, 0x08
		fild word ptr ds : [esi + edx]
		fmul dword ptr ds : [Start01XScale]
		fistp word ptr ds : [Start01TempASM]
		movsx edx, word ptr ds : [Start01TempASM]
		jmp jmpStart01X1Addr
	}
}

// ASM functions for Start01 Scale X-2
__declspec(naked) void __stdcall Start01ScaleX2ASM()
{
	__asm
	{
		mov ecx, dword ptr ds : [Start01Addr2]
		sub ecx, 0x06
		fild word ptr ds : [esi + ecx]
		fmul dword ptr ds : [Start01XScale]
		fistp word ptr ds : [Start01TempASM]
		movsx ecx, word ptr ds : [Start01TempASM]
		jmp jmpStart01X2Addr
	}
}

// ASM functions for Start01 Scale Y-1
__declspec(naked) void __stdcall Start01ScaleY1ASM()
{
	__asm
	{
		mov ecx, dword ptr ds : [Start01Addr2]
		sub ecx, 0x0A
		fild word ptr ds : [esi + ecx]
		fmul dword ptr ds : [Start01XScale]
		fistp word ptr ds : [Start01TempASM]
		movsx ecx, word ptr ds : [Start01TempASM]
		jmp jmpStart01Y1Addr
	}
}

// ASM functions for Start01 Scale Y-2
__declspec(naked) void __stdcall Start01ScaleY2ASM()
{
	__asm
	{
		mov edx, dword ptr ds : [Start01Addr2]
		sub edx, 0x04
		fild word ptr ds : [esi + edx]
		fmul dword ptr ds : [Start01XScale]
		fistp word ptr ds : [Start01TempASM]
		movsx edx, word ptr ds : [Start01TempASM]
		jmp jmpStart01Y2Addr
	}
}

// Get texture resolution
bool GetTextureRes(wchar_t *TexName, WORD &TextureXRes, WORD &TextureYRes)
{
	std::ifstream in(TexName, std::ifstream::ate | std::ifstream::binary);
	if (!in.is_open())
	{
		return false;
	}
	if (in.tellg() < 128)
	{
		in.close();
		return false;
	}
	for (auto& num : { 20, 24, 56 })
	{
		in.seekg(num);
		in.read((char*)&TextureXRes, 2);
		in.read((char*)&TextureYRes, 2);
		if (TextureXRes && TextureYRes)
		{
			in.close();
			return true;
		}
	}
	in.close();
	return false;
}

void ScaleTexture(wchar_t *TexName, float *XScaleAddress, float *YScaleAddress, WORD *WidthAddress, WORD *XPosAddress, DWORD ScaleType)
{
	WORD TextureXRes, TextureYRes;
	if (!GetTextureRes(TexName, TextureXRes, TextureYRes))
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get texture resolution for " << TexName << "!";
		return;
	}

	// Compute new scale
	float XScale = (float)(TextureXRes * 16);
	float YScale = (float)(TextureYRes * 16);

	// Aspect ratio
	float AspectRatio = (float)TextureYRes / (float)TextureXRes;

	// Compute width and XPos
	WORD Width, XPos;
	switch (ScaleType)
	{
	case 1:
		Width = (WORD)(4096 / AspectRatio);
		XPos = (WORD)(61440 - (Width - 4096));
		break;
	case 2:
		XPos = (WORD)(4096 / AspectRatio);
		Width = (WORD)(61440 - (XPos - 4096));
		break;
	default:
		Logging::Log() << __FUNCTION__ << " Error: failed to get texture 'Width' and 'XPos' for " << TexName << "!";
		return;
	}

	// Update memory
	Logging::LogDebug() << __FUNCTION__ << " Scaling texture: " << TexName << " Resolution: " << TextureXRes << "x" << TextureYRes << " XYScale: " << XScale << "x" << YScale << " Width: " << Width << " XPos: " << XPos;
	UpdateMemoryAddress(XScaleAddress, &XScale, sizeof(float));
	UpdateMemoryAddress(YScaleAddress, &YScale, sizeof(float));
	UpdateMemoryAddress(WidthAddress, &Width, sizeof(WORD));
	UpdateMemoryAddress(XPosAddress, &XPos, sizeof(WORD));
}

bool GetStart01Addresses()
{
	// Get addresses
	constexpr BYTE SearchBytes1[]{ 0x33, 0xC9, 0x83, 0xC4, 0x1C, 0x66, 0x83, 0xFF, 0x0B, 0x0F, 0x95, 0xC1, 0x8B, 0xD5, 0xC1, 0xE2, 0x04, 0x66, 0x89, 0x15 };
	Start01Addr1 = SearchAndGetAddresses(0x00495CEA, 0x00495F8A, 0x0049619A, SearchBytes1, sizeof(SearchBytes1), 0x16);
	if (Start01Addr1)
	{
		Start01Addr2 = *(DWORD*)(Start01Addr1 + 0x07);
	}
	constexpr BYTE SearchBytes3[]{ 0x66, 0x0D, 0x02, 0x00, 0x66, 0x0D, 0x04, 0x00, 0x68 };
	Start01Addr3 = SearchAndGetAddresses(0x00496974, 0x00496C14, 0x00496DF4, SearchBytes3, sizeof(SearchBytes3), 0x26);

	return (Start01Addr1 && Start01Addr2 && Start01Addr3);
}

void SetDefaultFullscreenBackground()
{
	if (!Start01Addr2 && !Start01Addr3 && !GetStart01Addresses())
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Fullscreen Start Background
	UpdateMemoryAddress((void*)Start01Addr3, "\x02\xF1", sizeof(WORD));						// Start Background Top: 61698 (int16)
	UpdateMemoryAddress((void*)(Start01Addr3 + 0x12), "\xFE\x0E", sizeof(WORD));			// Start Background Bottom: 3838 (int16)
	UpdateMemoryAddress((void*)Start01Addr2, "\x59\x00", sizeof(WORD));						// Logo Highlight Height: 89 (int16)
	UpdateMemoryAddress((void*)(Start01Addr3 + 0xAC), "\x92\xFF\xFF\xFF", sizeof(DWORD));	// Logo Highlight Y Pos: -110 (int32)
}

void ScaleStart01Texture(wchar_t *TexName)
{
	WORD TextureXRes, TextureYRes;
	if (!GetTextureRes(TexName, TextureXRes, TextureYRes))
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get texture resolution for " << TexName << "!";
		return;
	}

	if (!GetStart01Addresses())
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Compute new scale
	Start01XScale = (float)TextureXRes / 512.0f;
	Start01YScale = (float)TextureYRes / 512.0f;

	// jmp addresses
	jmpStart01X1Addr = (void*)(Start01Addr1 + 0x38);
	jmpStart01X2Addr = (void*)(Start01Addr1 + 0x57);
	jmpStart01Y1Addr = (void*)(Start01Addr1 + 0x49);
	jmpStart01Y2Addr = (void*)(Start01Addr1 + 0x69);

	// Update memory
	Logging::LogDebug() << __FUNCTION__ << " Scaling texture: " << TexName << " Resolution: " << TextureXRes << "x" << TextureYRes << " XYScale: " << Start01XScale << "x" << Start01YScale;
	if (*(BYTE*)(Start01Addr1 + 0x31) != 0xE9)
	{
		WriteJMPtoMemory((BYTE*)(Start01Addr1 + 0x31), *Start01ScaleX1ASM, 7);
		WriteJMPtoMemory((BYTE*)(Start01Addr1 + 0x50), *Start01ScaleX2ASM, 7);
		WriteJMPtoMemory((BYTE*)(Start01Addr1 + 0x42), *Start01ScaleY1ASM, 7);
		WriteJMPtoMemory((BYTE*)(Start01Addr1 + 0x62), *Start01ScaleY2ASM, 7);
	}

	// Fullscreen Start Background
	if (TextureXRes > 512 && TextureYRes > 512)
	{
		UpdateMemoryAddress((void*)Start01Addr3, "\x02\xF1", sizeof(WORD));						// Start Background Top: 61698 (int16)
		UpdateMemoryAddress((void*)(Start01Addr3 + 0x12), "\xFE\x0E", sizeof(WORD));			// Start Background Bottom: 3838 (int16)
		UpdateMemoryAddress((void*)Start01Addr2, "\x7E\x00", sizeof(WORD));						// Logo Highlight Height: 126 (int16)
		UpdateMemoryAddress((void*)(Start01Addr3 + 0xAC), "\x95\xFF\xFF\xFF", sizeof(DWORD));	// Logo Highlight Y Pos: -107 (int32)
	}
	else
	{
		SetDefaultFullscreenBackground();
	}
}

DWORD GetTexBufferSize()
{
	DWORD size = 0;

	for (const auto& path : {
		"data\\pic",
		"data\\pic\\add",
		"data\\pic\\apt",
		"data\\pic\\dls",
		"data\\pic\\effect",
		"data\\pic\\etc",
		"data\\pic\\hsp",
		"data\\pic\\htl",
		"data\\pic\\item",
		"data\\pic\\map",
		"data\\pic\\out",
		"data\\pic\\ufo",
		"data\\menu\\mc"
		})
	{
		if (PathFileExistsA(path))
		{
			for (const auto & entry : fs::directory_iterator(path))
			{
				if (entry.is_regular_file())
				{
					// Get size of file
					WIN32_FILE_ATTRIBUTE_DATA FileInformation = {};

					wchar_t Filename[MAX_PATH];
					if (GetFileAttributesEx(UpdateModPath(entry.path().c_str(), Filename), GetFileExInfoStandard, &FileInformation) && FileInformation.nFileSizeLow)
					{
						if (size < FileInformation.nFileSizeLow)
						{
							size = FileInformation.nFileSizeLow;
						}
					}
				}
			}
		}
	}
	return size;
}

void PatchTexAddr()
{
	// Get addresses
	const DWORD StaticAddr = 0x00401CC1;		// Address is the same on all binaries
	BYTE SearchBytes1[]{ 0x68, 0x00, 0x00, 0x00, 0x00 };
	memcpy((SearchBytes1 + 1), (void*)StaticAddr, sizeof(DWORD));
	const DWORD Addr1 = SearchAndGetAddresses(0x0044B99D, 0x0044BB3D, 0x0044BB3D, SearchBytes1, sizeof(SearchBytes1), 0x01);
	constexpr BYTE SearchBytes2[]{ 0x05, 0x00, 0x00, 0x08, 0x00 };
	const DWORD Addr2 = SearchAndGetAddresses(0x00496F87, 0x00497231, 0x00497331, SearchBytes2, sizeof(SearchBytes2), 0x00);
	constexpr BYTE SearchBytes3[]{ 0x05, 0x00, 0x48, 0x10, 0x00 };
	const DWORD Addr3 = SearchAndGetAddresses(0x0049B40A, 0x0049B6BA, 0x0049AF7A, SearchBytes3, sizeof(SearchBytes3), 0x00);

	// Checking address pointer
	if (!Addr1 || !Addr2 || !Addr3)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Get size of textures
	DWORD size = GetTexBufferSize();
	if (!size)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find texture buffer size!";
		return;
	}
	size += 2 * 1024 * 1024;	// Add 2 MB to buffer
	Logging::Log() << "Setting texture buffer size: " << size;

	// Allocate dynamic memory for loading textures
	BYTE *PtrBytes1 = new BYTE[size];
	ZeroMemory(PtrBytes1, size);		// Need to clear menory for debug mode, game expects the memory to be zeroed
	BYTE *PtrBytes2 = new BYTE[size];
	ZeroMemory(PtrBytes2, size);		// Need to clear menory for debug mode, game expects the memory to be zeroed
	if (!PtrBytes1 || !PtrBytes2)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to create texture buffer!";
		return;
	}

	// Logging update
	Logging::Log() << "Updating Texture memory address locations...";

	// Write new memory static address
	UpdateMemoryAddress((void*)StaticAddr, &PtrBytes1, sizeof(void*));

	// Write new memory address 1
	UpdateMemoryAddress((void*)Addr1, &PtrBytes1, sizeof(void*));

	// Write new memory address 2
	UpdateMemoryAddress((void*)Addr2, "\xB8", sizeof(BYTE));
	UpdateMemoryAddress((void*)(Addr2 + 1), &PtrBytes2, sizeof(void*));

	// Write new memory address 3
	UpdateMemoryAddress((void*)Addr3, "\xB8", sizeof(BYTE));
	UpdateMemoryAddress((void*)(Addr3 + 1), &PtrBytes2, sizeof(void*));

	{// start00.tex
		constexpr BYTE SearchBytes[]{ 0x66, 0x0D, 0x02, 0x00, 0x66, 0x0D, 0x04, 0x00, 0x68 };
		const DWORD BaseAddress = SearchAndGetAddresses(0x00496974, 0x00496C14, 0x00496DF4, SearchBytes, sizeof(SearchBytes), 0x1C);
		if (BaseAddress)
		{
			ScaleTexture(L"data\\pic\\etc\\start00.tex", (float*)(BaseAddress + 0x38), (float*)(BaseAddress + 0x42), (WORD*)(BaseAddress + 0x13), (WORD*)(BaseAddress + 0x01), 1);
		}
	}

	// start01.tex
	ScaleStart01Texture(L"data\\pic\\etc\\start01.tex");

	{// savebg.tbn2
		constexpr BYTE SearchBytes[]{ 0x66, 0x0D, 0x02, 0x00, 0x66, 0x0D, 0x04, 0x00, 0x66, 0xA3 };
		const DWORD BaseAddress = SearchAndGetAddresses(0x0044B4B8, 0x0044B658, 0x0044B658, SearchBytes, sizeof(SearchBytes), -0x28);
		if (BaseAddress)
		{
			const DWORD BaseAddress2 = *(DWORD*)(BaseAddress + 0x07);
			ScaleTexture(L"data\\menu\\mc\\savebg.tbn2", (float*)(BaseAddress + 0x64), (float*)(BaseAddress + 0x6E), (WORD*)(BaseAddress2 + 0x00), (WORD*)(BaseAddress2 + 0x04), 2);
			constexpr BYTE nop6[]{ 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
			UpdateMemoryAddress((void*)(BaseAddress + 0x05), (void*)nop6, sizeof(nop6));
			UpdateMemoryAddress((void*)(BaseAddress + 0x16), (void*)nop6, sizeof(nop6));
		}
	}
}

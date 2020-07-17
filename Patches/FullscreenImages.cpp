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
#include <string>
#include <regex>
#include "Patches.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

float SizeNormal = 1.0f;
float SizeFullscreen = 1.0f;
float AspectRatio = 1.0f;	// this value should be softcoded or you need to calculate the aspect ratio from one of the textures. (TexResY / TexResX) = AspectRatio

const DWORD Deuce = 2;

DWORD GameResX;				// Example: 1920 (can be acquired from 00A32898) 
DWORD GameResY;				// Example: 1080 (can be acquired from 00A3289C)

BYTE *ScreenPosX;			// Example: 0x00 (must be acquired from 01DBBFF7, byte)
BYTE *ScreenPosY;			// Example: 0x00 (must be acquired from 01DBBFF8, byte)

// placeholder variables, don't assign value
DWORD ScreenPosXtmp;
DWORD ScreenPosYtmp;
DWORD TempResultX;
DWORD TempResultY;
DWORD OffsetX;
DWORD OffsetY;

char **TexNameAddr = nullptr;
std::vector<std::string> imagelist;

BOOL CheckTexture()
{
	if (TexNameAddr && *TexNameAddr)
	{
		if (std::any_of(imagelist.begin(), imagelist.end(), [](const std::string & str) { return str.compare(*TexNameAddr) == 0; }))
		{
			return TRUE;
		}
	}
	return FALSE;
}

// ASM function to scale texture width
__declspec(naked) void __stdcall TexWidthASM()
{
	__asm
	{
		pushf
		push ebx
		push ecx
		call CheckTexture
		cmp eax, FALSE
		pop ecx
		pop ebx
		mov eax, dword ptr ds : [GameResX]
		je near NoScaling
	// Adjust scale
		fmul dword ptr ds : [SizeFullscreen]
		fmul dword ptr ds : [AspectRatio]
		jmp near Exit
	NoScaling:
		fmul dword ptr ds : [SizeNormal]
		fmul dword ptr ds : [AspectRatio]
	Exit:
		popf
		ret
	}
}

// ASM function to scale texture height
__declspec(naked) void __stdcall TexHeightASM()
{
	__asm
	{
		pushf
		push eax
		push ebx
		push ecx
		call CheckTexture
		cmp eax, FALSE
		pop ecx
		pop ebx
		pop eax
		fild dword ptr ds : [GameResY]
		je near NoScaling
	// Adjust scale
		fmul dword ptr ds : [SizeFullscreen]
		jmp near Exit
	NoScaling:
		fmul dword ptr ds : [SizeNormal]
	Exit:
		popf
		ret
	}
}

// ASM function to scale texture X-Pos
__declspec(naked) void __stdcall TexXPosASM()
{
	__asm
	{
		pushf
		push eax
		push ebx
		push ecx
		call CheckTexture
		cmp eax, FALSE
		pop ecx
		pop ebx
		pop eax
		je near NoScaling
	// Adjust scale
		fild dword ptr ds : [GameResX]			// loads game resolution x
		fmul dword ptr ds : [SizeFullscreen]	// multiplies by SizeFullscreen
		fmul dword ptr ds : [AspectRatio]		// multiplies by AspectRatio
		fstp dword ptr ds : [TempResultX]		// stores result
		jmp near Exit
	NoScaling:
		fild dword ptr ds : [GameResX]			// loads game resolution x
		fmul dword ptr ds : [SizeNormal]		// multiplies by Size
		fmul dword ptr ds : [AspectRatio]		// multiplies by AspectRatio
		fstp dword ptr ds : [TempResultX]		// stores result
	Exit:
		fild dword ptr ds : [GameResX]			// loads game resolution x
		fsub dword ptr ds : [TempResultX]		// subtracts TempResult
		mov edx, dword ptr ds : [ScreenPosX]
		movzx edx, byte ptr ds : [edx]
		mov ScreenPosXtmp, edx
		fiadd dword ptr ds : [ScreenPosXtmp]	// adds ScreenPosX
		fidiv dword ptr ds : [Deuce]			// divide by 2
		fistp dword ptr ds : [OffsetX]			// stores final result
		mov edx, dword ptr ds : [OffsetX]
		popf
		ret
	}
}

// ASM function to scale texture Y-Pos eax
__declspec(naked) void __stdcall TexYPosASM()
{
	__asm
	{
		pushf
		push ebx
		push ecx
		call CheckTexture
		cmp eax, FALSE
		pop ecx
		pop ebx
		je near NoScaling
	// Adjust scale
		fild dword ptr ds : [GameResY]			// loads game resolution y
		fmul dword ptr ds : [SizeFullscreen]	// multiplies by SizeFullscreen
		fstp dword ptr ds : [TempResultY]		// stores result
		jmp near Exit
	NoScaling:
		fild dword ptr ds : [GameResY]			// loads game resolution y
		fmul dword ptr ds : [SizeNormal]		// multiplies by Size
		fstp dword ptr ds : [TempResultY]		// stores result
	Exit:
		fild dword ptr ds : [GameResY]			// loads game resolution y
		fsub dword ptr ds : [TempResultY]		// subtracts TempResult
		mov eax, dword ptr ds : [ScreenPosY]
		movzx eax, byte ptr ds : [eax]
		mov ScreenPosYtmp, eax
		fiadd dword ptr ds : [ScreenPosYtmp]	// adds ScreenPosY
		fidiv dword ptr ds : [Deuce]			// divide by 2
		fistp dword ptr ds : [OffsetY]			// stores final result
		mov eax, dword ptr ds : [OffsetY]
		popf
		ret
	}
}

// ASM function to scale texture Y-Pos edx
__declspec(naked) void __stdcall TexYPosEDXASM()
{
	__asm
	{
		push eax
		call TexYPosASM
		mov edx, eax
		pop eax
		ret
	}
}

void SetFullscreenImagesRes(DWORD Width, DWORD Height)
{
	GameResX = Width;
	GameResY = Height;

	// Set scale ratio
	switch (FullscreenImages)
	{
	case 0: // original [Size = 1.0f]
		SizeFullscreen = 1.0f;
		break;
	case 1: // auto-scaling
		if (((float)GameResX / (float)GameResY) <= 16.0f / 9.0f)
		{
			SizeFullscreen = ((float)GameResX / (float)GameResY) * 0.75f;
			break;
		}
	case 2: // cropped [Size = 1.3333f]
		SizeFullscreen = 16.0f / 9.0f;
		break;
	case 3: // pillarboxed [Size = 1.25f]
		SizeFullscreen = 5.0f / 4.0f;
		break;
	}
}

void PatchFullscreenImages()
{
	// Get dat file path
	wchar_t datpath[MAX_PATH];
	GetModuleFileName(m_hModule, datpath, MAX_PATH);
	wcscpy_s(wcsrchr(datpath, '.'), MAX_PATH - wcslen(datpath), L".dat");

	// Open dat file
	std::ifstream myfile(datpath);
	if (!myfile)
	{
		return;
	}

	// Read contents of dat file
	std::string line;
	while (std::getline(myfile, line))
	{
		if (line.size())
		{
			imagelist.push_back(line);
		}
	}
	myfile.close();

	// Get address locations
	constexpr BYTE MemorySearchBytes[]{ 0xDE, 0xC1, 0xDA, 0x44, 0x24, 0x08, 0x59, 0xC3, 0x90, 0x90, 0x90 };
	DWORD MemAddress = SearchAndGetAddresses(0x0044A5D5, 0x0044A775, 0x0044A775, MemorySearchBytes, sizeof(MemorySearchBytes), -0x18);

	constexpr BYTE WidthSearchBytes[]{ 0x85, 0xD2, 0xDB, 0x44, 0x24, 0x00, 0x89, 0x44, 0x24, 0x00, 0xDB, 0x44, 0x24, 0x00, 0xD8, 0xCB };
	void *WidthAddr = (void*)SearchAndGetAddresses(0x0049F024, 0x0049F2D4, 0x0049EB94, WidthSearchBytes, sizeof(WidthSearchBytes), -0x3A);

	// Checking address pointer
	if (!MemAddress || !WidthAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	void *HeightAddr1 = (void*)((DWORD)WidthAddr + 0x5E);
	void *HeightAddr2 = (void*)((DWORD)HeightAddr1 + 0x1D2);
	void *HeightAddr3 = (void*)((DWORD)HeightAddr1 + 0x2D6);
	void *XPosAddr1   = (void*)((DWORD)HeightAddr1 - 0x041);
	void *XPosAddr2   = (void*)((DWORD)HeightAddr1 + 0x181);
	void *XPosAddr3   = (void*)((DWORD)HeightAddr1 + 0x29E);
	void *YPosAddr1   = (void*)((DWORD)HeightAddr1 + 0x012);
	void *YPosAddr2   = (void*)((DWORD)HeightAddr1 + 0x1E0);
	void *YPosEDXAddr = (void*)((DWORD)HeightAddr1 + 0x2E4);

	ScreenPosX = (BYTE*)*(DWORD*)MemAddress;
	ScreenPosY = (BYTE*)((DWORD)ScreenPosX + 1);

	TexNameAddr = (char**)*(DWORD*)(MemAddress - 0x85);

	// Write call addresses 
	Logging::Log() << __FUNCTION__ " Update texture scaling!";
	WriteCalltoMemory((BYTE*)WidthAddr, TexWidthASM);
	WriteCalltoMemory((BYTE*)HeightAddr1, TexHeightASM, 6);
	WriteCalltoMemory((BYTE*)HeightAddr2, TexHeightASM, 6);
	WriteCalltoMemory((BYTE*)HeightAddr3, TexHeightASM, 6);
	WriteCalltoMemory((BYTE*)XPosAddr1, TexXPosASM, 7);
	WriteCalltoMemory((BYTE*)XPosAddr2, TexXPosASM, 7);
	WriteCalltoMemory((BYTE*)XPosAddr3, TexXPosASM, 7);
	WriteCalltoMemory((BYTE*)YPosAddr1, TexYPosASM, 7);
	WriteCalltoMemory((BYTE*)YPosAddr2, TexYPosASM, 7);
	WriteCalltoMemory((BYTE*)YPosEDXAddr, TexYPosEDXASM, 7);
}

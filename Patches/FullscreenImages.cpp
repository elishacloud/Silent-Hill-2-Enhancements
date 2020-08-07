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

// Common Values
float SizeNormal = 1.0f;
float SizeFullscreen = 1.0f;
float AspectRatio = 1.0f;	// this value should be softcoded or you need to calculate the aspect ratio from one of the textures. (TexResY / TexResX) = AspectRatio

float ORG_TextureResX = 512.0f; // based on the original res X of whichever texture you decide to use
float ORG_TextureResY = 512.0f; // based on the original res Y of whichever texture you decide to use

float VerBoundPos = 240.0f;		// (240.0f / Size)
DWORD VerBoundPosInt = 240;		// (240.0f / Size)
float VerBoundNeg = 240.0f;		// (-240.0f / Size)

const DWORD Deuce = 2;

DWORD GameResX;				// Example: 1920 (can be acquired from 00A32898) 
DWORD GameResY;				// Example: 1080 (can be acquired from 00A3289C)

BYTE *ScreenPosX;			// Example: 0x00 (must be acquired from 01DBBFF7, byte)
BYTE *ScreenPosY;			// Example: 0x00 (must be acquired from 01DBBFF8, byte)

DWORD TextureResX;			// New texture X size
DWORD TextureResY;			// New texture Y size

float TextureScaleX;		// (TextureResX / ORG_TextureResX);
float TextureScaleY;		// (TextureResY / ORG_TextureResY);

// placeholder variables, don't assign value
DWORD ScreenPosXtmp;
DWORD ScreenPosYtmp;
DWORD TempResultX;
DWORD TempResultY;
DWORD OffsetX;
DWORD OffsetY;

void *VerBoundPosIntAddr = nullptr;
void *VerBoundNegAddr1 = nullptr;
void *VerBoundNegAddr2 = nullptr;
void *VerBoundNegAddr3 = nullptr;

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

// Get texture resolution
bool GetTextureRes(wchar_t *TexName, WORD &TextureX, WORD &TextureY)
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
		in.read((char*)&TextureX, 2);
		in.read((char*)&TextureY, 2);
		if (TextureX && TextureY)
		{
			in.close();
			return true;
		}
	}
	in.close();
	return false;
}

// Get texture resolution
bool GetTextureRes(wchar_t *TexName, DWORD &TextureX, DWORD &TextureY)
{
	WORD TextureXRes, TextureYRes;
	bool flag = GetTextureRes(TexName, TextureXRes, TextureYRes);
	TextureX = TextureXRes;
	TextureY = TextureYRes;
	return flag;
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

	if (VerBoundPosIntAddr && VerBoundNegAddr1 && VerBoundNegAddr2 && VerBoundNegAddr3)
	{
		VerBoundPos = (240.0f / SizeFullscreen);
		VerBoundPosInt = (DWORD)(240.0f / SizeFullscreen);
		VerBoundNeg = (-240.0f / SizeFullscreen);

		UpdateMemoryAddress(VerBoundPosIntAddr, &VerBoundPosInt, sizeof(DWORD));
		UpdateMemoryAddress(VerBoundNegAddr1, &VerBoundNeg, sizeof(DWORD));
		UpdateMemoryAddress(VerBoundNegAddr2, &VerBoundNeg, sizeof(DWORD));
		UpdateMemoryAddress(VerBoundNegAddr3, &VerBoundNeg, sizeof(DWORD));
	}
}

void Start00Scaling()
{
	// Get address locations
	constexpr BYTE Start00ScaleSearchBytes[]{ 0xD8, 0x5C, 0x24, 0x00, 0xDF, 0xE0, 0xF6, 0xC4, 0x01, 0x75, 0x0B, 0xE8 };
	void *Start00ScaleXAddr = (void*)SearchAndGetAddresses(0x004969FF, 0x00496C9F, 0x00496E7F, Start00ScaleSearchBytes, sizeof(Start00ScaleSearchBytes), -0x37);

	constexpr BYTE SaveBGScaleSearchBytes[]{ 0x33, 0x44, 0x24, 0x38, 0x89, 0x44, 0x24, 0x34, 0x8B, 0x44, 0x24, 0x3C, 0x3D, 0xBF, 0x27, 0x09, 0x00, 0x7E, 0x05 };
	void *SaveBGScaleXAddr = (void*)SearchAndGetAddresses(0x0044B528, 0x0044B6C8, 0x0044B6C8, SaveBGScaleSearchBytes, sizeof(SaveBGScaleSearchBytes), -0x34);

	constexpr BYTE LogoHighlightSearchBytes[]{ 0x66, 0x03, 0xCD, 0x8B, 0xC3, 0xC1, 0xE2, 0x04, 0x66, 0x89, 0x15 };
	void *LogoHighlightYPos = (void*)ReadSearchedAddresses(0x00495D22, 0x00495FC2, 0x004961D2, LogoHighlightSearchBytes, sizeof(LogoHighlightSearchBytes), -0x1B);

	// Checking address pointer
	if (!Start00ScaleXAddr || !SaveBGScaleXAddr || !LogoHighlightYPos)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	void *Start00PosXAddr = (void*)((DWORD)Start00ScaleXAddr - 0x25);
	void *Start00WidthAddr = (void*)((DWORD)Start00ScaleXAddr - 0x37);
	void *Start00HeightTop = (void*)((DWORD)Start00ScaleXAddr - 0x2E);
	void *Start00HeightBottom = (void*)((DWORD)Start00ScaleXAddr - 0x1C);

	void *SaveBGPosXAddr = (void*)*(DWORD*)((DWORD)SaveBGScaleXAddr - 0x4C);
	void *SaveBGWidthAddr = (void*)((DWORD)SaveBGPosXAddr - 0x04);
	void *SaveBGWidthNOPAddr = (void*)((DWORD)SaveBGScaleXAddr - 0x5F);
	void *SaveBGPosXNOPAddr = (void*)((DWORD)SaveBGScaleXAddr - 0x4E);

	void *LogoHighlightHeight = (void*)((DWORD)Start00ScaleXAddr + 0x7E);

	WORD Start00ResX, Start00ResY, SaveBGResX, SaveBGResY;
	if (!GetTextureRes(L"data\\pic\\etc\\start00.tex", Start00ResX, Start00ResY) ||
		!GetTextureRes(L"data\\menu\\mc\\savebg.tbn2", SaveBGResX, SaveBGResY))
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get texture resolution!";
		return;
	}

	float Start00_Ratio = ((float)Start00ResX / (float)Start00ResY);
	float Start00_ScaleX = (8192.0f * Start00_Ratio);
	WORD Start00_PosX = (WORD)(4096 * Start00_Ratio);
	WORD Start00_Width = (61440 - (Start00_PosX - 4096));

	float SaveBG_Ratio = ((float)SaveBGResX / (float)SaveBGResY);
	float SaveBG_ScaleX = (8192.0f * SaveBG_Ratio);
	WORD SaveBG_PosX = (WORD)(4096 * SaveBG_Ratio);
	WORD SaveBG_Width = (61440 - (SaveBG_PosX - 4096));

	// Write data to addresses
	UpdateMemoryAddress(Start00ScaleXAddr, &Start00_ScaleX, sizeof(float));
	UpdateMemoryAddress(Start00PosXAddr, &Start00_PosX, sizeof(WORD));
	UpdateMemoryAddress(Start00WidthAddr, &Start00_Width, sizeof(WORD));
	DWORD Value = 61698;
	UpdateMemoryAddress(Start00HeightTop, &Value, 2);
	Value = 3838;
	UpdateMemoryAddress(Start00HeightBottom, &Value, 2);

	UpdateMemoryAddress(SaveBGScaleXAddr, &SaveBG_ScaleX, sizeof(float));
	UpdateMemoryAddress(SaveBGPosXAddr, &SaveBG_PosX, sizeof(WORD));
	UpdateMemoryAddress(SaveBGWidthAddr, &SaveBG_Width, sizeof(WORD));
	UpdateMemoryAddress(SaveBGWidthNOPAddr, "\x90\x90\x90\x90\x90\x90", 6);
	UpdateMemoryAddress(SaveBGPosXNOPAddr, "\x90\x90\x90\x90\x90\x90", 6);

	if (Start00ResX > 512 && Start00ResY > 512)
	{
		Value = (WORD)-107;
		UpdateMemoryAddress(LogoHighlightHeight, &Value, 2);
		Value = 126;
		UpdateMemoryAddress(LogoHighlightYPos, &Value, 2);
	}
	else
	{
		Value = (WORD)-110;
		UpdateMemoryAddress(LogoHighlightHeight, &Value, 2);
		Value = 89;
		UpdateMemoryAddress(LogoHighlightYPos, &Value, 2);
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

	// Get texture size and scale
	if (!GetTextureRes(L"data\\pic\\out\\p_incar.tex", TextureResX, TextureResY))
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get texture resolution!";
		return;
	}
	Logging::Log() << "Texture resolution: " << TextureResX << "x" << TextureResY;
	TextureScaleX = ((float)TextureResX / ORG_TextureResX);
	TextureScaleY = ((float)TextureResY / ORG_TextureResY);

	// Get address locations
	constexpr BYTE MemorySearchBytes[]{ 0xDE, 0xC1, 0xDA, 0x44, 0x24, 0x08, 0x59, 0xC3, 0x90, 0x90, 0x90 };
	DWORD MemAddress = SearchAndGetAddresses(0x0044A5D5, 0x0044A775, 0x0044A775, MemorySearchBytes, sizeof(MemorySearchBytes), -0x18);

	constexpr BYTE WidthSearchBytes[]{ 0x85, 0xD2, 0xDB, 0x44, 0x24, 0x00, 0x89, 0x44, 0x24, 0x00, 0xDB, 0x44, 0x24, 0x00, 0xD8, 0xCB };
	void *WidthAddr = (void*)SearchAndGetAddresses(0x0049F024, 0x0049F2D4, 0x0049EB94, WidthSearchBytes, sizeof(WidthSearchBytes), -0x3A);

	constexpr BYTE VerBoundPosSearchBytes[]{ 0xDF, 0xE0, 0xF6, 0xC4, 0x05, 0x7A, 0x6D, 0xD9, 0x05 };
	void *VerBoundPosAddr1 = (void*)SearchAndGetAddresses(0x004A2A5F, 0x004A2D0F, 0x004A25CF, VerBoundPosSearchBytes, sizeof(VerBoundPosSearchBytes), 0x15);

	// Checking address pointer
	if (!MemAddress || !WidthAddr || !VerBoundPosAddr1)
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
	void *TextureScaleXAddr = (void*)((DWORD)WidthAddr + 0x4D0);
	void *TextureScaleYAddr = (void*)((DWORD)WidthAddr + 0x4E1);

	void *VerBoundPosAddr2 = (void*)((DWORD)VerBoundPosAddr1 + 0x013);
	void *VerBoundPosAddr3 = (void*)((DWORD)VerBoundPosAddr1 + 0x292);
	void *VerBoundPosAddr4 = (void*)((DWORD)VerBoundPosAddr1 + 0x2A9);
	void *VerBoundPosAddr5 = (void*)((DWORD)VerBoundPosAddr1 + 0x2D2);
	VerBoundPosIntAddr = (void*)((DWORD)VerBoundPosAddr1 + 0x048);
	VerBoundNegAddr1 = (void*)*(DWORD*)((DWORD)VerBoundPosAddr1 + 0x91);
	VerBoundNegAddr2 = (void*)((DWORD)VerBoundPosAddr1 + 0x42B);
	VerBoundNegAddr3 = (void*)((DWORD)VerBoundPosAddr1 + 0x0AE);

	ScreenPosX = (BYTE*)*(DWORD*)MemAddress;
	ScreenPosY = (BYTE*)((DWORD)ScreenPosX + 1);

	TexNameAddr = (char**)*(DWORD*)(MemAddress - 0x85);

	// Scale Start00 and SaveBG textures
	Start00Scaling();

	// Write data to addresses
	Logging::Log() << "Set texture scaling!";
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
	void *address = &TextureScaleX;
	UpdateMemoryAddress(TextureScaleXAddr, &address, sizeof(float));
	address = &TextureScaleY;
	UpdateMemoryAddress(TextureScaleYAddr, &address, sizeof(float));
	address = &VerBoundPos;
	UpdateMemoryAddress(VerBoundPosAddr1, &address, sizeof(float));
	UpdateMemoryAddress(VerBoundPosAddr2, &address, sizeof(float));
	UpdateMemoryAddress(VerBoundPosAddr3, &address, sizeof(float));
	UpdateMemoryAddress(VerBoundPosAddr4, &address, sizeof(float));
	UpdateMemoryAddress(VerBoundPosAddr5, &address, sizeof(float));
}

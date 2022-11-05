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
#include <vector>
#include <regex>
#include "Patches.h"
#include "FullscreenImages.h"
#include "Common\FileSystemHooks.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

// Common Values
float SizeNormal = 1.0f;
float SizeFullscreen = 1.0f;
float AspectRatio = 1.0f;		// this value should be softcoded or you need to calculate the aspect ratio from one of the textures. (TexResY / TexResX) = AspectRatio

float VerBoundPos = 240.0f;		// 240.0f / SizeNormal
DWORD VerBoundPosInt = 240;		// 240.0f / SizeNormal
float VerBoundNeg = 240.0f;		// -240.0f / SizeNormal

const DWORD Deuce = 2;

DWORD GameResX;					// Example: 1920 (can be acquired from 00A32898) 
DWORD GameResY;					// Example: 1080 (can be acquired from 00A3289C)

BYTE *ScreenPosX;				// Example: 0x00 (must be acquired from 01DBBFF7, byte)
BYTE *ScreenPosY;				// Example: 0x00 (must be acquired from 01DBBFF8, byte)

DWORD ORG_TextureResX;			// The original res X of texture
DWORD ORG_TextureResY;			// The original res Y of texture

DWORD TextureResX;				// New texture X size
DWORD TextureResY;				// New texture Y size

float TextureScaleX;			// TextureResX / ORG_TextureResX
float TextureScaleY;			// TextureResY / ORG_TextureResY

// Map scaling
DWORD MapTextureResX;			// New texture X size
DWORD MapTextureResY;			// New texture Y size

float MapAspectRatio;			// MapTextureResY / MapTextureResX
float MapResFormula;			// GameResY / (GameResX * (MapAspectRatio * 0.75f))
DWORD MapResX;					// GameResX * MapResFormula
DWORD MapResX_43;				// GameResX * ((GameResY / GameResX) / 0.75f)

float MapScaleX;				// 16.0f / MapAspectRatio

float MapMarkingWidth;			// 0.5f * MapAspectRatio
float MapMarkingPosX;			// 1.0f / MapAspectRatio
float MapMarkingPosX_00625 = 0.0625f;

float PlayerIconWidth;			// 1.0f * MapAspectRatio
float PlayerIconPosX;			// 1.0f * MapAspectRatio

float MapWidth;
float MapPosX;
DWORD MapScreenPosX;
float MapWidth_Divider = -1.0f;
float MapPosX_Divider = 2.0f;

// placeholder variables, don't assign value
DWORD ScreenPosXtmp;
DWORD ScreenPosYtmp;
DWORD TempResultX;
DWORD TempResultY;
DWORD OffsetX;
DWORD OffsetY;

void *jmpLoadTexture;

void *MapIDAddr = nullptr;

void *VerBoundPosIntAddr = nullptr;
void *VerBoundNegAddr1 = nullptr;
void *VerBoundNegAddr2 = nullptr;
void *VerBoundNegAddr3 = nullptr;

bool CheckingTexture = false;
char **TexNameAddr = nullptr;

void SetImageScaling();
void SetMapImageScaling();
bool GetTextureRes(char *TexName, DWORD &TextureX, DWORD &TextureY);

struct ImageCache
{
	void *Addr;
	BOOL Flag;
};

std::vector<ImageCache> TexList, MapList;

BOOL CheckTexture()
{
	static BOOL flag = FALSE;
	static void* last = nullptr;

	if (!TexNameAddr || !*TexNameAddr)
	{
		return FALSE;
	}

	if (last == *TexNameAddr ||
		std::any_of(TexList.begin(), TexList.end(), [](const ImageCache& TexCache) { if (TexCache.Addr == *TexNameAddr) { flag = TexCache.Flag; return true; } return false; }))
	{
		last = *TexNameAddr;
		return flag;
	}

	flag = (strcmp(*TexNameAddr, "data/etc/effect/lens_flare.tbn2") == 0) ? FALSE :
		(std::any_of(std::begin(DefaultTextureList), std::end(DefaultTextureList), [](const TexSize & TexItem) { return TexItem.IsScaled && strcmp(TexItem.Name, *TexNameAddr) == 0; }));
	
	TexList.push_back({ *TexNameAddr, flag });
	last = *TexNameAddr;

	return flag;
}

BOOL CheckMapTexture()
{
	static BOOL flag = FALSE;
	static void* last = nullptr;

	if (!TexNameAddr || !*TexNameAddr)
	{
		return FALSE;
	}

	if (last == *TexNameAddr ||
		std::any_of(MapList.begin(), MapList.end(), [](const ImageCache& TexCache) { if (TexCache.Addr == *TexNameAddr) { flag = TexCache.Flag; return true; } return false; }))
	{
		last = *TexNameAddr;
		return flag;
	}

	flag = flag = (strcmp(*TexNameAddr, "data/etc/effect/lens_flare.tbn2") == 0) ? FALSE :
		(std::any_of(std::begin(DefaultTextureList), std::end(DefaultTextureList), [](const TexSize & TexItem) { return TexItem.IsMap && strcmp(TexItem.Name, *TexNameAddr) == 0; }));
	
	MapList.push_back({ *TexNameAddr, flag });
	last = *TexNameAddr;

	return flag;
}

void GetTextureOnLoad()
{
	if (TexNameAddr && *TexNameAddr)
	{
		for (auto TexItem : DefaultTextureList)
		{
			if (TexItem.IsReference && strcmp(TexItem.Name, *TexNameAddr) == 0)
			{
				GetTextureRes(*TexNameAddr, TextureResX, TextureResY);
				ORG_TextureResX = TexItem.X;
				ORG_TextureResY = TexItem.Y;
				SetImageScaling();
				if (TexItem.IsMap)
				{
					MapTextureResX = TextureResX;
					MapTextureResY = TextureResY;
					SetMapImageScaling();
				}
				// Scale for X and Y needs to be the same on some screens
				if (TexItem.IsMap ||
					strncmp(*TexNameAddr, "data/menu/mc/savebg", 19) == 0 ||
					strncmp(*TexNameAddr, "data/pic/etc/start00", 20) == 0)
				{
					TextureScaleX = TextureScaleY;
				}
				break;
			}
		}
	}
}

// Check if passed filename is the texture being loaded
void OnFileLoadTex(LPCSTR lpFileName)
{
	if (!CheckingTexture && lpFileName && TexNameAddr && *TexNameAddr && CheckPathNameMatch(lpFileName, *TexNameAddr))
	{
		CheckingTexture = true;
		GetTextureOnLoad();
		CheckingTexture = false;
	}
}

// Runs each time a texture is loaded
void CheckLoadedTexture()
{
	if (!CheckingTexture)
	{
		CheckingTexture = true;
		GetTextureOnLoad();
		CheckingTexture = false;
	}
}

// ASM function to check texture on load
__declspec(naked) void __stdcall LoadTextureASM()
{
	__asm
	{
		push eax
		push edx
		call CheckLoadedTexture
		pop edx
		pop eax
		mov ecx, dword ptr ds : [esp + 0x40C]
		jmp jmpLoadTexture
	}
}

// ASM function to scale texture width
__declspec(naked) void __stdcall TexWidthASM()
{
	__asm
	{
		push ecx
		push edx
		call CheckTexture
		cmp eax, FALSE
		pop edx
		pop ecx
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
		ret
	}
}

// ASM function to scale texture height
__declspec(naked) void __stdcall TexHeightASM()
{
	__asm
	{
		push eax
		push ecx
		push edx
		call CheckTexture
		cmp eax, FALSE
		pop edx
		pop ecx
		pop eax
		fild dword ptr ds : [GameResY]
		je near NoScaling
	// Adjust scale
		fmul dword ptr ds : [SizeFullscreen]
		jmp near Exit
	NoScaling:
		fmul dword ptr ds : [SizeNormal]
	Exit:
		ret
	}
}

// ASM function to scale texture X-Pos
__declspec(naked) void __stdcall TexXPosASM()
{
	__asm
	{
		push eax
		push ecx
		push edx
		call CheckTexture
		cmp eax, FALSE
		pop edx
		pop ecx
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
		mov dword ptr ds : [ScreenPosXtmp], edx
		fiadd dword ptr ds : [ScreenPosXtmp]	// adds ScreenPosX
		fidiv dword ptr ds : [Deuce]			// divide by 2
		fistp dword ptr ds : [OffsetX]			// stores final result
		mov edx, dword ptr ds : [OffsetX]
		ret
	}
}

// ASM function to scale texture Y-Pos eax
__declspec(naked) void __stdcall TexYPosASM()
{
	__asm
	{
		push ecx
		push edx
		call CheckTexture
		cmp eax, FALSE
		pop edx
		pop ecx
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

// ASM function to scale Map Marking X Pos
__declspec(naked) void __stdcall MapMarkingXPosASM()
{
	__asm
	{
		fmul dword ptr ds : [MapMarkingPosX_00625]
		fdiv dword ptr ds : [MapMarkingPosX]
		ret
	}
}

// ASM function to scale Player Icon Width
__declspec(naked) void __stdcall PlayerIconWidthASM()
{
	__asm
	{
		fmul dword ptr ds : [PlayerIconWidth]
		fadd dword ptr ds : [esp + 0x18]
		fld dword ptr ds : [esp + 0x1C]
		ret
	}
}

// ASM function to scale Player Icon X Pos
__declspec(naked) void __stdcall PlayerIconXPosASM()
{
	__asm
	{
		fld dword ptr ds : [ebx]
		fmul dword ptr ds : [PlayerIconPosX]
		fstp dword ptr ds : [ebx]
		mov eax, dword ptr ds : [MapIDAddr]		// Map ID
		mov eax, dword ptr ds : [eax]
		ret
	}
}

// ASM function to scale Map Width
__declspec(naked) void __stdcall MapWidthASM()
{
	__asm
	{
		push eax
		push ecx
		push edx
		call CheckMapTexture
		cmp eax, FALSE
		pop edx
		pop ecx
		pop eax
		je near NoScaling
	// Adjust scale
		fadd dword ptr ds : [esp + 0x10]
		fadd dword ptr ds : [esp + 0x1C]
		fild dword ptr ds : [MapResX]			// Loads "MapResX" (example: 1920)
		fisub dword ptr ds : [MapResX_43]		// Subtracts by "MapResX_43"(example: 1440)
		fdiv dword ptr ds : [MapWidth_Divider]	// Divides by -1.0f (converts value to negative/positive)
		fstp dword ptr ds : [MapWidth]			// Stores value at "MapWidth"
		fadd dword ptr ds : [MapWidth]			// Adds "MapWidth"
		jmp near Exit
	NoScaling:
		fadd dword ptr ds : [esp + 0x10]
		fadd dword ptr ds : [esp + 0x1C]
	Exit:
		test eax, eax							// There's a conditional jump that depends on this! It must always be included!
		ret
	}
}

// ASM function to scale Map X Pos
__declspec(naked) void __stdcall MapXPosASM()
{
	__asm
	{
		push eax
		push ecx
		push edx
		call CheckMapTexture
		cmp eax, FALSE
		pop edx
		pop ecx
		pop eax
		je near NoScaling
	// Adjust scale
		mov dword ptr ds : [MapScreenPosX], edx		// Moves EDX value to "MapScreenPosX"
		fild dword ptr ds : [MapResX]				// Loads "MapResX" (example: 1920)
		fisub dword ptr ds : [MapResX_43]			// Subtracts by "MapResX_43"(example: 1440)
		fdiv dword ptr ds : [MapPosX_Divider]		// Divides by 2.0f
		fiadd dword ptr ds : [MapScreenPosX]		// Adds MapScreenPosX value
		fistp dword ptr ds : [MapPosX]				// Stores value at "MapPosX"
		mov edx, dword ptr ds : [MapPosX]			// moves MapPosX to EDX
		mov eax, dword ptr ds : [GameResY]			// moves GameResY to EAX
		jmp near Exit
	NoScaling:
		mov eax, dword ptr ds : [GameResY]			// moves GameResY to EAX
	Exit:
		ret
	}
}

// Get texture resolution
bool GetTextureRes(char *TexName, DWORD &TextureX, DWORD &TextureY)
{
	bool Result = false;

	const DWORD BytesToRead = 128;
	DWORD dwBytesRead;

	char TexPath[MAX_PATH];
	CopyReplaceSlash(TexPath, MAX_PATH, TexName);

	// Open file
	char Filename[MAX_PATH];
	HANDLE hFile = CreateFileA(GetFileModPath(TexPath, Filename), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	// Check file handle
	if (hFile == INVALID_HANDLE_VALUE)
	{
		LOG_LIMIT(100, __FUNCTION__ << " Error: failed to open file: '" << TexPath << "'");
		return false;
	}

	do {
		// Read data from file
		char TexData[BytesToRead];
		BOOL hRet = ReadFile(hFile, TexData, BytesToRead, &dwBytesRead, nullptr);
		if (dwBytesRead != BytesToRead || hRet == FALSE || memcmp(TexData, "\x01\x09\x99\x19", 4) != 0)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: file incorrect: '" << TexPath << "' " << BytesToRead << " " << dwBytesRead << " " << hRet << " " << Logging::hex(*(DWORD*)TexData));
			break;
		}
		// Get texture resolution
		for (auto& num : { 20, 24, 56 })
		{
			TextureX = *(WORD*)&TexData[num];
			TextureY = *(WORD*)&TexData[num + 2];
			if (TextureX && TextureY)
			{
				Result = true;
				break;
			}
		}
		if (!Result)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to get texture resolution: '" << TexPath << "'");
		}
	} while (FALSE);

	// Close file handle
	CloseHandle(hFile);

	return Result;
}

void SetImageScaling()
{
	TextureScaleX = (float)TextureResX / (float)ORG_TextureResX;
	TextureScaleY = (float)TextureResY / (float)ORG_TextureResY;
}

void SetMapImageScaling()
{
	MapAspectRatio = (float)MapTextureResY / (float)MapTextureResX;
	MapResFormula = (float)GameResY / ((float)GameResX * (MapAspectRatio * 0.75f));
	MapResX = (DWORD)(GameResX * MapResFormula);
	MapResX_43 = (DWORD)(GameResX * (((float)GameResY / (float)GameResX) / 0.75f));

	MapScaleX = 16.0f / MapAspectRatio;

	MapMarkingWidth = 0.5f * MapAspectRatio;
	MapMarkingPosX = 1.0f / MapAspectRatio;

	PlayerIconWidth = 1.0f * MapAspectRatio;
	PlayerIconPosX = 1.0f * MapAspectRatio;
}

void SetFullscreenImagesRes(DWORD Width, DWORD Height)
{
	GameResX = Width;
	GameResY = Height;

	float Ratio = (float)GameResX / (float)GameResY;

	float alg1 = 4.0f / 3.0f;
	float alg2 = 16.0f / 9.0f;
	float alg3 = Ratio * 0.75f;
	float alg4 = 1.25f + (Ratio - 1.25f) * (alg1 - 1.25f) / (alg2 - 1.25f);

	// Set scale ratio
	switch (FullscreenImages)
	{
	case 0: // original [Size = 1.0f]
		SizeFullscreen = 1.0f;
		break;
	case FIT_MEDIA: // pillarboxed / letterboxed [no cropping]
		SizeFullscreen = min(1.25f, alg3);
		break;
	default:
	case FILL_MEDIA: // cropped [zoom to fill screen]
		SizeFullscreen = min(alg2 * 0.75f, alg4);
		break;
	}

	if (VerBoundPosIntAddr && VerBoundNegAddr1 && VerBoundNegAddr2 && VerBoundNegAddr3)
	{
		VerBoundPos = 240.0f / SizeFullscreen;
		VerBoundPosInt = (DWORD)(240.0f / SizeFullscreen);
		VerBoundNeg = -240.0f / SizeFullscreen;

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

	DWORD Start00ResX, Start00ResY, SaveBGResX, SaveBGResY;
	if (!GetTextureRes("data\\pic\\etc\\start00.tex", Start00ResX, Start00ResY) ||
		!GetTextureRes("data\\menu\\mc\\savebg.tbn2", SaveBGResX, SaveBGResY))
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get texture resolution!";
		return;
	}

	float Start00_Ratio = (float)Start00ResX / (float)Start00ResY;
	float Start00_ScaleX = 8192.0f * Start00_Ratio;
	WORD Start00_PosX = (WORD)(4096 * Start00_Ratio);
	WORD Start00_Width = 61440 - (Start00_PosX - 4096);

	float SaveBG_Ratio = (float)SaveBGResX / (float)SaveBGResY;
	float SaveBG_ScaleX = 8192.0f * SaveBG_Ratio;
	WORD SaveBG_PosX = (WORD)(4096 * SaveBG_Ratio);
	WORD SaveBG_Width = 61440 - (SaveBG_PosX - 4096);

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

void PatchMapImages(DWORD WidthAddr)
{
	// Get address locations
	constexpr BYTE MapScaleSearchBytes[]{ 0xFF, 0xFF, 0x83, 0xC4, 0x10, 0xC3, 0x90, 0x90 };
	DWORD MapScaleAddr1 = SearchAndGetAddresses(0x0049E058, 0x0049E308, 0x0049DBC8, MapScaleSearchBytes, sizeof(MapScaleSearchBytes), -0xA8);

	constexpr BYTE MapMarkScaleSearchBytes[]{ 0xD8, 0xC2, 0xD9, 0x5C, 0x24, 0x10, 0xDD, 0xD8, 0xD9, 0x44, 0x24, 0x14, 0xD8, 0x0D };
	DWORD MapMarkScaleAddr1 = SearchAndGetAddresses(0x0049B678, 0x0049B928, 0x0049B1E8, MapMarkScaleSearchBytes, sizeof(MapMarkScaleSearchBytes), 0x0E);

	constexpr BYTE MapMarkAnimaScaleSearchBytes[]{ 0xD8, 0xC2, 0xD9, 0x5C, 0x24, 0x10, 0xDD, 0xD8, 0xD9, 0x44, 0x24, 0x18, 0xD8, 0x0D };
	DWORD MapMarkAnimaScaleAddr1 = SearchAndGetAddresses(0x0049CA1E, 0x0049CCCE, 0x0049C58E, MapMarkAnimaScaleSearchBytes, sizeof(MapMarkAnimaScaleSearchBytes), 0x0E);

	// Checking address pointer
	if (!MapScaleAddr1 || !MapMarkScaleAddr1 || !MapMarkAnimaScaleAddr1)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}

	// Maps
	DWORD MapScaleAddr2 = MapScaleAddr1 + 0x43;
	// Maps (Scripted Animation)
	DWORD MapAnimaScaleAddr1 = MapScaleAddr1 - 0x1E9;
	DWORD MapAnimaScaleAddr2 = MapScaleAddr1 - 0x1A6;
	// Map-Markings
	DWORD MapMarkScaleAddr2 = MapMarkScaleAddr1 + 0x20;
	// Map-Markings (Scripted Animation)
	DWORD MapMarkAnimaScaleAddr2 = MapMarkAnimaScaleAddr1 + 0x20;
	// Player-Icon
	DWORD PlayerIconScaleAddr1 = MapScaleAddr1 - 0x4B6;
	DWORD PlayerIconScaleAddr2 = PlayerIconScaleAddr1 + 0x29;
	DWORD PlayerIconScaleAddr3 = PlayerIconScaleAddr1 + 0x51;
	DWORD PlayerIconScaleAddr4 = PlayerIconScaleAddr1 + 0x84;
	// Map-Markings 
	DWORD MapMarkWidthAddr = MapMarkScaleAddr1 - 0x26;
	// Map-Markings (Scripted Animation)
	DWORD MapMarkAnimaWidthAddr = MapMarkAnimaScaleAddr1 - 0x26;
	// Map Marking X Pos
	DWORD MapMarkXPosAddr1 = MapMarkScaleAddr1 - 0x9D;
	DWORD MapMarkXPosAddr2 = MapMarkAnimaScaleAddr1 - 0xB9;
	// Player Icon Width
	DWORD PlayerIconWidthAddr1 = PlayerIconScaleAddr1 - 0x12B;
	DWORD PlayerIconWidthAddr2 = PlayerIconScaleAddr1 - 0xF5;
	DWORD PlayerIconWidthAddr3 = PlayerIconScaleAddr1 - 0xBF;
	DWORD PlayerIconWidthAddr4 = PlayerIconScaleAddr1 - 0x89;
	// Player Icon X Pos
	DWORD PlayerIconXPosAddr = PlayerIconScaleAddr1 - 0x360;
	// Map Width
	DWORD MapWidthAddr = WidthAddr + 0x322;
	// Map X Pos
	DWORD MapXPosAddr = WidthAddr + 0x305;
	// Map ID address
	MapIDAddr = (void*)*(DWORD*)(MapMarkScaleAddr1 - 0x255);

	// Write data to addresses
	Logging::Log() << "Set map scaling!";
	void *address = &MapScaleX;
	UpdateMemoryAddress((void*)MapScaleAddr1, &address, sizeof(float));
	UpdateMemoryAddress((void*)MapScaleAddr2, &address, sizeof(float));
	UpdateMemoryAddress((void*)MapAnimaScaleAddr1, &address, sizeof(float));
	UpdateMemoryAddress((void*)MapAnimaScaleAddr2, &address, sizeof(float));
	UpdateMemoryAddress((void*)MapMarkScaleAddr1, &address, sizeof(float));
	UpdateMemoryAddress((void*)MapMarkScaleAddr2, &address, sizeof(float));
	UpdateMemoryAddress((void*)MapMarkAnimaScaleAddr1, &address, sizeof(float));
	UpdateMemoryAddress((void*)MapMarkAnimaScaleAddr2, &address, sizeof(float));
	UpdateMemoryAddress((void*)PlayerIconScaleAddr1, &address, sizeof(float));
	UpdateMemoryAddress((void*)PlayerIconScaleAddr2, &address, sizeof(float));
	UpdateMemoryAddress((void*)PlayerIconScaleAddr3, &address, sizeof(float));
	UpdateMemoryAddress((void*)PlayerIconScaleAddr4, &address, sizeof(float));
	address = &MapMarkingWidth;
	UpdateMemoryAddress((void*)MapMarkWidthAddr, &address, sizeof(float));
	UpdateMemoryAddress((void*)MapMarkAnimaWidthAddr, &address, sizeof(float));
	WriteCalltoMemory((BYTE*)MapMarkXPosAddr1, MapMarkingXPosASM, 6);
	WriteCalltoMemory((BYTE*)MapMarkXPosAddr2, MapMarkingXPosASM, 6);
	WriteCalltoMemory((BYTE*)PlayerIconWidthAddr1, PlayerIconWidthASM, 8);
	WriteCalltoMemory((BYTE*)PlayerIconWidthAddr2, PlayerIconWidthASM, 8);
	WriteCalltoMemory((BYTE*)PlayerIconWidthAddr3, PlayerIconWidthASM, 8);
	WriteCalltoMemory((BYTE*)PlayerIconWidthAddr4, PlayerIconWidthASM, 8);
	WriteCalltoMemory((BYTE*)PlayerIconXPosAddr, PlayerIconXPosASM);
	WriteCalltoMemory((BYTE*)MapWidthAddr, MapWidthASM, 8);
	WriteCalltoMemory((BYTE*)MapXPosAddr, MapXPosASM);
}

void PatchFullscreenImages()
{
	// Get address locations
	constexpr BYTE MemorySearchBytes[]{ 0xDE, 0xC1, 0xDA, 0x44, 0x24, 0x08, 0x59, 0xC3, 0x90, 0x90, 0x90 };
	DWORD MemAddress = SearchAndGetAddresses(0x0044A5D5, 0x0044A775, 0x0044A775, MemorySearchBytes, sizeof(MemorySearchBytes), -0x18);

	constexpr BYTE WidthSearchBytes[]{ 0x85, 0xD2, 0xDB, 0x44, 0x24, 0x00, 0x89, 0x44, 0x24, 0x00, 0xDB, 0x44, 0x24, 0x00, 0xD8, 0xCB };
	void *WidthAddr = (void*)SearchAndGetAddresses(0x0049F024, 0x0049F2D4, 0x0049EB94, WidthSearchBytes, sizeof(WidthSearchBytes), -0x3A);

	constexpr BYTE VerBoundPosSearchBytes[]{ 0xDF, 0xE0, 0xF6, 0xC4, 0x05, 0x7A, 0x6D, 0xD9, 0x05 };
	void *VerBoundPosAddr1 = (void*)SearchAndGetAddresses(0x004A2A5F, 0x004A2D0F, 0x004A25CF, VerBoundPosSearchBytes, sizeof(VerBoundPosSearchBytes), 0x15);

	constexpr BYTE LoadTextureSearchBytes[]{ 0x8B, 0x8C, 0x24, 0x0C, 0x04, 0x00, 0x00, 0x56, 0x51, 0x8D, 0x54, 0x24, 0x0C, 0x52 };
	DWORD LoadTextureAddr = SearchAndGetAddresses(0x00449FFF, 0x0044A19F, 0x0044A19F, LoadTextureSearchBytes, sizeof(LoadTextureSearchBytes), 0x00);

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

	jmpLoadTexture = (void*)(LoadTextureAddr + 7);

	// Scale Start00 and SaveBG textures
	Start00Scaling();

	// Update Map scaling addresses
	PatchMapImages((DWORD)WidthAddr);

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
	WriteJMPtoMemory((BYTE*)LoadTextureAddr, LoadTextureASM, 7);
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

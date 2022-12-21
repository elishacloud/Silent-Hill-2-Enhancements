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

struct RESOLUTONTEXT
{
	char resStrBuf[27];
};

LONG MaxWidth = 0, MaxHeight = 0;
const DWORD MinWidth = 640;
const DWORD MinHeight = 480;

bool AutoScaleImages = false;
bool AutoScaleVideos = false;
BYTE *ResolutionIndex = nullptr;
BYTE *MenuResolutionIndex = nullptr;
BYTE *TextResIndex = nullptr;
DWORD InitialIndex = 1;
bool InitialIndexSet = false;
float *SetAspectRatio = nullptr;
RESOLUTONLIST *ResolutionArray = nullptr;
std::vector<RESOLUTONLIST> ResolutionVector;
std::vector<RESOLUTONTEXT> ResolutionText;

DWORD (*prepText)(char *str);
DWORD (*printTextPos)(char *str, int x, int y);

char *resStrPtr;

void GetConfiguredResolution(int &Width, int &Height)
{
	Width = ResolutionArray[*ResolutionIndex].Width;
	Height = ResolutionArray[*ResolutionIndex].Height;
}

void GetTextResolution(int &Width, int &Height)
{
	Width = ResolutionArray[*TextResIndex].Width;
	Height = ResolutionArray[*TextResIndex].Height;
}

void CreateResolutionText(int gWidth, int gHeight)
{
	char* text = "\\h%dx%d";
	if (abs((float)gWidth - (float)gHeight) < 1.0f)
		text = "\\h%dx%d (1:1)";
	else if (abs((float)gWidth / 3 - (float)gHeight / 2) < 1.0f)
		text = "\\h%dx%d (3:2)";
	else if (abs((float)gWidth / 4 - (float)gHeight / 1) < 1.0f)
		text = "\\h%dx%d (4:1)";
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
	else if (abs((float)gWidth / 16 - (float)gHeight / 9) < 0.5f)
		text = "\\h%dx%d (16:9)";
	else if (abs((float)gWidth / 16 - (float)gHeight / 10) < 0.5f)
		text = "\\h%dx%d (16:10)";
	else if (abs((float)gWidth / 17 - (float)gHeight / 9) < 0.5f ||
		abs((float)gWidth / 256 - (float)gHeight / 135) < 0.1f)
		text = "\\h%dx%d (17:9)";
	else if (abs((float)gWidth / 21 - (float)gHeight / 9) < 0.5f ||
		abs((float)gWidth / 64 - (float)gHeight / 27) < 0.5f ||
		abs((float)gWidth / 43 - (float)gHeight / 18) < 0.5f ||
		abs((float)gWidth / 12 - (float)gHeight / 5) < 1.0f)
		text = "\\h%dx%d (21:9)";
	else if (abs((float)gWidth / 25 - (float)gHeight / 16) < 0.5f)
		text = "\\h%dx%d (25:16)";
	else if (abs((float)gWidth / 32 - (float)gHeight / 9) < 0.5f)
		text = "\\h%dx%d (32:9)";
	else if (abs((float)gWidth / 32 - (float)gHeight / 15) < 0.5f)
		text = "\\h%dx%d (32:15)";
	else if (abs((float)gWidth / 48 - (float)gHeight / 9) < 0.5f)
		text = "\\h%dx%d (48:9)";

	RESOLUTONTEXT Buffer;
	sprintf_s((char *)Buffer.resStrBuf, sizeof(Buffer.resStrBuf), text, gWidth, gHeight);
	ResolutionText.push_back(Buffer);
}

int printResStr(unsigned short, unsigned char, int x, int y)
{
	resStrPtr = (char *)prepText(ResolutionText[*TextResIndex].resStrBuf);
	return printTextPos(resStrPtr, x, y);
}

extern char *getResolutionDescStr();

int printResDescStr(unsigned short, unsigned char, int x, int y)
{
	char *ptr = (char *)prepText(getResolutionDescStr());
	return printTextPos(ptr, x, y);
}

void *ResSelectStrRetAddr;

#pragma warning(suppress: 4740)
__declspec(naked) void __stdcall ResSelectStrASM()
{
	if (FixAdvancedOptions)
	{
		if (!isConfirmationPromptOpen()) // Needed to fix the advanced menu options. See AdvancedSettingsFix.cpp.
		{
			__asm {call printResStr}
		}
	}
	else
	{
		__asm {call printResStr}
	}

	__asm {jmp ResSelectStrRetAddr}
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

void SetDynamicScale(float AspectRatio)
{
	// Update FMV and fullscreen images
	if (AspectRatio <= 1.34f) // 4:3 (4:3, 5:4, etc.)
	{
		FullscreenImages = (AutoScaleImages) ? FIT_MEDIA : FullscreenImages;
		FullscreenVideos = (AutoScaleVideos) ? FIT_MEDIA : FullscreenVideos;
	}
	else if (AspectRatio < 1.76f) // 16:9 (16:10, 3:2, etc.)
	{
		FullscreenImages = (AutoScaleImages) ? FILL_MEDIA : FullscreenImages;
		FullscreenVideos = (AutoScaleVideos) ? FILL_MEDIA : FullscreenVideos;
	}
	else // AspectRatio >= 16:9 (16:9, 21:9, etc.)
	{
		FullscreenImages = (AutoScaleImages) ? FILL_MEDIA : FullscreenImages;
		FullscreenVideos = (AutoScaleVideos) ? FILL_MEDIA : FullscreenVideos;
	}
}

void UpdateResolutionPatches(LONG Width, LONG Height)
{
	static LONG OldWidth = 0, OldHeight = 0;

	// Check if resolution changed
	if (OldWidth != Width || OldHeight != Height)
	{
		Logging::Log() << "Setting resolution: " << Width << "x" << Height;

		// Set correct resolution for Room 312
		if (PauseScreenFix)
		{
			static LONG CachedWidth;
			CachedWidth = Width;
			SetRoom312Resolution(&CachedWidth);
		}

		// Set dynamic scaling
		if (AutoScaleImages || AutoScaleVideos)
		{
			SetDynamicScale((float)Width / (float)Height);
		}

		// Set fullscreen image resolution
		if (FullscreenImages)
		{
			SetFullscreenImagesRes(Width, Height);
		}

		// Set fullscreen video resolution
		if (FullscreenVideos)
		{
			SetFullscreenVideoRes(Width, Height);
		}
	}

	// Store new resolution
	OldWidth = Width;
	OldHeight = Height;
}

void UpdateWSF()
{
	// Set aspect ratio
	*SetAspectRatio = (float)ResX / (float)ResY;

	// Update Widescreen Fix for new resolution
	WSFInit();

	// Flush cache
	FlushInstructionCache(GetCurrentProcess(), nullptr, 0);

	// Update patches for resolution change
	UpdateResolutionPatches((LONG)ResX, (LONG)ResY);
}

void WSFDynamicStartup()
{
	// Store initual resolution
	if (!InitialIndexSet)
	{
		InitialIndexSet = true;
		InitialIndex = (*ResolutionIndex < 6) ? *ResolutionIndex : 1;
	}

	// Get saved resolution
	DWORD Width = 0, Height = 0;
	HRESULT hr = GetSavedResolution(Width, Height);

	// Check if resolution is found and set correct index
	bool found = false;
	if (SUCCEEDED(hr))
	{
		BYTE Index = 0;
		for (auto res : ResolutionVector)
		{
			if (res.Width == Width && res.Height == Height)
			{
				found = true;
				*ResolutionIndex = Index;
				break;
			}
			Index++;
		}
	}

	// Default to current resolution if index is too large or saved resolution is not found
	if (FAILED(hr) || (SUCCEEDED(hr) && !found) || *ResolutionIndex >= ResolutionVector.size())
	{
		*ResolutionIndex = (BYTE)(ResolutionVector.size() - 1);
		LONG screenWidth, screenHeight;
		GetDesktopRes(screenWidth, screenHeight);
		BYTE Index = 0;
		for (auto res : ResolutionVector)
		{
			if ((LONG)res.Width == screenWidth && (LONG)res.Height == screenHeight)
			{
				*ResolutionIndex = Index;
				break;
			}
			Index++;
		}
	}

	// Get initial resolution from index
	GetConfiguredResolution(ResX, ResY);

	// Update Widescreen Fix for initial resolution
	UpdateWSF();
}

void *jmpStartupRes = nullptr;
__declspec(naked) void __stdcall StartupResASM()
{
	__asm
	{
		push eax
		push ecx
		push edx
		call WSFDynamicStartup
		pop edx
		pop ecx
		pop eax
		jmp jmpStartupRes
	}
}

void WSFDynamicChange()
{
	// Detect if resolution changed
	int Width, Height;
	GetTextResolution(Width, Height);
	if (Width == ResX && Height == ResY)
	{
		return;
	}

	// Get updated resolution from index
	ResX = Width;
	ResY = Height;

	// Save updated resolution
	SaveResolution(ResX, ResY);

	// Update Widescreen Fix for new resolution
	UpdateWSF();
}

void *jmpChangeRes = nullptr;
__declspec(naked) void __stdcall ChangeResASM()
{
	__asm
	{
		push ebx
		push eax
		push ecx
		push edx
		call WSFDynamicChange
		pop edx
		pop ecx
		pop eax
		mov ebx, dword ptr ds : [TextResIndex]
		mov al, byte ptr ds : [ebx]
		pop ebx
		jmp jmpChangeRes
	}
}

void AddResolutionToList(DWORD Width, DWORD Height, bool force = false)
{
	if (ResolutionVector.size() >= 0xFF)
	{
		return;
	}

	bool found = false;
	for (auto res : ResolutionVector)
	{
		if (res.Width == Width && res.Height == Height)
		{
			found = true;
			break;
		}
	}
	bool NotTooLarge = ((ScreenMode == WINDOWED) ? (Width <= (DWORD)MaxWidth && Height <= (DWORD)MaxHeight) : true);
	bool NotTooSmall = (Width >= MinWidth && Height >= MinHeight);
	if (!found && (force || NotTooSmall) && NotTooLarge)
	{
		RESOLUTONLIST Resolution;
		Resolution.Width = Width;
		Resolution.Height = Height;
		ResolutionVector.push_back(Resolution);
		CreateResolutionText(Width, Height);
	}
}

void GetCustomResolutions()
{
	GetDesktopRes(MaxWidth, MaxHeight);

	do {
		// Exclusive fullscreen mode
		if (ScreenMode == EXCLUSIVE_FULLSCREEN)
		{
			IDirect3D8* pDirect3D = Direct3DCreate8Wrapper(D3D_SDK_VERSION);

			if (!pDirect3D)
			{
				Logging::Log() << __FUNCTION__ << " Error: failed to create Direct3D8!";
				break;
			}

			// Add custom resolution to list
			if (ResY && ResX)
			{
				AddResolutionToList(ResX, ResY, true);
			}

			// Setup display mode
			D3DDISPLAYMODE d3ddispmode;

			// Enumerate modes for format XRGB
			UINT modeCount = pDirect3D->GetAdapterModeCount(D3DADAPTER_DEFAULT);

			// Loop through each mode
			for (UINT i = 0; i < modeCount; i++)
			{
				// Get next resolution
				if (FAILED(pDirect3D->EnumAdapterModes(D3DADAPTER_DEFAULT, i, &d3ddispmode)))
				{
					break;
				}

				// Add resolution to list
				AddResolutionToList(d3ddispmode.Width, d3ddispmode.Height);
			}

			// Release Direct3D8
			pDirect3D->Release();
		}
		// Windowed and windowed fullscreen modes
		else
		{
			// Add custom resolution to list
			if (ResY && ResX)
			{
				AddResolutionToList(ResX, ResY, true);
			}

			// Get monitor info
			MONITORINFOEX infoex = {};
			infoex.cbSize = sizeof(MONITORINFOEX);
			BOOL bRet = GetMonitorInfo(GetMonitorHandle(), &infoex);

			// Get resolution list for specified monitor
			DEVMODE dm = {};
			dm.dmSize = sizeof(dm);
			for (int x = 0; EnumDisplaySettings(bRet ? infoex.szDevice : nullptr, x, &dm) != 0; x++)
			{
				// Add resolution to list
				AddResolutionToList(dm.dmPelsWidth, dm.dmPelsHeight);
			}
		}
	} while (FALSE);

	// Check if any resolutions were found
	if (ResolutionVector.empty())
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to get resolution list!  Adding default resolutions.";

		// Add default resolution list
		AddResolutionToList(640, 480);
		AddResolutionToList(800, 600);
		AddResolutionToList(1024, 768);
		AddResolutionToList(1280, 1024);
		AddResolutionToList(1600, 1200);
		AddResolutionToList(MaxWidth, MaxHeight);
	}
}

BYTE *ResStartupAddr = nullptr;
BYTE *ResSizeAddr = nullptr;
void SetResolutionList(DWORD Width, DWORD Height)
{
	if (WidescreenFix && DynamicResolution && ResStartupAddr && ResSizeAddr)
	{
		// If resolution list exists than replce with a new list
		if (ResolutionVector.size())
		{
			// Get current resolution from index
			int textWidth, textHeight;
			GetTextResolution(textWidth, textHeight);

			// Rebuild resolution list
			while (ResolutionVector.size())
			{
				ResolutionVector.pop_back();
			}
			while (ResolutionText.size())
			{
				ResolutionText.pop_back();
			}
			GetCustomResolutions();

			// Update current resolution index
			bool Found = false;
			for (DWORD x = 0; x < ResolutionVector.size(); x++)
			{
				if (ResolutionVector[x].Width == Width && ResolutionVector[x].Height == Height)
				{
					Found = true;
					*ResolutionIndex = (BYTE)x;
					break;
				}
			}
			if (!Found || *ResolutionIndex > ResolutionVector.size() - 1)
			{
				*ResolutionIndex = (BYTE)(ResolutionVector.size() - 1);
			}
			*(TextResIndex + 0x74) = *ResolutionIndex;
			*MenuResolutionIndex = *ResolutionIndex;

			// Update text resolution index
			Found = false;
			for (DWORD x = 0; x < ResolutionVector.size(); x++)
			{
				if (ResolutionVector[x].Width == (DWORD)textWidth && ResolutionVector[x].Height == (DWORD)textHeight)
				{
					Found = true;
					*TextResIndex = (BYTE)x;
					break;
				}
			}
			if (!Found || *TextResIndex > ResolutionVector.size() - 1)
			{
				*TextResIndex = *ResolutionIndex;
			}
		}

		// Update code to set the size of the resolution list
		BYTE ListSize = (BYTE)ResolutionVector.size();
		UpdateMemoryAddress((void *)(ResStartupAddr + 0x12 + 2), &ListSize, sizeof(BYTE));
		constexpr BYTE AsmBytes[] = { 0xB8, 0x01, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };	// mov eax, 1
		UpdateMemoryAddress((void *)(ResStartupAddr + 0x15 + 2), AsmBytes, sizeof(AsmBytes));
		--ListSize;
		UpdateMemoryAddress((void *)(ResSizeAddr + 1), &ListSize, sizeof(BYTE));
		DWORD delta = (GameVersion == SH2V_10) ? 0x33 : 0x31;
		UpdateMemoryAddress((void *)(ResSizeAddr + delta + 1), &ListSize, sizeof(BYTE));
	}
}

void SetResolutionPatch()
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

	// Get functions
	prepText = (DWORD(*)(char *str))(((BYTE*)DResAddrB + 0x15) + *(int *)((BYTE*)DResAddrB + 0x11));
	printTextPos = (DWORD(*)(char *str, int x, int y))(((BYTE*)DResAddrB + 0x1E) + *(int *)((BYTE*)DResAddrB + 0x1A));

	// Get resolution addresses
	constexpr BYTE SaveResIndexSearchBytes[] = { 0x8B, 0xF0, 0x83, 0xC4, 0x08, 0x85, 0xF6, 0x0F, 0x84 };
	void *SaveResIndexAddr = (BYTE*)SearchAndGetAddresses(0x004F7561, 0x004F7891, 0x004F71B1, SaveResIndexSearchBytes, sizeof(SaveResIndexSearchBytes), 0x5F);
	constexpr BYTE TextResIndexSearchBytes[] = { 0x68, 0x8B, 0x01, 0x00, 0x00, 0x6A, 0x46, 0x68, 0xD1, 0x00, 0x00, 0x00, 0x52, 0xE8 };
	TextResIndex = (BYTE*)ReadSearchedAddresses(0x00465631, 0x004658CD, 0x00465ADD, TextResIndexSearchBytes, sizeof(TextResIndexSearchBytes), 0x29);
	constexpr BYTE ResolutionIndexSearchBytes[] = { 0x6A, 0x01, 0x6A, 0x00, 0x50, 0xFF, 0x51, 0x34, 0x6A, 0x00, 0xE8 };
	ResolutionIndex = (BYTE*)ReadSearchedAddresses(0x004F633F, 0x004F65EF, 0x004F5EAF, ResolutionIndexSearchBytes, sizeof(ResolutionIndexSearchBytes), 0x10);
	constexpr BYTE MenuResolutionIndexSearchBytes[] = { 0x6A, 0x01, 0x6A, 0x00, 0x50, 0xFF, 0x51, 0x34, 0x6A, 0x00, 0xE8 };
	MenuResolutionIndex = (BYTE*)ReadSearchedAddresses(0x004F633F, 0x004F65EF, 0x004F5EAF, MenuResolutionIndexSearchBytes, sizeof(MenuResolutionIndexSearchBytes), 0x1C);
	constexpr BYTE ResolutionArraySearchBytes[] = { 0x10, 0x51, 0x56, 0x6A, 0x00, 0x50, 0xFF, 0x52, 0x1C, 0x8B, 0x54, 0x24, 0x10 };
	ResolutionArray = (RESOLUTONLIST*)ReadSearchedAddresses(0x004F5DEB, 0x004F609B, 0x004F595B, ResolutionArraySearchBytes, sizeof(ResolutionArraySearchBytes), 0xF);
	ResStartupAddr = (BYTE*)SearchAndGetAddresses(0x004F5DEB, 0x004F609B, 0x004F595B, ResolutionArraySearchBytes, sizeof(ResolutionArraySearchBytes), 0x4B);
	constexpr BYTE ChangeResSearchBytes[] = { 0x74, 0x16, 0x0F, 0xB6, 0xD0, 0x52, 0xE8 };
	BYTE *ChangeResAddr = (BYTE*)SearchAndGetAddresses(0x00464DF3, 0x00465083, 0x00465293, ChangeResSearchBytes, sizeof(ChangeResSearchBytes), -0xB);
	constexpr BYTE ResSizeSearchBytes[] = { 0x3C, 0x05, 0x73, 0x04, 0xFE, 0xC0, 0xEB, 0x02, 0x32, 0xC0, 0x0F, 0xB6, 0xC8, 0x51, 0xA2 };
	ResSizeAddr = (BYTE*)SearchAndGetAddresses(0x00465F2A, 0x004661CC, 0x004663DC, ResSizeSearchBytes, sizeof(ResSizeSearchBytes), 0x00);
	constexpr BYTE ResConfigSearchBytes[] = { 0x1B, 0xD2, 0x42, 0xC1, 0xE8, 0x10, 0x89, 0x15 };
	BYTE *ResConfigAddr = (BYTE*)SearchAndGetAddresses(0x004F71CD, 0x004F74FD, 0x004F6E1C, ResConfigSearchBytes, sizeof(ResConfigSearchBytes), -0x28);
	constexpr BYTE AspectRatioBytes[] = { 0x83, 0xEC, 0x3C, 0x55, 0x56, 0x8B, 0xF1, 0x8B, 0x0B, 0x89, 0x44, 0x24, 0x14, 0x57, 0x03, 0xC6, 0x8B, 0xFA, 0x89, 0x44, 0x24, 0x20 };
	SetAspectRatio = (float*)ReadSearchedAddresses(0x00458D80, 0x00458FE0, 0x00458FE0, AspectRatioBytes, sizeof(AspectRatioBytes), -0x44);
	if (!SaveResIndexAddr || !TextResIndex || !ResolutionIndex || !MenuResolutionIndex || !ResolutionArray || !ResStartupAddr || !ChangeResAddr || !ResSizeAddr || !ResConfigAddr || !SetAspectRatio)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory addresses!";
		return;
	}
	TextResIndex += 1;

	// Dynamic resolution
	if (DynamicResolution && WidescreenFix)
	{
		Logging::Log() << "Enabling Dynamic Resolution...";

		ResolutionVector.reserve(0xFF);		// Reserve space for max resolution limit
		GetCustomResolutions();

		if (ResolutionVector.size())
		{
			ResolutionArray = &ResolutionVector[0];

			BYTE *ArrayWidth = (BYTE*)ResolutionArray;
			BYTE *ArrayHeight = (BYTE*)ResolutionArray + 4;

			UpdateMemoryAddress((void *)(ResStartupAddr + 0x3E + 3), &ArrayWidth, sizeof(DWORD));
			UpdateMemoryAddress((void *)(ResStartupAddr + 0x4B + 3), &ArrayHeight, sizeof(DWORD));

			UpdateMemoryAddress((void *)(ResStartupAddr + 0x51D + 3), &ArrayWidth, sizeof(DWORD));
			UpdateMemoryAddress((void *)(ResStartupAddr + 0x529 + 3), &ArrayHeight, sizeof(DWORD));

			// Set the number of resolutions in the list
			SetResolutionList(ResX, ResY);

			DWORD Value = (DWORD)&InitialIndex;
			UpdateMemoryAddress(SaveResIndexAddr, &Value, sizeof(DWORD));
			jmpStartupRes = ResConfigAddr + 0xF;
			WriteJMPtoMemory(ResConfigAddr, *StartupResASM, 0xF);
			jmpChangeRes = ChangeResAddr + 5;
			WriteJMPtoMemory(ChangeResAddr, *ChangeResASM);
		}
	}

	// Check if resolution is locked
	if (*(DWORD*)((BYTE*)ResolutionArray) == *(DWORD*)((BYTE*)ResolutionArray + 8) &&
		*(DWORD*)((BYTE*)ResolutionArray + 4) == *(DWORD*)((BYTE*)ResolutionArray + 12))
	{
		Logging::Log() << "Enabling Resolution Lock...";

		// Lock resolution
		void *ResSelectorAddrExit = (void *)((BYTE*)ResSelectorAddr + exitOffset);
		WriteJMPtoMemory((BYTE*)ResSelectorAddr, ResSelectorAddrExit, 5);

		// Update resolution description string
		if (UseCustomExeStr)
		{
			WriteCalltoMemory(((BYTE*)DResAddrA + 0x55), *printResDescStr, 5);
		}
	}

	// Cache resolution text
	if (ResolutionText.empty())
	{
		for (int x = 0; x < 6; x++)
		{
			CreateResolutionText(ResolutionArray[x].Width, ResolutionArray[x].Height);
		}
	}

	// Update resolution strings
	ResSelectStrRetAddr = (DWORD *)(((BYTE*)DResAddrA + 0x92) + *(int *)((BYTE*)DResAddrA + 0x8E) + 5);
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

void *AdvOptionsSetsCallAddr = nullptr;
void *AdvOptionsSetsRetAddr = nullptr;
__declspec(naked) void __stdcall AdvOptionsSetsASM()
{
	__asm
	{
		test eax, eax
		jnz near Exit
		call AdvOptionsSetsCallAddr		// Set SH2 defaults

	Exit:
		call AdvOptionsSets				// Set "Best Graphics" defaults
		jmp AdvOptionsSetsRetAddr
	}
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

	DWORD CallAddr = 0;
	if (GameVersion == SH2V_DC)
	{
		CallAddr = (DWORD)((BYTE*)DOptAddrB + 0x77);
	}
	else
	{
		CallAddr = (DWORD)((BYTE*)DOptAddrB + 0x78);
	}

	AdvOptionsSetsCallAddr = (void*)(*(DWORD*)(CallAddr + 5) + (CallAddr + 5) + 4);
	AdvOptionsSetsRetAddr = (void*)(CallAddr + 11);
	WriteJMPtoMemory((BYTE*)CallAddr, AdvOptionsSetsASM, 11);
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

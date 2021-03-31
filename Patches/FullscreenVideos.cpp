/**
* Copyright (C) 2021 Elisha Riedlinger
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
#pragma warning(push)
#pragma warning(disable: 4178 4201 4305 4309 4458 4510 4996)
#include "External\injector\include\injector\injector.hpp"
#include "External\Hooking.Patterns\Hooking.Patterns.h"
#pragma warning(pop)
#include "Patches.h"
#include "FullscreenImages.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

namespace
{
	DWORD GameResX;
	DWORD GameResY;

	DWORD VideoResX;				// New video X size
	DWORD VideoResY;				// New video Y size

	bool ForceCropped = false;		// Force cropping certian videos

	void *WidthAddr1;
	void *WidthAddr2;
	void *WidthAddr3;
	void *HeightAddr1;
	void *HeightAddr2;
	void *HeightAddr3;

	// asm variables
	char *LoadingVideoName = nullptr;
	void *VideoCallAddr = nullptr;
	void *JmpVideoAddr = nullptr;
}

bool GetFMVRes(char *VideoName, DWORD &VideoX, DWORD &VideoY);

void SetVideoScaling();

// Check if loading a Game Result save
void UpdateLoadedVideo()
{
	// Get FMV resolution
	GetFMVRes(LoadingVideoName, VideoResX, VideoResY);

	// Scale FMV for displaying on screen
	ForceCropped = (strcmp(LoadingVideoName, "data\\movie\\water.bik") == 0);
	SetVideoScaling();
}

// ASM function to Fix Game Result Saves
__declspec(naked) void __stdcall LoadingVideoASM()
{
	__asm
	{
		pushf
		push eax
		push ecx
		push edx
		mov dword ptr ds : [LoadingVideoName], ecx
		call UpdateLoadedVideo
		pop edx
		pop ecx
		pop eax
		popf
		call VideoCallAddr
		jmp JmpVideoAddr
	}
}

void PatchFullscreenVideos()
{
	// Get address
	constexpr BYTE VideoSearchBytes[]{ 0x89, 0x44, 0x24, 0x20, 0x66, 0x89, 0x4C, 0x24, 0x24, 0x88, 0x54, 0x24, 0x26, 0x8D, 0x44, 0x24, 0x10, 0x50, 0x6A, 0x01 };
	BYTE *VideoAddr = (BYTE*)SearchAndGetAddresses(0x0043D7F5, 0x0043D9B5, 0x0043D9B5, VideoSearchBytes, sizeof(VideoSearchBytes), 0x39);

	// Checking address pointer
	if (!VideoAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return;
	}
	JmpVideoAddr = VideoAddr + 5;
	VideoCallAddr = (DWORD*)(((BYTE*)VideoAddr + 0x1) + *(int*)((BYTE*)VideoAddr + 0x01) + 4);

	//FMV Width
	auto FMVpattern1 = hook::pattern("A1 ? ? ? ? D9 15 ? ? ? ? D9 C2 89 15 ? ? ? ? D9 1D");
	auto FMVpattern2 = hook::pattern("D8 25 ? ? ? ? 8B 0D ? ? ? ? 85 C9 8B 15");
	auto FMVpattern3 = hook::pattern("8B 15 ? ? ? ? A1 ? ? ? ? 89 15 ? ? ? ? A3");
	WidthAddr1 = FMVpattern1.count(1).get(0).get<uint32_t>(1); //0043E318
	WidthAddr2 = FMVpattern2.count(1).get(0).get<uint32_t>(2); //0043E305
	WidthAddr3 = FMVpattern3.count(1).get(0).get<uint32_t>(2); //0043E2BF

	//FMV Height
	auto FMVpattern4 = hook::pattern("A1 ? ? ? ? 89 15 ? ? ? ? A3 ? ? ? ? C7 05");
	auto FMVpattern5 = hook::pattern("8B 15 ? ? ? ? A1 ? ? ? ? D9 15");
	auto FMVpattern6 = hook::pattern("D8 25 ? ? ? ? A1 ? ? ? ? 68");
	HeightAddr1 = FMVpattern4.count(1).get(0).get<uint32_t>(1); //0043E2C4
	HeightAddr2 = FMVpattern5.count(1).get(0).get<uint32_t>(2); //0043E313
	HeightAddr3 = FMVpattern6.count(1).get(0).get<uint32_t>(2); //0043E363

	// Update SH2 code
	Logging::Log() << "Fixing Movies ratio crash...";
	WriteJMPtoMemory((BYTE*)VideoAddr, *LoadingVideoASM, 5);
}

// Get FMV resolution
bool GetFMVRes(char *VideoName, DWORD &VideoX, DWORD &VideoY)
{
	HANDLE hFile;
	DWORD FileSize;
	const DWORD BytesToRead = 128;
	DWORD dwBytesRead;

	char VideoData[BytesToRead];
	hFile = CreateFileA(VideoName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	FileSize = GetFileSize(hFile, nullptr);

	if (FileSize < BytesToRead || FileSize == 0xFFFFFFFF ||
		!ReadFile(hFile, VideoData, BytesToRead, &dwBytesRead, nullptr) ||
		dwBytesRead < BytesToRead ||
		memcmp(VideoData, "\x42\x49\x4b", 3))
	{
		CloseHandle(hFile);
		return false;
	}
	for (auto& num : { 0x14 })
	{
		VideoX = *(WORD*)&VideoData[num];
		VideoY = *(WORD*)&VideoData[num + 4];
		if (VideoX && VideoY)
		{
			CloseHandle(hFile);
			return true;
		}
	}
	CloseHandle(hFile);
	return false;
}

void SetFullscreenVideoRes(DWORD Width, DWORD Height)
{
	GameResX = Width;
	GameResY = Height;
}

void SetVideoScaling()
{
	float fWidth = (float)GameResX;
	float fHeight = (float)GameResY;

	float TextOffset = (fWidth - fHeight * (4.0f / 3.0f)) / 2.0f;

	float GameRatio = (float)GameResX / (float)GameResY;
	float VideoRatio = (float)VideoResX / (float)VideoResY;

	static float FMVWidth = 0.0f;
	static float FMVHeight = 0.0f;

	// Set scale ratio
	switch ((ForceCropped) ? 2 : FullscreenVideos)
	{
	case 0: // original [Size = 1.0f]
		FMVWidth = TextOffset - (((fHeight * (5.0f / 3.0f)) - fHeight * (4.0f / 3.0f)) / 2.0f);
		FMVHeight = 0.0f;
		break;
	case 1: // pillarboxed / letterboxed [no cropping]
		if (GameRatio > VideoRatio)
		{
			FMVWidth = TextOffset - (((fHeight * VideoRatio) - fHeight * (4.0f / 3.0f)) / 2.0f);
			FMVHeight = 0.0f;
		}
		else
		{
			FMVWidth = TextOffset - (((fHeight * GameRatio) - fHeight * (4.0f / 3.0f)) / 2.0f);
			FMVHeight = ((fHeight - (fWidth / (float)VideoResX) * VideoResY) / 2.0f);
		}
		break;
	default:
	case 2: // cropped [zoom to fill screen]
		if (GameRatio > VideoRatio)
		{
			float MaxGameRatio = min(16.0f / 9.0f, GameRatio);
			FMVWidth = TextOffset - (((fHeight * MaxGameRatio) - fHeight * (4.0f / 3.0f)) / 2.0f);
			FMVHeight = ((fHeight - ((fHeight * MaxGameRatio) / (float)VideoResX) * VideoResY) / 2.0f);
		}
		else
		{
			FMVWidth = TextOffset - (((fHeight * VideoRatio) - fHeight * (4.0f / 3.0f)) / 2.0f);
			FMVHeight = 0.0f;
		}
		break;
	}

	//FMV Width
	injector::WriteMemory(WidthAddr1, &FMVWidth, true);
	injector::WriteMemory(WidthAddr2, &FMVWidth, true);
	injector::WriteMemory(WidthAddr3, &FMVWidth, true);

	//FMV Height
	injector::WriteMemory(HeightAddr1, &FMVHeight, true);
	injector::WriteMemory(HeightAddr2, &FMVHeight, true);
	injector::WriteMemory(HeightAddr3, &FMVHeight, true);
}

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
#include <fstream>
#include "Patches.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

void ScaleTexture(wchar_t *TexName, float *XScaleAddress, float *YScaleAddress, WORD *WidthAddress, WORD *XPosAddress)
{
	// Get texture resolution
	std::ifstream in(TexName, std::ifstream::ate | std::ifstream::binary);
	if (!in.is_open() || in.tellg() < 128)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to read texture '" << TexName << "'!";
		if (in.is_open())
		{
			in.close();
		}
		return;
	}
	WORD TextureXRes = 0, TextureYRes = 0;
	for (auto& num : { 20, 24, 56 })
	{
		in.seekg(num);
		in.read((char*)&TextureXRes, 2);
		in.read((char*)&TextureYRes, 2);
		if (TextureXRes && TextureYRes)
		{
			break;
		}
	}
	in.close();

	// Check texture resolution
	if (!TextureXRes || !TextureYRes)
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
	WORD Width = (WORD)(4096 / AspectRatio);
	WORD XPos = (WORD)(61440 - (Width - 4096));

	// Update memory
	Logging::Log() << __FUNCTION__ << " Scaling texture: " << TexName << " Resolution: " << TextureXRes << "x" << TextureYRes << " XYScale: " << XScale << "x" << YScale << " Width: " << Width << " XPos: " << XPos;
	UpdateMemoryAddress(XScaleAddress, &XScale, sizeof(float));
	UpdateMemoryAddress(YScaleAddress, &YScale, sizeof(float));
	UpdateMemoryAddress(WidthAddress, &Width, sizeof(WORD));
	UpdateMemoryAddress(XPosAddress, &XPos, sizeof(WORD));
}

void UpdateTexAddr()
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
	const DWORD size = 0x01400000;		// Just hard code to 20 MBs

	// Allocate dynamic memory for loading textures
	BYTE *PtrBytes1 = new BYTE[size];
	BYTE *PtrBytes2 = new BYTE[size];

	// Write new memory static address
	UpdateMemoryAddress((void*)StaticAddr, &PtrBytes1, sizeof(void*));

	// Write new memory address 1
	UpdateMemoryAddress((void*)Addr1, &PtrBytes1, sizeof(void*));

	// Write new memory address 2
	BYTE NewByte = 0xB8;
	UpdateMemoryAddress((void*)Addr2, &NewByte, sizeof(BYTE));
	UpdateMemoryAddress((void*)(Addr2 + 1), &PtrBytes2, sizeof(void*));

	// Write new memory address 3
	UpdateMemoryAddress((void*)Addr3, &NewByte, sizeof(BYTE));
	UpdateMemoryAddress((void*)(Addr3 + 1), &PtrBytes2, sizeof(void*));

	{// start00.tex
		constexpr BYTE SearchBytes[]{ 0x66, 0x0D, 0x02, 0x00, 0x66, 0x0D, 0x04, 0x00, 0x68 };
		const DWORD BaseAddress = SearchAndGetAddresses(0x00496974, 0x00496C14, 0x00496DF4, SearchBytes, sizeof(SearchBytes), 0x1C);
		if (BaseAddress)
		{
			ScaleTexture(L"data\\pic\\etc\\start00.tex", (float*)(BaseAddress + 0x38), (float*)(BaseAddress + 0x42), (WORD*)(BaseAddress + 0x13), (WORD*)(BaseAddress + 0x01));
		}
	}

	{// savebg.tbn2
		constexpr BYTE SearchBytes[]{ 0x66, 0x0D, 0x02, 0x00, 0x66, 0x0D, 0x04, 0x00, 0x66, 0xA3 };
		const DWORD BaseAddress = SearchAndGetAddresses(0x0044B4B8, 0x0044B658, 0x0044B658, SearchBytes, sizeof(SearchBytes), -0x28);
		if (BaseAddress)
		{
			const DWORD BaseAddress2 = *(DWORD*)(BaseAddress + 0x07);
			ScaleTexture(L"data\\menu\\mc\\savebg.tbn2", (float*)(BaseAddress + 0x64), (float*)(BaseAddress + 0x6E), (WORD*)(BaseAddress2 + 0x00), (WORD*)(BaseAddress2 + 0x04));
			constexpr BYTE nop6[]{ 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
			UpdateMemoryAddress((void*)(BaseAddress + 0x05), (void*)nop6, sizeof(nop6));
			UpdateMemoryAddress((void*)(BaseAddress + 0x16), (void*)nop6, sizeof(nop6));
		}
	}
}

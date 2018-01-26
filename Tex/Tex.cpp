/**
* Copyright (C) 2017 Elisha Riedlinger
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
#include <string>
#include "Tex.h"
#include "..\Common\Utils.h"
#include "..\Common\Logging.h"

char *PtrBytes = nullptr;

void UpdateTexAddr()
{
	/*DWORD start = 0x00401000;
	const DWORD distance = 0x00227FFF;
	const DWORD Offset = start;
	BYTE TestData[] = { 0x40, 0xEC, 0xDB, 0x01 };
	while (Offset >= start && Offset < start + distance)
	{
		void *TestPtr = GetAddressOfData(TestData, 4, 1, Offset, distance - (Offset - start));

		// Log message
		if (TestPtr) Log() << " ***  Logging pointer at address: " << TestPtr;
		Offset = (DWORD)TestPtr + 0x00000010;
	}

	return; //  <----  *********************************************



	// Find address for sddata.bin file pointer
	void *sfxAddr = (void*)0x00401CC1;

	// Log message
	Log() << "Found sddata.bin pointer at address: " << sfxAddr;

	// Alocate memory
	const DWORD size = 12000000;
	PtrBytes = new char[size + 1];

	// Update sddata.bin pointer address
	DWORD oldProtect;
	if (VirtualProtect(sfxAddr, 4, PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		Log() << "Updating sddata.bin pointer memory addresses";

		// Write to memory
		*((DWORD *)((DWORD)sfxAddr)) = (DWORD)PtrBytes + 140;

		// Restore protection
		VirtualProtect(sfxAddr, 4, oldProtect, &oldProtect);
	}

	return; //  <----  ********************************************/



	// Loop through each texture
	for (size_t x = 0; x < 1; x++)
	{
		// Get file path
		char myPath[MAX_PATH];
		GetModuleFileNameA(nullptr, myPath, MAX_PATH);
		char* p_pName = strrchr(myPath, '\\');
		strcpy_s(p_pName, MAX_PATH - strlen(myPath), TexAddrList[x].name);

		// Get file size
		Log() << "Opening " << myPath;
		std::ifstream infile;
		infile.open(myPath, std::ios::binary | std::ios::in | std::ios::ate);
		DWORD size = (DWORD)infile.tellg() + 1024;
		infile.close();

		if (size > 0)
		{
			void *TexAddr;
			// Find address for texture file pointer
			const DWORD start = 0x00401000;
			const DWORD distance = 0x00227FFF;
			TexAddr = GetAddressOfData(TexAddrList[x].vDC, 5, 1, start, distance);																				// Directors Cut
			TexAddr = ((DWORD)TexAddr < start || (DWORD)TexAddr > start + distance) ? GetAddressOfData(TexAddrList[x].v10, 5, 1, start, distance) : TexAddr;	// v1.0
			TexAddr = ((DWORD)TexAddr < start || (DWORD)TexAddr > start + distance) ? GetAddressOfData(TexAddrList[x].v11, 5, 1, start, distance) : TexAddr;	// v1.1
			TexAddr = ((DWORD)TexAddr < start || (DWORD)TexAddr > start + distance) ? nullptr : TexAddr;

			if (TexAddr)
			{
				// Log message
				Log() << "Found '" << TexAddrList[x].name << "' pointer at address: " << TexAddr;

				// Alocate memory
				char *PtrBytes;
				PtrBytes = new char[size * 8 + 1];

				// Update sddata.bin pointer address
				DWORD oldProtect;
				if (VirtualProtect(TexAddr, 5, PAGE_EXECUTE_READWRITE, &oldProtect))
				{
					Log() << "Updating '" << TexAddrList[x].name << "' pointer memory addresses";

					// Write to memory
					*((DWORD *)((DWORD)TexAddr + 1)) = (DWORD)PtrBytes + 140;

					// Restore protection
					VirtualProtect(TexAddr, 5, oldProtect, &oldProtect);
				}
				else
				{
					Log() << "Could not write to memory!";
				}
			}
			else
			{
				Log() << "Could not find '" << TexAddrList[x].name << "' pointer address in memory!";
			}
		}
		else
		{
			Log() << "'" << TexAddrList[x].name << "' is zero bytes!";
		}
	}
}

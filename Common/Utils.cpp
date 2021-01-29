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
#include <atlstr.h>
#include <string>
#include <iostream>
#include <filesystem>
#include "Utils.h"
#include "Patches\Patches.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"

#ifndef _DPI_AWARENESS_CONTEXTS_

#define _DPI_AWARENESS_CONTEXTS_

DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);

typedef enum DPI_AWARENESS {
	DPI_AWARENESS_INVALID = -1,
	DPI_AWARENESS_UNAWARE = 0,
	DPI_AWARENESS_SYSTEM_AWARE = 1,
	DPI_AWARENESS_PER_MONITOR_AWARE = 2
} DPI_AWARENESS;

#define DPI_AWARENESS_CONTEXT_UNAWARE               ((DPI_AWARENESS_CONTEXT)-1)
#define DPI_AWARENESS_CONTEXT_SYSTEM_AWARE          ((DPI_AWARENESS_CONTEXT)-2)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE     ((DPI_AWARENESS_CONTEXT)-3)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2  ((DPI_AWARENESS_CONTEXT)-4)
#define DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED     ((DPI_AWARENESS_CONTEXT)-5)

typedef enum DPI_HOSTING_BEHAVIOR {
	DPI_HOSTING_BEHAVIOR_INVALID = -1,
	DPI_HOSTING_BEHAVIOR_DEFAULT = 0,
	DPI_HOSTING_BEHAVIOR_MIXED = 1
} DPI_HOSTING_BEHAVIOR;

#endif

typedef enum PROCESS_DPI_AWARENESS {
	PROCESS_DPI_UNAWARE = 0,
	PROCESS_SYSTEM_DPI_AWARE = 1,
	PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

typedef HRESULT(WINAPI *SetProcessDpiAwarenessProc)(PROCESS_DPI_AWARENESS value);
typedef BOOL(WINAPI *SetProcessDPIAwareProc)();
typedef BOOL(WINAPI *SetProcessDpiAwarenessContextProc)(DPI_AWARENESS_CONTEXT value);

extern bool m_StopThreadFlag;
std::vector<HMODULE> custom_dll;		// Used for custom dll's and asi plugins

// Search memory for byte array
void *GetAddressOfData(const void *data, size_t len, DWORD step)
{
	return GetAddressOfData(data, len, step, 0);
}

// Search memory for byte array
void *GetAddressOfData(const void *data, size_t len, DWORD step, DWORD start, DWORD distance)
{
	HANDLE hProcess = GetCurrentProcess();
	if (hProcess)
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);

		MEMORY_BASIC_INFORMATION info;
		std::string chunk;
		BYTE* p = (BYTE*)start;
		while (p < si.lpMaximumApplicationAddress && (DWORD)p < start + distance)
		{
			if (VirtualQueryEx(hProcess, p, &info, sizeof(info)) == sizeof(info))
			{
				p = (BYTE*)info.BaseAddress;
				chunk.resize(info.RegionSize);
				SIZE_T bytesRead;
				if (ReadProcessMemory(hProcess, p, &chunk[0], info.RegionSize, &bytesRead))
				{
					for (size_t i = 0; i < (bytesRead - len); i += step)
					{
						if ((DWORD)p + i > start)
						{
							if (memcmp(data, &chunk[i], len) == 0)
							{
								return (BYTE*)p + i;
							}
						}
						if ((DWORD)p > start + distance)
						{
							return nullptr;
						}
					}
				}
				p += info.RegionSize;
			}
			else
			{
				return nullptr;
			}
		}
	}
	return nullptr;
}

// Checks the value of two data segments
bool CheckMemoryAddress(void *dataAddr, void *dataBytes, size_t dataSize, bool WriteLog)
{
	if (!dataAddr || !dataBytes || !dataSize)
	{
		return false;
	}

	// VirtualProtect first to make sure patch_address is readable
	DWORD dwPrevProtect;
	if (!VirtualProtect(dataAddr, dataSize, PAGE_READONLY, &dwPrevProtect))
	{
		Logging::Log() << __FUNCTION__ << " Error: could not read memory address";
		return false;
	}

	bool flag = (memcmp(dataAddr, dataBytes, dataSize) == 0);

	// Restore protection
	VirtualProtect(dataAddr, dataSize, dwPrevProtect, &dwPrevProtect);

	if (!flag && WriteLog)
	{
		Logging::Log() << __FUNCTION__ << " Error: memory address not found!";
	}

	// Return results
	return flag;
}

// Checks mulitple memory addresses
void *CheckMultiMemoryAddress(void *dataAddr10, void *dataAddr11, void *dataAddrDC, void *dataBytes, size_t dataSize)
{
	void *MemAddress = nullptr;
	// v1.0
	if (!MemAddress && (GameVersion == SH2V_10 || GameVersion == SH2V_UNKNOWN))
	{
		MemAddress = (CheckMemoryAddress(dataAddr10, dataBytes, dataSize)) ? dataAddr10 : nullptr;
	}
	// v1.1
	if (!MemAddress && (GameVersion == SH2V_11 || GameVersion == SH2V_UNKNOWN))
	{
		MemAddress = (CheckMemoryAddress(dataAddr11, dataBytes, dataSize)) ? dataAddr11 : nullptr;
	}
	// vDC
	if (!MemAddress && (GameVersion == SH2V_DC || GameVersion == SH2V_UNKNOWN))
	{
		MemAddress = (CheckMemoryAddress(dataAddrDC, dataBytes, dataSize)) ? dataAddrDC : nullptr;
	}
	// Return address
	return MemAddress;
}

// Search for memory addresses
DWORD SearchAndGetAddresses(DWORD dataAddr10, DWORD dataAddr11, DWORD dataAddrDC, const BYTE *dataBytes, size_t dataSize, int ByteDelta)
{
	// Get address
	DWORD MemoryAddr = (DWORD)CheckMultiMemoryAddress((void*)dataAddr10, (void*)dataAddr11, (void*)dataAddrDC, (void*)dataBytes, dataSize);

	// Search for address
	if (!MemoryAddr)
	{
		DWORD SearchAddr = (GameVersion == SH2V_10) ? dataAddr10 : (GameVersion == SH2V_11) ? dataAddr11 : (GameVersion == SH2V_DC) ? dataAddrDC : dataAddr10;
		MemoryAddr = (DWORD)GetAddressOfData(dataBytes, dataSize, 1, SearchAddr - 0x800, 2600);
		Logging::Log() << __FUNCTION__ << " searching for memory address! Found = " << (void*)MemoryAddr;
	}

	// Checking address pointer
	if (!MemoryAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
		return NULL;
	}
	MemoryAddr = MemoryAddr + ByteDelta;

	// Return address found
	return MemoryAddr;
}

// Search for memory addresses
DWORD ReadSearchedAddresses(DWORD dataAddr10, DWORD dataAddr11, DWORD dataAddrDC, const BYTE *dataBytes, size_t dataSize, int ByteDelta)
{
	// Search for address
	DWORD MemoryAddr = SearchAndGetAddresses(dataAddr10, dataAddr11, dataAddrDC, dataBytes, dataSize, ByteDelta);

	// If address exists then read memory and return address
	if (MemoryAddr)
	{
		DWORD Address;
		memcpy(&Address, (void*)MemoryAddr, sizeof(DWORD));
		return Address;
	}

	// Return NULL
	return NULL;
}

// Search and log address
void SearchAndLogAddress(DWORD FindAddress)
{
	void *Address = (void*)0x00410000;
	for (int x = -3; x < 4; x++)
	{
		do {
			DWORD SearchAddress = FindAddress + x;
			Address = GetAddressOfData(&SearchAddress, sizeof(DWORD), 1, (DWORD)Address, 0x005F0000 - (DWORD)Address);
			Logging::Log() << "Address found: " << Address;
		} while (Address);
	}
}

// Update memory
bool UpdateMemoryAddress(void *dataAddr, const void *dataBytes, size_t dataSize)
{
	if (!dataAddr || !dataBytes || !dataSize)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid memory data";
		return false;
	}

	// VirtualProtect first to make sure patch_address is readable
	DWORD dwPrevProtect;
	if (!VirtualProtect(dataAddr, dataSize, PAGE_READWRITE, &dwPrevProtect))
	{
		Logging::Log() << __FUNCTION__ << " Error: could not write to memory address";
		return false;
	}

	// Update memory
	memcpy(dataAddr, dataBytes, dataSize);

	// Restore protection
	VirtualProtect(dataAddr, dataSize, dwPrevProtect, &dwPrevProtect);

	// Flush cache
	FlushInstructionCache(GetCurrentProcess(), dataAddr, dataSize);

	// Return
	return true;
}

// Write a address to memory
bool WriteAddresstoMemory(BYTE *dataAddr, const void *JMPAddr, DWORD count, BYTE command)
{
	if (!dataAddr || !JMPAddr)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid memory data";
		return false;
	}

	if (count < 5)
	{
		Logging::Log() << __FUNCTION__ << " Error: invalid count";
		return false;
	}

	// VirtualProtect first to make sure patch_address is readable
	DWORD dwPrevProtect;
	if (!VirtualProtect(dataAddr, count, PAGE_READWRITE, &dwPrevProtect))
	{
		Logging::Log() << __FUNCTION__ << " Error: could not read memory address";
		return false; // access denied
	}

	// command (4-byte relative)
	*dataAddr = command;
	// relative jmp address
	*((DWORD*)(dataAddr + 1)) = (DWORD)JMPAddr - (DWORD)dataAddr - 5;

	for (DWORD x = 5; x < count; x++)
	{
		*((BYTE*)(dataAddr + x)) = 0x90;
	}

	// Restore protection
	VirtualProtect(dataAddr, count, dwPrevProtect, &dwPrevProtect);

	// Flush cache
	FlushInstructionCache(GetCurrentProcess(), dataAddr, count);

	// Return
	return true;
}

// Write a call to memory
bool WriteCalltoMemory(BYTE *dataAddr, const void *JMPAddr, DWORD count)
{
	// 0xE8 call (4-byte relative)
	return WriteAddresstoMemory(dataAddr, JMPAddr, count, 0xE8);
}

// Write a jmp to memory
bool WriteJMPtoMemory(BYTE *dataAddr, const void *JMPAddr, DWORD count)
{
	// 0xE9 jmp (4-byte relative)
	return WriteAddresstoMemory(dataAddr, JMPAddr, count, 0xE9);
}

// Replace memory
DWORD ReplaceMemoryBytes(void *dataSrc, void *dataDest, size_t size, DWORD start, DWORD distance, DWORD count)
{
	DWORD counter = 0;
	DWORD StartAddr = start;
	DWORD EndAddr = start + distance;

	// Update memory
	while (StartAddr < EndAddr)
	{
		// Get next address
		void *NextAddr = GetAddressOfData(dataSrc, size, 1, start, EndAddr - StartAddr);
		if (!NextAddr)
		{
			return counter;
		}
		StartAddr = (DWORD)NextAddr + size;

		// Write to memory
		UpdateMemoryAddress(NextAddr, dataDest, size);
		counter++;
		if (count && count == counter)
		{
			return counter;
		}
	}
	return counter;
}

// Set Single Core Affinity
void SetSingleCoreAffinity()
{
	Logging::Log() << "Setting SingleCoreAffinity...";
	DWORD_PTR ProcessAffinityMask, SystemAffinityMask;
	HANDLE hCurrentProcess = GetCurrentProcess();
	if (GetProcessAffinityMask(hCurrentProcess, &ProcessAffinityMask, &SystemAffinityMask))
	{
		DWORD_PTR AffinityMask = 1;
		while (AffinityMask && (AffinityMask & ProcessAffinityMask) == 0)
		{
			AffinityMask <<= 1;
		}
		if (AffinityMask)
		{
			SetProcessAffinityMask(hCurrentProcess, ((AffinityMask << (SingleCoreAffinity - 1)) & ProcessAffinityMask) ? (AffinityMask << (SingleCoreAffinity - 1)) : AffinityMask);
		}
	}
	CloseHandle(hCurrentProcess);
}

// Sets application DPI aware which disables DPI virtulization/High DPI scaling for this process
void SetDPIAware()
{
	Logging::Log() << "Disabling High DPI Scaling...";

	BOOL setDpiAware = FALSE;
	HMODULE hUser32 = LoadLibrary(L"user32.dll");
	HMODULE hShcore = LoadLibrary(L"shcore.dll");
	if (hUser32 && !setDpiAware)
	{
		SetProcessDpiAwarenessContextProc setProcessDpiAwarenessContext = (SetProcessDpiAwarenessContextProc)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");

		if (setProcessDpiAwarenessContext)
		{
			setDpiAware |= setProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
		}
	}
	if (hShcore && !setDpiAware)
	{
		SetProcessDpiAwarenessProc setProcessDpiAwareness = (SetProcessDpiAwarenessProc)GetProcAddress(hShcore, "SetProcessDpiAwareness");

		if (setProcessDpiAwareness)
		{
			setDpiAware |= SUCCEEDED(setProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE));
		}
	}
	if (hUser32 && !setDpiAware)
	{
		SetProcessDPIAwareProc setProcessDPIAware = (SetProcessDPIAwareProc)GetProcAddress(hUser32, "SetProcessDPIAware");

		if (setProcessDPIAware)
		{
			setDpiAware |= setProcessDPIAware();
		}
	}

	if (!setDpiAware)
	{
		Logging::Log() << "Failed to disable High DPI Scaling!";
	}
}

// Add HMODULE to vector
void AddHandleToVector(HMODULE dll)
{
	if (dll)
	{
		custom_dll.push_back(dll);
	}
}

// Unload standard modules
void UnloadAllModules()
{
	for (HMODULE it : custom_dll)
	{
		if (it)
		{
			FreeLibrary(it);
		}
	}
}

DWORD ConvertFloat(float num)
{
	return *((DWORD*)&num);
}

void GetFileDate(std::filesystem::file_time_type filetime, char *strOutput, size_t size)
{
	if (!strOutput)
	{
		return;
	}

	time_t rawtime = std::chrono::duration_cast<std::chrono::seconds>(filetime.time_since_epoch()).count() - 11644473600;
	struct tm ts;

	// Format time, "mm/dd/yyyy hh:mm AM"
	if (localtime_s(&ts, &rawtime) == 0)
	{
		strftime(strOutput, size, "%m/%d/%Y  %I:%M %p", &ts);
	}
}

void GetFileSize(uintmax_t fsize, char *strOutput, size_t size)
{
	if (!strOutput)
	{
		return;
	}

	// Get file size
	std::string strSize;
	for (int x = 1; x < 18; x++)
	{
		if (fsize && x % 4 == 0)
		{
			strSize.insert(0, ",");
		}
		else if (!fsize)
		{
			strSize.insert(0, " ");
		}
		else
		{
			char num[2] = { '\0' };
			num[0] = (fsize % 10) + 48;
			strSize.insert(0, num);
			fsize /= 10;
		}
	}
	strcpy_s(strOutput, size, strSize.c_str());
}

void LogDirectory()
{
	wchar_t path[MAX_PATH];
	GetModuleFileName(nullptr, path, MAX_PATH);
	wcscpy_s(wcsrchr(path, '\\'), MAX_PATH - wcslen(path), L"\0");

	Logging::Log() << "|- ";
	Logging::Log() << "|-  Directory of " << path;
	Logging::Log() << "|- ";

	// Directories
	for (const auto & entry : std::filesystem::directory_iterator(path))
	{
		if (entry.is_directory())
		{
			// Get file date
			char filetime[80] = { "\0" };
			GetFileDate(entry.last_write_time(), filetime, sizeof(filetime));

			CStringA filename(entry.path().filename().c_str());

			Logging::Log() << "|- " << filetime << "    <DIR>          " << filename;
		}
	}
	// Files
	for (const auto & entry : std::filesystem::directory_iterator(path))
	{
		if (!entry.is_directory())
		{
			// Get file date
			char filetime[80] = { "\0" };
			GetFileDate(entry.last_write_time(), filetime, sizeof(filetime));

			// Get file size
			char filesize[80] = { "\0" };
			GetFileSize(entry.file_size(), filesize, sizeof(filesize));

			// Get file name
			CStringA filename(entry.path().filename().c_str());

			Logging::Log() << "|- " << filetime << " " << filesize << " " << filename;
		}
	}

	Logging::Log() << "|--------------------------------";
}

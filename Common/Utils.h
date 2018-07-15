#pragma once

#include <vector>

void *GetAddressOfData(const void *data, size_t len, DWORD step = 1);
void *GetAddressOfData(const void *data, size_t len, DWORD step, DWORD start, DWORD distance = 0x0FFFFFFF);
bool CheckMemoryAddress(void *dataAddr, void *dataBytes, DWORD dataSize);
bool UpdateMemoryAddress(void *dataAddr, void *dataBytes, DWORD dataSize);
bool WriteJMPtoMemory(BYTE *dataAddr, void *JMPAddr, DWORD count = 5);
bool ReplaceMemoryBytes(void *dataSrc, void *dataDest, size_t size, DWORD start, DWORD distance, DWORD count = 0);
void AddHandleToVector(HMODULE dll);

extern std::vector<HMODULE> custom_dll;		// Used for custom dll's and asi plugins

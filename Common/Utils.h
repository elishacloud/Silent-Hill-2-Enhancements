#pragma once

#include <string>
#include <vector>
#include <Shldisp.h>
#include "External\MemoryModule\MemoryModule.h"

void *GetAddressOfData(const void *data, size_t len, DWORD step = 1);
void *GetAddressOfData(const void *data, size_t len, DWORD step, DWORD start, DWORD distance = 0x0FFFFFFF);
bool CheckMemoryAddress(void *dataAddr, void *dataBytes, size_t dataSize, bool WriteLog = true);
void *CheckMultiMemoryAddress(void *dataAddr10, void *dataAddr11, void *dataAddrDC, void *dataBytes, size_t dataSize);
DWORD SearchAndGetAddresses(DWORD dataAddr10, DWORD dataAddr11, DWORD dataAddrDC, const BYTE *dataBytes, size_t dataSize, int ByteDelta);
DWORD ReadSearchedAddresses(DWORD dataAddr10, DWORD dataAddr11, DWORD dataAddrDC, const BYTE *dataBytes, size_t dataSize, int ByteDelta);
void SearchAndLogAddress(DWORD SearchAddress);
bool UpdateMemoryAddress(void *dataAddr, const void *dataBytes, size_t dataSize);
bool WriteCalltoMemory(BYTE *dataAddr, const void *JMPAddr, DWORD count = 5);
bool WriteJMPtoMemory(BYTE *dataAddr, const void *JMPAddr, DWORD count = 5);
DWORD ReplaceMemoryBytes(void *dataSrc, void *dataDest, size_t size, DWORD start, DWORD distance, DWORD count = 0);
void LogAffinity();
DWORD_PTR GetProcessMask();
void SetSingleCoreAffinity();
void SetMultiCoreAffinity();
void SetDPIAware();
HMODULE SetAppTheme();
void SetWindowTheme(HWND hWnd);
void AddHandleToVector(HMODULE dll);
void UnloadAllModules();
DWORD ConvertFloat(float num);
HMEMORYMODULE LoadResourceToMemory(DWORD ResID);
void ExtractFileFromResource(DWORD ResID, char* lpFilepath);
void ExtractFileFromResource(DWORD ResID, wchar_t* lpFilepath);
HRESULT UnZipFile(BSTR sourceZip, BSTR destFolder);
bool GetModulePath(char *path, rsize_t size);
bool GetModulePath(wchar_t *path, rsize_t size);
bool GetSH2FolderPath(char *path, rsize_t size);
bool GetSH2FolderPath(wchar_t *path, rsize_t size);
bool GetConfigName(char* ConfigName, rsize_t size, char* ext);
bool GetConfigName(wchar_t* ConfigName, rsize_t size, wchar_t* ext);
bool CheckPathNameMatch(LPCSTR lpFileName1, LPCSTR lpFileName2);
void CopyReplaceSlash(char* DestStr, size_t Size, LPCSTR SrcStr);
BOOL GetAppsLightMode();
HMONITOR GetMonitorHandle();
void GetDesktopRes(LONG &screenWidth, LONG &screenHeight);
void GetDesktopRect(RECT &screenRect);
bool ReadRegistryStruct(const std::wstring& lpzSection, const std::wstring& lpzKey, const LPVOID& lpStruct, UINT uSizeStruct);
bool WriteRegistryStruct(const std::wstring& lpzSection, const std::wstring& lpzKey, DWORD dwType, const LPVOID lpStruct, UINT uSizeStruct);
HRESULT GetSavedResolution(DWORD &Width, DWORD &Height);
HRESULT SaveResolution(DWORD Width, DWORD Height);
void LogDirectory();
void LogAllModules();
void RunDelayedOneTimeItems();

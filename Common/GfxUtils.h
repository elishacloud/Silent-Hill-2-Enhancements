// added by iOrange in 2024

#pragma once

#include "Wrappers\d3d8\d3d8wrapper.h"

// flags for the GfxCreateTextureFromFile
#define GCTFF_BUILD_MIPS    0x00000001  // will generate mips if not present in the file

// a replacement for the old and ugly D3DXCreateTextureFromFile
HRESULT GfxCreateTextureFromFileA(LPDIRECT3DDEVICE8 device, LPCSTR srcFile, LPDIRECT3DTEXTURE8* dstTexture, DWORD flags);
HRESULT GfxCreateTextureFromFileW(LPDIRECT3DDEVICE8 device, LPCWSTR srcFile, LPDIRECT3DTEXTURE8* dstTexture, DWORD flags);

HRESULT GfxCreateTextureFromFileInMem(LPDIRECT3DDEVICE8 device, void* fileMem, DWORD fileSize, LPDIRECT3DTEXTURE8* dstTexture);

#ifdef UNICODE
#define GfxCreateTextureFromFile GfxCreateTextureFromFileW
#else
#define GfxCreateTextureFromFile GfxCreateTextureFromFileA
#endif

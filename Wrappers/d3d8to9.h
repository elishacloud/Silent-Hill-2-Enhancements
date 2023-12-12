#pragma once

#include "External\d3d8to9\source\d3d8to9.hpp"

typedef IDirect3D8 *(WINAPI *Direct3DCreate8Proc)(UINT);

void EnableD3d8to9();
HRESULT WINAPI d8_ValidatePixelShader(DWORD* pixelshader, DWORD* reserved1, BOOL flag, DWORD* toto);
HRESULT WINAPI d8_ValidateVertexShader(DWORD* vertexshader, DWORD* reserved1, DWORD* reserved2, BOOL flag, DWORD* toto);
Direct3D8 *WINAPI Direct3DCreate8to9(UINT SDKVersion);

extern bool IsUsingD3d8to9;
extern Direct3DCreate8Proc m_pDirect3DCreate8;

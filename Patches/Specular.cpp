#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Patches.h"
#include "Common\Utils.h"
#include "Wrappers\d3d8\DirectX81SDK\include\d3d8.h"

struct LightSource
{
	D3DLIGHT8 light;
	float extraFloats[12]; // Purposes unknown
};

static LightSource fakeLight = {{D3DLIGHT_DIRECTIONAL}};
static bool useFakeLight = false;

static auto getLightSourceCount_50C590 = *reinterpret_cast<int(__cdecl*)()>(0x50C590);
int __cdecl Part1()
{
	// This function replaces a call to `int getLightSourceCount_50C590()`
	// When no light sources are present and the function would normally return 0, we set our boolean and return 1 instead

	useFakeLight = false;
	int lightSourceCount = getLightSourceCount_50C590();

	if (lightSourceCount == 0)
	{
		useFakeLight = true;
		return 1;
	}

	return lightSourceCount;
}

static auto getLightSourceStruct_50C5A0 = *reinterpret_cast<LightSource* (__cdecl*)(int)>(0x50C5A0);
LightSource* __cdecl Part2(int index)
{
	// This function replaces a call to `LightSourceStruct* getLightSourceStruct_50C5A0(index)`
	// When no light sources are present we substitute a minimal one of our own

	if (useFakeLight)
		return &fakeLight;
	else
		return getLightSourceStruct_50C5A0(index);
}

static auto& pD3DDevice_A32894 = *reinterpret_cast<IDirect3DDevice8**>(0xA32894);
HRESULT __stdcall Part3(IDirect3DDevice8* /*This*/, DWORD Register, void* pConstantData, DWORD ConstantCount)
{
	// This function replaces a call to `HRESULT pD3DDevice_A32894->SetPixelShaderConstant(Register, pConstantData, ConstantCount)`
	// When using our fake light source we adjust the intensity of the specular highlights to 25%

	if (useFakeLight)
	{
		// If these values are all 0.0 then material type was not 4 (glossy), but multiplying by 0 has no affect anyway
		((float*)pConstantData)[0] *= 0.25f; // r
		((float*)pConstantData)[1] *= 0.25f; // g
		((float*)pConstantData)[2] *= 0.25f; // b
	  //((float*)pConstantData)[3] *= 1.00f; // a (unused)
	}

	return pD3DDevice_A32894->SetPixelShaderConstant(Register, pConstantData, ConstantCount);
}

void PatchSpecular()
{
	WriteCalltoMemory((BYTE*)0x4FECD0, Part1, 5);
	WriteCalltoMemory((BYTE*)0x4FED28, Part2, 5);
	WriteCalltoMemory((BYTE*)0x501E1B, Part3, 6);
}

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Patches.h"
#include "Common\Utils.h"
#include "Wrappers\d3d8\DirectX81SDK\include\d3d8.h"

enum class ModelId
{
	James = 257,
	MariaSH2 = 262,
	MariaBFAW = 270,
	MariaRevolver = 2057,
};

struct LightSource
{
	D3DLIGHT8 light;
	float extraFloats[12]; // Purposes unknown
};

static LightSource fakeLight = { {D3DLIGHT_DIRECTIONAL} };
static bool useFakeLight = false;
static bool inSpecialLightZone = false;
static int fakeLightIndex = -1;

static auto getModelId_50B6C0 = reinterpret_cast<ModelId(__cdecl*)()>(0x50B6C0);
static auto getLightSourceCount_50C590 = *reinterpret_cast<int(__cdecl*)()>(0x50C590);
static auto getLightSourceStruct_50C5A0 = *reinterpret_cast<LightSource * (__cdecl*)(int)>(0x50C5A0);
static auto& pD3DDevice_A32894 = *reinterpret_cast<IDirect3DDevice8**>(0xA32894);

int __cdecl Part1()
{
	// This function replaces a call to `int getLightSourceCount_50C590()`
	// When no light sources are present and the function would normally return 0, we set our boolean and return 1 instead

	inSpecialLightZone = false;
	useFakeLight = true;
	fakeLightIndex = -1;

	int lightSourceCount = getLightSourceCount_50C590();

	for (int i = 0; lightSourceCount > 0 && i < lightSourceCount; i++)
	{
		auto pLight = getLightSourceStruct_50C5A0(i);
		if (pLight->light.Type == D3DLIGHT_DIRECTIONAL)
			useFakeLight = false;

		if (pLight->light.Type == D3DLIGHT_SPOT)
			inSpecialLightZone = true;
	}

	if (useFakeLight)
	{
		fakeLightIndex = lightSourceCount;
		return lightSourceCount + 1;
	}

	return lightSourceCount;
}

LightSource* __cdecl Part2(int index)
{
	// This function replaces a call to `LightSourceStruct* getLightSourceStruct_50C5A0(index)`
	// When no light sources are present we substitute a minimal one of our own

	if (useFakeLight && index == fakeLightIndex)
		return &fakeLight;
	else
		return getLightSourceStruct_50C5A0(index);
}

HRESULT __stdcall Part3(IDirect3DDevice8* /*This*/, DWORD Register, void* pConstantData, DWORD ConstantCount)
{
	// This function replaces a call to `HRESULT pD3DDevice_A32894->SetPixelShaderConstant(Register, pConstantData, ConstantCount)`
	// When using our fake light source we adjust the intensity of the specular highlights to 25%

	auto constants = reinterpret_cast<float*>(pConstantData);
	if (constants[0] != 0.0f || constants[1] != 0.0f || constants[2] != 0.0f)
	{
		// 75% if we're James and in a special lighting zone
		if (getModelId_50B6C0() == ModelId::James && inSpecialLightZone)
		{
			constants[0] = 0.75f;
			constants[1] = 0.75f;
			constants[2] = 0.75f;
			return pD3DDevice_A32894->SetPixelShaderConstant(Register, pConstantData, ConstantCount);
		}

		// 50% If we're not James and we're in a special lighting zone
		if (getModelId_50B6C0() != ModelId::James && inSpecialLightZone)
		{
			constants[0] = 0.50f;
			constants[1] = 0.50f;
			constants[2] = 0.50f;
			return pD3DDevice_A32894->SetPixelShaderConstant(Register, pConstantData, ConstantCount);
		}

		// 50% If we're not James and the flashlight is on
		if (getModelId_50B6C0() != ModelId::James && !inSpecialLightZone && !useFakeLight)
		{
			constants[0] = 0.50f;
			constants[1] = 0.50f;
			constants[2] = 0.50f;
			return pD3DDevice_A32894->SetPixelShaderConstant(Register, pConstantData, ConstantCount);
		}

		// Default to 25% specularity
		constants[0] = 0.25f;
		constants[1] = 0.25f;
		constants[2] = 0.25f;
		return pD3DDevice_A32894->SetPixelShaderConstant(Register, pConstantData, ConstantCount);
	}

	// Material is not glossy, don't bother
	return pD3DDevice_A32894->SetPixelShaderConstant(Register, pConstantData, ConstantCount);
}

void PatchSpecular()
{
	WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4FECD0), Part1, 5);
	WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4FED28), Part2, 5);
	WriteCalltoMemory(reinterpret_cast<BYTE*>(0x501E1B), Part3, 6);
}

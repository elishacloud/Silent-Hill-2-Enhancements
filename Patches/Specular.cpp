#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Patches.h"
#include "Common\Utils.h"
#include "Wrappers\d3d8\DirectX81SDK\include\d3d8.h"

enum class ModelId
{
	hhh_mar = 262,
	//hhh_mar_notex = 262,
	lll_mar = 261,
	rhhh_mar = 294,
	//rhhh_mar_notex = 294,
	rlll_mar = 293,
	lxx_mar = 270,
	rlxx_mar = 302,
	//rlxx_mar_notex = 302,

	hhh_jms = 259,
	//hhh_jms_notex = 259,
	hhl_jms = 258,
	//hhl_jms_notex = 258,
	hll_jms = 257,
	//hll_jms_notex = 257,
	lll_jms = 256,
	//lll_jms_hand = 256,
	//lll_jms_kaku = 256,
	//lll_jms_n = 256,
	//lll_jms_normal = 256,
	//lll_jms_notex = 256,
	rhhh_jms = 291,
	rhhl_jms = 290,
	rhll_jms = 289,
	rlll_jms = 288
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

bool isJames(ModelId id)
{
	switch (id)
	{
	case ModelId::hhh_jms:
	case ModelId::hhl_jms:
	case ModelId::hll_jms:
	case ModelId::lll_jms:
	case ModelId::rhhh_jms:
	case ModelId::rhhl_jms:
	case ModelId::rhll_jms:
	case ModelId::rlll_jms:
		return true;
	}

	return false;
}

bool isMaria(ModelId id)
{
	switch (id)
	{
	case ModelId::hhh_mar:
	case ModelId::lll_mar:
	case ModelId::rhhh_mar:
	case ModelId::rlll_mar:
	case ModelId::lxx_mar:
	case ModelId::rlxx_mar:
		return true;
	}

	return false;
}

int __cdecl Part1()
{
	// This function replaces a call to `int getLightSourceCount_50C590()`
	// When no D3D_DIRECTIONAL light sources exist, set our booleans and return 1 greater than reality

	inSpecialLightZone = false;
	useFakeLight = true;
	fakeLightIndex = -1;

	int lightSourceCount = getLightSourceCount_50C590();

	for (int i = 0; i < lightSourceCount; i++)
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
	// When we hit the index 1 greater than the real light source count, return our fake

	if (useFakeLight && index == fakeLightIndex)
		return &fakeLight;
	else
		return getLightSourceStruct_50C5A0(index);
}

HRESULT __stdcall Part3(IDirect3DDevice8* /*This*/, DWORD Register, void* pConstantData, DWORD ConstantCount)
{
	// This function replaces a call to `HRESULT pD3DDevice_A32894->SetPixelShaderConstant(Register, pConstantData, ConstantCount)`
	// Adjust opacity depending on the situation and model

	auto constants = reinterpret_cast<float*>(pConstantData);
	if (constants[0] != 0.0f || constants[1] != 0.0f || constants[2] != 0.0f)
	{
		ModelId modelId = getModelId_50B6C0();

		if (isJames(modelId) && inSpecialLightZone)
		{
			// 75% if we're James and in a special lighting zone
			constants[0] = 0.75f;
			constants[1] = 0.75f;
			constants[2] = 0.75f;
		}
		else if (!isJames(modelId) && !isMaria(modelId) && inSpecialLightZone)
		{
			// 50% If we're not James or Maria and we're in a special lighting zone
			constants[0] = 0.50f;
			constants[1] = 0.50f;
			constants[2] = 0.50f;
		}
		else if (!isJames(modelId) && !isMaria(modelId) && !inSpecialLightZone && !useFakeLight)
		{
			// 50% If we're not James or Maria and the flashlight is on
			constants[0] = 0.50f;
			constants[1] = 0.50f;
			constants[2] = 0.50f;
		}
		else if (isMaria(modelId) && !inSpecialLightZone && useFakeLight)
		{
			// 10% If we're Maria and the flashlight is off
			constants[0] = 0.10f;
			constants[1] = 0.10f;
			constants[2] = 0.10f;
		}
		else
		{
			// Default to 25% specularity
			constants[0] = 0.25f;
			constants[1] = 0.25f;
			constants[2] = 0.25f;
		}
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

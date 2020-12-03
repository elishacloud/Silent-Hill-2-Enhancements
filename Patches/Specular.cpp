#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Patches.h"
#include "Common\Utils.h"
#include "Wrappers\d3d8\DirectX81SDK\include\d3d8.h"
#include "ModelIds.h"

struct LightSource
{
	D3DLIGHT8 light;
	float extraFloats[12]; // Purposes unknown
};

struct ModelOffsetTable
{
	int field_0;
	int field_4;
	int skeleton_points_offset;
	int skeleton_point_count;
	int skeleton_index_buffer_part_1_offset;
	int field_14;
	int skeleton_index_buffer_part_2_offset;
	int field_1C;
	int materialCount;
	unsigned int materialsOffset;
	int field_2C;
	unsigned int offset_30;
	int field_34;
	unsigned int offset_38;
	int field_3C;
	unsigned int offset_40;
	int field_44;
	int field_48;
	int field_4C;
	int field_50;
};

struct ModelMaterial
{
	int materialLength;
	int reserved0;
	int unkU16Count0;
	unsigned int unkU16Array0Offset;
	int unkU16Count1;
	unsigned int unkU16Array1Offset;
	int unkU16Count2;
	unsigned int unkU16Array2Offset;
	unsigned int samplerStatesOffset;
	char materialType;
	char unkMaterialSubtype;
	char poseId;
	char unkByte0x27;
	int cullMode;
	float unkDiffuseFloat;
	float unkAmbientFloat;
	float specularHighlightScale;
	void* reserved1;
	int reserved2;
	float diffuseX;
	float diffuseR;
	float diffuseG;
	float diffuseB;
	float ambientX;
	float ambientR;
	float ambientG;
	float ambientB;
	float specularX;
	float specularR;
	float specularG;
	float specularB;
	int reserved3;
	int unkIndex;
	int primCount;
	int reserved4;
};

static LightSource fakeLight = { {D3DLIGHT_DIRECTIONAL} };
static bool useFakeLight = false;
static bool inSpecialLightZone = false;
static int fakeLightIndex = -1;
static int materialCount = 0;
static ModelMaterial* pFirstMaterial = nullptr;
static ModelMaterial* pCurrentMaterial = nullptr;

static auto GetModelId_50B6C0 = reinterpret_cast<ModelId(__cdecl*)()>(0x50B6C0);
static auto GetLightSourceCount_50C590 = *reinterpret_cast<int(__cdecl*)()>(0x50C590);
static auto GetLightSourceStruct_50C5A0 = *reinterpret_cast<LightSource * (__cdecl*)(int)>(0x50C5A0);
static auto& pD3DDevice_A32894 = *reinterpret_cast<IDirect3DDevice8**>(0xA32894);
static auto ActorDrawOpaque_501540 = reinterpret_cast<void(__cdecl*)(ModelMaterial*)>(0x501540);
static auto DoActorOpaqueStuff_501F90 = reinterpret_cast<void(__cdecl*)(ModelOffsetTable*, void*)>(0x501F90);

int GetCurrentMaterialIndex()
{
	int index = 0;
	ModelMaterial* pCursor = pFirstMaterial;

	while (index < materialCount)
	{
		if (pCurrentMaterial == pCursor)
			return index;

		pCursor = reinterpret_cast<ModelMaterial*>((reinterpret_cast<char*>(pCursor) + pCursor->materialLength));
		index++;
	}
}

bool IsJames(ModelId id)
{
	switch (id)
	{
	case ModelId::chr_jms_lll_jms:
	case ModelId::chr_jms_hll_jms:
	case ModelId::chr_jms_hhl_jms:
	case ModelId::chr_jms_hhh_jms:
	case ModelId::chr_jms_rlll_jms:
	case ModelId::chr_jms_rhll_jms:
	case ModelId::chr_jms_rhhl_jms:
	case ModelId::chr_jms_rhhh_jms:
		return true;
	}

	return false;
}

bool IsMariaExcludingEyes(ModelId id)
{
	switch (id)
	{
	case ModelId::chr_mar_lll_mar:
	case ModelId::chr_mar_hhh_mar:
	case ModelId::chr2_mar_lxx_mar:
	case ModelId::chr_mar_rlll_mar:
	case ModelId::chr_mar_rhhh_mar:
	case ModelId::chr2_mar_rlxx_mar:
		if (GetCurrentMaterialIndex() != 3) return true;
		break;
	case ModelId::chr_item_dmr:
		if (GetCurrentMaterialIndex() != 1) return true;
		break;
	}

	return false;
}

bool IsMariaEyes(ModelId id)
{
	switch (id)
	{
	case ModelId::chr_mar_lll_mar:
	case ModelId::chr_mar_hhh_mar:
	case ModelId::chr2_mar_lxx_mar:
	case ModelId::chr_mar_rlll_mar:
	case ModelId::chr_mar_rhhh_mar:
	case ModelId::chr2_mar_rlxx_mar:
		if (GetCurrentMaterialIndex() == 3) return true;
		break;
	case ModelId::chr_item_dmr:
		if (GetCurrentMaterialIndex() == 1) return true;
		break;
	}

	return false;
}

void __cdecl Part0(ModelOffsetTable* pOffsetTable, void* arg2)
{
	// This function replaces a call to `void DoActorOpaqueStuff_501F90(ModelOffsetTable* pOffsetTable, void* arg2)`
	// Backup the materialCount and pointer to first material for use in later Parts

	materialCount = pOffsetTable->materialCount;
	pFirstMaterial = (ModelMaterial*)((char*)pOffsetTable + pOffsetTable->materialsOffset);

	DoActorOpaqueStuff_501F90(pOffsetTable, arg2);
}

int __cdecl Part1()
{
	// This function replaces a call to `int GetLightSourceCount_50C590()`
	// When no D3D_DIRECTIONAL light sources exist, set our booleans and return 1 greater than reality

	inSpecialLightZone = false;
	useFakeLight = true;
	fakeLightIndex = -1;
	pCurrentMaterial = nullptr;

	int lightSourceCount = GetLightSourceCount_50C590();

	for (int i = 0; i < lightSourceCount; i++)
	{
		auto pLight = GetLightSourceStruct_50C5A0(i);
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
	// This function replaces a call to `LightSourceStruct* GetLightSourceStruct_50C5A0(int index)`
	// When we hit the index 1 greater than the real light source count, return our fake

	if (useFakeLight && index == fakeLightIndex)
		return &fakeLight;
	else
		return GetLightSourceStruct_50C5A0(index);
}

void Part3(ModelMaterial* pModelMaterial)
{
	// This function replaces a call to `void ActorDrawOpaque_501540(ModelMaterial* pModelMaterial)`
	// We copy off the pointer to a static variable, so we can reference it in Part 4

	pCurrentMaterial = pModelMaterial;
	ActorDrawOpaque_501540(pModelMaterial);
}

HRESULT __stdcall Part4(IDirect3DDevice8* /*This*/, DWORD Register, void* pConstantData, DWORD ConstantCount)
{
	// This function replaces a call to `HRESULT pD3DDevice_A32894->SetPixelShaderConstant(DWORD Register, void* pConstantData, DWORD ConstantCount)`
	// Adjust opacity depending on the situation and model

	auto constants = reinterpret_cast<float*>(pConstantData);
	if (constants[0] != 0.0f || constants[1] != 0.0f || constants[2] != 0.0f)
	{
		ModelId modelId = GetModelId_50B6C0();

		if (IsJames(modelId)) // James
		{
			if (inSpecialLightZone)
			{
				// 75% if in a special lighting zone or not in cutscene 0 and flashlight is on
				constants[0] = 0.75f;
				constants[1] = 0.75f;
				constants[2] = 0.75f;
			}
			else
			{
				// Default to 25% specularity
				constants[0] = 0.25f;
				constants[1] = 0.25f;
				constants[2] = 0.25f;
			}
		}
		else if (IsMariaExcludingEyes(modelId)) // Maria, but not her eyes
		{
			if (!useFakeLight || inSpecialLightZone)
			{
				// 20% If in a special lighting zone and/or flashlight is on
				constants[0] = 0.20f;
				constants[1] = 0.20f;
				constants[2] = 0.20f;
			}
			else
			{
				// Default to 5% specularity
				constants[0] = 0.05f;
				constants[1] = 0.05f;
				constants[2] = 0.05f;
			}
		}
		else if (IsMariaEyes(modelId)) // Maria's Eyes
		{
			// 50% specularity
			constants[0] = 0.50f;
			constants[1] = 0.50f;
			constants[2] = 0.50f;
		}
		else if (modelId == ModelId::chr_bos_bos) // Final boss
		{
			// 25% specularity
			constants[0] = 0.25f;
			constants[1] = 0.25f;
			constants[2] = 0.25f;
		}
		else if ((modelId == ModelId::chr_agl_agl || modelId == ModelId::chr_agl_ragl) && GetCurrentMaterialIndex() == 3) // Angela's eyes
		{
			if (useFakeLight && !inSpecialLightZone && GetCutsceneID() != 0x53)
			{
				// 25% specularity if flashlight is off and not in special light zone or in cutscene 0x53
				constants[0] = 0.25f;
				constants[1] = 0.25f;
				constants[2] = 0.25f;
			}
			else
			{
				// Default to 50% specularity
				constants[0] = 0.50f;
				constants[1] = 0.50f;
				constants[2] = 0.50f;
			}
		}
		else if (modelId == ModelId::chr_mry_mry) // Mary (Healthy)
		{
			// 50% specularity
			constants[0] = 0.50f;
			constants[1] = 0.50f;
			constants[2] = 0.50f;
		}
		else // Everything else
		{
			if (!useFakeLight || inSpecialLightZone)
			{
				// 40% If in a special lighting zone and/or flashlight is on
				constants[0] = 0.40f;
				constants[1] = 0.40f;
				constants[2] = 0.40f;
			}
			else
			{
				// Default to 15% specularity
				constants[0] = 0.15f;
				constants[1] = 0.15f;
				constants[2] = 0.15f;
			}
		}
	}

	return pD3DDevice_A32894->SetPixelShaderConstant(Register, pConstantData, ConstantCount);
}

void PatchSpecular()
{
	WriteCalltoMemory(reinterpret_cast<BYTE*>(0x50EB2B), Part0, 5);
	WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4FECD0), Part1, 5);
	WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4FED28), Part2, 5);
	WriteCalltoMemory(reinterpret_cast<BYTE*>(0x501F77), Part3, 5);
	WriteCalltoMemory(reinterpret_cast<BYTE*>(0x501E1B), Part4, 6);
}

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Patches.h"
#include "Common\Utils.h"
#include "Wrappers\d3d8\DirectX81SDK\include\d3d8.h"
#include "ModelID.h"

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

ModelID(__cdecl* GetModelID)() = nullptr;
static int(__cdecl* GetLightSourceCount)() = nullptr;
static LightSource* (__cdecl* GetLightSourceStruct)(int) = nullptr;
static void(__cdecl* ActorDrawOpaque)(ModelMaterial*) = nullptr;
static void(__cdecl* DoActorOpaqueStuff)(ModelOffsetTable*, void*) = nullptr;
static IDirect3DDevice8** pD3DDevice;

static int GetCurrentMaterialIndex()
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

	return -1;
}

static bool IsJames(ModelID id)
{
	switch (id)
	{
	case ModelID::chr_jms_lll_jms:
	case ModelID::chr_jms_hll_jms:
	case ModelID::chr_jms_hhl_jms:
	case ModelID::chr_jms_hhh_jms:
	case ModelID::chr_jms_rlll_jms:
	case ModelID::chr_jms_rhll_jms:
	case ModelID::chr_jms_rhhl_jms:
	case ModelID::chr_jms_rhhh_jms:
		return true;
	}

	return false;
}

static bool IsMariaExcludingEyes(ModelID id)
{
	switch (id)
	{
	case ModelID::chr_mar_lll_mar:
	case ModelID::chr_mar_hhh_mar:
	case ModelID::chr2_mar_lxx_mar:
	case ModelID::chr_mar_rlll_mar:
	case ModelID::chr_mar_rhhh_mar:
	case ModelID::chr2_mar_rlxx_mar:
		if (GetCurrentMaterialIndex() != 3) return true;
		break;
	case ModelID::chr_item_dmr:
		if (GetCurrentMaterialIndex() != 1) return true;
		break;
	}

	return false;
}

static bool IsMariaEyes(ModelID id)
{
	switch (id)
	{
	case ModelID::chr_mar_lll_mar:
	case ModelID::chr_mar_hhh_mar:
	case ModelID::chr2_mar_lxx_mar:
	case ModelID::chr_mar_rlll_mar:
	case ModelID::chr_mar_rhhh_mar:
	case ModelID::chr2_mar_rlxx_mar:
		if (GetCurrentMaterialIndex() == 3) return true;
		break;
	case ModelID::chr_item_dmr:
		if (GetCurrentMaterialIndex() == 1) return true;
		break;
	}

	return false;
}

static void __cdecl Part0(ModelOffsetTable* pOffsetTable, void* arg2)
{
	// This function replaces a call to `void DoActorOpaqueStuff(ModelOffsetTable* pOffsetTable, void* arg2)`
	// Backup the materialCount and pointer to first material for use in later Parts

	materialCount = pOffsetTable->materialCount;
	pFirstMaterial = (ModelMaterial*)((char*)pOffsetTable + pOffsetTable->materialsOffset);

	DoActorOpaqueStuff(pOffsetTable, arg2);
}

static int __cdecl Part1()
{
	// This function replaces a call to `int GetLightSourceCount()`
	// When no D3D_DIRECTIONAL light sources exist, set our booleans and return 1 greater than reality

	inSpecialLightZone = false;
	useFakeLight = true;
	fakeLightIndex = -1;
	pCurrentMaterial = nullptr;

	int lightSourceCount = GetLightSourceCount();

	for (int i = 0; i < lightSourceCount; i++)
	{
		auto pLight = GetLightSourceStruct(i);
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

static LightSource* __cdecl Part2(int index)
{
	// This function replaces a call to `LightSourceStruct* GetLightSourceStruct(int index)`
	// When we hit the index 1 greater than the real light source count, return our fake

	if (useFakeLight && index == fakeLightIndex)
		return &fakeLight;
	else
		return GetLightSourceStruct(index);
}

static void Part3(ModelMaterial* pModelMaterial)
{
	// This function replaces a call to `void ActorDrawOpaque(ModelMaterial* pModelMaterial)`
	// We copy off the pointer to a static variable, so we can reference it in Part 4

	pCurrentMaterial = pModelMaterial;
	ActorDrawOpaque(pModelMaterial);
}

static HRESULT __stdcall Part4(IDirect3DDevice8* /*This*/, DWORD Register, void* pConstantData, DWORD ConstantCount)
{
	// This function replaces a call to `HRESULT pD3DDevice->SetPixelShaderConstant(DWORD Register, void* pConstantData, DWORD ConstantCount)`
	// Adjust opacity depending on the situation and model

	auto constants = reinterpret_cast<float*>(pConstantData);
	if (constants[0] != 0.0f || constants[1] != 0.0f || constants[2] != 0.0f)
	{
		ModelID modelID = GetModelID();

		if (IsJames(modelID)) // James
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
		else if (IsMariaExcludingEyes(modelID)) // Maria, but not her eyes
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
		else if (IsMariaEyes(modelID)) // Maria's Eyes
		{
			// 50% specularity
			constants[0] = 0.50f;
			constants[1] = 0.50f;
			constants[2] = 0.50f;
		}
		else if (modelID == ModelID::chr_bos_bos) // Final boss
		{
			// 25% specularity
			constants[0] = 0.25f;
			constants[1] = 0.25f;
			constants[2] = 0.25f;
		}
		else if ((modelID == ModelID::chr_agl_agl || modelID == ModelID::chr_agl_ragl) && GetCurrentMaterialIndex() == 3) // Angela's eyes
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
		else if (modelID == ModelID::chr_mry_mry) // Mary (Healthy)
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

	return (*pD3DDevice)->SetPixelShaderConstant(Register, pConstantData, ConstantCount);
}

void FindGetModelID()
{
	switch (GameVersion)
	{
	case SH2V_10:
		GetModelID = reinterpret_cast<decltype(GetModelID)>(0x50B6C0);
		break;
	case SH2V_11:
		GetModelID = reinterpret_cast<decltype(GetModelID)>(0x50B9F0);
		break;
	case SH2V_DC:
		GetModelID = reinterpret_cast<decltype(GetModelID)>(0x50B310);
		break;
	}
}

void PatchSpecular()
{
	switch (GameVersion)
	{
	case SH2V_10:
		GetLightSourceCount = *reinterpret_cast<int(__cdecl*)()>(0x50C590);
		GetLightSourceStruct = *reinterpret_cast<LightSource * (__cdecl*)(int)>(0x50C5A0);
		ActorDrawOpaque = reinterpret_cast<void(__cdecl*)(ModelMaterial*)>(0x501540);
		DoActorOpaqueStuff = reinterpret_cast<void(__cdecl*)(ModelOffsetTable*, void*)>(0x501F90);

		pD3DDevice = reinterpret_cast<IDirect3DDevice8**>(0xA32894);

		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x50EB2B), Part0, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4FECD0), Part1, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4FED28), Part2, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x501F77), Part3, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x501E1B), Part4, 6);
		break;
	case SH2V_11:
		GetLightSourceCount = *reinterpret_cast<int(__cdecl*)()>(0x50C8C0);
		GetLightSourceStruct = *reinterpret_cast<LightSource * (__cdecl*)(int)>(0x50C8D0);
		ActorDrawOpaque = reinterpret_cast<void(__cdecl*)(ModelMaterial*)>(0x501870);
		DoActorOpaqueStuff = reinterpret_cast<void(__cdecl*)(ModelOffsetTable*, void*)>(0x5022C0);

		pD3DDevice = reinterpret_cast<IDirect3DDevice8**>(0xA36494);

		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x50EE5B), Part0, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4FF000), Part1, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4FF058), Part2, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x5022A7), Part3, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x50214B), Part4, 6);
		break;
	case SH2V_DC:
		GetLightSourceCount = *reinterpret_cast<int(__cdecl*)()>(0x50C1E0);
		GetLightSourceStruct = *reinterpret_cast<LightSource * (__cdecl*)(int)>(0x50C1F0);
		ActorDrawOpaque = reinterpret_cast<void(__cdecl*)(ModelMaterial*)>(0x501190);
		DoActorOpaqueStuff = reinterpret_cast<void(__cdecl*)(ModelOffsetTable*, void*)>(0x501BE0);

		pD3DDevice = reinterpret_cast<IDirect3DDevice8**>(0xA35494);

		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x50E77B), Part0, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4FE920), Part1, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4FE978), Part2, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x501BC7), Part3, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x501A6B), Part4, 6);
		break;
	}
}

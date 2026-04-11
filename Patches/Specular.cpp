#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Patches.h"
#include "Common\Utils.h"
#include "Wrappers\d3d8\DirectX81SDK\include\d3d8.h"
#include "ModelID.h"
#include "ActorDrawingHook.h"

struct LightSource
{
	D3DLIGHT8 light;
	float extraFloats[12]; // Purpose unknown
};


int SpecularFlag = 0;
bool UseFakeLight = false;
bool InSpecialLightZone = false;

static LightSource fakeLight = { {D3DLIGHT_DIRECTIONAL} };
static int fakeLightIndex = -1;
static int materialCount = 0;
static ModelMaterial* pMaterialArray = nullptr;
static ModelMaterial* pCurrentMaterial = nullptr;

ModelID(__cdecl* GetModelID)() = nullptr;

static int(__cdecl* GetLightSourceCount)() = nullptr;
static LightSource* (__cdecl* GetLightSourceAt)(int) = nullptr;

static void(__cdecl* ActorOpaqueDraw)(ModelMaterial*) = nullptr;

int GetCurrentMaterialIndex()
{
	int index = 0;
	ModelMaterial* pCursor = pMaterialArray;

	while (index < materialCount)
	{
		if (pCurrentMaterial == pCursor)
			return index;

		pCursor = reinterpret_cast<ModelMaterial*>((reinterpret_cast<char*>(pCursor) + pCursor->materialLength));
		index++;
	}

	return -1;
}

bool IsJames(ModelID id)
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

bool IsMariaExcludingEyes(ModelID id)
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

bool IsMariaEyes(ModelID id)
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


static int __cdecl HookGetLightSourceCount()
{
	// Here we hook a call to `int GetLightSourceCount()`
	// This function usually just returns the amount of LightSource structures in the LightSourceArray
	// Hooking allows us to return a count 1 greater than reality when no D3D_DIRECTIONAL light sources exist

	InSpecialLightZone = false;
	UseFakeLight = true;
	fakeLightIndex = -1;
	pCurrentMaterial = nullptr;

	int lightSourceCount = GetLightSourceCount();

	for (int i = 0; i < lightSourceCount; i++)
	{
		auto pLight = GetLightSourceAt(i);
		if (pLight->light.Type == D3DLIGHT_DIRECTIONAL)
			UseFakeLight = false;

		if (pLight->light.Type == D3DLIGHT_SPOT)
			InSpecialLightZone = true;
	}

	if (UseFakeLight)
	{
		fakeLightIndex = lightSourceCount;
		return lightSourceCount + 1;
	}

	return lightSourceCount;
}

static LightSource* __cdecl HookGetLightSourceAt(int index)
{
	// Here we hook a call to `LightSource* GetLightSourceAt(int index)`
	// This function usually returns the a pointer to the LightSource at the supplied index
	// Hooking allows us to return fakeLight when the index is 1 greater than the true array length

	if (UseFakeLight && index == fakeLightIndex)
		return &fakeLight;
	else
		return GetLightSourceAt(index);
}

static void HookActorOpaqueDraw(ModelMaterial* pModelMaterial)
{
	// Here we hook a call to `void ActorOpaqueDraw(ModelMaterial* pModelMaterial)`
	// Hooking allows us to note the current material for later use

	SpecularFlag = 2;

	pCurrentMaterial = pModelMaterial;
	ActorOpaqueDraw(pModelMaterial);
}

void FindGetModelID()
{
	if (GetModelID)
		return;

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
    RegisterActorDrawTopPrologue([](ModelOffsetTable* pOffsetTable, void* /*arg2*/)->bool {
        // Here we hook a call to `void ActorDrawTop(ModelOffsetTable* pOffsetTable, void* arg2)`
        // This is the earliest function we're concerned with during an Actor draw
        // Hooking allows us to note the materialCount and pointer to the materialArray for later traversal

        materialCount = pOffsetTable->materialCount;
        pMaterialArray = reinterpret_cast<ModelMaterial*>(reinterpret_cast<char*>(pOffsetTable) + pOffsetTable->materialsOffset);

        // return false to not skip the actual draw
        return(false);
    });



	switch (GameVersion)
	{
	case SH2V_10:
		GetLightSourceCount = reinterpret_cast<decltype(GetLightSourceCount)>(0x50C590);
		GetLightSourceAt = reinterpret_cast<decltype(GetLightSourceAt)>(0x50C5A0);
		ActorOpaqueDraw = reinterpret_cast<decltype(ActorOpaqueDraw)>(0x501540);

		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4FECD0), HookGetLightSourceCount, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4FED28), HookGetLightSourceAt, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x501F77), HookActorOpaqueDraw, 5);
		break;

	case SH2V_11:
		GetLightSourceCount = reinterpret_cast<decltype(GetLightSourceCount)>(0x50C8C0);
		GetLightSourceAt = reinterpret_cast<decltype(GetLightSourceAt)>(0x50C8D0);
		ActorOpaqueDraw = reinterpret_cast<decltype(ActorOpaqueDraw)>(0x501870);

		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4FF000), HookGetLightSourceCount, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4FF058), HookGetLightSourceAt, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x5022A7), HookActorOpaqueDraw, 5);
		break;

	case SH2V_DC:
		GetLightSourceCount = reinterpret_cast<decltype(GetLightSourceCount)>(0x50C1E0);
		GetLightSourceAt = reinterpret_cast<decltype(GetLightSourceAt)>(0x50C1F0);
		ActorOpaqueDraw = reinterpret_cast<decltype(ActorOpaqueDraw)>(0x501190);

		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4FE920), HookGetLightSourceCount, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4FE978), HookGetLightSourceAt, 5);
		WriteCalltoMemory(reinterpret_cast<BYTE*>(0x501BC7), HookActorOpaqueDraw, 5);
		break;
	}
}

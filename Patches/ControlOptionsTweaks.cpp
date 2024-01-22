/**
* Copyright (C) 2024 mercury501
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "External\injector\include\injector\injector.hpp"
#include "External\injector\include\injector\utility.hpp"
#include "External\Hooking.Patterns\Hooking.Patterns.h"
#include "ControlOptionsTweaks.h"

ControllerIcons ControllerIconsRef;

LPDIRECT3DDEVICE8 CODirectXInterface = nullptr;
LPDIRECT3DTEXTURE8  g_pTexture = NULL;

D3DMATRIX COWorldMatrix =
{
  1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 0.f, 0.f, 1.f
};

void testDraw()
{

    if (!CODirectXInterface || g_pTexture == NULL)
    {
        return;
    }

    float tuStart = 0.f;
    float tuEnd = 0.25f;
    float tvStart = 0.1818f;
    float tvEnd = 0.2268f;

    struct Vertex
    {
        float x, y, z, rhw;
        float tu, tv; // Texture coordinates
    };

    Vertex vertices[] =
    {
        { 100.0f, 100.0f, 0.5f, 1.0f, tuStart, tvStart },
        { 100.0f, 200.0f, 0.5f, 1.0f, tuStart, tvEnd },
        { 200.0f, 200.0f, 0.5f, 1.0f, tuEnd,   tvEnd },
        { 200.0f, 100.0f, 0.5f, 1.0f, tuEnd,   tvStart }
    };

    CODirectXInterface->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_TEX1);

    CODirectXInterface->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
    CODirectXInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, 0);

    CODirectXInterface->SetRenderState(D3DRS_FOGENABLE, FALSE);

    CODirectXInterface->SetTextureStageState(0, D3DTSS_COLOROP, 1);
    CODirectXInterface->SetTextureStageState(0, D3DTSS_ALPHAOP, 1);

    CODirectXInterface->SetTextureStageState(1, D3DTSS_COLOROP, 1);
    CODirectXInterface->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);

    //ProxyInterface->SetTexture(0, 0);

    CODirectXInterface->SetTransform(D3DTS_WORLDMATRIX(0x56), &COWorldMatrix);

    CODirectXInterface->SetTexture(0, g_pTexture);

    CODirectXInterface->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    CODirectXInterface->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    CODirectXInterface->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

    CODirectXInterface->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

    CODirectXInterface->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(Vertex));

    CODirectXInterface->SetRenderState(D3DRS_ALPHAREF, 2);
    CODirectXInterface->SetRenderState(D3DRS_FOGENABLE, 1);
}

void PatchControlOptionsMenu()
{
	WriteCalltoMemory((BYTE*)0x00467985, testDraw, 0x05);
}

void ControllerIcons::HandleControllerIcons(LPDIRECT3DDEVICE8 ProxyInterface)
{
	CODirectXInterface = ProxyInterface;

	if (g_pTexture == NULL)
	{
		LPCWSTR texFile = L"icon_set.png";

		Logging::LogDebug() << "Controller icon set texture loading result: " << D3DXCreateTextureFromFile(CODirectXInterface, texFile, &g_pTexture);
	}


}
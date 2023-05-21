/**
* Copyright (C) 2023 mercury501
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


#include "MasterVolume.h"

#define MAX_SQUARES 15

struct CUSTOMVERTEX_DIF
{
    FLOAT x, y, z, rhw;
    DWORD color;
};

D3DMATRIX matrix =
{
  1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 0.f, 0.f, 1.f
};

D3DCOLOR SelectedLightGray = 0x40B0B0B0;
D3DCOLOR SelectedDarkGray = 0x40404040;

D3DCOLOR SelectedDarkGold = 0x00404020;
D3DCOLOR SelectedLightGold = 0x00B0B058;

float XPos = 835.948;
float YPos = 571.875;

float XOffset = 20.299;
float YOffset = 42.699;
float multiplier = 1.f;
float spacing = 10.f;

bool once = true;

int mousepos = 0;

void MasterVolume::HandleMasterVolumeSlider(LPDIRECT3DDEVICE8 ProxyInterface)
{

    if (ShowDebugOverlay)
    {
        int temp = GetMouseHorizontalPosition();

        if (temp > mousepos)
            spacing += 0.1;
        else if (temp < mousepos)
            spacing -= 0.1;

        mousepos = temp;

        AuxDebugOvlString = "\rValue: ";
        AuxDebugOvlString.append(std::to_string(spacing));

    }

	if (true)
	{
		DrawMasterVolumeSlider(ProxyInterface);
	}


}

void MasterVolume::DrawMasterVolumeSlider(LPDIRECT3DDEVICE8 ProxyInterface)
{

    int Squares = 5;
    float TempX = XPos;

    CUSTOMVERTEX_DIF TriangleVertices[MAX_SQUARES * 6];
    
    for (int i = 0; i < Squares; i++)
    {
        TriangleVertices[(i * 6)] =     { TempX,                 YPos,                   0.f, 1.f, SelectedLightGold};
        TriangleVertices[(i * 6) + 1] = { TempX,                 (YPos + YOffset),       0.f, 1.f, SelectedLightGold };
        TriangleVertices[(i * 6) + 2] = { (TempX + XOffset),     YPos,                   0.f, 1.f, SelectedLightGold };

        TriangleVertices[(i * 6) + 3] = { TempX,                (YPos + YOffset),   0.f, 1.f, SelectedDarkGold };
        TriangleVertices[(i * 6) + 4] = { (TempX + XOffset),    (YPos + YOffset),   0.f, 1.f, SelectedDarkGold };
        TriangleVertices[(i * 6) + 5] = { (TempX + XOffset),    YPos,               0.f, 1.f, SelectedDarkGold };

        TempX += spacing;
    }
    
    ProxyInterface->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

    ProxyInterface->SetRenderState(D3DRS_ALPHAREF, 1);
    ProxyInterface->SetRenderState(D3DRS_LIGHTING, 0);
    ProxyInterface->SetRenderState(D3DRS_SPECULARENABLE, 0);
    ProxyInterface->SetRenderState(D3DRS_ZENABLE, 1);
    ProxyInterface->SetRenderState(D3DRS_ZWRITEENABLE, 1);
    ProxyInterface->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
    ProxyInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
    ProxyInterface->SetRenderState(D3DRS_FOGENABLE, 0);

    ProxyInterface->SetTextureStageState(0, D3DTSS_COLOROP, 1);
    ProxyInterface->SetTextureStageState(0, D3DTSS_ALPHAOP, 1);

    ProxyInterface->SetTextureStageState(1, D3DTSS_COLOROP, 1);
    ProxyInterface->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);

    ProxyInterface->SetTexture(0, 0);

    ProxyInterface->SetTransform(D3DTS_WORLDMATRIX(0x56), &matrix);

    ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLELIST, Squares * 2, &TriangleVertices, 20);

    ProxyInterface->SetRenderState(D3DRS_ALPHAREF, 2);
    ProxyInterface->SetRenderState(D3DRS_FOGENABLE, 1);
}
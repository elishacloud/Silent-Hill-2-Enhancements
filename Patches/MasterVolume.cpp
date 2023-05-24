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

float XPos = 836.147;
float YPos = 571.875;

float XOffset = 20.299;
float YOffset = 42.699;
float multiplier = 1.f;
float spacing = 30.f;
float trans = 100.f;
float rotation = D3DX_PI;

bool once = true;

int mousepos = 0;

int32_t* VerticalRatio = (int32_t*)0x00a33484;
int32_t* HorizontalRatio = (int32_t*)0x00a33480;

int counterStart = 0x11;  // 0x04;

void MasterVolume::HandleMasterVolumeSlider(LPDIRECT3DDEVICE8 ProxyInterface)
{
    /*
    if (ShowDebugOverlay)
    {
        int temp = GetMouseHorizontalPosition();

        if (temp > mousepos)
            rotation += 0.1f;
        else if (temp < mousepos)
            rotation -= 0.1f;

        mousepos = temp;

        AuxDebugOvlString = "\rValue: ";
        AuxDebugOvlString.append(std::to_string(rotation));

    }*/

    if (true)
    {
        DrawMasterVolumeSlider(ProxyInterface);
    }
}

void MasterVolume::TranslateVertexBuffer(MasterVertex* vertices, int count, float x, float y)
{
    D3DXMATRIX TranslateMatrix;
    D3DXMatrixTranslation(&TranslateMatrix, x, y, 0.f);

    this->ApplyVertexBufferTransformation(vertices, count, TranslateMatrix);
}

void MasterVolume::RotateVertexBuffer(MasterVertex* vertices, int count, float angle)
{
    D3DXMATRIX RotationMatrix;
    D3DXMatrixRotationZ(&RotationMatrix, rotation);

    this->ApplyVertexBufferTransformation(vertices, count, RotationMatrix);
}

void MasterVolume::ScaleVertexBuffer(MasterVertex* vertices, int count, float x, float y)
{
    D3DXMATRIX ScalingMatrix;
    D3DXMatrixScaling(&ScalingMatrix, 5.f, 5.0f, 1.f);

    this->ApplyVertexBufferTransformation(vertices, count, ScalingMatrix);
}

void MasterVolume::ApplyVertexBufferTransformation(MasterVertex* vertices, int count, D3DXMATRIX matrix)
{
    D3DXVECTOR3 temp;

    for (int i = 0; i < count; i++)
    {
        D3DXVec3TransformCoord(&temp, &vertices[i].coords, &matrix);

        vertices[i].coords = temp;
    }
}

void MasterVolume::SetVertexBufferColor(MasterVertex* vertices, int count, DWORD color)
{
    for (int i = 0; i < count; i++)
    {
        vertices[i].color = color;
    }
}

void MasterVolume::DrawMasterVolumeSlider(LPDIRECT3DDEVICE8 ProxyInterface)
{
    if (!ShowDebugOverlay)
        return;



    float VRatio = (float)*VerticalRatio;
    float HRatio = (float)*HorizontalRatio;

    int counter = counterStart;


    int Squares = 5;
    float ScreenWidth = HRatio;
    float TempX = ScreenWidth / 2 + (float)(counter + 7) * ScreenWidth * 0.001953125;

    //AuxDebugOvlString = std::to_string(TempX);

    CUSTOMVERTEX_DIF TriangleVertices[6];
    /*
    for (int i = 0; i < Squares; i++)
    {
        TriangleVertices[(i * 6)]     = { TempX,                 YPos,              0.f, 1.f, SelectedLightGold };
        TriangleVertices[(i * 6) + 1] = { TempX,                 (YPos + YOffset),  0.f, 1.f, SelectedLightGold };
        TriangleVertices[(i * 6) + 2] = { (TempX + XOffset),     YPos,              0.f, 1.f, SelectedLightGold };

        TriangleVertices[(i * 6) + 3] = { TempX,                (YPos + YOffset),   0.f, 1.f, SelectedDarkGold };
        TriangleVertices[(i * 6) + 4] = { (TempX + XOffset),    (YPos + YOffset),   0.f, 1.f, SelectedDarkGold };
        TriangleVertices[(i * 6) + 5] = { (TempX + XOffset),    YPos,               0.f, 1.f, SelectedDarkGold };

        counter += 0x0B;
        TempX = ScreenWidth / 2 + (float)(counter + 7) * ScreenWidth * 0.001953125;

        //TempX += XOffset + spacing;
    }*/
    /*
    TriangleVertices[0] = { 1139.844, 622.500, 0.000, 1.000, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40) };
    TriangleVertices[1] = { 1135.156, 626.250, 0.000, 1.000, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40) };
    TriangleVertices[2] = { 1139.844, 665.625, 0.000, 1.000, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40) };
    TriangleVertices[3] = { 1135.156, 661.875, 0.000, 1.000, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40) };
    TriangleVertices[4] = { 1118.750, 665.625, 0.000, 1.000, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40) };
    TriangleVertices[5] = { 1123.438, 661.875, 0.000, 1.000, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40) };
    */
    // x:  - 1129.297, y: - 644.0625
    TriangleVertices[0] = { 10.547, -21.5625, 0.000, 1.000, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40) };
    TriangleVertices[1] = { 5.859, -17.8125, 0.000, 1.000, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40) };
    TriangleVertices[2] = { 10.547, 21.5625, 0.000, 1.000, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40) };
    TriangleVertices[3] = { 5.859, 17.8125, 0.000, 1.000, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40) };
    TriangleVertices[4] = { -10.547, 21.5625, 0.000, 1.000, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40) };
    TriangleVertices[5] = { -5.859, 17.8125, 0.000, 1.000, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40) };

    D3DXMATRIX scalingMatrix;
    D3DXMATRIX translateMatrix;
    D3DXMATRIX rotationMatrix;

    D3DXMatrixScaling(&scalingMatrix, 5.f, 5.0f, 1.f);
    D3DXMatrixTranslation(&translateMatrix, 800.f, 450.f, 0.f);
    D3DXMatrixRotationZ(&rotationMatrix, rotation);

    MasterVertex newVecs[6] =
    {
        { D3DXVECTOR3(10.547, -21.5625, 0.000) , 1.f, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40)},
        { D3DXVECTOR3( 5.859, -17.8125, 0.000) , 1.f, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40)},
        { D3DXVECTOR3( 10.547, 21.5625, 0.000) , 1.f, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40)},
        { D3DXVECTOR3(  5.859, 17.8125, 0.000) , 1.f, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40)},
        { D3DXVECTOR3(-10.547, 21.5625, 0.000) , 1.f, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40)},
        { D3DXVECTOR3( -5.859, 17.8125, 0.000) , 1.f, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40)}
    };

    this->ApplyVertexBufferTransformation(newVecs, 6, scalingMatrix);
    this->ApplyVertexBufferTransformation(newVecs, 6, rotationMatrix);
    this->ApplyVertexBufferTransformation(newVecs, 6, translateMatrix);

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

    //ProxyInterface->SetTransform(D3DTS_WORLDMATRIX(0x56), &xMatrix);

    ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 4, &newVecs, 20);

    ProxyInterface->SetRenderState(D3DRS_ALPHAREF, 2);
    ProxyInterface->SetRenderState(D3DRS_FOGENABLE, 1);
}


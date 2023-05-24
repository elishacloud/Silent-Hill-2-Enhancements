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

#define SQUARES 4

// Bezel L flipped horizontally
MasterVertex BezelVertices[6] =
{
    { D3DXVECTOR3(10.547, -21.5625, 0.000) , 1.f, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40)},
    { D3DXVECTOR3(5.859, -17.8125, 0.000) , 1.f, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40)},
    { D3DXVECTOR3(10.547, 21.5625, 0.000) , 1.f, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40)},
    { D3DXVECTOR3(5.859, 17.8125, 0.000) , 1.f, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40)},
    { D3DXVECTOR3(-10.547, 21.5625, 0.000) , 1.f, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40)},
    { D3DXVECTOR3(-5.859, 17.8125, 0.000) , 1.f, D3DCOLOR_ARGB(0x40,0x40,0x40,0x40)}
};

MasterVertex InnerSquare[SQUARES] =
{
    { D3DXVECTOR3(5.f, 10.f, 0.000),1.000,D3DCOLOR_ARGB(0x40,0x50,0x50,0x50)},
    { D3DXVECTOR3(5.f, -10.f, 0.000),1.000,D3DCOLOR_ARGB(0x40,0x50,0x50,0x50)},
    { D3DXVECTOR3(-5.f, 10.f, 0.000),1.000,D3DCOLOR_ARGB(0x40,0x50,0x50,0x50)},
    { D3DXVECTOR3(-5.f, -10.f, 0.000),1.000,D3DCOLOR_ARGB(0x40,0x50,0x50,0x50)}

};


D3DMATRIX WorldMatrix =
{
  1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 0.f, 0.f, 1.f
};

D3DCOLOR SelectedLightGray = D3DCOLOR_ARGB(0x40, 0xB0, 0xB0, 0xB0);
D3DCOLOR SelectedDarkGray = D3DCOLOR_ARGB(0x40, 0x40, 0x40, 0x40);

D3DCOLOR SelectedDarkGold = D3DCOLOR_ARGB(0x00, 0x40, 0x40, 0x20);
D3DCOLOR SelectedLightGold = D3DCOLOR_ARGB(0x00, 0xB0, 0xB0, 0x58);

float XPos = 836.147;
float YPos = 571.875;

float XOffset = 20.299;
float YOffset = 42.699;
float multiplier = 1.f;
float spacing = 30.f;
float trans = 100.f;

bool once = true;

int mousepos = 0;

int32_t* VerticalRatio = (int32_t*)0x00a33484;
int32_t* HorizontalRatio = (int32_t*)0x00a33480;

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
    D3DXMatrixRotationZ(&RotationMatrix, angle);

    this->ApplyVertexBufferTransformation(vertices, count, RotationMatrix);
}

void MasterVolume::ScaleVertexBuffer(MasterVertex* vertices, int count, float x, float y)
{
    D3DXMATRIX ScalingMatrix;
    D3DXMatrixScaling(&ScalingMatrix, x, y, 1.f);

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

void MasterVolume::CopyVertexBuffer(MasterVertex* source, MasterVertex* destination, int count)
{
    for (int i = 0; i < count; i++)
    {
        destination[i] = { D3DXVECTOR3(source[i].coords.x, source[i].coords.y, source[i].coords.z), source[i].rhw, source[i].color };
    }
}

void MasterVolume::DrawMasterVolumeSlider(LPDIRECT3DDEVICE8 ProxyInterface)
{

    MasterVertex BottomBezel[6];
    MasterVertex TopBezel[6];
    MasterVertex square[SQUARES];

    this->CopyVertexBuffer(BezelVertices, BottomBezel, 6);
    this->CopyVertexBuffer(BezelVertices, TopBezel, 6);
    this->CopyVertexBuffer(InnerSquare, square, SQUARES);

    D3DXMATRIX scalingMatrix;
    D3DXMATRIX translateMatrix;
    D3DXMATRIX BezelTranslateMatrix;
    D3DXMATRIX rotationMatrix;

    D3DXMatrixScaling(&scalingMatrix, 10.f, 10.0f, 1.f);
    D3DXMatrixTranslation(&translateMatrix, (float)GetMouseHorizontalPosition(), (float)GetMouseVerticalPosition(), 0.f);
    D3DXMatrixTranslation(&BezelTranslateMatrix, 400.f, 400.f, 0.f);
    D3DXMatrixRotationZ(&rotationMatrix, D3DX_PI);

    this->ApplyVertexBufferTransformation(BottomBezel, 6, scalingMatrix);
    this->ApplyVertexBufferTransformation(BottomBezel, 6, BezelTranslateMatrix);
    this->SetVertexBufferColor(BottomBezel, 6, SelectedLightGold);

    this->ApplyVertexBufferTransformation(TopBezel, 6, scalingMatrix);
    this->ApplyVertexBufferTransformation(TopBezel, 6, rotationMatrix);
    this->ApplyVertexBufferTransformation(TopBezel, 6, BezelTranslateMatrix);
    this->SetVertexBufferColor(TopBezel, 6, SelectedDarkGold);

    this->ApplyVertexBufferTransformation(square, SQUARES, scalingMatrix);
    this->ApplyVertexBufferTransformation(square, SQUARES, translateMatrix);

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
    
    ProxyInterface->SetTransform(D3DTS_WORLDMATRIX(0x56), &WorldMatrix);

    ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 4, &TopBezel, 20);
    ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 4, &BottomBezel, 20);
    ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &square, 20);

    ProxyInterface->SetRenderState(D3DRS_ALPHAREF, 2);
    ProxyInterface->SetRenderState(D3DRS_FOGENABLE, 1);
}


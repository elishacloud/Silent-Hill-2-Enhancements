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

D3DMATRIX WorldMatrix =
{
  1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 0.f, 0.f, 1.f
};

//int32_t* VerticalRatio = (int32_t*)0x00a33484;
//int32_t* HorizontalRatio = (int32_t*)0x00a33480;

int test = 5;

MasterVolumeSlider MasterVolumeSliderRef;

void MasterVolume::HandleMasterVolumeSlider(LPDIRECT3DDEVICE8 ProxyInterface)
{
    static int mousepos = 0;
    
    if (ShowDebugOverlay)
    {
        int temp = GetMouseHorizontalPosition();

        if (temp > mousepos)
            test += 1;
        else if (temp < mousepos)
            test -= 1;

        if (test > 0xF)
            test = 0xF;
        if (test < 0)
            test = 0;

        mousepos = temp;

        AuxDebugOvlString = "\rValue: ";
        AuxDebugOvlString.append(std::to_string(test));

    }
    if (ShowDebugOverlay)
        MasterVolumeSliderRef.DrawSlider(ProxyInterface, test, test != 5);
}

void MasterVolumeSlider::InitVertices()
{
    this->LastBufferHeight = BufferHeight;
    this->LastBufferWidth = BufferWidth;

    //TODO remove
    int32_t* VerticalInternal = (int32_t*)0x00a33484;
    int32_t* HorizontalInternal = (int32_t*)0x00a33480;

    //TODO temporary poc
    float spacing = (25.781 * (float)*HorizontalInternal) / 1200.f;
    float xScaling = (float)*HorizontalInternal / 1200.f;
    float yScaling = (float)*VerticalInternal / 900.f;

    float UlteriorOffset = (BufferWidth - (float)*HorizontalInternal) / 2;
    
    float xOffset = (645.703 * (float)*HorizontalInternal) / 1200.f + UlteriorOffset;
    float yOffset = (593.4375 * (float)*VerticalInternal) / 900.f;

    for (int i = 0; i < 0xF; i++)
    {
        this->CopyVertexBuffer(this->BezelVertices, this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM);
        this->CopyVertexBuffer(this->BezelVertices, this->FinalBezels[i].BotVertices, BEZEL_VERT_NUM);

        this->CopyVertexBuffer(this->RectangleVertices, this->FinalPips[i].vertices, RECT_VERT_NUM);

        // Flip the top bezel
        this->RotateVertexBuffer(this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM, D3DX_PI);
        
        // Scaling
        this->ScaleVertexBuffer(this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM, xScaling, yScaling);
        this->ScaleVertexBuffer(this->FinalBezels[i].BotVertices, BEZEL_VERT_NUM, xScaling, yScaling);
        this->ScaleVertexBuffer(this->FinalPips[i].vertices, RECT_VERT_NUM, xScaling, yScaling);

        // Translating
        this->TranslateVertexBuffer(this->FinalPips[i].vertices, RECT_VERT_NUM, xOffset + ((float)i * spacing), yOffset);

        float DeltaX = this->FinalPips[i].vertices[0].coords.x - this->FinalBezels[i].BotVertices[3].coords.x;
        float DeltaY = this->FinalPips[i].vertices[0].coords.y - this->FinalBezels[i].BotVertices[3].coords.y;

        this->TranslateVertexBuffer(this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM, DeltaX, DeltaY);
        this->TranslateVertexBuffer(this->FinalBezels[i].BotVertices, BEZEL_VERT_NUM, DeltaX, DeltaY);
    }
}

void MasterVolumeSlider::DrawSlider(LPDIRECT3DDEVICE8 ProxyInterface, int value, bool ValueChanged)
{
    if (LastBufferHeight != BufferHeight || LastBufferWidth != BufferWidth)
        this->InitVertices();

    //TODO address for selected option

    const int color = true ? 0 : 1;

    // Set up the graphics' color
    if (ValueChanged)
    {
        for (int i = 0; i < 0xF; i++)
        {
            this->SetVertexBufferColor(this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM, this->LightGoldBezel[color]);
            this->SetVertexBufferColor(this->FinalBezels[i].BotVertices, BEZEL_VERT_NUM, this->DarkGoldBezel[color]);
        }

        // Set inner rectangle color, based on the current value
        for (int i = 0; i < 0xF; i++)
        {
            if (value <= i)
                this->SetVertexBufferColor(this->FinalPips[i].vertices, RECT_VERT_NUM, this->ActiveGoldSquare[color]);
            else
                this->SetVertexBufferColor(this->FinalPips[i].vertices, RECT_VERT_NUM, this->InactiveGoldSquare[color]);
        }
    }
    else
    {
        for (int i = 0; i < 0xF; i++)
        {
            this->SetVertexBufferColor(this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM, this->LightGrayBezel[color]);
            this->SetVertexBufferColor(this->FinalBezels[i].BotVertices, BEZEL_VERT_NUM, this->DarkGrayBezel[color]);
        }

        // Set inner rectangle color, based on the current value
        for (int i = 0; i < 0xF; i++)
        {
            if (value <= i)
                this->SetVertexBufferColor(this->FinalPips[i].vertices, RECT_VERT_NUM, this->ActiveGraySquare[color]);
            else
                this->SetVertexBufferColor(this->FinalPips[i].vertices, RECT_VERT_NUM, this->InactiveGraySquare[color]);
        }
    }

    ProxyInterface->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

    ProxyInterface->SetRenderState(D3DRS_ALPHAREF, 1);
    ProxyInterface->SetRenderState(D3DRS_LIGHTING, 0);
    ProxyInterface->SetRenderState(D3DRS_SPECULARENABLE, 0); //TODO check 
    ProxyInterface->SetRenderState(D3DRS_ZENABLE, 1);
    ProxyInterface->SetRenderState(D3DRS_ZWRITEENABLE, 1);
    ProxyInterface->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
    ProxyInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, 0);

    ProxyInterface->SetRenderState(D3DRS_FOGENABLE, FALSE);

    ProxyInterface->SetTextureStageState(0, D3DTSS_COLOROP, 1);
    ProxyInterface->SetTextureStageState(0, D3DTSS_ALPHAOP, 1);

    ProxyInterface->SetTextureStageState(1, D3DTSS_COLOROP, 1);
    ProxyInterface->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);

    ProxyInterface->SetTexture(0, 0);
    
    ProxyInterface->SetTransform(D3DTS_WORLDMATRIX(0x56), &WorldMatrix);

    // Draw every active bezel
    for (int i = 0; i < value; i++)
    {
        ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 4, this->FinalBezels[i].TopVertices, 20);
        ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 4, this->FinalBezels[i].BotVertices, 20);
    }
    
    // Draw every inner rectangle
    for (int i = 0; i < 0xF; i++)
        ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, this->FinalPips[i].vertices, 20);

    ProxyInterface->SetRenderState(D3DRS_ALPHAREF, 2);
    ProxyInterface->SetRenderState(D3DRS_FOGENABLE, 1);
}

void MasterVolumeSlider::TranslateVertexBuffer(MasterVertex* vertices, int count, float x, float y)
{
    D3DXMATRIX TranslateMatrix;
    D3DXMatrixTranslation(&TranslateMatrix, x, y, 0.f);

    this->ApplyVertexBufferTransformation(vertices, count, TranslateMatrix);
}

void MasterVolumeSlider::RotateVertexBuffer(MasterVertex* vertices, int count, float angle)
{
    D3DXMATRIX RotationMatrix;
    D3DXMatrixRotationZ(&RotationMatrix, angle);

    this->ApplyVertexBufferTransformation(vertices, count, RotationMatrix);
}

void MasterVolumeSlider::ScaleVertexBuffer(MasterVertex* vertices, int count, float x, float y)
{
    D3DXMATRIX ScalingMatrix;
    D3DXMatrixScaling(&ScalingMatrix, x, y, 1.f);

    this->ApplyVertexBufferTransformation(vertices, count, ScalingMatrix);
}

void MasterVolumeSlider::ApplyVertexBufferTransformation(MasterVertex* vertices, int count, D3DXMATRIX matrix)
{
    D3DXVECTOR3 temp;

    for (int i = 0; i < count; i++)
    {
        D3DXVec3TransformCoord(&temp, &vertices[i].coords, &matrix);

        vertices[i].coords = temp;
    }
}

void MasterVolumeSlider::SetVertexBufferColor(MasterVertex* vertices, int count, DWORD color)
{
    for (int i = 0; i < count; i++)
    {
        vertices[i].color = color;
    }
}

void MasterVolumeSlider::CopyVertexBuffer(MasterVertex* source, MasterVertex* destination, int count)
{
    for (int i = 0; i < count; i++)
    {
        destination[i] = { D3DXVECTOR3(source[i].coords.x, source[i].coords.y, source[i].coords.z), source[i].rhw, source[i].color };
    }
}
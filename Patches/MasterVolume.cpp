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

#include "External\injector\include\injector\injector.hpp"
#include "External\injector\include\injector\utility.hpp"
#include "External\Hooking.Patterns\Hooking.Patterns.h"
#include "MasterVolume.h"

D3DMATRIX WorldMatrix =
{
  1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 0.f, 0.f, 1.f
};

BYTE* ChangedOptionsCheckReturn = nullptr;
BYTE* DiscardOptionsBackingOutReturn = nullptr;
BYTE* DiscardOptionsNoBackingOutReturn = nullptr;
BYTE* ChangeMasterVolumeReturn = nullptr;

BYTE* MoveRightArrowHitboxReturn = nullptr;
DWORD* RightArrowDefaultPointer = nullptr;
DWORD RightArrowDefault = 0;

static int SavedMasterVolumeLevel = 0;
static int CurrentMasterVolumeLevel = 0;

injector::hook_back<void(__cdecl*)(DWORD*)> orgDrawOptions;
injector::hook_back<void(__cdecl*)(int32_t, int32_t)> orgDrawArrowRight;
injector::hook_back<void(__cdecl*)(int32_t)> orgConfirmOptionsFun;
injector::hook_back<int32_t(__cdecl*)(int32_t, float, DWORD)> orgPlaySound;

LPDIRECT3DDEVICE8 DirectXInterface = nullptr;

MasterVolume MasterVolumeRef;
MasterVolumeSlider MasterVolumeSliderRef;
bool DiscardOptions = false;
int ChangeMasterVolume = 0;

int32_t PlaySound_Hook(int32_t SoundId, float volume, DWORD param3)
{
    if (GetSelectedOption() == 0x07)
    {
        return 0;
    }

    return orgPlaySound.fun(SoundId, volume, param3);
}

#pragma warning(disable : 4100)
void __cdecl DrawArrowRight_Hook(int32_t param1, int32_t param2)
{
    orgDrawArrowRight.fun(0xC5, param2);
}

void __cdecl DrawOptions_Hook(DWORD* pointer)
{
    orgDrawOptions.fun(pointer);

    MasterVolumeSliderRef.DrawSlider(DirectXInterface, CurrentMasterVolumeLevel, CurrentMasterVolumeLevel != SavedMasterVolumeLevel);
}

void __cdecl ConfirmOptions_Hook(int32_t param)
{
    MasterVolumeRef.HandleConfirmOptions(true);

    orgConfirmOptionsFun.fun(param);
}

#pragma warning(disable : 4740)
__declspec(naked) void __stdcall SetRightArrowHitbox()
{
    RightArrowDefault = *RightArrowDefaultPointer;

    if (GetSelectedOption() == 0x07)
    {
        __asm
        {   // In the Master Volume option, move the arrow to the right
            mov eax, 0xA8
        }
    }
    else
    {
        __asm
        {   // In another option, use the default from the game
            mov eax, RightArrowDefault
        }
    }

    __asm
    {
        jmp MoveRightArrowHitboxReturn
    }
}

__declspec(naked) void __stdcall ChangeSpeakerConfigCheck()
{
    __asm
    {
        mov dl, byte ptr[CurrentMasterVolumeLevel]
        cmp dl, byte ptr[SavedMasterVolumeLevel]

        jmp ChangedOptionsCheckReturn;
    }
}

__declspec(naked) void __stdcall DiscardOptionsBackingOut()
{
    DiscardOptions = true;

    __asm
    {
        jmp DiscardOptionsBackingOutReturn;
    }
}

__declspec(naked) void __stdcall DiscardOptionsNoBackingOut()
{
    DiscardOptions = true;

    __asm
    {
        jmp DiscardOptionsNoBackingOutReturn;
    }
}

__declspec(naked) void __stdcall IncrementMasterVolume()
{
    ChangeMasterVolume = 1;

    __asm
    {
        jmp ChangeMasterVolumeReturn;
    }
}

__declspec(naked) void __stdcall DecrementMasterVolume()
{
    ChangeMasterVolume = -1;

    __asm
    {
        jmp ChangeMasterVolumeReturn;
    }
}

void PatchMasterVolumeSlider()
{
    // Initialize pointers
    ChangedOptionsCheckReturn = GetCheckForChangedOptionsPointer() + 0x0C;

    BYTE* DiscardOptionsBOAddr = GetDiscardOptionBOPointer() + (GameVersion == SH2V_DC ? 0x01 : 0x00);
    BYTE* DiscardOptionsAddr = GetDiscardOptionPointer();

    DiscardOptionsBackingOutReturn = GetDiscardOptionBOPointer();
    DiscardOptionsNoBackingOutReturn = GetDiscardOptionPointer();

    ChangeMasterVolumeReturn = GetDecrementMasterVolumePointer() + (GameVersion == SH2V_DC ? 0x1C : 0x16);

    MoveRightArrowHitboxReturn = GetOptionsRightArrowHitboxPointer() + 0x05;
    RightArrowDefaultPointer = *(DWORD**)(GetOptionsRightArrowHitboxPointer() + 0x01);    

    BYTE* DecrementVolumeAddr = GetDecrementMasterVolumePointer() + (GameVersion == SH2V_DC ? -0x02 : 0x0);
    BYTE* IncrementVolumeAddr = GetIncrementMasterVolumePointer() + (GameVersion == SH2V_DC ? -0x07 : 0x0);

    BYTE* RenderRightArrowAddr = GetRenderOptionsRightArrowFunPointer() + (GameVersion == SH2V_DC ? 0x02 : 0x0);

    // Hook options drawing to draw at the same time
    orgDrawOptions.fun = injector::MakeCALL(GetDrawOptionsFunPointer(), DrawOptions_Hook, true).get();

    // Hook right arrow drawing to move it to the right 
    orgDrawArrowRight.fun = injector::MakeCALL(RenderRightArrowAddr, DrawArrowRight_Hook, true).get();

    // Skip drawing the old option text 
    UpdateMemoryAddress((void*)GetSpkOptionTextOnePointer(), "\x90\x90\x90\x90\x90", 5);
    UpdateMemoryAddress((void*)GetSpkOptionTextTwoPointer(), "\x90\x90\x90\x90\x90", 5);

    // Inject our values in the game's check for changed settings
    WriteJMPtoMemory(GetCheckForChangedOptionsPointer(), ChangeSpeakerConfigCheck, 0x0C);

    // Set the DiscardOptions flag when restoring saved settings
    WriteJMPtoMemory(DiscardOptionsBOAddr - 0x06, DiscardOptionsBackingOut, 0x06);
    WriteJMPtoMemory(DiscardOptionsAddr - 0x06, DiscardOptionsNoBackingOut, 0x06);

    // Detour execution to change the hitbox position
    WriteJMPtoMemory(GetOptionsRightArrowHitboxPointer(), SetRightArrowHitbox, 0x05);

    // Set the ChangeMasterVolumeValue to update the value
    WriteJMPtoMemory(IncrementVolumeAddr, IncrementMasterVolume, 0x07);
    WriteJMPtoMemory(DecrementVolumeAddr, DecrementMasterVolume, 0x07);

    // hook the function that is called when confirming changed options
    orgConfirmOptionsFun.fun = injector::MakeCALL(GetConfirmOptionsOnePointer(), ConfirmOptions_Hook, true).get();
    injector::MakeCALL(GetConfirmOptionsTwoPointer(), ConfirmOptions_Hook, true).get();

    // Hook the function that plays sounds at the end of the options switch
    orgPlaySound.fun = injector::MakeCALL(GetPlaySoundFunPointer(), PlaySound_Hook, true).get();
}

void MasterVolume::ChangeMasterVolumeValue(int delta)
{
    if (!IsInMainOptionsMenu() || delta == 0)
        return;

    if ((delta < 0 && CurrentMasterVolumeLevel > 0) || (delta > 0 && CurrentMasterVolumeLevel < 0x0F))
    {
        CurrentMasterVolumeLevel += delta;
        ConfigData.VolumeLevel = CurrentMasterVolumeLevel;
        SetNewVolume();

        // Play the ding sound when changing volume
        orgPlaySound.fun(0x2719, 1.0, 0);
    }

    ChangeMasterVolume = 0;
}

void MasterVolume::HandleConfirmOptions(bool ConfirmChange)
{
    if (!ConfirmChange)
    {
        ConfigData.VolumeLevel = SavedMasterVolumeLevel;
        CurrentMasterVolumeLevel = SavedMasterVolumeLevel;
    }

    SaveConfigData();
    SetNewVolume();
}

void MasterVolume::HandleMasterVolume(LPDIRECT3DDEVICE8 ProxyInterface)
{
    if (!EnableMasterVolume)
        return;

    DirectXInterface = ProxyInterface;

    if (DiscardOptions)
    {
        this->HandleConfirmOptions(false);
        DiscardOptions = false;
    }

    // If we just entered the main options menu
    if (IsInOptionsMenu())
    {
        this->ChangeMasterVolumeValue(ChangeMasterVolume);

        if (!this->EnteredOptionsMenu)
        {
            SavedMasterVolumeLevel = ConfigData.VolumeLevel;
            CurrentMasterVolumeLevel = SavedMasterVolumeLevel;

            this->EnteredOptionsMenu = true;
        }
        else
        {
            ConfigData.VolumeLevel = CurrentMasterVolumeLevel;
        }    
    }
    else
    {
        this->EnteredOptionsMenu = false;
    }
}

void MasterVolumeSlider::InitVertices()
{
    this->LastBufferHeight = BufferHeight;
    this->LastBufferWidth = BufferWidth;

    int32_t VerticalInternal = GetInternalVerticalRes();
    int32_t HorizontalInternal = GetInternalHorizontalRes();

    float spacing = (25.781 * (float)HorizontalInternal) / 1200.f;
    float xScaling = (float)HorizontalInternal / 1200.f;
    float yScaling = (float)VerticalInternal / 900.f;

    float UlteriorOffset = (BufferWidth - (float)HorizontalInternal) / 2;
    
    float xOffset = (645.703 * (float)HorizontalInternal) / 1200.f + UlteriorOffset;
    float yOffset = (593.4375 * (float)VerticalInternal) / 900.f - ((50.625f * (float)VerticalInternal) / 900.f);

    for (int i = 0; i < 0xF; i++)
    {
        this->CopyVertexBuffer(this->BezelVertices, this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM);
        this->CopyVertexBuffer(this->BezelVertices, this->FinalBezels[i].BotVertices, BEZEL_VERT_NUM);

        this->CopyVertexBuffer(this->RectangleVertices, this->FinalRects[i].vertices, RECT_VERT_NUM);

        // Flip the top bezel
        this->RotateVertexBuffer(this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM, D3DX_PI);
        
        // Scaling
        this->ScaleVertexBuffer(this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM, xScaling, yScaling);
        this->ScaleVertexBuffer(this->FinalBezels[i].BotVertices, BEZEL_VERT_NUM, xScaling, yScaling);
        this->ScaleVertexBuffer(this->FinalRects[i].vertices, RECT_VERT_NUM, xScaling, yScaling);

        // Translating
        this->TranslateVertexBuffer(this->FinalRects[i].vertices, RECT_VERT_NUM, xOffset + ((float)i * spacing), yOffset);

        float DeltaX = this->FinalRects[i].vertices[0].coords.x - this->FinalBezels[i].BotVertices[3].coords.x;
        float DeltaY = this->FinalRects[i].vertices[0].coords.y - this->FinalBezels[i].BotVertices[3].coords.y;

        this->TranslateVertexBuffer(this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM, DeltaX, DeltaY);
        this->TranslateVertexBuffer(this->FinalBezels[i].BotVertices, BEZEL_VERT_NUM, DeltaX, DeltaY);
    }
}

void MasterVolumeSlider::DrawSlider(LPDIRECT3DDEVICE8 ProxyInterface, int value, bool ValueChanged)
{
    if (!IsInMainOptionsMenu())
        return;

    if (LastBufferHeight != BufferHeight || LastBufferWidth != BufferWidth)
        this->InitVertices();

    const int selected = GetSelectedOption() == 0x07 ? 0 : 1;

    // Set up the graphics' color
    if (ValueChanged)
    {
        for (int i = 0; i < 0xF; i++)
        {
            this->SetVertexBufferColor(this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM, this->LightGoldBezel[selected]);
            this->SetVertexBufferColor(this->FinalBezels[i].BotVertices, BEZEL_VERT_NUM, this->DarkGoldBezel[selected]);
        }

        // Set inner rectangle color, based on the current value
        for (int i = 0; i < 0xF; i++)
        {
            if (value <= i)
                this->SetVertexBufferColor(this->FinalRects[i].vertices, RECT_VERT_NUM, this->InactiveGoldSquare[selected]);
            else
                this->SetVertexBufferColor(this->FinalRects[i].vertices, RECT_VERT_NUM, this->ActiveGoldSquare[selected]);
        }
    }
    else
    {
        for (int i = 0; i < 0xF; i++)
        {
            this->SetVertexBufferColor(this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM, this->LightGrayBezel[selected]);
            this->SetVertexBufferColor(this->FinalBezels[i].BotVertices, BEZEL_VERT_NUM, this->DarkGrayBezel[selected]);
        }

        // Set inner rectangle color, based on the current value
        for (int i = 0; i < 0xF; i++)
        {
            if (value <= i)
                this->SetVertexBufferColor(this->FinalRects[i].vertices, RECT_VERT_NUM, this->InactiveGraySquare[selected]);
            else
                this->SetVertexBufferColor(this->FinalRects[i].vertices, RECT_VERT_NUM, this->ActiveGraySquare[selected]);
        }
    }

    ProxyInterface->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

    ProxyInterface->SetRenderState(D3DRS_ALPHAREF, 1);
    ProxyInterface->SetRenderState(D3DRS_LIGHTING, 0);
    ProxyInterface->SetRenderState(D3DRS_SPECULARENABLE, 0);
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
        ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, this->FinalRects[i].vertices, 20);

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

bool IsInMainOptionsMenu()
{
    return GetOptionsPage() == 0x02 && GetOptionsSubPage() == 0x00;
}

bool IsInOptionsMenu()
{
    return GetEventIndex() == 0x07 &&
        (GetOptionsPage() == 0x02 || GetOptionsPage() == 0x07 || GetOptionsPage() == 0x04) &&
        (GetOptionsSubPage() == 0x00 || GetOptionsSubPage() == 0x01);
}

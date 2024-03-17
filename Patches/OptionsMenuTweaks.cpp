/**
* Copyright (C) 2024 mercury501, Polymega, iOrange, Aero_
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
#include "Common\FileSystemHooks.h"
#include "Common\GfxUtils.h"
#include "OptionsMenuTweaks.h"

extern char* getControlOptionsStr();

bool DrawOptionsHookEnabled = false;
bool wasInControlOptions = false;

// Master volume
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
injector::hook_back<void(__cdecl*)(int32_t, int32_t)> orgDrawArrowRightMasterVolume;
injector::hook_back<void(__cdecl*)(int32_t)> orgConfirmOptionsFun;
injector::hook_back<int32_t(__cdecl*)(int32_t, float, DWORD)> orgPlaySound;

LPDIRECT3DDEVICE8 DirectXInterface = nullptr;

MasterVolume MasterVolumeRef;
MasterVolumeSlider MasterVolumeSliderRef;
bool DiscardOptions = false;
int ChangeMasterVolume = 0;

// Control options
ButtonIcons ButtonIconsRef;

const float ControlOptionRedGreen = 0.502f;
const float ControlOptionSelectedBlue = 0.8785f;
const float ControlOptionUnselectedBlue = 0.502f;
const float ControlOptionsLocked = 0.251f;
const float ControlOptionsChangingRed = 0.8785f;
const float ControlOptionsChangingGreenBlue = 0.502f;

/*
// Assembled with `psa.exe -h0` from DirectX 8.1b SDK
    ps.1.1

    tex t0
    mul r0, t0, c0
*/

DWORD ModulationPixelShaderAsm[] = {
    0xffff0101, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000042,
    0xb00f0000, 0x00000005, 0x800f0000, 0xb0e40000,
    0xa0e40000, 0x0000ffff
};

int32_t PlaySound_Hook(int32_t SoundId, float volume, DWORD param3)
{
    if (GetSelectedOption() == 0x07)
    {
        return 0;
    }

    return orgPlaySound.fun(SoundId, volume, param3);
}

#pragma warning(disable : 4100)
void __cdecl DrawArrowRightMasterVolume_Hook(int32_t param1, int32_t param2)
{
    orgDrawArrowRightMasterVolume.fun(0xC5, param2);
}

void __cdecl DrawOptions_Hook(DWORD* pointer)
{
    orgDrawOptions.fun(pointer);

    if (EnableMasterVolume)
    {
        MasterVolumeSliderRef.DrawSlider(DirectXInterface, CurrentMasterVolumeLevel, CurrentMasterVolumeLevel != SavedMasterVolumeLevel);
    }

    if (ReplaceButtonText != BUTTON_ICONS_DISABLED)
    {
        ButtonIconsRef.DrawIcons(DirectXInterface);
    }
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

void EnableDrawOptionsHook()
{
    if (DrawOptionsHookEnabled)
    {
        return;
    }

    // Hook options drawing to draw at the same time
    orgDrawOptions.fun = injector::MakeCALL(GetDrawOptionsFunPointer(), DrawOptions_Hook, true).get();

    DrawOptionsHookEnabled = true;
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

    EnableDrawOptionsHook();

    // Hook right arrow drawing to move it to the right 
    orgDrawArrowRightMasterVolume.fun = injector::MakeCALL(RenderRightArrowAddr, DrawArrowRightMasterVolume_Hook, true).get();

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

void PatchControlOptionsMenu()
{
    BYTE* FunctionDrawControllerValues = (BYTE*)0x00467985; //TODO address
    BYTE* FunctionDrawDashes = (BYTE*)0x00467606; //TODO address

    EnableDrawOptionsHook();

    UpdateMemoryAddress(FunctionDrawControllerValues, "\x90\x90\x90\x90\x90", 0x05);
    UpdateMemoryAddress(FunctionDrawDashes, "\x90\x90\x90\x90\x90", 0x05);
}

// Graphics utils
void ApplyVertexBufferTransformation(TexturedVertex* vertices, int count, D3DXMATRIX matrix)
{
    D3DXVECTOR3 temp;

    for (int i = 0; i < count; i++)
    {
        D3DXVec3TransformCoord(&temp, &vertices[i].coords, &matrix);

        vertices[i].coords = temp;
    }
}

void ApplyVertexBufferTransformation(ColorVertex* vertices, int count, D3DXMATRIX matrix)
{
    D3DXVECTOR3 temp;

    for (int i = 0; i < count; i++)
    {
        D3DXVec3TransformCoord(&temp, &vertices[i].coords, &matrix);

        vertices[i].coords = temp;
    }
}

void TranslateVertexBuffer(TexturedVertex* vertices, int count, float x, float y)
{
    D3DXMATRIX TranslateMatrix;
    D3DXMatrixTranslation(&TranslateMatrix, x, y, 0.f);

    ApplyVertexBufferTransformation(vertices, count, TranslateMatrix);
}

void ScaleVertexBuffer(TexturedVertex* vertices, int count, float x, float y)
{
    D3DXMATRIX ScalingMatrix;
    D3DXMatrixScaling(&ScalingMatrix, x, y, 1.f);

    ApplyVertexBufferTransformation(vertices, count, ScalingMatrix);
}

void CopyVertexBuffer(TexturedVertex* source, TexturedVertex* destination, int count)
{
    for (int i = 0; i < count; i++)
    {
        destination[i] = { D3DXVECTOR3(source[i].coords.x, source[i].coords.y, source[i].coords.z), source[i].rhw, source[i].u, source[i].v};
    }
}

void TranslateVertexBuffer(ColorVertex* vertices, int count, float x, float y)
{
    D3DXMATRIX TranslateMatrix;
    D3DXMatrixTranslation(&TranslateMatrix, x, y, 0.f);

    ApplyVertexBufferTransformation(vertices, count, TranslateMatrix);
}

void RotateVertexBuffer(ColorVertex* vertices, int count, float angle)
{
    D3DXMATRIX RotationMatrix;
    D3DXMatrixRotationZ(&RotationMatrix, angle);

    ApplyVertexBufferTransformation(vertices, count, RotationMatrix);
}

void ScaleVertexBuffer(ColorVertex* vertices, int count, float x, float y)
{
    D3DXMATRIX ScalingMatrix;
    D3DXMatrixScaling(&ScalingMatrix, x, y, 1.f);

    ApplyVertexBufferTransformation(vertices, count, ScalingMatrix);
}

void SetVertexBufferColor(ColorVertex* vertices, int count, DWORD color)
{
    for (int i = 0; i < count; i++)
    {
        vertices[i].color = color;
    }
}

void CopyVertexBuffer(ColorVertex* source, ColorVertex* destination, int count)
{
    for (int i = 0; i < count; i++)
    {
        destination[i] = { D3DXVECTOR3(source[i].coords.x, source[i].coords.y, source[i].coords.z), source[i].rhw, source[i].color };
    }
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
        CopyVertexBuffer(this->BezelVertices, this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM);
        CopyVertexBuffer(this->BezelVertices, this->FinalBezels[i].BotVertices, BEZEL_VERT_NUM);

        CopyVertexBuffer(this->RectangleVertices, this->FinalRects[i].vertices, RECT_VERT_NUM);

        // Flip the top bezel
        RotateVertexBuffer(this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM, D3DX_PI);
        
        // Scaling
        ScaleVertexBuffer(this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM, xScaling, yScaling);
        ScaleVertexBuffer(this->FinalBezels[i].BotVertices, BEZEL_VERT_NUM, xScaling, yScaling);
        ScaleVertexBuffer(this->FinalRects[i].vertices, RECT_VERT_NUM, xScaling, yScaling);

        // Translating
        TranslateVertexBuffer(this->FinalRects[i].vertices, RECT_VERT_NUM, xOffset + ((float)i * spacing), yOffset);

        float DeltaX = this->FinalRects[i].vertices[0].coords.x - this->FinalBezels[i].BotVertices[3].coords.x;
        float DeltaY = this->FinalRects[i].vertices[0].coords.y - this->FinalBezels[i].BotVertices[3].coords.y;

        TranslateVertexBuffer(this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM, DeltaX, DeltaY);
        TranslateVertexBuffer(this->FinalBezels[i].BotVertices, BEZEL_VERT_NUM, DeltaX, DeltaY);
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
            SetVertexBufferColor(this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM, this->LightGoldBezel[selected]);
            SetVertexBufferColor(this->FinalBezels[i].BotVertices, BEZEL_VERT_NUM, this->DarkGoldBezel[selected]);
        }

        // Set inner rectangle color, based on the current value
        for (int i = 0; i < 0xF; i++)
        {
            if (value <= i)
                SetVertexBufferColor(this->FinalRects[i].vertices, RECT_VERT_NUM, this->InactiveGoldSquare[selected]);
            else
                SetVertexBufferColor(this->FinalRects[i].vertices, RECT_VERT_NUM, this->ActiveGoldSquare[selected]);
        }
    }
    else
    {
        for (int i = 0; i < 0xF; i++)
        {
            SetVertexBufferColor(this->FinalBezels[i].TopVertices, BEZEL_VERT_NUM, this->LightGrayBezel[selected]);
            SetVertexBufferColor(this->FinalBezels[i].BotVertices, BEZEL_VERT_NUM, this->DarkGrayBezel[selected]);
        }

        // Set inner rectangle color, based on the current value
        for (int i = 0; i < 0xF; i++)
        {
            if (value <= i)
                SetVertexBufferColor(this->FinalRects[i].vertices, RECT_VERT_NUM, this->InactiveGraySquare[selected]);
            else
                SetVertexBufferColor(this->FinalRects[i].vertices, RECT_VERT_NUM, this->ActiveGraySquare[selected]);
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
    {
        ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, this->FinalRects[i].vertices, 20);
    }

    ProxyInterface->SetRenderState(D3DRS_ALPHAREF, 2);
    ProxyInterface->SetRenderState(D3DRS_FOGENABLE, 1);
}

/*
Page : subpage menu
1-2 : 0     main options screen
2   : 1     game options screen
3   : 0     brightness adjust screen
7   : 0     advanced options screen
4   : 0     control options screen
*/

bool IsInMainOptionsMenu()
{
    return GetOptionsPage() == 0x02 && GetOptionsSubPage() == 0x00;
}

bool IsInOptionsMenu()
{
    return GetEventIndex() == EVENT_OPTIONS_FMV &&
        (GetOptionsPage() == 0x02 || GetOptionsPage() == 0x07 || GetOptionsPage() == 0x04) &&
        (GetOptionsSubPage() == 0x00 || GetOptionsSubPage() == 0x01);
}

bool IsInControlOptionsMenu()
{
    return IsInOptionsMenu() && GetOptionsPage() == 0x04;
}

void ButtonIcons::DrawIcons(LPDIRECT3DDEVICE8 ProxyInterface)
{
    if (!IsInControlOptionsMenu() || !ProxyInterface || ReplaceButtonText == BUTTON_ICONS_DISABLED)
    {
        return;
    }

    if (LastBufferHeight != BufferHeight || LastBufferWidth != BufferWidth || ButtonIconsTexture == NULL)
    {
        this->Init(ProxyInterface);
    }

    if (ButtonIconsTexture == NULL)
    {
        ReplaceButtonText = BUTTON_ICONS_DISABLED;
        Logging::Log() << __FUNCTION__ << " ERROR: Couldn't load button icons texture.";
        return;
    }

    ProxyInterface->SetRenderState(D3DRS_FOGENABLE, FALSE);
    ProxyInterface->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

    ProxyInterface->SetRenderState(D3DRS_ALPHAREF, 1);
    ProxyInterface->SetRenderState(D3DRS_LIGHTING, 0);
    ProxyInterface->SetRenderState(D3DRS_SPECULARENABLE, 0);
    ProxyInterface->SetRenderState(D3DRS_ZENABLE, 1);
    ProxyInterface->SetRenderState(D3DRS_ZWRITEENABLE, 1);
    ProxyInterface->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
    ProxyInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, 0);

    ProxyInterface->SetTextureStageState(0, D3DTSS_COLOROP, 1);
    ProxyInterface->SetTextureStageState(0, D3DTSS_ALPHAOP, 1);

    ProxyInterface->SetTextureStageState(1, D3DTSS_COLOROP, 1);
    ProxyInterface->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);
    
    ProxyInterface->SetTexture(0, 0);

    ProxyInterface->SetTransform(D3DTS_WORLDMATRIX(0x56), &WorldMatrix);
    
    ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, this->LineVertices[0], 20);
    ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, this->LineVertices[1], 20);
    
    ProxyInterface->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_TEX1);

    ProxyInterface->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    ProxyInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

    ProxyInterface->SetRenderState(D3DRS_FOGENABLE, FALSE);

    ProxyInterface->SetTextureStageState(0, D3DTSS_COLOROP, 1);
    ProxyInterface->SetTextureStageState(0, D3DTSS_ALPHAOP, 1);

    ProxyInterface->SetTextureStageState(1, D3DTSS_COLOROP, 1);
    ProxyInterface->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);

    ProxyInterface->SetTransform(D3DTS_WORLDMATRIX(0x56), &WorldMatrix);

    ProxyInterface->SetTexture(0, ButtonIconsTexture);

    D3DXVECTOR4 UnselectedSubtractionFactor(ControlOptionRedGreen, ControlOptionRedGreen, ControlOptionUnselectedBlue, 1.0f);
    D3DXVECTOR4 SelectedSubtractionFactor(ControlOptionRedGreen, ControlOptionRedGreen, ControlOptionSelectedBlue, 1.0f);
    D3DXVECTOR4 LockedSubtractionFactor(ControlOptionsLocked, ControlOptionsLocked, ControlOptionsLocked, 1.0f);
    D3DXVECTOR4 ChangingSubtractionFactor(ControlOptionsChangingRed, ControlOptionsChangingGreenBlue, ControlOptionsChangingGreenBlue, 1.0f);

    ProxyInterface->SetPixelShader(ModulationPixelShader);

    ProxyInterface->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    ProxyInterface->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    ProxyInterface->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

    ProxyInterface->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

    for (int i = 0; i < this->quadsNum; i++)
    {
        switch (this->quads[i].state)
        {
        case OptionState::LOCKED:
            ProxyInterface->SetPixelShaderConstant(0, &LockedSubtractionFactor.x, 1);
            break;

        case OptionState::SELECTED:
            ProxyInterface->SetPixelShaderConstant(0, &SelectedSubtractionFactor.x, 1);
            break;

        case OptionState::CHANGING:
            ProxyInterface->SetPixelShaderConstant(0, &ChangingSubtractionFactor.x, 1);
            break;

        default:
        case OptionState::STANDARD:
            ProxyInterface->SetPixelShaderConstant(0, &UnselectedSubtractionFactor.x, 1);
            break;
        }

        ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, this->quads[i].vertices, sizeof(TexturedVertex));

        // Draw Dpad arrows next to movement keys
        if (DPadMovementFix == 1 &&  this->quads[i].HasUlteriorQuad)
        {
            IconQuad UlteriorQuad;
            float vStarting = NULL;

            CopyVertexBuffer(this->quads[i].vertices, UlteriorQuad.vertices, 4);

            switch (this->quads[i].bind)
            {
            case ControllerButton::L_DOWN:
                vStarting = this->GetVStartingValue(ControllerButton::D_DOWN);
                break;

            case ControllerButton::L_LEFT:
                vStarting = this->GetVStartingValue(ControllerButton::D_LEFT);
                break;

            case ControllerButton::L_RIGHT:
                vStarting = this->GetVStartingValue(ControllerButton::D_RIGHT);
                break;

            case ControllerButton::L_UP:
            default:
                vStarting = this->GetVStartingValue(ControllerButton::D_UP);
                break;
            }

            UlteriorQuad.vertices[0].u = this->GetUStartingValue();
            UlteriorQuad.vertices[0].v = vStarting;

            UlteriorQuad.vertices[1].u = this->GetUStartingValue();
            UlteriorQuad.vertices[1].v = vStarting + this->GetVOffset();

            UlteriorQuad.vertices[2].u = this->GetUStartingValue() + this->GetUOffset();
            UlteriorQuad.vertices[2].v = vStarting + this->GetVOffset();

            UlteriorQuad.vertices[3].u = this->GetUStartingValue() + this->GetUOffset();
            UlteriorQuad.vertices[3].v = vStarting;

            float xOffset = floorf(this->quads[i].vertices[2].coords.x - this->quads[i].vertices[0].coords.x);

            TranslateVertexBuffer(UlteriorQuad.vertices, 4, xOffset, 0.f);

            ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, UlteriorQuad.vertices, sizeof(TexturedVertex));
        }
    }

    ProxyInterface->SetPixelShader(0);
    ProxyInterface->SetRenderState(D3DRS_ALPHAREF, 2);
    ProxyInterface->SetRenderState(D3DRS_FOGENABLE, 1);
}

void ButtonIcons::HandleControllerIcons(LPDIRECT3DDEVICE8 ProxyInterface)
{
    if (!IsInControlOptionsMenu() || ReplaceButtonText == BUTTON_ICONS_DISABLED)
    {
        if (ButtonIconsTexture != NULL)
        {
            ButtonIconsTexture->Release();
            ButtonIconsTexture = NULL;
        }

        return;
    }

    DirectXInterface = ProxyInterface;

    this->UpdateBinds();
}

void ButtonIcons::Init(LPDIRECT3DDEVICE8 ProxyInterface)
{
    if (!ProxyInterface || ReplaceButtonText == BUTTON_ICONS_DISABLED)
    {
        return;
    }

    if (ModulationPixelShader == NULL)
    {
        HRESULT hr = ProxyInterface->CreatePixelShader(ModulationPixelShaderAsm, &this->ModulationPixelShader);

        if (FAILED(hr))
        {
            Logging::Log() << __FUNCTION__ << " ERROR: Couldn't create pixel shader: " << Logging::hex(hr);
            return;
        }
    }

    if (ButtonIconsTexture == NULL)
    {
        char TexturePath[MAX_PATH];
        strcpy_s(TexturePath, MAX_PATH, "data\\pic\\etc\\");
        strcat_s(TexturePath, MAX_PATH, "controller_buttons_icons.png");

        char Filename[MAX_PATH];
        char* FinalPathChars = GetFileModPath(TexturePath, Filename);

        wchar_t FinalPath[MAX_PATH];
        mbstowcs(FinalPath, FinalPathChars, strlen(FinalPathChars) + 1);

        HRESULT hr = GfxCreateTextureFromFileW(ProxyInterface, (LPCWSTR)FinalPath, &ButtonIconsTexture, 0);

        if (FAILED(hr))
        {
            ReplaceButtonText = BUTTON_ICONS_DISABLED;
            Logging::Log() << __FUNCTION__ << " ERROR: Couldn't create texture: " << Logging::hex(hr);
            return;
        }
    }

    this->LastBufferHeight = BufferHeight;
    this->LastBufferWidth = BufferWidth;

    int32_t VerticalInternal = GetInternalVerticalRes();
    int32_t HorizontalInternal = GetInternalHorizontalRes();

    float UlteriorOffset = (BufferWidth - (float)HorizontalInternal) / 2;

    const float xScaling = (float)HorizontalInternal / 1200.f;
    const float yScaling = (float)VerticalInternal / 900.f;

    const float HorizontalOffset = (891.f * (float)HorizontalInternal) / 1200.f + UlteriorOffset;
    const float VerticalOffset = (141.f * (float)VerticalInternal) / 900.f;

    const float x = 76.f;
    const float y = 57.f;

    const float LineHorizontalOffset = ((600.f * (float)HorizontalInternal) / 1200.f) + UlteriorOffset;
    const float TopLineVerticalOffset = (90.f * (float)VerticalInternal) / 900.f;
    const float BottomLineVerticalOffset = (790.f * (float)VerticalInternal) / 900.f;

    CopyVertexBuffer(this->TemplateLineVertices, this->LineVertices[0], RECT_VERT_NUM);
    CopyVertexBuffer(this->TemplateLineVertices, this->LineVertices[1], RECT_VERT_NUM);

    ScaleVertexBuffer(this->LineVertices[0], RECT_VERT_NUM, xScaling, yScaling);
    ScaleVertexBuffer(this->LineVertices[1], RECT_VERT_NUM, xScaling, yScaling);

    TranslateVertexBuffer(this->LineVertices[0], RECT_VERT_NUM, LineHorizontalOffset, TopLineVerticalOffset);
    TranslateVertexBuffer(this->LineVertices[1], RECT_VERT_NUM, LineHorizontalOffset, BottomLineVerticalOffset);

    for (int i = 0; i < BUTTON_QUADS_NUM; i++)
    {
        const float xx = x * xScaling;
        const float yy = y * yScaling;
        const float vo = VerticalOffset + (i * y * yScaling);

        this->quads[i].vertices[0].coords = { floorf(HorizontalOffset), floorf(vo), 0.0f };
        this->quads[i].vertices[1].coords = { floorf(HorizontalOffset),  floorf(yy + vo), 0.0f };
        this->quads[i].vertices[2].coords = { floorf(xx + HorizontalOffset), floorf(yy + vo), 0.0f };
        this->quads[i].vertices[3].coords = { floorf(xx + HorizontalOffset), floorf(vo), 0.0f };
    }
}

void ButtonIcons::UpdateBinds()
{
    if (ReplaceButtonText == BUTTON_ICONS_DISABLED)
    {
        return;
    }

    if (!this->ControllerBindsAddr)
    {
        this->ControllerBindsAddr = GetKeyBindsPointer() + 0xD0;
    }

    this->binds[0] = ControllerButton::L_LEFT;
    this->binds[1] = ControllerButton::L_RIGHT;
    this->binds[2] = ControllerButton::L_UP;
    this->binds[3] = ControllerButton::L_DOWN;

    for (int i = 0; (i + 4) < this->BindsNum; i++)
    {
        // the first 4 keybinds are static, the movement stick
        this->binds[i + 4] = (ControllerButton) this->ControllerBindsAddr[i * 0x08];
        
        // Skip drawing dpad arrows, since they don't work with DPadMovementFix
        if (DPadMovementFix == 1 && this->binds[i + 4] >= ControllerButton::D_UP && this->binds[i + 4] <= ControllerButton::D_LEFT)
        {
            this->binds[i + 4] = ControllerButton::NO_BIND;
        }
    }

    ButtonIconsRef.UpdateUVs();
}

// Display mode
BYTE* DisplayModeArrowRetAddr = nullptr;
BYTE* DisplayModeValueHighlightRetAddr = nullptr;
BYTE* DisplayModeValuePrintRetAddr = nullptr;
BYTE* ConfirmInitialOptionValueRetAddr = nullptr;
BYTE* DiscardOptionValueRetAddr = nullptr;
BYTE* CheckStoredOptionvalueRetAddr = nullptr;
BYTE* DisplayModeOptionColorCheckRetAddr = nullptr;
BYTE* StoreInitialOptionValueRetAddr = nullptr;
BYTE* SetHighlightColorRetAddr = nullptr;
BYTE* InputConditionChangeRetAddr1 = nullptr;
BYTE* InputConditionChangeRetAddr2 = nullptr;
BYTE* InputConditionChangeRetAddr3 = nullptr;
BYTE* InputConditionChangeRetAddr4 = nullptr;
BYTE* InputConditionChangeRetAddr5 = nullptr;

BYTE* DisplayModeValueChangedRetAddr = nullptr;

BYTE* StoredDisplayModeValue = nullptr;

int8_t DisplayModeValue = 0; //TODO retrieve from stored option

__declspec(naked) void __stdcall DisplayModeArrow()
{
    __asm
    {
        movzx eax, byte ptr[DisplayModeValue]
        add eax, 0x100
        push eax
        jmp DisplayModeArrowRetAddr
    }
}

__declspec(naked) void __stdcall DisplayModeValueHighlight()
{
    __asm
    {
        movzx ax, byte ptr[DisplayModeValue]

        jmp DisplayModeValueHighlightRetAddr
    }
}

__declspec(naked) void __stdcall DisplayModeValuePrint()
{
    __asm
    {
        movzx ax, byte ptr[DisplayModeValue]

        jmp DisplayModeValuePrintRetAddr
    }
}

__declspec(naked) void __stdcall ConfirmInitialOptionValue()
{
    __asm
    {
        mov dl, byte ptr[DisplayModeValue]

        jmp ConfirmInitialOptionValueRetAddr
    }
}

__declspec(naked) void __stdcall DiscardInitialOptionValue()
{
    __asm
    {
        mov byte ptr[DisplayModeValue], cl

        jmp DiscardOptionValueRetAddr
    }
}

__declspec(naked) void __stdcall CheckStoredOptionValue()
{
    __asm
    {
        mov cl, byte ptr[DisplayModeValue]

        jmp CheckStoredOptionvalueRetAddr
    }
}

__declspec(naked) void __stdcall ColorCheckStoredOptionValue()
{
    __asm
    {
        mov dl, byte ptr[DisplayModeValue]

        jmp DisplayModeOptionColorCheckRetAddr
    }
}

__declspec(naked) void __stdcall StoreInitialOptionValue()
{
    *StoredDisplayModeValue = DisplayModeValue;

    __asm
    {
        jmp StoreInitialOptionValueRetAddr
    }
}

__declspec(naked) void __stdcall SetHighlightColor()
{
    __asm
    {
        mov cl, byte ptr[DisplayModeValue]

        jmp SetHighlightColorRetAddr
    }
}

__declspec(naked) void __stdcall IncrementDisplayModeValue()
{
    DisplayModeValue += DisplayModeValue != 0x02 ? 1 : -2;
    
    __asm
    {
        jmp DisplayModeValueChangedRetAddr
    }
}

__declspec(naked) void __stdcall DecrementDisplayModeValue()
{
    DisplayModeValue += DisplayModeValue != 0x00 ? -1 : 2;

    __asm
    {
        jmp DisplayModeValueChangedRetAddr
    }
}

__declspec(naked) void __stdcall ConditionChangeOne()
{
    __asm
    {
        test eax, eax

        jnz DecrementDisplayModeValue

        push 0x4

        jmp InputConditionChangeRetAddr1
    }
}

__declspec(naked) void __stdcall ConditionChangeTwo()
{
    __asm
    {
        test eax, eax
        jnz DecrementDisplayModeValue

        cmp edi, 0x1
        jz DecrementDisplayModeValue

        push ebp

        jmp InputConditionChangeRetAddr2
    }
}

__declspec(naked) void __stdcall ConditionChangeThree()
{
    __asm
    {
        jnz IncrementDisplayModeValue

        jmp InputConditionChangeRetAddr3
    }
}

__declspec(naked) void __stdcall ConditionChangeFour()
{
    __asm
    {
        test eax, eax
        jnz IncrementDisplayModeValue

        push 0x8

        jmp InputConditionChangeRetAddr4
    }
}

__declspec(naked) void __stdcall ConditionChangeFive()
{
    __asm
    {
        jmp IncrementDisplayModeValue
    }
}

void PatchDisplayMode()
{

    /*TODO
    * replace high res textures option description
    * store options in configdata
    * option for health indicator
    * set high res textures enabled 
    */

    BYTE* RenderRightArrowAddr = (BYTE*)0x00465bf4; //TODO address
    BYTE* HighResTextName1 = (BYTE*)0x00465060;
    BYTE* HighResTextName2 = (BYTE*)0x0046561c;
    BYTE* HighResTextValue1 = (BYTE*)0x0046525c;
    BYTE* HighResTextValue2 = (BYTE*)0x00465664;

    StoredDisplayModeValue = (BYTE*)0x00941784;

    BYTE* HighResTextArrow = (BYTE*)0x00465994;
    DisplayModeArrowRetAddr = HighResTextArrow + 0x16;
    BYTE* StringOffsetToNop = HighResTextArrow + 0x1C;

    BYTE* DisplayModeValueHighlightAddr = (BYTE*)0x00465656;
    DisplayModeValueHighlightRetAddr = DisplayModeValueHighlightAddr + 0x08;

    BYTE* DisplayModeValuePrintAddr = (BYTE*)0x00465240;
    DisplayModeValuePrintRetAddr = DisplayModeValuePrintAddr + 0x08;

    BYTE* ConfirmInitialOptionValueAddr = (BYTE*)0x00464e11;
    ConfirmInitialOptionValueRetAddr = ConfirmInitialOptionValueAddr + 0x06;
    
    BYTE* DiscardOptionValueAddr = (BYTE*)0x00464f39;
    DiscardOptionValueRetAddr = DiscardOptionValueAddr + 0x06;

    BYTE* CheckStoredOptionvalueAddr = (BYTE*)0x00464ba6;
    CheckStoredOptionvalueRetAddr = CheckStoredOptionvalueAddr + 0x06;

    BYTE* DisplayModeOptionColorCheckAddr = (BYTE*)0x00465221;
    DisplayModeOptionColorCheckRetAddr = DisplayModeOptionColorCheckAddr + 0x06;

    BYTE* StoreInitialOptionValueAddr = (BYTE*)0x00462d07;
    StoreInitialOptionValueRetAddr = StoreInitialOptionValueAddr + 0x06;
    BYTE* NopOriginalStoreOptionAddr = StoreInitialOptionValueAddr + 0x21;

    BYTE* SetHighlightColorAddr = (BYTE*)0x00465649;
    SetHighlightColorRetAddr = SetHighlightColorAddr + 0x06;

    BYTE* InputConditionChangeAddr1 = (BYTE*)0x465e80; //TODO address
    InputConditionChangeRetAddr1 = InputConditionChangeAddr1 + 0x06;

    BYTE* InputConditionChangeAddr2 = InputConditionChangeAddr1 + 0x0E;
    InputConditionChangeRetAddr2 = InputConditionChangeAddr2 + 0x05;

    BYTE* InputConditionChangeAddr3 = InputConditionChangeAddr1 + 0x31;
    InputConditionChangeRetAddr3 = InputConditionChangeAddr3 + 0x07;

    BYTE* InputConditionChangeAddr4 = InputConditionChangeAddr1 + 0x21;
    InputConditionChangeRetAddr4 = InputConditionChangeAddr4 + 0x06;

    BYTE* InputConditionChangeAddr5 = InputConditionChangeAddr1 + 0x41;
    InputConditionChangeRetAddr5 = InputConditionChangeAddr5 + 0x05;

    DisplayModeValueChangedRetAddr = (BYTE*)0x4661af; //TODO address

    /*
    * 0x0FE display mode
    * 0x0FF change the display mode
    * 0x100 windowed 
    * 0x101 FS windowed 
    * 0x102 Full Screen
    */

    // Display mode option name offset
    UpdateMemoryAddress(HighResTextName1, "\xFE", 0x01);
    UpdateMemoryAddress(HighResTextName2, "\xFE", 0x01);
    // Display mode option value offset
    UpdateMemoryAddress(HighResTextValue1, "\x00\x01", 0x02);
    UpdateMemoryAddress(HighResTextValue2, "\x00\x01", 0x02);

    // Display mode option arrow
    WriteJMPtoMemory(HighResTextArrow, DisplayModeArrow, 15);
    UpdateMemoryAddress(StringOffsetToNop, "\x90\x90\x90\x90\x90", 0x05);
    // Display mode option value and highlight
    WriteJMPtoMemory(DisplayModeValueHighlightAddr, DisplayModeValueHighlight, 0x08); //TODO highlight wrong color, tied to inputs
    WriteJMPtoMemory(SetHighlightColorAddr, SetHighlightColor, 0x06);
    WriteJMPtoMemory(DisplayModeValuePrintAddr, DisplayModeValuePrint, 0x08);

    // Override checks for changed option
    WriteJMPtoMemory(ConfirmInitialOptionValueAddr, ConfirmInitialOptionValue, 0x06);
    WriteJMPtoMemory(DiscardOptionValueAddr, DiscardInitialOptionValue, 0x06);
    WriteJMPtoMemory(CheckStoredOptionvalueAddr, CheckStoredOptionValue, 0x06);
    WriteJMPtoMemory(DisplayModeOptionColorCheckAddr, ColorCheckStoredOptionValue, 0x06);

    // Divert saving the option value for checks
    WriteJMPtoMemory(StoreInitialOptionValueAddr, StoreInitialOptionValue, 0x06);
    UpdateMemoryAddress(NopOriginalStoreOptionAddr, "\x90\x90\x90\x90\x90\x90", 0x06);

    // Change the conditional that changes the value
    WriteJMPtoMemory(InputConditionChangeAddr1, ConditionChangeOne, 0x06);
    WriteJMPtoMemory(InputConditionChangeAddr2, ConditionChangeTwo, 0x05);
    WriteJMPtoMemory(InputConditionChangeAddr3, ConditionChangeThree, 0x07);
    WriteJMPtoMemory(InputConditionChangeAddr4, ConditionChangeFour, 0x06);
    WriteJMPtoMemory(InputConditionChangeAddr5, ConditionChangeFive, 0x05);
}

void HandleDisplayMode() //TODO needed?
{
    //TODO remove
    AuxDebugOvlString = "\rDisplay mode value: ";
    AuxDebugOvlString.append(std::to_string(DisplayModeValue));
}

// Health indicator option


void PatchHealthIndicatorOption()
{

}
/**
* Copyright (C) 2026 Murugo
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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "ActorDrawingHook.h"
#include "Common\FileSystemHooks.h"
#include "Common\ModelGLTF.h"
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"
#include "Patches.h"

#include <sstream>
#include <filesystem>

static ModelGLTF*               gMdrModel = nullptr;
static std::filesystem::path    gModelPath;
static LARGE_INTEGER            gQPCFreq = {};
static bool                     gActive = false;
static double                   gStartTime = 0.0;
static float                    gModelAnimTimer = 0.0f;

DWORD gMdrVSShader = 0;
DWORD gMdrPSShader = 0;

void* jmpSkipBlendDrawReturnAddr = nullptr;

static D3DXMATRIX* gViewTransform = reinterpret_cast<D3DXMATRIX*>(0x1F7D530);  // TODO: Needs 1.1 and DC addresses?

static ModelGLTF* GetOrCreateModel(IDirect3DDevice8* device) {
    if (!gMdrModel && !gModelPath.empty()) {
        gMdrModel = new ModelGLTF(ModelGLTF::VertexType::PosNormalTexcoord, false);
        if (!gMdrModel->LoadFromFile(gModelPath.u8string(), device)) {
            delete gMdrModel;
            gMdrModel = nullptr;
        }
    }

    return gMdrModel;
}

void RunMariaCutsceneModel() {
    // Pre-load replacement model during room transition before cutscene starts
    if (GetEventIndex() == 3 && GetRoomID() == R_HTL_ALT_RPT_BOSS_RM) {
        GetOrCreateModel(GetD3dDevice());
    }
}

static void DrawMariaModel(IDirect3DDevice8* device) {
    const double timeNow = GetCutsceneTimer() / 30.0f;
    const double timeDelta = static_cast<float>(timeNow - gStartTime);
    gStartTime = timeNow;

    ModelGLTF* model = GetOrCreateModel(device);
    if (!model) {
        return;
    }

    D3DXMATRIX dmrXForm;
    D3DXMatrixIdentity(&dmrXForm);

    D3DXMATRIX actorXForm;
    D3DXMatrixMultiply(&actorXForm, &dmrXForm, gViewTransform);

    model->Update(static_cast<float>(timeDelta), actorXForm, &gModelAnimTimer);

    DWORD backCulling = 0;
    device->GetRenderState(D3DRS_CULLMODE, &backCulling);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

    DWORD savedPS, savedVS;
    device->GetPixelShader(&savedPS);
    device->GetVertexShader(&savedVS);

    device->SetPixelShader(gMdrPSShader);
    device->SetVertexShader(gMdrVSShader);

    model->DrawWithBlendPass(device);

    device->SetPixelShader(savedPS);
    device->SetVertexShader(savedVS);

    device->SetRenderState(D3DRS_CULLMODE, backCulling);
}

constexpr ModelOffsetTable kDmrModelTable = { -65533, 4, 176, 11, 880, 7, 896, 912, 22, 1952, 6, 32512, 4, 1360, 18, 1376, 1520, 0, 1808, 0 };

bool IsDmrModel(ModelOffsetTable* model) {
    return *model == kDmrModelTable;
}

// Skips drawing translucent meshes from dmr.mdl
__declspec(naked) void __stdcall SkipBlendDrawASM() {
    __asm {
        push esi
        call IsDmrModel
        add esp, 0x04
        test al, al
        jz ExitASM
        pop ebx
        add esp, 0x08
        ret

        ExitASM :
        mov ebx, dword ptr ds : [esi + 0x2C]
            push ebp
            mov ebp, dword ptr ds : [esi + 0x28]
            jmp jmpSkipBlendDrawReturnAddr
    }
}

void PatchMariaCutsceneModel() {
    gModelPath = GetModPath("");
    gModelPath = gModelPath / R"(model\dmr.glb)";
    std::error_code errorCode{};
    if (!std::filesystem::exists(gModelPath, errorCode)) {
        gModelPath.clear();
        return;
    }

    constexpr BYTE SearchBytes[]{ 0x83, 0xEC, 0x08, 0x53, 0x8B, 0x5E, 0x2C };
    DWORD SkipBlendDrawInjectAddr = SearchAndGetAddresses(0x00504E00, 0x00505130, 0x00504A50, SearchBytes, sizeof(SearchBytes), 0x04, __FUNCTION__);
    if (!SkipBlendDrawInjectAddr) {
        Logging::Log() << __FUNCTION__ << "Error: failed to find memory address!";
        return;
    }
    jmpSkipBlendDrawReturnAddr = (void*)(SkipBlendDrawInjectAddr + 0x07);
    WriteJMPtoMemory((BYTE*)SkipBlendDrawInjectAddr, SkipBlendDrawASM, 0x07);

    RegisterActorDrawTopPrologue([](ModelOffsetTable* model, void* /*arg2*/)->bool {
        if (GetCutsceneID() == CS_HTL_ALT_RPT_BOSS_INTRO) {
            if (IsDmrModel(model)) {
                if (!gActive) {
                    gStartTime = GetCutsceneTimer() / 30.0f;
                    gModelAnimTimer = gStartTime;
                    gActive = true;
                }
                return true;
            }
        }
        else {
            gActive = false;
        }
        // do not skip other stuff
        return false;
        });

    RegisterActorDrawTopEpilogue([](ModelOffsetTable* model, void* /*arg2*/)->bool {
        if (GetCutsceneID() == CS_HTL_ALT_RPT_BOSS_INTRO && IsDmrModel(model)) {
            IDirect3DDevice8* device = GetD3dDevice();
            if (device) {
                DrawMariaModel(device);
            }
        }
        // return false to not skip the actual draw
        return false;
        });
}

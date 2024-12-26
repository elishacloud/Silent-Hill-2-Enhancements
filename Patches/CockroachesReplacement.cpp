#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Wrappers/d3d8/DirectX81SDK/include/d3d8.h"
#include "Patches.h"
#include "Common/ModelGLTF.h"
#include "Common/FileSystemHooks.h"

#include <random>
#include <filesystem>

std::filesystem::path               gModelPath;
static LARGE_INTEGER                gQPCFreq = {};
static ModelGLTF*                   gCocroachesModel = nullptr;
static float                        gUpdateTimers[32] = {};

// mov     eax, 0x00A3F5AC
// ...
// fmul    dword ptr [eax-1Ch]
D3DXMATRIX* gActorTransformMatrix = reinterpret_cast<D3DXMATRIX*>(0xA3F590);  // 0x00A3F5AC - 0x1C

void PatchCockroachesReplacement() {
    gModelPath = GetModPath("");
    gModelPath = gModelPath / R"(model\ty3.glb)";
    std::error_code errorCode{};
    if (!std::filesystem::exists(gModelPath, errorCode)) {
        gModelPath.clear();
        return;
    }

    // push 0C0A00000h
    constexpr BYTE PushRoachesJiggle0ValueSearchBytes[]{ 0x68, 0x00, 0x00, 0xA0, 0xC0 };
    DWORD PushRoachesJiggle0ValueAddr = SearchAndGetAddresses(0x004AE37C, 0x004AE37C, 0x004AE37C, PushRoachesJiggle0ValueSearchBytes, sizeof(PushRoachesJiggle0ValueSearchBytes), 0x00, __FUNCTION__);
    if(!PushRoachesJiggle0ValueAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address (roach0)!";
        return;
    }

    // push 03DCCCCCDh
    constexpr BYTE PushRoachesJiggle1ValueSearchBytes[]{ 0x68, 0xCD, 0xCC, 0xCC, 0x3D };
    DWORD PushRoachesJiggle1ValueAddr = SearchAndGetAddresses(0x004AE3BB, 0x004AE3BB, 0x004AE3BB, PushRoachesJiggle1ValueSearchBytes, sizeof(PushRoachesJiggle1ValueSearchBytes), 0x00, __FUNCTION__);
    if(!PushRoachesJiggle1ValueAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address (roach1)!";
        return;
    }

    Logging::Log() << "Patching roaches jiggle...";

    constexpr BYTE PushRoachesJiggleNULLValue[]{ 0x68, 0x00, 0x00, 0x00, 0x00 };
    UpdateMemoryAddress((void*)PushRoachesJiggle0ValueAddr, PushRoachesJiggleNULLValue, sizeof(PushRoachesJiggleNULLValue));
    UpdateMemoryAddress((void*)PushRoachesJiggle1ValueAddr, PushRoachesJiggleNULLValue, sizeof(PushRoachesJiggleNULLValue));

    // iOrange - randomize updateTimer so that our bugs have visually different movements
    std::random_device rnd;
    std::mt19937 rndEngine(rnd());
    std::uniform_real_distribution<float> rndDist(0.0f, 8.0f);
    const size_t numTimers = std::size(gUpdateTimers);
    for (size_t i = 0; i < numTimers; ++i) {
        gUpdateTimers[i] = rndDist(rndEngine);
    }
}

#pragma warning (suppress : 4505)
static double TimeGetNowSec() {
    if (!gQPCFreq.QuadPart) {
        ::QueryPerformanceFrequency(&gQPCFreq);
    }

    LARGE_INTEGER qpcNow = {};
    ::QueryPerformanceCounter(&qpcNow);
    return static_cast<double>(qpcNow.QuadPart) / static_cast<double>(gQPCFreq.QuadPart);
}

static ModelGLTF* GetOrCreateModel(IDirect3DDevice8* device) {
    if (!gCocroachesModel && !gModelPath.empty()) {
        gCocroachesModel = new ModelGLTF(ModelGLTF::VertexType::PosNormalTexcoord, false);
        if (!gCocroachesModel->LoadFromFile(gModelPath.u8string(), device)) {
            delete gCocroachesModel;
            gCocroachesModel = nullptr;
        }
    }

    return gCocroachesModel;
}

HRESULT DrawCockroachesReplacement_DIP(int* counter, float deltaTime, LPDIRECT3DDEVICE8 ProxyInterface, D3DPRIMITIVETYPE Type, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
    UNREFERENCED_PARAMETER(MinVertexIndex);
    UNREFERENCED_PARAMETER(startIndex);

    HRESULT result = (HRESULT)-1;
    if (gModelPath.empty()) {
        return result;
    }

    const DWORD roomID = GetRoomID();

    if (roomID == R_BUG_RM) {
        const bool isThisTy3Body = (Type == D3DPT_TRIANGLESTRIP && NumVertices == 161 && primCount == 336);
        const bool isThisTy2Body = (Type == D3DPT_TRIANGLESTRIP && NumVertices == 23 && primCount == 48);
        const bool isThisTy3Shadow = (Type == D3DPT_TRIANGLESTRIP && NumVertices == 189 && primCount == 560);

        if (isThisTy3Body || isThisTy2Body) {
            ModelGLTF* model = GetOrCreateModel(ProxyInterface);
            if (!model) {
                return result;
            }

            D3DXMATRIX actorXForm;
            D3DXMatrixTranspose(&actorXForm, gActorTransformMatrix);

            model->Update(deltaTime, actorXForm, &gUpdateTimers[*counter]);
            counter[0]++;

            result = model->Draw(ProxyInterface);
        } else if (isThisTy3Shadow) {
            // just ignore it
            result = S_OK;
        }
    }

    return result;
}

HRESULT DrawCockroachesReplacement_DP(LPDIRECT3DDEVICE8 ProxyInterface, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
    HRESULT result = (HRESULT)-1;
    if (gModelPath.empty()) {
        return result;
    }

    const DWORD roomID = GetRoomID();

    if (roomID == R_BUG_RM) {
        if (PrimitiveType == D3DPT_TRIANGLELIST && StartVertex == 0) {
            if (PrimitiveCount == 256 || PrimitiveCount == 160) { // ty3 bug limbs
                result = S_OK;
            } else if (PrimitiveCount == 96) { // ty2 bug limbs
                IUnknownPtr<IDirect3DTexture8> texture;
                ProxyInterface->GetTexture(0, (IDirect3DBaseTexture8**)texture.ReleaseAndGetAddressOf());

                D3DSURFACE_DESC desc;
                texture->GetLevelDesc(0, &desc);

                if (desc.Format == D3DFMT_DXT3) { // the animated model texture is DXT5
                    result = S_OK;
                }
            }
        }
    }

    return result;
}

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Patches.h"
#include "Common\IUnknownPtr.h"
#include "Common\Settings.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"
#include "Wrappers\d3d8\DirectX81SDK\include\d3dx8math.h"
#include "Common/ModelGLTF.h"
#include "Common/FileSystemHooks.h"
#include "ActorDrawingHook.h"

#include <sstream>
#include <filesystem>

static IDirect3DDevice8* (__cdecl* SH2_GetD3dDevice)() = (IDirect3DDevice8* (__cdecl*)())(0x4F5480);

static ModelGLTF*               gLeversModel = nullptr;
static std::filesystem::path    gModelPath;
static LARGE_INTEGER            gQPCFreq = {};
static double                   gStartTime = 0.0;

static ModelGLTF* GetOrCreateModel(IDirect3DDevice8* device) {
    if (!gLeversModel && !gModelPath.empty()) {
        gLeversModel = new ModelGLTF(ModelGLTF::VertexType::PosNormalTexcoord, false);
        if (!gLeversModel->LoadFromFile(gModelPath.u8string(), device)) {
            delete gLeversModel;
            gLeversModel = nullptr;
        }
    }

    return gLeversModel;
}

static double TimeGetNowSec() {
    if (!gQPCFreq.QuadPart) {
        ::QueryPerformanceFrequency(&gQPCFreq);
    }

    LARGE_INTEGER qpcNow = {};
    ::QueryPerformanceCounter(&qpcNow);
    return static_cast<double>(qpcNow.QuadPart) / static_cast<double>(gQPCFreq.QuadPart);
}

static void DrawLeversModel(IDirect3DDevice8* device) {
    if (!gStartTime) {
        gStartTime = TimeGetNowSec();
    }
    const double timeNow = TimeGetNowSec();
    const double timeDelta = static_cast<float>(timeNow - gStartTime);
    gStartTime = timeNow;

    ModelGLTF* model = GetOrCreateModel(device);
    if (!model) {
        return;
    }

    D3DXMATRIX actorXForm;
    D3DXMatrixIdentity(&actorXForm);

    model->Update(timeDelta, actorXForm);

    DWORD alphaBlend, alphaTest = 0;
    device->GetRenderState(D3DRS_ALPHABLENDENABLE, &alphaBlend);
    device->GetRenderState(D3DRS_ALPHATESTENABLE, &alphaTest);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    // Use texture color and alpha
    DWORD colorArg[3], colorOp, alphaArg[3], alphaOp, tcIdx;
    device->GetTextureStageState(0, D3DTSS_COLORARG0, &colorArg[0]);
    device->GetTextureStageState(0, D3DTSS_COLORARG1, &colorArg[1]);
    device->GetTextureStageState(0, D3DTSS_COLORARG2, &colorArg[2]);
    device->GetTextureStageState(0, D3DTSS_COLOROP, &colorOp);
    device->GetTextureStageState(0, D3DTSS_ALPHAARG0, &alphaArg[0]);
    device->GetTextureStageState(0, D3DTSS_ALPHAARG1, &alphaArg[1]);
    device->GetTextureStageState(0, D3DTSS_ALPHAARG2, &alphaArg[2]);
    device->GetTextureStageState(0, D3DTSS_ALPHAOP, &alphaOp);

    device->GetTextureStageState(0, D3DTSS_TEXCOORDINDEX, &tcIdx);
    device->SetTextureStageState(0, D3DTSS_COLORARG0, D3DTA_CURRENT);
    device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
    device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG0, D3DTA_CURRENT);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
    device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);

    DWORD savedPS, savedVS;
    device->GetPixelShader(&savedPS);
    device->GetVertexShader(&savedVS);

    device->SetPixelShader(0u);
    device->SetVertexShader(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);

    model->Draw(device, TRUE);

    device->SetPixelShader(savedPS);
    device->SetVertexShader(savedVS);

    device->SetTextureStageState(0, D3DTSS_COLORARG0, colorArg[0]);
    device->SetTextureStageState(0, D3DTSS_COLORARG1, colorArg[1]);
    device->SetTextureStageState(0, D3DTSS_COLORARG2, colorArg[2]);
    device->SetTextureStageState(0, D3DTSS_COLOROP, colorOp);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG0, alphaArg[0]);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG1, alphaArg[1]);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG2, alphaArg[2]);
    device->SetTextureStageState(0, D3DTSS_ALPHAOP, alphaOp);
    device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, tcIdx);

    device->SetRenderState(D3DRS_ALPHABLENDENABLE, alphaBlend);
    device->SetRenderState(D3DRS_ALPHATESTENABLE, alphaTest);
}

constexpr ModelOffsetTable kLeversModelTable = { -65533, 4, 176, 3, 368, 0, 384, 384, 4, 640, 0, 1952, 1, 384, 4, 400, 432, 0, 496, 0 };
constexpr ModelOffsetTable kDogModelTable = { -65533, 4, 176, 31, 2160, 43, 2192, 2288, 3, 5248, 0, 21792, 2, 5040, 2, 5056, 5072, 0, 5104, 0 };

void PatchDogRoom() {
    RegisterActorDrawTopEpilogue([](ModelOffsetTable* model, void* /*arg2*/)->bool {
        const DWORD roomID = GetRoomID();
        if (roomID == R_END_DOG_RM) {
            if (*model == kDogModelTable) {
                IDirect3DDevice8* device = SH2_GetD3dDevice();
                if (device) {
                    DrawLeversModel(device);
                }
            }
        }

        // return false to not skip the actual draw
        return(false);
    });

    gModelPath = GetModPath("");
    gModelPath = gModelPath / R"(model\mon.glb)";
    std::error_code errorCode{};
    if (!std::filesystem::exists(gModelPath, errorCode)) {
        gModelPath.clear();
        return;
    }
}

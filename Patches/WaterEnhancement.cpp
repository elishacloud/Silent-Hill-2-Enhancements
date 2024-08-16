#include <Windows.h>

#include "Common\Settings.h"
#include "Common\Utils.h"
#include "Common\IUnknownPtr.h"
#include "Common\GfxUtils.h"
#include "Patches.h"
#include "Wrappers\d3d8\DirectX81SDK\include\d3d8.h"
#include "Wrappers\d3d8\DirectX81SDK\include\d3dx8math.h"

#include <cmath>

#include "WaterEnhancement_dudv.h"

DWORD vsDeclWater[] = {
    D3DVSD_STREAM(0),
    D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3),
    D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR),
    D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2),
    D3DVSD_END()
};


// eax == 3
//eax, dword ptr[eax * 4 + 1DB9288h]
DWORD* g_vsHandles_1DB9288 = reinterpret_cast<DWORD*>(0x1DB9288);

#define WATER_VSHADER_ORIGINAL  (g_vsHandles_1DB9288[3])
#define WATER_FVF               (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

#define WATER_TEXTURE_SLOT_REFRACTION   1
#define WATER_TEXTURE_SLOT_DUDV         2

/*
vs.1.1

m4x4 r0, v0, c32
mov oPos, r0
mov oT0, v7
add oT1, v7, c90
mov oT2, r0
mov oD0, v5
*/
DWORD g_WaterVSBytecode[] = {
    0xfffe0101, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000014,
    0x800f0000, 0x90e40000, 0xa0e40020, 0x00000001,
    0xc00f0000, 0x80e40000, 0x00000001, 0xe00f0000,
    0x90e40007, 0x00000002, 0xe00f0001, 0x90e40007,
    0xa0e4005a, 0x00000001, 0xe00f0002, 0x80e40000,
    0x00000001, 0xd00f0000, 0x90e40005, 0x0000ffff
};

/*
ps.1.4
def c0, 0.5, 0.5, 1, 1
def c1, 0.5, -0.5, 1, 1
def c2, 0.01, 0.01, 0.01, 0.01

// calc projected uv
texcrd r3.xy, t2_dw.xyw

// save aside base texture coords
texcrd r4.xyz, t0

// sample dudv map, restore and rescale
texld r2, t1
mul r2, c2, r2_bx2

// fill the void
mov r3.zw, c0
mov r4.zw, c0

// [-1;1] -> [0;1]
mad r3, r3, c1, c0
// disturb projected uv
add r3, r3, r2
// pass along the disturbed base texture coords
add r2, r4, r2

phase

// sample water texture
texld r0, r2
// sample refraction texture
texld r1, r3
// tint water texture by the vertex colour
mul r0, r0, v0
// blend them
lrp_sat r0, r0.w, r0, r1
// fill alpha
mov r0.w, c0
*/
DWORD g_WaterPSBytecode[] = {
    0xffff0104, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000051,
    0xa00f0000, 0x3f000000, 0x3f000000, 0x3f800000,
    0x3f800000, 0x00000051, 0xa00f0001, 0x3f000000,
    0xbf000000, 0x3f800000, 0x3f800000, 0x00000051,
    0xa00f0002, 0x3c23d70a, 0x3c23d70a, 0x3c23d70a,
    0x3c23d70a, 0x00000051, 0xa00f0003, 0x00000000,
    0x3f54fdf4, 0x3f0e00d2, 0x00000000, 0x00000051,
    0xa00f0004, 0x00000000, 0x3f800000, 0x00000000,
    0x00000000, 0x00000040, 0x80030003, 0xbaf40002,
    0x00000040, 0x80070004, 0xb0e40000, 0x00000042,
    0x800f0002, 0xb0e40001, 0x00000005, 0x800f0002,
    0xa0e40002, 0x84e40002, 0x00000001, 0x800c0003,
    0xa0e40000, 0x00000001, 0x800c0004, 0xa0e40000,
    0x00000004, 0x800f0003, 0x80e40003, 0xa0e40001,
    0xa0e40000, 0x00000002, 0x800f0003, 0x80e40003,
    0x80e40002, 0x00000002, 0x800f0002, 0x80e40004,
    0x80e40002, 0x0000fffd, 0x00000042, 0x800f0000,
    0x80e40002, 0x00000042, 0x800f0001, 0x80e40003,
    0x00000005, 0x800f0000, 0x80e40000, 0x90e40000,
    0x00000012, 0x801f0000, 0x80ff0000, 0x80e40000,
    0x80e40001, 0x00000001, 0x80080000, 0xa0e40000,
    0x0000ffff
};

DWORD g_WaterVSHandle = 0;
DWORD g_WaterPSHandle = 0;

IDirect3DTexture8* g_ScreenCopyTexture = nullptr;
IDirect3DSurface8* g_ScreenCopySurface = nullptr;
IDirect3DTexture8* g_DuDvTexture = nullptr;

LARGE_INTEGER      g_QPCFreq = {};
uint64_t           g_StartTimeMS = 0;

static uint64_t TimeGetNowMS() {
    if (!g_QPCFreq.QuadPart) {
        ::QueryPerformanceFrequency(&g_QPCFreq);
    }

    LARGE_INTEGER qpcNow = {};
    ::QueryPerformanceCounter(&qpcNow);
    return (qpcNow.QuadPart * 1000) / g_QPCFreq.QuadPart;
}

template <typename T>
static void SafeRelease(T*& ptr) {
    if (ptr) {
        ptr->Release();
        ptr = nullptr;
    }
}

D3DSURFACE_DESC g_CachedBackbufferSurfaceDesc = {};
static bool CompareSurfaceDescs(const D3DSURFACE_DESC& a, const D3DSURFACE_DESC& b) {
    return a.Format == b.Format && a.Width == b.Width && a.Height == b.Height;
}

static void WaterEnhancedGrabScreen(LPDIRECT3DDEVICE8 Device) {
    IUnknownPtr<IDirect3DSurface8> backBufferSurface;
    HRESULT hr = Device->GetRenderTarget(backBufferSurface.ReleaseAndGetAddressOf());
    if (SUCCEEDED(hr)) {
        D3DSURFACE_DESC desc = {};
        backBufferSurface->GetDesc(&desc);

        if (!g_ScreenCopySurface || !CompareSurfaceDescs(g_CachedBackbufferSurfaceDesc, desc)) {
            memcpy(&g_CachedBackbufferSurfaceDesc, &desc, sizeof(desc));

            SafeRelease(g_ScreenCopySurface);
            SafeRelease(g_ScreenCopyTexture);

            hr = Device->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &g_ScreenCopyTexture);
            if (FAILED(hr)) {
                return;
            }

            hr = g_ScreenCopyTexture->GetSurfaceLevel(0, &g_ScreenCopySurface);
            if (FAILED(hr)) {
                SafeRelease(g_ScreenCopySurface);
                SafeRelease(g_ScreenCopyTexture);
                return;
            }

            D3DSURFACE_DESC newDesc = {};
            g_ScreenCopySurface->GetDesc(&newDesc);

            newDesc.Pool = newDesc.Pool;
        }

        if (g_ScreenCopySurface) {
            hr = Device->CopyRects(backBufferSurface.GetPtr(), nullptr, 0, g_ScreenCopySurface, nullptr);
        }
    }
}

void WaterEnhancedReleaseScreenCopy() {
    SafeRelease(g_ScreenCopySurface);
    SafeRelease(g_ScreenCopyTexture);
}

static void LoadDUDV(LPDIRECT3DDEVICE8 Device) {
    if (g_DuDvTexture) {
        return;
    }

    HRESULT hr = GfxCreateTextureFromFileInMem(Device, DuDv_128x128_data, sizeof(DuDv_128x128_data), &g_DuDvTexture);
    if (FAILED(hr)) {
        g_DuDvTexture = nullptr;
    }
}

static float GetFracPart(float f) {
    return f - std::floorf(f);
}

static void SaveTextureStates(LPDIRECT3DDEVICE8 Device, const DWORD stage, DWORD* saveTo) {
    Device->GetTextureStageState(stage, D3DTSS_ADDRESSU, &saveTo[0]);
    Device->GetTextureStageState(stage, D3DTSS_ADDRESSV, &saveTo[1]);
    Device->GetTextureStageState(stage, D3DTSS_MAGFILTER, &saveTo[2]);
    Device->GetTextureStageState(stage, D3DTSS_MINFILTER, &saveTo[3]);
    Device->GetTextureStageState(stage, D3DTSS_MIPFILTER, &saveTo[4]);
}

static void RestoreTextureStates(LPDIRECT3DDEVICE8 Device, const DWORD stage, const DWORD* loadFrom) {
    Device->SetTextureStageState(stage, D3DTSS_ADDRESSU, loadFrom[0]);
    Device->SetTextureStageState(stage, D3DTSS_ADDRESSV, loadFrom[1]);
    Device->SetTextureStageState(stage, D3DTSS_MAGFILTER, loadFrom[2]);
    Device->SetTextureStageState(stage, D3DTSS_MINFILTER, loadFrom[3]);
    Device->SetTextureStageState(stage, D3DTSS_MIPFILTER, loadFrom[4]);
}

static bool CheckWaterPrimitivesCountByRoom(const UINT PrimitiveCount) {
    bool isWater = false;

    const DWORD roomID = GetRoomID();

    switch (roomID) {
        // Pyramidhead submerge
        case R_APT_W_STAIRCASE_N:
            isWater = (PrimitiveCount == 38u);
        break;
        // Strange Area 2
        case R_STRANGE_AREA_2_B:
            isWater = (PrimitiveCount == 60u || PrimitiveCount == 32u || PrimitiveCount == 12u);
        break;
        // Labyrinth West
        case R_LAB_BOTTOM_C:
        case R_LAB_BOTTOM_E:
        case R_LAB_BOTTOM_F:
        case R_LAB_BOTTOM_G:
        case R_LAB_BOTTOM_H:
        case R_LAB_BOTTOM_I:
            isWater = (PrimitiveCount <= 46u && PrimitiveCount >= 10u) && PrimitiveCount != 22u; // excluding muzzle
        break;
        // Hotel Alternate Basement
        case R_HTL_ALT_EMPLOYEE_STAIRS:
        case R_HTL_ALT_BAR:
        case R_HTL_ALT_BAR_KITCHEN:
        case R_HTL_ALT_ELEVATOR:
        case R_HTL_ALT_EMPLOYEE_HALL_BF:
            isWater = (PrimitiveCount <= 82u && PrimitiveCount >= 16u) && PrimitiveCount != 22u; // excluding muzzle
        break;
        // Hotel Alternate 1F
        case R_FINAL_BOSS_RM:
            isWater = (PrimitiveCount == 56u);
        break;
    }

    return isWater;
}

HRESULT DrawWaterEnhanced(bool needToGrabScreenForWater, LPDIRECT3DDEVICE8 Device, D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride) {
    // looks like water
    if (PrimitiveType == D3DPT_TRIANGLESTRIP && CheckWaterPrimitivesCountByRoom(PrimitiveCount) && VertexStreamZeroStride == 24u && pVertexStreamZeroData != nullptr) {
        DWORD currVS = 0u;
        Device->GetVertexShader(&currVS);
        DWORD currPS = 0u;
        Device->GetPixelShader(&currPS);

        if ((currVS == WATER_VSHADER_ORIGINAL || currVS == WATER_FVF) && currPS == 0u && g_WaterVSHandle != 0u && g_WaterPSHandle != 0u) {
            if (needToGrabScreenForWater) {
                WaterEnhancedGrabScreen(Device);
            }
            LoadDUDV(Device);

            if (!g_StartTimeMS) {
                g_StartTimeMS = TimeGetNowMS();
            }
            const uint64_t timeNow = TimeGetNowMS();
            const uint64_t timeDelta = timeNow - g_StartTimeMS;

            const float fraction = GetFracPart(static_cast<float>(timeDelta) * 0.00005f);
            const D3DXVECTOR4 uvAddition(fraction, fraction, fraction, fraction);
            

            IDirect3DBaseTexture8* tex0 = nullptr;
            IDirect3DBaseTexture8* tex1 = nullptr;
            Device->GetTexture(WATER_TEXTURE_SLOT_REFRACTION, &tex0);
            Device->GetTexture(WATER_TEXTURE_SLOT_DUDV, &tex1);

            Device->SetVertexShader(g_WaterVSHandle);
            Device->SetPixelShader(g_WaterPSHandle);

            Device->SetTexture(WATER_TEXTURE_SLOT_REFRACTION, g_ScreenCopyTexture);
            Device->SetTexture(WATER_TEXTURE_SLOT_DUDV, g_DuDvTexture);

            DWORD texStageRefrStates[5] = {};
            DWORD texStageDudvStates[5] = {};

            SaveTextureStates(Device, WATER_TEXTURE_SLOT_REFRACTION, texStageRefrStates);
            SaveTextureStates(Device, WATER_TEXTURE_SLOT_DUDV, texStageDudvStates);

            Device->SetTextureStageState(WATER_TEXTURE_SLOT_REFRACTION, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
            Device->SetTextureStageState(WATER_TEXTURE_SLOT_REFRACTION, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
            Device->SetTextureStageState(WATER_TEXTURE_SLOT_REFRACTION, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
            Device->SetTextureStageState(WATER_TEXTURE_SLOT_REFRACTION, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
            Device->SetTextureStageState(WATER_TEXTURE_SLOT_REFRACTION, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

            Device->SetTextureStageState(WATER_TEXTURE_SLOT_DUDV, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
            Device->SetTextureStageState(WATER_TEXTURE_SLOT_DUDV, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
            Device->SetTextureStageState(WATER_TEXTURE_SLOT_DUDV, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
            Device->SetTextureStageState(WATER_TEXTURE_SLOT_DUDV, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
            Device->SetTextureStageState(WATER_TEXTURE_SLOT_DUDV, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

            Device->SetVertexShaderConstant(90, &uvAddition, 1);

            HRESULT hr = Device->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);

            Device->SetVertexShader(currVS);
            Device->SetPixelShader(currPS);

            Device->SetTexture(WATER_TEXTURE_SLOT_REFRACTION, tex0);
            Device->SetTexture(WATER_TEXTURE_SLOT_DUDV, tex1);

            RestoreTextureStates(Device, WATER_TEXTURE_SLOT_REFRACTION, texStageRefrStates);
            RestoreTextureStates(Device, WATER_TEXTURE_SLOT_DUDV, texStageDudvStates);

            return hr;
        }
    }

    return -1;
}

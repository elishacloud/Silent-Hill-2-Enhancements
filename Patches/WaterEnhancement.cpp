#include <Windows.h>

#include "Common\Settings.h"
#include "Common\Utils.h"
#include "Common\IUnknownPtr.h"
#include "Common\GfxUtils.h"
#include "Patches.h"
#include "Wrappers\d3d8\DirectX81SDK\include\d3d8.h"
#include "Wrappers\d3d8\DirectX81SDK\include\d3dx8math.h"

#include <cmath>
#include <functional>

#include "WaterEnhancement_dudv.h"
#include "WaterEnhancement_caustics.h"

DWORD vsDeclWater[] = {
    D3DVSD_STREAM(0),
    D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3),
    D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR),
    D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2),
    D3DVSD_END()
};


// eax == 3
//eax, dword ptr[eax * 4 + 1DB9288h]
static DWORD* g_vsHandles = nullptr;
static DWORD* g_cemeteryWaterPlaneCheckAddr = nullptr;
static DWORD g_cemeteryWaterPlaneCheckValue = NULL;
static float* g_cemeteryWaterRGB = NULL;
static float* g_cemeteryWaterAlpha = NULL;
static float* g_lakeWaterRGBA = NULL;

BYTE*(*shGetTexture)(UINT);

constexpr double kPi = 3.141592653589793;
constexpr UINT kExteriorWaterTextureId = 0x52F6;
constexpr UINT kCemeteryLeaveTextureId = 0x52F7;
constexpr UINT kLakeRebirthTextureId = 0x1B58;
constexpr float kWorldScale = 1.0f / 3000.0f;
constexpr float kLakeWaterCullDistZ = -8000.0f;
constexpr float kRebirthFogMinDistance = 15000.0f;

#define WATER_VSHADER_ORIGINAL  (g_vsHandles[3])
#define WATER_FVF               (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

#define WATER_TEXTURE_SLOT_BASE             0
#define WATER_TEXTURE_SLOT_REFRACTION       1
#define WATER_TEXTURE_SLOT_DUDV             2
#define WATER_TEXTURE_SLOT_CAUSTICS         3

#define WATER_DUDV_SCALE_PS_CB_SLOT         4
#define WATER_DUDV_SPEC_SCALE_PS_CB_SLOT    5
#define WATER_SPEC_MULT_PS_CB_SLOT          6
#define WATER_FOG_COLOUR_PS_CB_SLOT         7

#define WATER_UVADD_VS_CB_SLOT              90
#define WATER_UVMUL_VS_CB_SLOT              91
#define WATER_WORLD_VS_CB_SLOT              92
#define WATER_COLOUR_VS_CB_SLOT             93
#define WATER_UVOFFS_VS_CB_SLOT             94

/*
vs.1.1
// to clip pos
m4x4 r0, v0, c32
mov oPos, r0
// apply base UV offset
add r1, v7.xy, c94.xy
// pass-through UVs
mov oT0, r1
// animate UVs for the DuDv map
add oT1, r1, c90
// pass our ptojected pos for the refraction perspective UVs
mov oT2, r0
// scale caustics UVs
mul oT3, r1, c91
// pass-through vertex colour
mov oD0, v5
*/
DWORD g_WaterVSBytecode[] = {
    0xfffe0101, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000014,
    0x800f0000, 0x90e40000, 0xa0e40020, 0x00000001,
    0xc00f0000, 0x80e40000, 0x00000002, 0x800f0001,
    0x90540007, 0xa054005e, 0x00000001, 0xe00f0000,
    0x80e40001, 0x00000002, 0xe00f0001, 0x80e40001,
    0xa0e4005a, 0x00000001, 0xe00f0002, 0x80e40000,
    0x00000005, 0xe00f0003, 0x80e40001, 0xa0e4005b,
    0x00000001, 0xd00f0000, 0x90e40005, 0x0000ffff
};

/*
vs.1.1
// to clip pos
m4x4 r0, v0, c32
mov oPos, r0
// make main UVs by using world pos
mul r1, v0.xzzz, c92
// apply base UV offset
add r1, r1.xy, c94.xy
// pass-through UVs
mov oT0, r1
// animate UVs for the DuDv map
add oT1, r1, c90
// pass our ptojected pos for the refraction perspective UVs
mov oT2, r0
// scale caustics UVs
mul oT3, r1, c91
// pass-through vertex colour (contains fog)
mov oD0, v5

*/
DWORD g_WaterPondVSBytecode[] = {
    0xfffe0101, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000014,
    0x800f0000, 0x90e40000, 0xa0e40020, 0x00000001,
    0xc00f0000, 0x80e40000, 0x00000005, 0x800f0001,
    0x90a80000, 0xa0e4005c, 0x00000002, 0x800f0001,
    0x80540001, 0xa054005e, 0x00000001, 0xe00f0000,
    0x80e40001, 0x00000002, 0xe00f0001, 0x80e40001,
    0xa0e4005a, 0x00000001, 0xe00f0002, 0x80e40000,
    0x00000005, 0xe00f0003, 0x80e40001, 0xa0e4005b,
    0x00000001, 0xd00f0000, 0x90e40005, 0x0000ffff
};

/*
ps.1.4
def c0, 0.5, 0.5, 1, 1
def c1, 0.5, -0.5, 1, 1

// calc projected uv
texcrd r3.xy, t2_dw.xyw
// save aside base texture coords
texcrd r4.xyz, t0
// save aside caustics texture coords
texcrd r5.xyz, t3
// sample dudv map, restore and rescale
texld r2, t1
mul r1, c4, r2_bx2
// fill the void
mov r3.zw, c0
mov r4.zw, c0
mov r5.zw, c0
// [-1;1] -> [0;1]
mad r3, r3, c1, c0
// disturb projected uv
add r3, r3, r1
// disturb caustics uv
mad r5, r2_bx2, c5, r5
// pass along the disturbed base texture coords
add r2, r4, r1

phase

// sample water texture
texld r0, r2
// sample refraction texture
texld r1, r3
// sample caustics texture
texld r3, r5
// tint water texture by the vertex colour
mul r0, r0, v0
// blend them
lrp_sat r0, r0.w, r0, r1
// add modulated caustics
mad_sat r0, r3, c6, r0
// fill alpha
mov r0.w, c0
*/
DWORD g_WaterPSBytecode[] = {
    0xffff0104, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000051,
    0xa00f0000, 0x3f000000, 0x3f000000, 0x3f800000,
    0x3f800000, 0x00000051, 0xa00f0001, 0x3f000000,
    0xbf000000, 0x3f800000, 0x3f800000, 0x00000040,
    0x80030003, 0xbaf40002, 0x00000040, 0x80070004,
    0xb0e40000, 0x00000040, 0x80070005, 0xb0e40003,
    0x00000042, 0x800f0002, 0xb0e40001, 0x00000005,
    0x800f0001, 0xa0e40004, 0x84e40002, 0x00000001,
    0x800c0003, 0xa0e40000, 0x00000001, 0x800c0004,
    0xa0e40000, 0x00000001, 0x800c0005, 0xa0e40000,
    0x00000004, 0x800f0003, 0x80e40003, 0xa0e40001,
    0xa0e40000, 0x00000002, 0x800f0003, 0x80e40003,
    0x80e40001, 0x00000004, 0x800f0005, 0x84e40002,
    0xa0e40005, 0x80e40005, 0x00000002, 0x800f0002,
    0x80e40004, 0x80e40001, 0x0000fffd, 0x00000042,
    0x800f0000, 0x80e40002, 0x00000042, 0x800f0001,
    0x80e40003, 0x00000042, 0x800f0003, 0x80e40005,
    0x00000005, 0x800f0000, 0x80e40000, 0x90e40000,
    0x00000012, 0x801f0000, 0x80ff0000, 0x80e40000,
    0x80e40001, 0x00000004, 0x801f0000, 0x80e40003,
    0xa0e40006, 0x80e40000, 0x00000001, 0x80080000,
    0xa0e40000, 0x0000ffff
};

/*
ps.1.4
  def c0, 0.5, 0.5, 1, 1
  def c1, 0.5, -0.5, 1, 1

  // calc projected uv
  texcrd r3.xy, t2_dw.xyw
  // save aside base texture coords
  texcrd r4.xyz, t0
  // save aside caustics texture coords
  texcrd r5.xyz, t3
  // sample dudv map, restore and rescale
  texld r2, t1
  mul r1, c4, r2_bx2
  // fill the void
  mov r3.zw, c0
  mov r4.zw, c0
  mov r5.zw, c0
  // [-1;1] -> [0;1]
  mad r3, r3, c1, c0
  // disturb projected uv
  add r3, r3, r1
  // disturb caustics uv
  mad r5, r2_bx2, c5, r5
  // pass along the disturbed base texture coords
  add r2, r4, r1

phase

  // sample water texture
  texld r0, r2
  // sample refraction texture
  texld r1, r3
  // sample caustics texture
  texld r3, r5
  // tint water texture by the vertex colour
  mul r0, r0, v0
  // blend them
  lrp_sat r0, r0.w, r0, r1
  // add modulated caustics
  mad_sat r0, r3, c6, r0
  // fill alpha
  mov r0.w, c0
*/
DWORD g_WaterPondPSBytecode[] = {
    0xffff0104, 0x00000051, 0xa00f0000, 0x3f000000,
    0x3f000000, 0x3f800000, 0x3f800000, 0x00000051,
    0xa00f0001, 0x3f000000, 0xbf000000, 0x3f800000,
    0x3f800000, 0x00000040, 0x80030003, 0xbaf40002,
    0x00000040, 0x80070004, 0xb0e40000, 0x00000040,
    0x80070005, 0xb0e40003, 0x00000042, 0x800f0002,
    0xb0e40001, 0x00000005, 0x800f0001, 0xa0e40004,
    0x84e40002, 0x00000001, 0x800c0003, 0xa0e40000,
    0x00000001, 0x800c0004, 0xa0e40000, 0x00000001,
    0x800c0005, 0xa0e40000, 0x00000004, 0x800f0003,
    0x80e40003, 0xa0e40001, 0xa0e40000, 0x00000002,
    0x800f0003, 0x80e40003, 0x80e40001, 0x00000004,
    0x800f0005, 0x84e40002, 0xa0e40005, 0x80e40005,
    0x00000002, 0x800f0002, 0x80e40004, 0x80e40001,
    0x0000fffd, 0x00000042, 0x800f0000, 0x80e40002,
    0x00000042, 0x800f0001, 0x80e40003, 0x00000042,
    0x800f0003, 0x80e40005, 0x00000005, 0x800f0000,
    0x80e40000, 0x90e40000, 0x00000012, 0x801f0000,
    0x80ff0000, 0x80e40000, 0x80e40001, 0x00000004,
    0x801f0000, 0x80e40003, 0xa0e40006, 0x80e40000,
    0x00000001, 0x80080000, 0xa0e40000, 0x0000ffff,
};

DWORD g_WaterVSHandle = 0;
DWORD g_WaterPondVSHandle = 0;
DWORD g_WaterPSHandle = 0;
DWORD g_WaterPondPSHandle = 0;

IDirect3DTexture8* g_ScreenCopyTexture = nullptr;
IDirect3DSurface8* g_ScreenCopySurface = nullptr;
IDirect3DTexture8* g_DuDvTexture = nullptr;
IDirect3DTexture8* g_CausticsTexture = nullptr;

static LARGE_INTEGER    g_QPCFreq = {};

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

static void WaterEnhancedGrabScreen(LPDIRECT3DDEVICE8 Device, LPDIRECT3DSURFACE8 backBufferSurface) {
    if (backBufferSurface) {
        D3DSURFACE_DESC desc = {};
        backBufferSurface->GetDesc(&desc);

        if (!g_ScreenCopySurface || !CompareSurfaceDescs(g_CachedBackbufferSurfaceDesc, desc)) {
            memcpy(&g_CachedBackbufferSurfaceDesc, &desc, sizeof(desc));

            SafeRelease(g_ScreenCopySurface);
            SafeRelease(g_ScreenCopyTexture);

            HRESULT hr = Device->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &g_ScreenCopyTexture);
            if (FAILED(hr)) {
                return;
            }

            hr = g_ScreenCopyTexture->GetSurfaceLevel(0, &g_ScreenCopySurface);
            if (FAILED(hr)) {
                SafeRelease(g_ScreenCopySurface);
                SafeRelease(g_ScreenCopyTexture);
                return;
            }
        }

        if (g_ScreenCopySurface) {
            Device->CopyRects(backBufferSurface, nullptr, 0, g_ScreenCopySurface, nullptr);
        }
    }
}

void WaterEnhancedReleaseScreenCopy() {
    SafeRelease(g_ScreenCopySurface);
    SafeRelease(g_ScreenCopyTexture);
}

static void LoadWaterUtilityTextures(LPDIRECT3DDEVICE8 Device) {
    if (!g_DuDvTexture) {
        HRESULT hr = GfxCreateTextureFromFileInMem(Device, DuDv_128x128_data, sizeof(DuDv_128x128_data), &g_DuDvTexture);
        if (FAILED(hr)) {
            g_DuDvTexture = nullptr;
        }
    }

    if (!g_CausticsTexture) {
        HRESULT hr = GfxCreateTextureFromFileInMem(Device, Caustics_128x128_data, sizeof(Caustics_128x128_data), &g_CausticsTexture);
        if (FAILED(hr)) {
            g_CausticsTexture = nullptr;
        }
    }
}

static double GetFracPart(double f) {
    return f - std::floor(f);
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

static bool CheckWaterPrimitivesCountByRoom(const UINT PrimitiveCount, bool DrawUP) {
    bool isWater = false;

    const DWORD roomID = GetRoomID();

    if (!DrawUP) {
        switch (roomID) {
            // Lake
            case R_TOWN_LAKE:
                isWater = (PrimitiveCount == 104u && GetCutsceneID() == CS_END_REBIRTH_EPILOGUE);
            break;
            // Town East (along entrance road)
            case R_TOWN_EAST:
                isWater = (PrimitiveCount == 104u);
            break;
        }
        return isWater;
    }

    switch (roomID) {
        // Pond
        case R_FOREST_CEMETERY:
            isWater = (PrimitiveCount == 102u);
        break;
        // Lake
        case R_TOWN_LAKE:
            isWater = (PrimitiveCount == 68u);
        break;
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
            isWater = (PrimitiveCount <= 46u && PrimitiveCount >= 10u);
        break;
        // Hotel Alternate Basement
        case R_HTL_ALT_EMPLOYEE_STAIRS:
        case R_HTL_ALT_BAR:
        case R_HTL_ALT_BAR_KITCHEN:
        case R_HTL_ALT_ELEVATOR:
        case R_HTL_ALT_EMPLOYEE_HALL_BF:
            isWater = (PrimitiveCount <= 82u && PrimitiveCount >= 16u);
        break;
        // Hotel Alternate 1F
        case R_FINAL_BOSS_RM:
            isWater = (PrimitiveCount == 56u);
        break;
    }

    return isWater;
}

static void GetWaterConstantsByRoom(D3DXVECTOR4& specMult, D3DXVECTOR4& specUvMult, D3DXVECTOR4& dudvScale, D3DXVECTOR4& dudvSpecScale) {
    const DWORD roomID = GetRoomID();
    specMult = { 0.0f, 0.0f, 0.0f, 0.0f };
    specUvMult = { 2.0f, 2.0f, 2.0f, 2.0f };
    dudvScale = { 0.01f, 0.01f, 0.01f, 0.01f };
    dudvSpecScale = { 0.04f, 0.04f, 0.04f, 0.04f };

    switch (roomID) {
        // Pond
        case R_FOREST_CEMETERY:
        {
            dudvScale = { 0.005f, 0.005f, 0.005f, 0.005f };
            const float specMultOverride = (GetCutsceneID() == CS_END_LEAVE_LETTER) ? 0.05f : water_spec_mult_cemetery;
            specMult = { specMultOverride, specMultOverride, specMultOverride, 0.0f };
            specUvMult = { water_spec_uv_mult_cemetery, water_spec_uv_mult_cemetery, water_spec_uv_mult_cemetery, water_spec_uv_mult_cemetery };
        }
        break;
        // Lake
        case R_TOWN_LAKE:
        {
            dudvScale = { 0.005f, 0.005f, 0.005f, 0.005f };
            specMult = { water_spec_mult_lake, water_spec_mult_lake, water_spec_mult_lake, 0.0f };
            specUvMult = { water_spec_uv_mult_lake, water_spec_uv_mult_lake, water_spec_uv_mult_lake, water_spec_uv_mult_lake };
        }
        break;
        // Town East (along entrance road)
        case R_TOWN_EAST:
        {
            dudvScale = { 0.005f, 0.005f, 0.005f, 0.005f };
            specMult = { water_spec_mult_town_east, water_spec_mult_town_east, water_spec_mult_town_east, 0.0f };
            specUvMult = { water_spec_uv_mult_town_east, water_spec_uv_mult_town_east, water_spec_uv_mult_town_east, water_spec_uv_mult_town_east };
        }
        break;
        // Pyramidhead submerge
        case R_APT_W_STAIRCASE_N:
            specMult = { water_spec_mult_apt_staircase, water_spec_mult_apt_staircase, water_spec_mult_apt_staircase, 0.0f };
        break;
        // Strange Area 2
        case R_STRANGE_AREA_2_B:
            specMult = { water_spec_mult_strange_area, water_spec_mult_strange_area, water_spec_mult_strange_area, 0.0f };
        break;
        // Labyrinth West
        case R_LAB_BOTTOM_C:
        case R_LAB_BOTTOM_E:
        case R_LAB_BOTTOM_F:
        case R_LAB_BOTTOM_G:
        case R_LAB_BOTTOM_H:
        case R_LAB_BOTTOM_I:
            specMult = { water_spec_mult_labyrinth, water_spec_mult_labyrinth, water_spec_mult_labyrinth, 0.0f };
        break;
        // Hotel Alternate Basement
        case R_HTL_ALT_EMPLOYEE_STAIRS:
        case R_HTL_ALT_BAR:
        case R_HTL_ALT_BAR_KITCHEN:
        case R_HTL_ALT_ELEVATOR:
        case R_HTL_ALT_EMPLOYEE_HALL_BF:
        // Hotel Alternate 1F
        case R_FINAL_BOSS_RM: {
            specMult = { water_spec_mult_hotel, water_spec_mult_hotel, water_spec_mult_hotel, 0.0f };
            specUvMult = { water_spec_uv_mult_hotel, water_spec_uv_mult_hotel, water_spec_uv_mult_hotel, water_spec_uv_mult_hotel };
            const float f = water_spec_uv_mult_hotel * 0.02f;
            dudvSpecScale = { f, f, f, f };
        } break;
    }
}

IDirect3DBaseTexture8* GetTexture(UINT id)
{
    BYTE* texturePtr = shGetTexture(id);
    if (texturePtr == nullptr)
    {
        return nullptr;
    }
    return (IDirect3DBaseTexture8*)*(DWORD*)(texturePtr + 0x8C);
}

D3DXVECTOR4 GetBaseUVOffset(DWORD roomID, DWORD cutsceneID, int64_t inGameTimerMs, LPDIRECT3DDEVICE8 Device) {
    D3DXVECTOR4 uvOffset;

    if (roomID != R_FOREST_CEMETERY && roomID != R_TOWN_LAKE)
        return uvOffset;

    const double speedU = roomID == R_FOREST_CEMETERY ? water_uv_scroll_u_speed_cemetery : cutsceneID == CS_END_REBIRTH_EPILOGUE ? water_uv_scroll_u_speed_lake_rebirth : water_uv_scroll_u_speed_lake;
    const double speedV = roomID == R_FOREST_CEMETERY ? water_uv_scroll_v_speed_cemetery : cutsceneID == CS_END_REBIRTH_EPILOGUE ? water_uv_scroll_v_speed_lake_rebirth : water_uv_scroll_v_speed_lake;
    uvOffset = {
        static_cast<float>(GetFracPart(static_cast<double>(inGameTimerMs) * 0.00005 * speedU) * 20.0),
        static_cast<float>(GetFracPart(static_cast<double>(inGameTimerMs) * 0.00005 * speedV) * 20.0),
        0.0f,
        0.0f
    };
    if (roomID == R_TOWN_LAKE && GetCutsceneID() != CS_END_REBIRTH_EPILOGUE) {
        D3DXMATRIX transform;
        Device->GetTransform(D3DTS_WORLDMATRIX(0), &transform);
        uvOffset.x += transform.m[3][0] * kWorldScale;
        uvOffset.y += transform.m[3][2] * kWorldScale;
    };
    return uvOffset;
}

HRESULT DrawWaterEnhanced(bool needToGrabScreenForWater, int64_t inGameTimerMs, LPDIRECT3DDEVICE8 Device, LPDIRECT3DSURFACE8 backBufferSurface, UINT PrimitiveCount, bool DrawUP, std::function<HRESULT()> DrawFunc) {
    DWORD colorOp0 = 0;
    Device->GetTextureStageState(0, D3DTSS_COLOROP, &colorOp0);

    const DWORD roomID = GetRoomID();

    if ((colorOp0 == D3DTOP_MODULATE2X || roomID == R_FOREST_CEMETERY || roomID == R_TOWN_LAKE || roomID == R_TOWN_EAST) && CheckWaterPrimitivesCountByRoom(PrimitiveCount, DrawUP)) {
        DWORD currVS = 0u;
        Device->GetVertexShader(&currVS);
        DWORD currPS = 0u;
        Device->GetPixelShader(&currPS);

        const bool waterShaderActive = g_WaterVSHandle != 0u && g_WaterPSHandle != 0u && currVS != g_WaterPondVSHandle && currVS != g_WaterVSHandle && currPS != g_WaterPondPSHandle && currPS != g_WaterPSHandle;
        if (waterShaderActive) {
            if (needToGrabScreenForWater) {
                WaterEnhancedGrabScreen(Device, backBufferSurface);
            }
            LoadWaterUtilityTextures(Device);

            const float fraction = static_cast<float>(GetFracPart(static_cast<double>(inGameTimerMs) * 0.00005));
            const D3DXVECTOR4 uvAddition(fraction, fraction, fraction, fraction);

            D3DXVECTOR4 specMult;
            D3DXVECTOR4 specUvMult;
            D3DXVECTOR4 dudvScale;
            D3DXVECTOR4 dudvSpecScale;
            GetWaterConstantsByRoom(specMult, specUvMult, dudvScale, dudvSpecScale);

            IDirect3DBaseTexture8* tex0 = nullptr;
            IDirect3DBaseTexture8* tex1 = nullptr;
            IDirect3DBaseTexture8* tex2 = nullptr;
            Device->GetTexture(WATER_TEXTURE_SLOT_REFRACTION, &tex0);
            Device->GetTexture(WATER_TEXTURE_SLOT_DUDV, &tex1);
            Device->GetTexture(WATER_TEXTURE_SLOT_CAUSTICS, &tex2);

            Device->SetVertexShader((roomID == R_FOREST_CEMETERY || (roomID == R_TOWN_LAKE && GetCutsceneID() != CS_END_REBIRTH_EPILOGUE)) ? g_WaterPondVSHandle : g_WaterVSHandle);
            Device->SetPixelShader((roomID == R_FOREST_CEMETERY || roomID == R_TOWN_LAKE) ? g_WaterPondPSHandle : g_WaterPSHandle);

            Device->SetTexture(WATER_TEXTURE_SLOT_REFRACTION, g_ScreenCopyTexture);
            Device->SetTexture(WATER_TEXTURE_SLOT_DUDV, g_DuDvTexture);
            Device->SetTexture(WATER_TEXTURE_SLOT_CAUSTICS, g_CausticsTexture);

            DWORD texStageRefrStates[5] = {};
            DWORD texStageDudvStates[5] = {};
            DWORD texStageCausticsStates[5] = {};

            SaveTextureStates(Device, WATER_TEXTURE_SLOT_REFRACTION, texStageRefrStates);
            SaveTextureStates(Device, WATER_TEXTURE_SLOT_DUDV, texStageDudvStates);
            SaveTextureStates(Device, WATER_TEXTURE_SLOT_CAUSTICS, texStageCausticsStates);

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

            Device->SetTextureStageState(WATER_TEXTURE_SLOT_CAUSTICS, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
            Device->SetTextureStageState(WATER_TEXTURE_SLOT_CAUSTICS, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
            Device->SetTextureStageState(WATER_TEXTURE_SLOT_CAUSTICS, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
            Device->SetTextureStageState(WATER_TEXTURE_SLOT_CAUSTICS, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
            Device->SetTextureStageState(WATER_TEXTURE_SLOT_CAUSTICS, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

            Device->SetVertexShaderConstant(WATER_UVADD_VS_CB_SLOT, &uvAddition, 1);
            Device->SetVertexShaderConstant(WATER_UVMUL_VS_CB_SLOT, &specUvMult, 1);

            DWORD fogEnablePreserve = 0;
            DWORD fogModePreserve = 0;

            Device->GetRenderState(D3DRS_FOGENABLE, &fogEnablePreserve);
            Device->GetRenderState(D3DRS_FOGTABLEMODE, &fogModePreserve);

            Device->SetRenderState(D3DRS_FOGENABLE, TRUE);
            Device->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);

            D3DXVECTOR4 uvOffset{ 0.0f, 0.0f, 0.0f, 0.0f };
            
            if (roomID == R_FOREST_CEMETERY || roomID == R_TOWN_LAKE) {
                D3DXVECTOR4 worldDiv = { kWorldScale, kWorldScale, kWorldScale, kWorldScale };
                Device->SetVertexShaderConstant(WATER_WORLD_VS_CB_SLOT, &worldDiv, 1);

                D3DXVECTOR4 waterColour = { 0.32f, 0.32f, 0.32f, 0.35f };
                Device->SetVertexShaderConstant(WATER_COLOUR_VS_CB_SLOT, &waterColour, 1);

                DWORD dwFogColour = 0u;
                Device->GetRenderState(D3DRS_FOGCOLOR, &dwFogColour);
                D3DXVECTOR4 fogColour(float((dwFogColour >> 16) & 0xFF) / 255.0f, float((dwFogColour >> 8) & 0xFF) / 255.0f, float(dwFogColour & 0xFF) / 255.0f, 1.0f);
                Device->SetPixelShaderConstant(WATER_FOG_COLOUR_PS_CB_SLOT, &fogColour, 1u);

                IDirect3DBaseTexture8* tex = nullptr;
                const DWORD cutsceneID = GetCutsceneID();
                switch (cutsceneID)
                {
                case CS_END_LEAVE_LETTER:
                    tex = GetTexture(kCemeteryLeaveTextureId);
                    break;
                case CS_END_REBIRTH_EPILOGUE:
                    tex = GetTexture(kLakeRebirthTextureId);
                    break;
                default:
                    tex = GetTexture(kExteriorWaterTextureId);
                }
                if (tex != nullptr)
                {
                    Device->SetTexture(WATER_TEXTURE_SLOT_BASE, tex);

                    Device->SetTextureStageState(WATER_TEXTURE_SLOT_BASE, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
                    Device->SetTextureStageState(WATER_TEXTURE_SLOT_BASE, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
                    Device->SetTextureStageState(WATER_TEXTURE_SLOT_BASE, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
                    Device->SetTextureStageState(WATER_TEXTURE_SLOT_BASE, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
                    Device->SetTextureStageState(WATER_TEXTURE_SLOT_BASE, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
                }

                uvOffset = GetBaseUVOffset(roomID, cutsceneID, inGameTimerMs, Device);
            }

            Device->SetPixelShaderConstant(WATER_DUDV_SCALE_PS_CB_SLOT, &dudvScale, 1u);
            Device->SetPixelShaderConstant(WATER_DUDV_SPEC_SCALE_PS_CB_SLOT, &dudvSpecScale, 1u);
            Device->SetPixelShaderConstant(WATER_SPEC_MULT_PS_CB_SLOT, &specMult, 1u);
            Device->SetVertexShaderConstant(WATER_UVOFFS_VS_CB_SLOT, &uvOffset, 1);

            HRESULT hr = DrawFunc();

            Device->SetRenderState(D3DRS_FOGENABLE, fogEnablePreserve);
            Device->SetRenderState(D3DRS_FOGTABLEMODE, fogModePreserve);

            Device->SetVertexShader(currVS);
            Device->SetPixelShader(currPS);

            Device->SetTexture(WATER_TEXTURE_SLOT_REFRACTION, tex0);
            Device->SetTexture(WATER_TEXTURE_SLOT_DUDV, tex1);
            Device->SetTexture(WATER_TEXTURE_SLOT_CAUSTICS, tex2);

            RestoreTextureStates(Device, WATER_TEXTURE_SLOT_REFRACTION, texStageRefrStates);
            RestoreTextureStates(Device, WATER_TEXTURE_SLOT_DUDV, texStageDudvStates);
            RestoreTextureStates(Device, WATER_TEXTURE_SLOT_CAUSTICS, texStageCausticsStates);

            return hr;
        }
    }

    return -1;
}

void PatchWaterEnhancement()
{
    switch (GameVersion)
    {
    case SH2V_10:
        g_vsHandles = reinterpret_cast<DWORD*>(0x1DB9288);
        g_cemeteryWaterPlaneCheckAddr = reinterpret_cast<DWORD*>(0x1F7AA2C);
        break;
    case SH2V_11:
        g_vsHandles = reinterpret_cast<DWORD*>(0x1DBCE88);
        g_cemeteryWaterPlaneCheckAddr = reinterpret_cast<DWORD*>(0x1F7E62C);
        break;
    case SH2V_DC:
        g_vsHandles = reinterpret_cast<DWORD*>(0x1DBBE88);
        g_cemeteryWaterPlaneCheckAddr = reinterpret_cast<DWORD*>(0x1F7D62C);
        break;
    case SH2V_UNKNOWN:
        Logging::Log() << __FUNCTION__ << " Error: unknown game version!";
        WaterEnhancedRender = false;
        return;
    }

    constexpr BYTE CemeteryWaterAlphaSearchBytes[]{ 0x89, 0x95, 0xC0, 0xFE, 0xFF, 0xFF, 0x8B, 0x45, 0x84 };
    g_cemeteryWaterAlpha = (float*)ReadSearchedAddresses(0x004D3505, 0x004D37B5, 0x004D3075, CemeteryWaterAlphaSearchBytes, sizeof(CemeteryWaterAlphaSearchBytes), -0x10, __FUNCTION__);
    if (!g_cemeteryWaterAlpha)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        WaterEnhancedRender = false;
        return;
    }
    g_cemeteryWaterRGB = g_cemeteryWaterAlpha - 4;

    constexpr BYTE LakeWaterAlphaSearchBytes[]{ 0x83, 0xC4, 0x0C, 0x68, 0x00, 0x00, 0x7F, 0x43, 0x6A, 0x00, 0x8D, 0x85, 0xEC, 0xFD, 0xFF, 0xFF };
    g_lakeWaterRGBA = (float*)ReadSearchedAddresses(0x004D5075, 0x004D5325, 0x004D4BE5, LakeWaterAlphaSearchBytes, sizeof(LakeWaterAlphaSearchBytes), -0x17, __FUNCTION__);
    if (!g_lakeWaterRGBA)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        WaterEnhancedRender = false;
        return;
    }

    constexpr BYTE GetTextureSearchBytes[]{ 0xB9, 0x2E, 0x00, 0x00, 0x00, 0x8B, 0xFE, 0xF3, 0xAB, 0x8B, 0x7C, 0x24, 0x10 };
    const DWORD GetTextureAddr = SearchAndGetAddresses(0x0045AE98, 0x0045B0F8, 0x0045B0F8, GetTextureSearchBytes, sizeof(GetTextureSearchBytes), 0x0F, __FUNCTION__);
    if (!GetTextureAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        WaterEnhancedRender = false;
        return;
    }
    shGetTexture = (BYTE*(*)(UINT))((BYTE*)(GetTextureAddr + 0x04) + *(DWORD*)GetTextureAddr);

    constexpr BYTE CullDistSearchBytes[]{ 0x83, 0xC4, 0x10, 0xEB, 0x06, 0xC7, 0x07, 0x01, 0x00, 0x00, 0x00 };
    DWORD LakeWaterCullDistZAddr = SearchAndGetAddresses(0x004D5768, 0x004D5A18, 0x004D52D8, CullDistSearchBytes, sizeof(CullDistSearchBytes), 0x11, __FUNCTION__);
    if (!LakeWaterCullDistZAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        WaterEnhancedRender = false;
        return;
    }

    constexpr BYTE RebirthFogMinDistSearchBytes[]{ 0x83, 0xF8, 0x4D, 0x75, 0x43, 0xD9, 0x05 };
    DWORD RebirthFogMinDistAddr = ReadSearchedAddresses(0x0057E765, 0x0057F015, 0x0057E935, RebirthFogMinDistSearchBytes, sizeof(RebirthFogMinDistSearchBytes), 0x91, __FUNCTION__);
    if (!RebirthFogMinDistAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        WaterEnhancedRender = false;
        return;
    }

    // Increase distance after which the lake water disappears when James leaves the hotel dock
    const float* LakeWaterCullDistZ = &kLakeWaterCullDistZ;
    UpdateMemoryAddress((void*)LakeWaterCullDistZAddr, &LakeWaterCullDistZ, sizeof(float));

    // Adjust min fog distance during Rebirth ending cutscene
    UpdateMemoryAddress((void*)RebirthFogMinDistAddr, &kRebirthFogMinDistance, sizeof(float));
}

void CheckCemeteryWaterCulling()
{
    if (!WaterEnhancedRender)
		return;

	static bool cemeteryWaterCullingDisabled = false;

    if (!cemeteryWaterCullingDisabled && GetCutsceneID() == CS_ANGELA_CEMETERY)
    {
		ReadMemoryAddress(g_cemeteryWaterPlaneCheckAddr, &g_cemeteryWaterPlaneCheckValue, sizeof(DWORD));
        UpdateMemoryAddress(g_cemeteryWaterPlaneCheckAddr, "\x00\x04\x00\x00", sizeof(DWORD));
        cemeteryWaterCullingDisabled = true;
	}
    else if (cemeteryWaterCullingDisabled && GetCutsceneID() != CS_ANGELA_CEMETERY)
    {
        UpdateMemoryAddress(g_cemeteryWaterPlaneCheckAddr, &g_cemeteryWaterPlaneCheckValue, sizeof(DWORD));
        cemeteryWaterCullingDisabled = false;
	}
}

void UpdateExteriorWaterVertexColors()
{
    if (!WaterEnhancedRender)
        return;

    const DWORD roomID = GetRoomID();
    if (g_cemeteryWaterRGB && g_cemeteryWaterAlpha && roomID == R_FOREST_CEMETERY)
    {
        if (GetCutsceneID() == CS_END_LEAVE_LETTER)
        {
            if (GetTexture(kCemeteryLeaveTextureId) != nullptr)
            {
                g_cemeteryWaterRGB[0] = g_cemeteryWaterRGB[1] = g_cemeteryWaterRGB[2] = 128.0f;
                *g_cemeteryWaterAlpha = water_alpha_cemetery;
            }
        }
        else if (GetTexture(kExteriorWaterTextureId) != nullptr)
        {
            g_cemeteryWaterRGB[0] = g_cemeteryWaterRGB[1] = g_cemeteryWaterRGB[2] = water_rgb_cemetery;
            *g_cemeteryWaterAlpha = water_alpha_cemetery;
        }
    }
    if (g_lakeWaterRGBA && roomID == R_TOWN_LAKE && GetTexture(kExteriorWaterTextureId) != nullptr)
    {
        g_lakeWaterRGBA[0] = g_lakeWaterRGBA[1] = g_lakeWaterRGBA[2] = water_rgb_lake;
        g_lakeWaterRGBA[3] = water_alpha_lake;
    }
}

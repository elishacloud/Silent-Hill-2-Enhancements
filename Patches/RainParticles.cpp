#include "TransparentDrawTypes.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"
#include "Patches\Patches.h"
#include "Wrappers\d3d8\DirectX81SDK\include\d3dx8.h"
#include <vector>

DWORD g_VsDeclRain[] = {
    D3DVSD_STREAM(0),
    D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3),
    D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR),
    D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2),
    D3DVSD_END()
};

/*
vs.1.1
// c32 = wvp matrix
// c90 = camera right vector
// c91 = world up vector
// c92 = james position
// c93 = flashlight direction
// c94 = parameters (halfWidth, halfHeight, cos(45°), flashlightActive)

// Part 1: Build raindrops
mul r0.x, v7.x, c94.x // offset * halfWidth
mul r0.y, v7.y, c94.y // offset * halfHeight

mad r1, c90, r0.x, v0 // move along right vector
mad r1, c91, r0.y, r1 // move along up vector

m4x4 oPos, r1, c32 // transform to clip space

// Part 2: Light raindrops
// get direction vector from james to vertex
sub r0, v0, c92
mov r0.y, c92.y

// normalise dir vector
dp3 r1.x, r0, r0
rsq r1.x, r1.x
mul r0, r0, r1.x

dp3 r1.x, r0, c93 // compare dir vec with flashlight vec
sge r2.x, r1.x, c94.z // if within 45° set r2 to 1.0 or 0.0
mul r2.x, r2.x, c94.w // if flashlight not active zero out r2

max oD0.a, v5.a, r2.x // set diffuse alpha to r2 or original value
mov oD0.rgb, v5.rgb
*/
DWORD g_RainVSBytecode[] = {
    0xfffe0101, 0x0009fffe, 0x58443344, 0x68532038,
    0x72656461, 0x73734120, 0x6c626d65, 0x56207265,
    0x69737265, 0x30206e6f, 0x0031392e, 0x00000005,
    0x80010000, 0x90000007, 0xa000005e, 0x00000005,
    0x80020000, 0x90550007, 0xa055005e, 0x00000004,
    0x800f0001, 0xa0e4005a, 0x80000000, 0x90e40000,
    0x00000004, 0x800f0001, 0xa0e4005b, 0x80550000,
    0x80e40001, 0x00000014, 0xc00f0000, 0x80e40001,
    0xa0e40020, 0x00000002, 0x800f0000, 0x90e40000,
    0xa1e4005c, 0x00000001, 0x80020000, 0xa055005c,
    0x00000008, 0x80010001, 0x80e40000, 0x80e40000,
    0x00000007, 0x80010001, 0x80000001, 0x00000005,
    0x800f0000, 0x80e40000, 0x80000001, 0x00000008,
    0x80010001, 0x80e40000, 0xa0e4005d, 0x0000000d,
    0x80010002, 0x80000001, 0xa0aa005e, 0x00000005,
    0x80010002, 0x80000002, 0xa0ff005e, 0x0000000b,
    0xd0080000, 0x90ff0005, 0x80000002, 0x00000001,
    0xd0070000, 0x90a40005, 0x0000ffff
};

DWORD g_RainVSHandle = 0;

struct LineVertex {
    float x, y, z;
    D3DCOLOR diffuse;
};

struct TriVertex {
    float x, y, z;
    D3DCOLOR diffuse;
    float offX, offY;
};

// Original SH2 addresses
static void (*originalDrawTransparent)(DrawCalls*) = nullptr;
static LineVertex* rainLines = nullptr;

static std::vector<TriVertex> triListBackup;

static std::vector<TriVertex> transformRain(UINT primCount)
{
    if (primCount == 0)
        return {};

    std::vector<TriVertex> triList;

    for (size_t i = 0; i < primCount; i += 2)
    {
        LineVertex v0 = rainLines[i];
        LineVertex v1 = rainLines[i+1];

        const D3DVECTOR center = { v0.x, (v0.y + v1.y) / 2, v0.z };
        const TriVertex verts[] = {
            { center.x, center.y, center.z, v0.diffuse,  0.0f,   1.0f   },
            { center.x, center.y, center.z, v1.diffuse,  1.0f,  -0.975f },
            { center.x, center.y, center.z, v1.diffuse, -1.0f,  -0.975f },
            { center.x, center.y, center.z, v1.diffuse,  0.83f, -0.989f },
            { center.x, center.y, center.z, v1.diffuse, -0.83f, -0.989f },
            { center.x, center.y, center.z, v1.diffuse,  0.0f,  -1.0f   }
        };

        triList.insert(triList.end(), {
            verts[0], verts[2], verts[1],
            verts[2], verts[3], verts[1],
            verts[2], verts[4], verts[3],
            verts[4], verts[5], verts[3]
        });
    }

    return triList;
}

static void drawRain(std::vector<TriVertex>& triList)
{
    constexpr D3DMATRIX identity = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    if (triList.size() > 0)
    {
        IDirect3DDevice8* device = static_cast<IDirect3DDevice8*>(GetD3dDevice());

        constexpr float upVec[4] = { 0.0f, -1.0f, 0.0f, 0.0f };

        D3DMATRIX view;
        device->GetTransform(D3DTS_VIEW, &view);
        D3DXVECTOR4 rightVec = { view._11, 0.0f, view._31, 0.0f };
        D3DXVec4Normalize(&rightVec, &rightVec);

        const D3DVECTOR jamesPos = { GetJamesPosX(), 0.0f, GetJamesPosZ() };

        D3DXVECTOR3 flashlightDir = { GetFlashlightDirX(), 0.0f, GetFlashlightDirZ() };
        D3DXVec3Normalize(&flashlightDir, &flashlightDir);

        const float params[4] = {
            1.0f,        // raindrop half width
            50.0f,       // raindrop half height, all raindrops are 100 world units tall (I think)
            0.70710677f, // cos(45°), angle for illuminating raindrops
            static_cast<float>(GetFlashlightSwitch() && GetFlashlightAvailable()) // flashlight active
        };

        device->SetRenderState(D3DRS_LIGHTING, FALSE);
        device->SetRenderState(D3DRS_FOGENABLE, FALSE);
        device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        device->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
        device->SetRenderState(D3DRS_COLORVERTEX, TRUE);
        device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
        device->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
        
        device->SetTexture(0, NULL);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

        device->SetTransform(D3DTS_WORLD, &identity);

        device->SetVertexShaderConstant(90, &rightVec, 1);
        device->SetVertexShaderConstant(91, &upVec, 1);
        device->SetVertexShaderConstant(92, &jamesPos, 1);
        device->SetVertexShaderConstant(93, &flashlightDir, 1);
        device->SetVertexShaderConstant(94, &params, 1);

        device->SetVertexShader(g_RainVSHandle);

        device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, triList.size() / 3, triList.data(), sizeof(TriVertex));
    }

    triListBackup.clear();
}

static void hookDrawRain()
{
    UINT primCount;
    __asm mov primCount, esi
    
    // Accumulate rain in case of multiple calls (e.g. Hospital Courtyard after Flesh Lips)
    auto rain = transformRain(primCount);
    triListBackup.insert(triListBackup.end(), rain.begin(), rain.end());

    // Don't call the original function
}

static void hookDrawTransparent(DrawCalls* pDrawCalls)
{
    DrawCallNode rain = {};
    rain.type = DrawCallType::Custom;
    rain.typeCustom.func = reinterpret_cast<void(*)(DWORD)>(drawRain);
    rain.typeCustom.arg = reinterpret_cast<DWORD>(&triListBackup);

    // Insert rain NODES_FROM_TAIL nodes before the end of the draw call list
    constexpr int NODES_FROM_TAIL = 9;
    DrawCallNode* slow = pDrawCalls->head;
    DrawCallNode* fast = pDrawCalls->head;

    // Move fast pointer NODES_FROM_TAIL nodes ahead
    for (int i = 0; fast->pNext && i < NODES_FROM_TAIL; i++)
        fast = fast->pNext;

    while (fast->pNext) {
        slow = slow->pNext;
        fast = fast->pNext;
    }

    rain.pNext = slow->pNext;
    slow->pNext = &rain;

    originalDrawTransparent(pDrawCalls);

    // Remove rain from the linked-list in case the game tries to free it later
    slow->pNext = rain.pNext;
}

void PatchRainParticles()
{
    switch (GameVersion)
    {
    case SH2V_10:
        originalDrawTransparent = reinterpret_cast<void (*)(DrawCalls*)>(0x5AFEE0);
        rainLines = reinterpret_cast<LineVertex*>(0x963880);

        WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4EE4C6), hookDrawRain);
        WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4EE8E0), hookDrawRain);
        WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4762A5), hookDrawTransparent);
        break;
    case SH2V_11:
        originalDrawTransparent = reinterpret_cast<void (*)(DrawCalls*)>(0x5B0810);
        rainLines = reinterpret_cast<LineVertex*>(0x967480);

        WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4EE776), hookDrawRain);
        WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4EEB90), hookDrawRain);
        WriteCalltoMemory(reinterpret_cast<BYTE*>(0x476545), hookDrawTransparent);
        break;
    case SH2V_DC:
        originalDrawTransparent = reinterpret_cast<void (*)(DrawCalls*)>(0x5B0130);
        rainLines = reinterpret_cast<LineVertex*>(0x966480);

        WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4EE036), hookDrawRain);
        WriteCalltoMemory(reinterpret_cast<BYTE*>(0x4EE450), hookDrawRain);
        WriteCalltoMemory(reinterpret_cast<BYTE*>(0x476755), hookDrawTransparent);
        break;
    case SH2V_UNKNOWN:
        Logging::Log() << __FUNCTION__ << " Error: unknown game version!";
        return;
    }
}

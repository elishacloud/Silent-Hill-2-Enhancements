//#define WIN32_LEAN_AND_MEAN
#include "EnvSpecular.h"
#include "Common\Utils.h"
#include "Wrappers\d3d8\DirectX81SDK\include\d3dx8math.h"

//#include <Windows.h>
//#include "Patches.h"
//#include "Wrappers\d3d8\DirectX81SDK\include\d3d8.h"
//#include "ModelID.h"

DWORD myVsShaderHandle = 0;
DWORD myPsShaderHandle = 0;

IDirect3DDevice8** (__cdecl* GetD3dDevice_4F5480)() = (IDirect3DDevice8 * *(__cdecl *)())(0x4F5480);

void(__cdecl* sub_500930)(BYTE*, UINT, UINT, UINT, UINT) = (void(__cdecl*)(BYTE*, UINT, UINT, UINT, UINT))(0x500930);
void(__cdecl* sub_501540_orig)(struct_a1*) = (void(__cdecl*)(struct_a1*))(0x501540);
float(__cdecl* GetFloat_AAA5E0_50C500)() = (float(__cdecl*)())0x50C500;
void(__cdecl* GetSomeMatrix_50C570)(D3DMATRIX*) = (void(__cdecl*)(D3DMATRIX*))0x50C570;

void(__cdecl* sub_5B20C0)() = (void(__cdecl*)())0x5B20C0;
void(__cdecl* sub_5B2190)(DWORD) = (void(__cdecl*)(DWORD))0x5B2190;
void(__cdecl* sub_5B2240)(DWORD) = (void(__cdecl*)(DWORD))0x5B2240;


char* byte_8F0338 = reinterpret_cast<char*>(0x8F0338);
char* g_materialTypes_8F0339 = reinterpret_cast<char*>(0x8F0339);
D3DPRIMITIVETYPE* g_primitiveTypes_8F0378 = reinterpret_cast<D3DPRIMITIVETYPE*>(0x8F0378);

DWORD*& off_8F03C0 = *reinterpret_cast<DWORD**>(0x8F03C0);
DWORD*& off_8F03C4 = *reinterpret_cast<DWORD**>(0x8F03C4);
DWORD*& off_8F03C8 = *reinterpret_cast<DWORD**>(0x8F03C8);
DWORD*& off_8F03CC = *reinterpret_cast<DWORD**>(0x8F03CC);
DWORD& dword_8F03D0 = *reinterpret_cast<DWORD*>(0x8F03D0);

DWORD& dword_8F03E8 = *reinterpret_cast<DWORD*>(0x8F03E8);
DWORD& dword_8F0430 = *reinterpret_cast<DWORD*>(0x8F0430);
DWORD& dword_8F0488 = *reinterpret_cast<DWORD*>(0x8F0488);
DWORD& dword_8F04E0 = *reinterpret_cast<DWORD*>(0x8F04E0);
DWORD& dword_8F0540 = *reinterpret_cast<DWORD*>(0x8F0540);

DWORD& dword_1DB9288 = *reinterpret_cast<DWORD*>(0x1DB9288);
DWORD& dword_1DB928C = *reinterpret_cast<DWORD*>(0x1DB928C);
DWORD& dword_1DB9290 = *reinterpret_cast<DWORD*>(0x1DB9290);
DWORD& dword_1DB9294 = *reinterpret_cast<DWORD*>(0x1DB9294);
DWORD& dword_1DB92A8 = *reinterpret_cast<DWORD*>(0x1DB92A8);

DWORD& dword_8F05D0 = *reinterpret_cast<DWORD*>(0x8F05D0);
DWORD& dword_1DB92AC = *reinterpret_cast<DWORD*>(0x1DB92AC);

IDirect3DDevice8*& g_d3d8Device_A32894 = *reinterpret_cast<IDirect3DDevice8**>(0xA32894);

IDirect3DVertexBuffer8** g_vertexBuffers_A33540 = reinterpret_cast<IDirect3DVertexBuffer8**>(0xA33540);

float* flt_A33590 = reinterpret_cast<float*>(0xA33590); // array with len = 32?

int& g_HACK_DX_CONFIG_USE_PIXEL_SHADERS_A33370 = *reinterpret_cast<int*>(0xA33370);
BOOL& g_HACK_DX_CONFIG_USE_VERTEX_SHADERS_A33374 = *reinterpret_cast<BOOL*>(0xA33374);
int& dword_A333B8 = *reinterpret_cast<int*>(0xA333B8);
DWORD& dword_A333C0 = *reinterpret_cast<DWORD*>(0xA333C0);
unsigned int& dword_A333E8 = *reinterpret_cast<unsigned int*>(0xA333E8);

D3DMATRIX& g_modelViewProjectionMatrix_1DB8608 = *reinterpret_cast<D3DMATRIX*>(0x1DB8608);

int& g_materialIndex_1DB8A28 = *reinterpret_cast<int*>(0x1DB8A28);
DWORD* g_vsHandles_1DB88A8 = reinterpret_cast<DWORD*>(0x1DB88A8);
DWORD* g_psHandles_1DB89A8 = reinterpret_cast<DWORD*>(0x1DB89A8);

float& g_fogStart_1F5EE70 = *reinterpret_cast<float*>(0x1F5EE70);
float& g_fogEnd_1F5EE74 = *reinterpret_cast<float*>(0x1F5EE74);

struct_dword_1F7D4AC*& g_commonTextures_1F7D4AC = *reinterpret_cast<struct_dword_1F7D4AC**>(0x1F7D4AC);

float& float_1F7D510 = *reinterpret_cast<float*>(0x1F7D510);
float& float_1F7D514 = *reinterpret_cast<float*>(0x1F7D514);
float& float_1F7D518 = *reinterpret_cast<float*>(0x1F7D518);
float& float_1F7D51C = *reinterpret_cast<float*>(0x1F7D51C);

float* diffuseTint_1F7D65C = reinterpret_cast<float*>(0x1F7D65C);

IDirect3DBaseTexture8*& g_cubeMapTexture_1F7D710 = *reinterpret_cast<IDirect3DBaseTexture8**>(0x1F7D710);
BOOL& g_flashlightOn_1F7D718 = *reinterpret_cast<BOOL*>(0x1F7D718);

DWORD& nurseFlashlightOnVsHandle_1F7D6A0 = *reinterpret_cast<DWORD*>(0x1F7D6A0); // Our cool vs shader?
DWORD* dword_1F7D6C4 = reinterpret_cast<DWORD*>(0x1F7D6C4); // An array of ps shaders?
//dword_1F7D714 used to index above

MapVsConstants& g_mapVsConstants_1DB86E0 = *reinterpret_cast<MapVsConstants*>(0x1DB86E0);

// 4 vert shaders are still created elsewhere
void __cdecl CreateShaders_5AF110()
{
    if (GetD3dDevice_4F5480())
    {
        if (g_HACK_DX_CONFIG_USE_VERTEX_SHADERS_A33374)
        {
            g_d3d8Device_A32894->CreateVertexShader(off_8F03C0, &dword_8F03E8, &dword_1DB9288, 0);
            g_d3d8Device_A32894->CreateVertexShader(off_8F03C4, &dword_8F0430, &dword_1DB928C, 0);
            g_d3d8Device_A32894->CreateVertexShader(off_8F03C8, &dword_8F0488, &dword_1DB9290, 0);
            g_d3d8Device_A32894->CreateVertexShader(off_8F03CC, &dword_8F04E0, &dword_1DB9294, 0);
            g_d3d8Device_A32894->CreateVertexShader(&dword_8F03D0, &dword_8F0540, &dword_1DB92A8, 0);
        }
        else
        {
            g_d3d8Device_A32894->CreateVertexShader(off_8F03C0, 0, &dword_1DB9288, 0);
            g_d3d8Device_A32894->CreateVertexShader(off_8F03C4, 0, &dword_1DB928C, 0);
            g_d3d8Device_A32894->CreateVertexShader(off_8F03C8, 0, &dword_1DB9290, 0);
            g_d3d8Device_A32894->CreateVertexShader(off_8F03CC, 0, &dword_1DB9294, 0);
            g_d3d8Device_A32894->CreateVertexShader(&dword_8F03D0, 0, &dword_1DB92A8, 0);
        }

        if (g_HACK_DX_CONFIG_USE_PIXEL_SHADERS_A33370)
            g_d3d8Device_A32894->CreatePixelShader(&dword_8F05D0, &dword_1DB92AC);
    }
}

// Known arguments 0, 1, 2, 4, 6
// Only 1 or 2 in outdoor scenes
void __cdecl sub_5B2D40(int a1)
{
    D3DMATERIALCOLORSOURCE emissiveMaterialSource;
    float ambientRed;
    float ambientGreen;
    float ambientBlue;
    D3DMATERIAL8 material;

    g_d3d8Device_A32894->SetRenderState(D3DRS_FOGSTART, *((DWORD*)&g_fogStart_1F5EE70));
    g_d3d8Device_A32894->SetRenderState(D3DRS_FOGEND, *((DWORD*)&g_fogEnd_1F5EE74));

    DWORD vsHandle = g_vsHandles_1DB88A8[16 * g_materialIndex_1DB8A28 + 2 * a1];
    
    if (g_HACK_DX_CONFIG_USE_VERTEX_SHADERS_A33374)
    {
        g_d3d8Device_A32894->SetVertexShader(vsHandle);
    }
    else
    {
        g_d3d8Device_A32894->SetVertexShader(vsHandle);
        
        switch (byte_8F0338[16 * g_materialIndex_1DB8A28 + 2 * a1])
        {
        case 1:
        case 2:
        case 3:
        case 4:
            g_d3d8Device_A32894->SetRenderState(D3DRS_FOGENABLE, 1);
            g_d3d8Device_A32894->SetRenderState(D3DRS_FOGVERTEXMODE, 3);
            g_d3d8Device_A32894->SetRenderState(D3DRS_FOGTABLEMODE, 0);
            g_d3d8Device_A32894->SetRenderState(D3DRS_COLORVERTEX, 1);
            g_d3d8Device_A32894->SetRenderState(D3DRS_SPECULARENABLE, 0);
            g_d3d8Device_A32894->SetRenderState(D3DRS_LIGHTING, 1);
           
            memset(&material.Ambient, 0, 12);
            material.Ambient.a = 1.0;
            memset(&material, 0, 12);
            material.Diffuse.a = 1.0;
            memset(&material.Specular, 0, 12);
            material.Specular.a = 1.0;
            material.Emissive.r = 1.0;
            material.Emissive.g = 1.0;
            material.Emissive.b = 1.0;
            material.Emissive.a = 1.0;
            material.Power = 0.0;

            g_d3d8Device_A32894->SetMaterial(&material);
            g_d3d8Device_A32894->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, 0);
            g_d3d8Device_A32894->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, 0);
            g_d3d8Device_A32894->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, 1);
            break;

        case 5:
        case 6:
            g_d3d8Device_A32894->SetRenderState(D3DRS_FOGENABLE, 1);
            g_d3d8Device_A32894->SetRenderState(D3DRS_FOGVERTEXMODE, 3);
            g_d3d8Device_A32894->SetRenderState(D3DRS_FOGTABLEMODE, 0);
            g_d3d8Device_A32894->SetRenderState(D3DRS_LIGHTING, 1);

            if (dword_A333B8)
            {
                g_d3d8Device_A32894->SetRenderState(D3DRS_COLORVERTEX, 0);
                
                ambientRed = 0.0;
                ambientGreen = 0.0;
                ambientBlue = 0.0;
                emissiveMaterialSource = D3DMCS_MATERIAL;
            }
            else
            {
                g_d3d8Device_A32894->SetRenderState(D3DRS_COLORVERTEX, 1);
                
                ambientRed = 1.0;
                ambientGreen = 1.0;
                ambientBlue = 1.0;
                emissiveMaterialSource = D3DMCS_COLOR1;
            }

            material.Ambient.r = ambientRed;
            material.Ambient.a = 1.0;
            material.Ambient.b = ambientBlue;
            material.Ambient.g = ambientGreen;

            g_d3d8Device_A32894->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, emissiveMaterialSource);

            material.Diffuse.b = 1.0;
            material.Diffuse.g = 1.0;
            material.Diffuse.r = 1.0;
            memset(&material.Specular, 0, 12);
            material.Diffuse.a = 1.0;
            memset(&material.Emissive, 0, 12);
            material.Specular.a = 1.0;
            material.Emissive.a = 1.0;
            material.Power = 0.0;

            g_d3d8Device_A32894->SetMaterial(&material);
            g_d3d8Device_A32894->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, 0);
            g_d3d8Device_A32894->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, 0);
            g_d3d8Device_A32894->SetRenderState(D3DRS_NORMALIZENORMALS, 0);
            break;

        case 7:
        case 8:
            g_d3d8Device_A32894->SetRenderState(D3DRS_FOGENABLE, 1);
            g_d3d8Device_A32894->SetRenderState(D3DRS_FOGVERTEXMODE, 3);
            g_d3d8Device_A32894->SetRenderState(D3DRS_LIGHTING, 1);
            g_d3d8Device_A32894->SetRenderState(D3DRS_FOGTABLEMODE, 0);
            g_d3d8Device_A32894->SetRenderState(D3DRS_COLORVERTEX, 1);

            material.Ambient.r = 1.0;
            material.Ambient.g = 1.0;
            material.Ambient.b = 1.0;
            material.Ambient.a = 1.0;
            material.Diffuse.r = 1.0;
            material.Diffuse.g = 1.0;
            material.Diffuse.b = 1.0;
            material.Diffuse.a = 1.0;
            memset(&material.Specular, 0, 12);
            material.Specular.a = 1.0;
            memset(&material.Emissive, 0, 12);
            material.Emissive.a = 1.0;
            material.Power = 0.0;

            g_d3d8Device_A32894->SetMaterial(&material);
            g_d3d8Device_A32894->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, 0);
            g_d3d8Device_A32894->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, 0);
            g_d3d8Device_A32894->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, 1);
            g_d3d8Device_A32894->SetRenderState(D3DRS_NORMALIZENORMALS, 0);
            break;

        default:
            g_d3d8Device_A32894->SetRenderState(D3DRS_FOGENABLE, 0);
            g_d3d8Device_A32894->SetRenderState(D3DRS_SPECULARENABLE, 0);
            g_d3d8Device_A32894->SetRenderState(D3DRS_LIGHTING, 0);
            break;
        }
    }
    if (g_HACK_DX_CONFIG_USE_PIXEL_SHADERS_A33370)
    {
        g_d3d8Device_A32894->SetPixelShader(g_psHandles_1DB89A8[8 * g_materialIndex_1DB8A28 + a1]);
        sub_5B20C0();
        
        char matType = g_materialTypes_8F0339[16 * g_materialIndex_1DB8A28 + 2 * a1];
        if (matType == 3 || matType == 1)
            g_d3d8Device_A32894->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, 256);
        else
            g_d3d8Device_A32894->SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
    }
    else
    {
        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
        
        switch (g_materialTypes_8F0339[16 * g_materialIndex_1DB8A28 + 2 * a1])
        {
        case 1:
        case 3:
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLOROP, 4);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLORARG1, 2);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLORARG2, 0);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAOP, 2);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_RESULTARG, 1);

            if (!dword_A333B8)
            {
                g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_COLOROP, 1);
                g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);
                break;
            }

            g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, 259);
            g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 131073);
            g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_COLOROP, 4);
            g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_COLORARG1, 2);
            g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_COLORARG2, 1);
            g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_ALPHAOP, 2);
            g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_ALPHAARG1, 1);
            g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_RESULTARG, 1);
            g_d3d8Device_A32894->SetTextureStageState(2, D3DTSS_COLOROP, 1);
            g_d3d8Device_A32894->SetTextureStageState(2, D3DTSS_ALPHAOP, 1);
            break;

        case 2:
        case 4:
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLOROP, 4);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLORARG1, 2);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLORARG2, 0);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAOP, 2);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_RESULTARG, 1);
            g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_COLOROP, 1);
            g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);
            break;

        case 5:
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAOP, 2);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLOROP, 2);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLORARG1, 2);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_RESULTARG, 1);
            g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_COLOROP, 1);
            g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);
            break;

        default:
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLOROP, 4);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLORARG1, 3);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLORARG2, 2);
            g_d3d8Device_A32894->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFF0000FF);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAOP, 2);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAARG1, 3);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_RESULTARG, 1);
            g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_COLOROP, 1);
            break;
        }
    }
}

// Function responsible for drawing MAP
void __cdecl sub_5B4940(Something* toRender)
{
    SubSomething3* pSubStruct3; // edi
    UINT primitiveCount; // ebx
    SubSomething* baseTextures; // esi
    int off_0x04; // eax
    SubSubSomething* off_0x0C; // esi
    unsigned int i; // edi
    bool v8; // cf
    SubSomething3* v9; // [esp+60h] [ebp-A4h]
    int v10; // [esp+64h] [ebp-A0h]
    UINT startIndex; // [esp+68h] [ebp-9Ch]
    unsigned int v12; // [esp+6Ch] [ebp-98h]
    int v13; // [esp+70h] [ebp-94h]
    IDirect3DIndexBuffer8* indexBuffer; // [esp+74h] [ebp-90h]
    VBStruct* vertexBufferStruct; // [esp+78h] [ebp-8Ch]
    SubSomething2* strides; // [esp+BCh] [ebp-48h]
    D3DMATERIAL8 material; // [esp+C0h] [ebp-44h] BYREF

    strides = toRender->strides;
    vertexBufferStruct = toRender->vertexBufferStruct;
    pSubStruct3 = toRender->pSubStruct3;
    v9 = pSubStruct3;
    indexBuffer = toRender->indexBuffer;
    v10 = 8;
    v13 = -1;
    startIndex = 0;
    v12 = 0;

    primitiveCount = 0; // Not original

    if (toRender->off_0x30)
    {
        while (1)
        {
            baseTextures = &toRender->pSubStruct[pSubStruct3->off_0x00];
            g_d3d8Device_A32894->SetTexture(0, baseTextures->tex1);     // base diffuse texture
            
            if (g_HACK_DX_CONFIG_USE_VERTEX_SHADERS_A33374 && g_HACK_DX_CONFIG_USE_PIXEL_SHADERS_A33370)
                g_d3d8Device_A32894->SetTexture(1, baseTextures->tex2); // Haven't seen an instance where this is non-null

            if (v10 != baseTextures->off_0x10)
            {
                v10 = baseTextures->off_0x10;
                sub_5B2D40(v10);
            }

            if (!baseTextures->tex1 && !g_HACK_DX_CONFIG_USE_VERTEX_SHADERS_A33374 && !v10)
            {
                memset(&material.Diffuse, 0, 12);
                memset(&material.Ambient, 0, 12);
                memset(&material.Emissive, 0, 12);
                memset(&material.Specular, 0, 12);
                
                material.Diffuse.a = 1.0;
                material.Ambient.a = 1.0;
                material.Emissive.a = 1.0;
                material.Specular.a = 1.0;
                material.Power = 0.0;
                
                g_d3d8Device_A32894->SetMaterial(&material);
                g_d3d8Device_A32894->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
                
                v10 = -1;
            }

            sub_5B2190(baseTextures->off_0x00);
            sub_5B2240(baseTextures->off_0x04);

            off_0x04 = pSubStruct3->off_0x04;
            
            if (v13 != off_0x04)
            {
                v13 = pSubStruct3->off_0x04;
                g_d3d8Device_A32894->SetStreamSource(0, vertexBufferStruct[off_0x04].vertexBuffer, strides[off_0x04 + 1].off_0x00);
                pSubStruct3 = v9;
            }

            off_0x0C = pSubStruct3->off_0x0C;
            i = 0;
            
            if (v9->off_0x08)
            {
                while (i < v9->off_0x08)
                {
                    if (g_primitiveTypes_8F0378[off_0x0C->off_0x02] == D3DPT_TRIANGLELIST)
                    {
                        primitiveCount = off_0x0C->off_0x00 * off_0x0C->off_0x03 / 3;
                    }
                    else if (g_primitiveTypes_8F0378[off_0x0C->off_0x02] == D3DPT_TRIANGLESTRIP)
                    {
                        primitiveCount = off_0x0C->off_0x00 * off_0x0C->off_0x03 - 2;
                    }

                    g_d3d8Device_A32894->SetIndices(indexBuffer, 0);
                    
                    // MY CODE
                    if (startIndex == 28189 && primitiveCount == 234) // If drawing the shop window
                    {
                        // Assign specular highlight texture to slot 4
                        g_d3d8Device_A32894->SetTexture(4, g_commonTextures_1F7D4AC[14440].tex);

                        constexpr float specularSize = 3.545f; // default = 3.545

                        // Get the mystery constants in the same way as the nurse
                        D3DMATRIX matrix;
                        GetSomeMatrix_50C570(&matrix); // Gets matrix at 0xAAA634

                        float shaderConstantMulti[4] = { 0 };
                        shaderConstantMulti[0] = matrix._11 * specularSize;
                        shaderConstantMulti[1] = matrix._21 * specularSize;
                        shaderConstantMulti[2] = matrix._31 * specularSize;
                        shaderConstantMulti[3] = 0.5;

                        float vsConstant_3[4] = { 0 };
                        vsConstant_3[0] = matrix._12 * specularSize;
                        vsConstant_3[1] = matrix._22 * specularSize;
                        vsConstant_3[2] = matrix._32 * specularSize;
                        vsConstant_3[3] = 0.5;

                        // Set to some unused registers
                        g_d3d8Device_A32894->SetVertexShaderConstant(28, shaderConstantMulti, 1);
                        g_d3d8Device_A32894->SetVertexShaderConstant(29, vsConstant_3, 1);

                        DWORD currVs;
                        DWORD currPs;
                        g_d3d8Device_A32894->GetVertexShader(&currVs);
                        g_d3d8Device_A32894->GetPixelShader(&currPs);
                        g_d3d8Device_A32894->SetVertexShader(myVsShaderHandle);
                        g_d3d8Device_A32894->SetPixelShader(myPsShaderHandle);

                        // Set up sampler states
                        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
                        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
                        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
                        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
                        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

                        g_d3d8Device_A32894->DrawIndexedPrimitive(g_primitiveTypes_8F0378[off_0x0C->off_0x02], off_0x0C->off_0x04, off_0x0C->off_0x06 - off_0x0C->off_0x04 + 1, startIndex, primitiveCount);

                        g_d3d8Device_A32894->SetVertexShader(currVs);
                        g_d3d8Device_A32894->SetPixelShader(currPs);

                        g_d3d8Device_A32894->SetTexture(4, 0);
                    }
                    else
                    {
                        g_d3d8Device_A32894->DrawIndexedPrimitive(g_primitiveTypes_8F0378[off_0x0C->off_0x02], off_0x0C->off_0x04, off_0x0C->off_0x06 - off_0x0C->off_0x04 + 1, startIndex, primitiveCount);
                    }
                    // END MY CODE

                    // ORIGINAL CODE
                    // g_d3d8Device_A32894->DrawIndexedPrimitive(g_primitiveTypes_8F0378[off_0x0C->off_0x02], off_0x0C->off_0x04, off_0x0C->off_0x06 - off_0x0C->off_0x04 + 1, startIndex, primitiveCount);
                    // END ORIGINAL CODE

                    startIndex += off_0x0C->off_0x00 * off_0x0C->off_0x03;
                    
                    ++off_0x0C;
                    ++i;
                }
            }

            v8 = v12 + 1 < toRender->off_0x30;
            v9 = (SubSomething3*)off_0x0C;
            ++v12;

            if (!v8)
                break;

            pSubStruct3 = (SubSomething3*)off_0x0C;
        }
    }
}

void MatrixVectorProduct_4FE750(D3DMATRIX* pM, D3DXVECTOR4* pV, D3DMATRIX* pOut)
{
    double x = pV->x;
    pOut->_11 = x * pM->_11;
    pOut->_12 = x * pM->_12;
    pOut->_13 = x * pM->_13;
    pOut->_14 = x * pM->_14;

    double y = pV->y;
    pOut->_21 = y * pM->_21;
    pOut->_22 = y * pM->_22;
    pOut->_23 = y * pM->_23;
    pOut->_24 = y * pM->_24;

    double z = pV->z;
    pOut->_31 = z * pM->_31;
    pOut->_32 = z * pM->_32;
    pOut->_33 = z * pM->_33;
    pOut->_34 = z * pM->_34;

    double w = pV->w;
    pOut->_41 = w * pM->_41;
    pOut->_42 = w * pM->_42;
    pOut->_43 = w * pM->_43;
    pOut->_44 = w * pM->_44;
}

void D3DXMatrixTranspose_4199CA(D3DMATRIX* pOut, CONST D3DMATRIX* pM) {
    pOut->_11 = pM->_11;
    pOut->_21 = pM->_21;
    pOut->_31 = pM->_31;
    pOut->_41 = pM->_41;

    pOut->_12 = pM->_12;
    pOut->_22 = pM->_22;
    pOut->_32 = pM->_32;
    pOut->_42 = pM->_42;

    pOut->_13 = pM->_13;
    pOut->_23 = pM->_23;
    pOut->_33 = pM->_33;
    pOut->_43 = pM->_43;

    pOut->_14 = pM->_14;
    pOut->_24 = pM->_24;
    pOut->_34 = pM->_34;
    pOut->_44 = pM->_44;
}

HRESULT AnimateActor_5014C0(struct_a1* toRender)
{
    IDirect3DVertexBuffer8* vertexBuffer; // esi

    unsigned int vbIndex = 8;
    while (toRender->subStruct->numVertices + 4 > 1 << vbIndex)
    {
        if (++vbIndex > 14)
        {
            vertexBuffer = 0;
            goto LABEL_5;
        }
    }

    vertexBuffer = g_vertexBuffers_A33540[vbIndex];

LABEL_5:
    BYTE* vertexData = nullptr;
    UINT sizeToLock = 32 * toRender->subStruct->numVertices;
    
    if (dword_A333E8)
        vertexBuffer->Lock(0, sizeToLock, &vertexData, 0);
    else
        vertexBuffer->Lock(0, sizeToLock, &vertexData, D3DLOCK_DISCARD);

    sub_500930(vertexData, toRender->subStruct->dword8, 0, toRender->subStruct->numVertices, toRender->subStruct->dwordC);
    
    return vertexBuffer->Unlock();
}

// Function responsible for drawing ACTORS
void __cdecl sub_501540(struct_a1* toRender)
{
    unsigned int v1; // edi
    bool cullMode; // zf
    //double v3; // st7
    //float v4; // edx
    //float v5; // ecx
    //double v6; // st7
    //float v7; // edx
    //float v8; // ecx
    int dword20; // esi
    DWORD v10; // edx
    unsigned __int8* v11; // esi
    char byte24; // al
    //IDirect3DDevice8* ppDirect3DDevice8_A32894_1; // ecx
    unsigned __int8* v14; // eax
    float* v15; // edx
    unsigned int dword8; // esi
    unsigned int v17; // ecx
    unsigned __int8* v18; // eax
    unsigned int v19; // ebx
    float* v20; // edx
    struct_dword_1F7D4AC* v21; // esi
    unsigned int vbIndex; // ecx
    IDirect3DVertexBuffer8* vertexBuffer; // ecx
    D3DTEXTURESTAGESTATETYPE v24; // [esp+1A0h] [ebp-E0h]
    float float_AAA5E0; // [esp+1B8h] [ebp-C8h]
    float specularSize; // [esp+1B8h] [ebp-C8h]
    float shaderConstantMulti[4]; // [esp+1BCh] [ebp-C4h] BYREF
    unsigned int v28; // [esp+1CCh] [ebp-B4h]
    float vsConstant_1[4]; // [esp+1D0h] [ebp-B0h] BYREF
    float vsConstant_3[4]; // [esp+1E0h] [ebp-A0h] BYREF
    float vsConstant[4]; // [esp+1F0h] [ebp-90h] BYREF
    D3DMATRIX matrix; // [esp+200h] [ebp-80h] BYREF
    D3DMATRIX matrix_1; // [esp+240h] [ebp-40h] BYREF

    float_AAA5E0 = GetFloat_AAA5E0_50C500();
    v1 = 0;
    cullMode = toRender->cullMode == 0;
    v28 = 0;

    if (cullMode)
        g_d3d8Device_A32894->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    else
        g_d3d8Device_A32894->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

    if (toRender->byte24 == 1)
    {
        vsConstant_1[3] = 1.0;
        vsConstant_1[2] = 1.0;
        vsConstant_1[1] = 1.0;
        vsConstant_1[0] = 1.0;
    }
    else
    {
        vsConstant_1[0] = float_1F7D510 * toRender->float54;
        vsConstant_1[1] = float_1F7D514 * toRender->float58;
        vsConstant_1[2] = float_1F7D518 * toRender->float5C;
        vsConstant_1[3] = float_1F7D51C * (float_AAA5E0 * toRender->float30);
    }

    vsConstant[0] = toRender->tintR;
    vsConstant[1] = toRender->tintG;
    vsConstant[2] = toRender->tintB;
    vsConstant[3] = float_AAA5E0 * toRender->tintA;

    if (g_HACK_DX_CONFIG_USE_VERTEX_SHADERS_A33374)
    {
        g_d3d8Device_A32894->SetVertexShaderConstant(43, vsConstant, 1);
        g_d3d8Device_A32894->SetVertexShaderConstant(44, vsConstant_1, 1);
    }

    if (g_flashlightOn_1F7D718 && g_HACK_DX_CONFIG_USE_PIXEL_SHADERS_A33370) // If flashlight ON
    {
        shaderConstantMulti[0] = diffuseTint_1F7D65C[0] * vsConstant[0]; // red
        shaderConstantMulti[1] = diffuseTint_1F7D65C[1] * vsConstant[1]; // green
        shaderConstantMulti[2] = diffuseTint_1F7D65C[2] * vsConstant[2]; // blue
        shaderConstantMulti[3] = diffuseTint_1F7D65C[3];                 // alpha (determines how black the texture appears)

        g_d3d8Device_A32894->SetPixelShaderConstant(2, shaderConstantMulti, 1);
    }

    dword20 = toRender->dword20;
    v10 = toRender->byte0[dword20];
    v11 = &toRender->byte0[dword20];
    
    g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ADDRESSU, v10);
    g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ADDRESSV, v11[1]);
    g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_MAGFILTER, v11[2]);
    g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_MINFILTER, v11[3]);

    if (toRender->byte24 == 4) // If a reflective material?
    {
        specularSize = toRender->specularSize; // Nurse = 3.545
        
        if (g_HACK_DX_CONFIG_USE_VERTEX_SHADERS_A33374)
        {
            GetSomeMatrix_50C570(&matrix);
            
            shaderConstantMulti[0] = matrix._11 * specularSize;
            shaderConstantMulti[1] = matrix._21 * specularSize;
            shaderConstantMulti[2] = matrix._31 * specularSize;
            shaderConstantMulti[3] = 0.5;

            vsConstant_3[0] = matrix._12 * specularSize;
            vsConstant_3[1] = matrix._22 * specularSize;
            vsConstant_3[2] = matrix._32 * specularSize;
            vsConstant_3[3] = 0.5;

            g_d3d8Device_A32894->SetVertexShaderConstant(89, shaderConstantMulti, 1);
            g_d3d8Device_A32894->SetVertexShaderConstant(90, vsConstant_3, 1);
        }
        else
        {
            GetSomeMatrix_50C570(&matrix);

            vsConstant_3[0] = specularSize;
            vsConstant_3[1] = specularSize;
            vsConstant_3[2] = specularSize;
            vsConstant_3[3] = 1.0;

            MatrixVectorProduct_4FE750(&matrix, (D3DXVECTOR4*)vsConstant_3, &matrix_1);
            matrix_1._31 = 0.5;
            matrix_1._32 = 0.5;

            D3DXMatrixTranspose_4199CA(&matrix, &matrix_1);

            g_d3d8Device_A32894->SetTransform(D3DTS_TEXTURE1, &matrix);
        }
    }

    g_d3d8Device_A32894->SetTexture(0, g_commonTextures_1F7D4AC[*(__int16*)&toRender->byte0[toRender->dword1C] + 0x3828].tex);
    g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
    
    if (g_flashlightOn_1F7D718 && g_HACK_DX_CONFIG_USE_PIXEL_SHADERS_A33370) // If flashlight ON
        goto LABEL_23;

    if (!g_HACK_DX_CONFIG_USE_VERTEX_SHADERS_A33374)
    {
        g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_RESULTARG, D3DTA_CURRENT);
        byte24 = toRender->byte24;
        
        if (byte24 != 1)
        {
            if (byte24 == 4)
            {
                g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
                g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
                g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                g_d3d8Device_A32894->SetTexture(1, g_commonTextures_1F7D4AC[14440].tex);
                g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1 | D3DTSS_TCI_CAMERASPACENORMAL);
                g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
                g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
                g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);

                if (dword_A333C0)
                    g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATEALPHA_ADDCOLOR);
                else
                    g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);

                g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
                g_d3d8Device_A32894->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
                g_d3d8Device_A32894->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
                goto LABEL_23;
            }

            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            v24 = D3DTSS_COLOROP;
            
            goto LABEL_22;
        }

        g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

    LABEL_21:
        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        v24 = D3DTSS_ALPHAOP;

    LABEL_22:
        g_d3d8Device_A32894->SetTextureStageState(1, v24, D3DTOP_DISABLE);
        goto LABEL_23;
    }
    
    g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_RESULTARG, D3DTA_CURRENT);
    
    if (toRender->byte24 == 1)
    {
        g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
    else
    {
        g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        g_d3d8Device_A32894->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        
        if (!toRender->byte25)
            goto LABEL_21;

        g_d3d8Device_A32894->SetTexture(1, g_commonTextures_1F7D4AC[14440].tex);
        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDCURRENTALPHA);
        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
        g_d3d8Device_A32894->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        g_d3d8Device_A32894->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }

LABEL_23:
    if (g_flashlightOn_1F7D718 && g_HACK_DX_CONFIG_USE_PIXEL_SHADERS_A33370) // If flashlight ON
    {
        g_d3d8Device_A32894->SetTexture(1, g_commonTextures_1F7D4AC[14440].tex);
        g_d3d8Device_A32894->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
        g_d3d8Device_A32894->SetTexture(2, g_cubeMapTexture_1F7D710);
        g_d3d8Device_A32894->SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 2);
        g_d3d8Device_A32894->SetTextureStageState(2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
        g_d3d8Device_A32894->SetTextureStageState(2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
        g_d3d8Device_A32894->SetTextureStageState(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
        g_d3d8Device_A32894->SetTextureStageState(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
        g_d3d8Device_A32894->SetTextureStageState(2, D3DTSS_MIPFILTER, D3DTEXF_NONE);
        
        if (toRender->byte24 == 4)
        {
            shaderConstantMulti[0] = toRender->specularR * 0.0078125;
            shaderConstantMulti[1] = toRender->specularG * 0.0078125;
            shaderConstantMulti[2] = toRender->specularB * 0.0078125;
        }
        else
        {
            memset(shaderConstantMulti, 0, 12);
        }

        shaderConstantMulti[3] = 0.0;
        g_d3d8Device_A32894->SetPixelShaderConstant(3, shaderConstantMulti, 1);
    }
    
    v14 = &toRender->byte0[toRender->dwordC];
    
    if (toRender->dword8)
    {
        v15 = flt_A33590;
        do
        {
            std::memcpy(v15, &g_commonTextures_1F7D4AC[16 * *(unsigned __int16*)v14], 64);
            dword8 = toRender->dword8;
            v14 += 2;
            v17 = v28 + 1;
            v15 += 16;
            ++v28;
        } while (v28 < dword8);
        v1 = v17;
    }

    v18 = &toRender->byte0[toRender->dword14];
    v19 = 0;
    
    if (toRender->dword10)
    {
        v20 = &flt_A33590[16 * v1];
        do
        {
            v21 = &g_commonTextures_1F7D4AC[16 * *(unsigned __int16*)v18 + 2048];
            v18 += 2;
            std::memcpy(v20, v21, 64);
            ++v19;
            v20 += 16;
        } while (v19 < toRender->dword10);
    }
    
    AnimateActor_5014C0(toRender);
    g_d3d8Device_A32894->SetIndices(toRender->subStruct->indexBuffer, 0);
    vbIndex = 8;
    
    while (toRender->subStruct->numVertices + 4 > 1 << vbIndex)
    {
        if (++vbIndex > 14)
        {
            vertexBuffer = 0;
            goto LABEL_50;
        }
    }

    vertexBuffer = g_vertexBuffers_A33540[vbIndex];

LABEL_50:

    // MY CODE
    DWORD currVs;
    g_d3d8Device_A32894->GetVertexShader(&currVs);

    if (currVs == nurseFlashlightOnVsHandle_1F7D6A0) // If current vs is the one used by the nurse
    {
        constexpr float specTint[] = { 1.0f, 0.0f, 0.0f, 0.0f };
        g_d3d8Device_A32894->SetPixelShaderConstant(3, specTint, 1);

        // Replace current vs and ps with custom shaders
        g_d3d8Device_A32894->SetVertexShader(myVsShaderHandle);
        g_d3d8Device_A32894->SetPixelShader(myPsShaderHandle);

        // Draw as usual
        g_d3d8Device_A32894->SetStreamSource(0, vertexBuffer, 32);
        g_d3d8Device_A32894->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, toRender->subStruct->numVertices, 0, toRender->primCount - 2);

        g_d3d8Device_A32894->SetVertexShader(currVs); // revert the vertex shader
    }
    else
    {
        g_d3d8Device_A32894->SetStreamSource(0, vertexBuffer, 32);
        g_d3d8Device_A32894->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, toRender->subStruct->numVertices, 0, toRender->primCount - 2);
    }
    // END MY CODE

    // ORIGINAL CODE
    // g_d3d8Device_A32894->SetStreamSource(0, vertexBuffer, 32);
    // g_d3d8Device_A32894->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, toRender->subStruct->numVertices, 0, toRender->primCount - 2);
    // END ORIGINAL CODE
}

void PatchEnvSpecular()
{
    //WriteCalltoMemory(reinterpret_cast<BYTE*>(0x476C81), CreateShaders_5AF110);

    //WriteCalltoMemory(reinterpret_cast<BYTE*>(0x5B0568), sub_5B2D40);
    //WriteCalltoMemory(reinterpret_cast<BYTE*>(0x5B05D9), sub_5B2D40);
    //WriteCalltoMemory(reinterpret_cast<BYTE*>(0x5B49F4), sub_5B2D40);

    // Reimpl function responsible for drawing ACTORS
    WriteCalltoMemory(reinterpret_cast<BYTE*>(0x501F77), sub_501540);

    // Reimpl function responsible for drawing MAP
    //WriteCalltoMemory(reinterpret_cast<BYTE*>(0x5B4CB1), sub_5B4940);
    //WriteCalltoMemory(reinterpret_cast<BYTE*>(0x5B5188), sub_5B4940);
    //WriteCalltoMemory(reinterpret_cast<BYTE*>(0x5B5269), sub_5B4940);
}

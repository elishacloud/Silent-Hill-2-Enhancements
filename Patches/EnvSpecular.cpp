//#define WIN32_LEAN_AND_MEAN
#include "EnvSpecular.h"
#include "Common\Settings.h"
#include "Common\Utils.h"
#include "Patches.h"
#include "Wrappers\d3d8\DirectX81SDK\include\d3dx8math.h"

#include <cmath>    // for std::powf

DWORD windowVsHandle = 0;
DWORD windowPsHandle = 0;

DWORD magentaPsHandle = 0;

IDirect3DTexture8* g_SpecularLUT = nullptr;
#define SPECULAR_LUT_TEXTURE_SLOT 1

void(__cdecl* sub_5B20C0)() = (void(__cdecl*)())0x5B20C0;
void(__cdecl* sub_5B2190)(DWORD) = (void(__cdecl*)(DWORD))0x5B2190;
void(__cdecl* sub_5B2240)(DWORD) = (void(__cdecl*)(DWORD))0x5B2240;

char* byte_8F0338 = reinterpret_cast<char*>(0x8F0338);
char* g_materialTypes_8F0339 = reinterpret_cast<char*>(0x8F0339);
D3DPRIMITIVETYPE* g_primitiveTypes_8F0378 = reinterpret_cast<D3DPRIMITIVETYPE*>(0x8F0378);

IDirect3DDevice8*& g_d3d8Device_A32894 = *reinterpret_cast<IDirect3DDevice8**>(0xA32894);
int& g_HACK_DX_CONFIG_USE_PIXEL_SHADERS_A33370 = *reinterpret_cast<int*>(0xA33370);
BOOL& g_HACK_DX_CONFIG_USE_VERTEX_SHADERS_A33374 = *reinterpret_cast<BOOL*>(0xA33374);
int& dword_A333B8 = *reinterpret_cast<int*>(0xA333B8);

int& g_materialIndex_1DB8A28 = *reinterpret_cast<int*>(0x1DB8A28);
DWORD* g_vsHandles_1DB88A8 = reinterpret_cast<DWORD*>(0x1DB88A8);
DWORD* g_psHandles_1DB89A8 = reinterpret_cast<DWORD*>(0x1DB89A8);

float& g_fogStart_1F5EE70 = *reinterpret_cast<float*>(0x1F5EE70);
float& g_fogEnd_1F5EE74 = *reinterpret_cast<float*>(0x1F5EE74);

float* g_FlashLightPos = reinterpret_cast<float*>(0x01FB7D18);
float* g_FlashLightDir = reinterpret_cast<float*>(0x01FB7D28);

static void GenerateSpecularLUT() {
    // keep the with-to_height ratio <= 8:1
    constexpr UINT kSpecularLutW = 512u;
    constexpr UINT kSpecularLutH = 64u;
    // tune this
    const float kSpecPower = EnvSpecPower;

    HRESULT hr = g_d3d8Device_A32894->CreateTexture(kSpecularLutW, kSpecularLutH, 1u, 0u, D3DFMT_A8, D3DPOOL_MANAGED, &g_SpecularLUT);
    if (SUCCEEDED(hr)) {
        D3DLOCKED_RECT lockedRect{};
        hr = g_SpecularLUT->LockRect(0u, &lockedRect, nullptr, 0);

        if (SUCCEEDED(hr)) {
            BYTE* a8 = reinterpret_cast<BYTE*>(lockedRect.pBits);
            for (UINT y = 0u; y < kSpecularLutH; ++y) {
                for (UINT x = 0u; x < kSpecularLutW; ++x, ++a8) {
                    const float f = std::powf(static_cast<float>(x) / static_cast<float>(kSpecularLutW - 1u), kSpecPower);
                    *a8 = static_cast<BYTE>((std::min)(255.0f, (std::max)(0.0f, f * 255.0f)));
                }
            }

            g_SpecularLUT->UnlockRect(0u);
        }
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
    SubSomething3* pSubStruct3;
    UINT primitiveCount;
    MapMaterial* mapMaterial;
    int off_0x04;
    SubSubSomething* off_0x0C;
    unsigned int i;
    bool v8;
    SubSomething3* v9;
    int mode;
    UINT startIndex;
    unsigned int v12;
    int v13;
    IDirect3DIndexBuffer8* indexBuffer;
    VBStruct* vertexBufferStruct;
    SubSomething2* strides;
    D3DMATERIAL8 material;

    strides = toRender->strides;
    vertexBufferStruct = toRender->vertexBufferStruct;
    pSubStruct3 = toRender->pSubStruct3;
    v9 = pSubStruct3;
    indexBuffer = toRender->indexBuffer;
    mode = 8;
    v13 = -1;
    startIndex = 0;
    v12 = 0;

    primitiveCount = 0; // Not original

    if (toRender->off_0x30)
    {
        while (1)
        {
            mapMaterial = &toRender->mapMaterial[pSubStruct3->off_0x00];
            g_d3d8Device_A32894->SetTexture(0, mapMaterial->tex1);     // base diffuse texture
            
            if (g_HACK_DX_CONFIG_USE_VERTEX_SHADERS_A33374 && g_HACK_DX_CONFIG_USE_PIXEL_SHADERS_A33370)
                g_d3d8Device_A32894->SetTexture(1, mapMaterial->tex2); // Haven't seen an instance where this is non-null

            if (mode != mapMaterial->mode_0x10)
            {
                mode = mapMaterial->mode_0x10;
                sub_5B2D40(mode);
            }

            if (!mapMaterial->tex1 && !g_HACK_DX_CONFIG_USE_VERTEX_SHADERS_A33374 && !mode)
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
                
                mode = -1;
            }

            sub_5B2190(mapMaterial->materialColor_0x00);
            sub_5B2240(mapMaterial->overlayColor_0x04);

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
                    if (toRender->mapMaterial->mode_0x10 == 1 || toRender->mapMaterial->mode_0x10 == 2) // If drawing a cutout or specular material
                    {
                        if (!g_SpecularLUT) {
                            GenerateSpecularLUT();
                        }

                        // Assign specular highlight texture to slot 1
                        IDirect3DBaseTexture8* savedTexture = nullptr;
                        g_d3d8Device_A32894->GetTexture(SPECULAR_LUT_TEXTURE_SLOT, &savedTexture);
                        g_d3d8Device_A32894->SetTexture(SPECULAR_LUT_TEXTURE_SLOT, g_SpecularLUT);
                        // Set up sampler states
                        g_d3d8Device_A32894->SetTextureStageState(SPECULAR_LUT_TEXTURE_SLOT, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
                        g_d3d8Device_A32894->SetTextureStageState(SPECULAR_LUT_TEXTURE_SLOT, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
                        g_d3d8Device_A32894->SetTextureStageState(SPECULAR_LUT_TEXTURE_SLOT, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
                        g_d3d8Device_A32894->SetTextureStageState(SPECULAR_LUT_TEXTURE_SLOT, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
                        g_d3d8Device_A32894->SetTextureStageState(SPECULAR_LUT_TEXTURE_SLOT, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

                        // tune this!
                        float specColor[4] = { EnvSpecRed, EnvSpecGreen, EnvSpecBlue, EnvSpecAlpha };
                        g_d3d8Device_A32894->SetVertexShaderConstant(27, specColor, 1);

                        g_d3d8Device_A32894->SetVertexShaderConstant(28, g_FlashLightPos, 1);

                        float cameraPos[4] = {
                            GetInGameCameraPosX(),
                            GetInGameCameraPosY(),
                            GetInGameCameraPosZ(),
                            0.0f
                        };

                        g_d3d8Device_A32894->SetVertexShaderConstant(29, cameraPos, 1);

                        DWORD currVs;
                        DWORD currPs;
                        g_d3d8Device_A32894->GetVertexShader(&currVs);
                        g_d3d8Device_A32894->GetPixelShader(&currPs);
                        g_d3d8Device_A32894->SetVertexShader(windowVsHandle);

                        g_d3d8Device_A32894->SetPixelShader(windowPsHandle);

                        // Set up sampler states
                        g_d3d8Device_A32894->SetTextureStageState(4, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
                        g_d3d8Device_A32894->SetTextureStageState(4, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
                        g_d3d8Device_A32894->SetTextureStageState(4, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
                        g_d3d8Device_A32894->SetTextureStageState(4, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
                        g_d3d8Device_A32894->SetTextureStageState(4, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

                        g_d3d8Device_A32894->DrawIndexedPrimitive(g_primitiveTypes_8F0378[off_0x0C->off_0x02], off_0x0C->off_0x04, off_0x0C->off_0x06 - off_0x0C->off_0x04 + 1, startIndex, primitiveCount);

                        g_d3d8Device_A32894->SetVertexShader(currVs);
                        g_d3d8Device_A32894->SetPixelShader(currPs);

                        g_d3d8Device_A32894->SetTexture(SPECULAR_LUT_TEXTURE_SLOT, savedTexture);
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

void PatchEnvSpecular()
{
    // Reimpl function responsible for drawing MAP
    WriteCalltoMemory(reinterpret_cast<BYTE*>(0x5B4CB1), sub_5B4940);
    WriteCalltoMemory(reinterpret_cast<BYTE*>(0x5B5188), sub_5B4940);
    WriteCalltoMemory(reinterpret_cast<BYTE*>(0x5B5269), sub_5B4940);
}

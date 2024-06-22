#include "FlashlightReflection.h"
#include "Patches.h"
#include "Common\IUnknownPtr.h"
#include "Common\Settings.h"
#include "Common\Utils.h"
#include "Wrappers\d3d8\DirectX81SDK\include\d3dx8math.h"

#include <cmath>    // for std::powf

DWORD windowVsHandle = 0;
DWORD windowPsHandle = 0;
DWORD vcolorVsHandle = 0;

DWORD hospitalDoorVsHandle = 0;
DWORD hospitalDoorPsHandles[4] = {0, 0, 0, 0};

constexpr float specColor[4] = { 0.4f, 0.4f, 0.4f, 1.0f };
#define SPECULAR_POWER 400.0f

#define WINDOW_VSHADER_ORIGINAL  (g_vsHandles_1DB88A8[2])
#define VCOLOR_VSHADER_ORIGINAL  (g_vsHandles_1DB88A8[8])

#define MODEL_VSHADER_ORIGINAL   (g_mdlVsHandles_1F7D684[7])

#define VSHADER_FLASHLIGHT_POS_REGISTER 90
#define VSHADER_CAMERA_POS_REGISTER     91

IDirect3DTexture8* g_SpecularLUT = nullptr;
#define SPECULAR_LUT_TEXTURE_SLOT 1

IDirect3DTexture8*& g_flashLightTexture_1F5F16C = *reinterpret_cast<IDirect3DTexture8**>(0x1F5F16C);

DWORD* g_vsHandles_1DB88A8 = reinterpret_cast<DWORD*>(0x1DB88A8);

DWORD* g_mdlVsHandles_1F7D684 = reinterpret_cast<DWORD*>(0x1F7D684); // 11 vertex shader handles
DWORD* g_mdlPsHandles_1F7D6C4 = reinterpret_cast<DWORD*>(0x1F7D6C4); // 5 pixel shader handles

float* g_FlashLightPos = reinterpret_cast<float*>(0x01FB7D18);

static int IsPixelShaderMDLFadeOrFullBright(DWORD handle) {
    if (g_mdlPsHandles_1F7D6C4[0] == handle) {
        return 0;
    }  else if (g_mdlPsHandles_1F7D6C4[1] == handle) {
        return 1;
    } else if (g_mdlPsHandles_1F7D6C4[2] == handle) {
        return 2;
    } else if (g_mdlPsHandles_1F7D6C4[3] == handle) {
        return 3;
    } else {
        return -1;
    }
}

static void GenerateSpecularLUT(LPDIRECT3DDEVICE8 ProxyInterface) {
    // keep the with-to_height ratio <= 8:1
    constexpr UINT kSpecularLutW = 512u;
    constexpr UINT kSpecularLutH = 64u;
    // tune this
    const float kSpecPower = SPECULAR_POWER;

    HRESULT hr = ProxyInterface->CreateTexture(kSpecularLutW, kSpecularLutH, 1u, 0u, D3DFMT_A8, D3DPOOL_MANAGED, &g_SpecularLUT);
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

HRESULT DrawFlashlightReflection(LPDIRECT3DDEVICE8 ProxyInterface, D3DPRIMITIVETYPE Type, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
    IUnknownPtr<IDirect3DTexture8> texture;
	ProxyInterface->GetTexture(0, (IDirect3DBaseTexture8**)texture.ReleaseAndGetAddressOf());

	D3DSURFACE_DESC desc;
	texture->GetLevelDesc(0, &desc);

	DWORD currVs, currPs;
	ProxyInterface->GetVertexShader(&currVs);
	ProxyInterface->GetPixelShader(&currPs);

	int flashlightPhase = IsPixelShaderMDLFadeOrFullBright(currPs);

	// SH2 uses DXT3 exclusively for transparent textures, when DXT4 is detected we know it is one of our
	// enhanced textures which we've added a "grime" layer to to restore the flashlight reflection effect
	if ((currVs == WINDOW_VSHADER_ORIGINAL || currVs == VCOLOR_VSHADER_ORIGINAL) && desc.Format == D3DFMT_DXT4)
	{
		if (!g_SpecularLUT) {
			GenerateSpecularLUT(ProxyInterface);
		}

		const bool isVColorGeometry = (currVs == VCOLOR_VSHADER_ORIGINAL);

		// Assign specular highlight texture to slot 1
        IUnknownPtr<IDirect3DBaseTexture8> savedTexture;
		ProxyInterface->GetTexture(SPECULAR_LUT_TEXTURE_SLOT, savedTexture.ReleaseAndGetAddressOf());
		ProxyInterface->SetTexture(SPECULAR_LUT_TEXTURE_SLOT, g_SpecularLUT);

		// Set up sampler states
		ProxyInterface->SetTextureStageState(SPECULAR_LUT_TEXTURE_SLOT, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		ProxyInterface->SetTextureStageState(SPECULAR_LUT_TEXTURE_SLOT, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		ProxyInterface->SetTextureStageState(SPECULAR_LUT_TEXTURE_SLOT, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		ProxyInterface->SetTextureStageState(SPECULAR_LUT_TEXTURE_SLOT, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		ProxyInterface->SetTextureStageState(SPECULAR_LUT_TEXTURE_SLOT, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		ProxyInterface->SetVertexShaderConstant(27, specColor, 1);

		float cameraPos[4] = {
			GetInGameCameraPosX(),
			GetInGameCameraPosY(),
			GetInGameCameraPosZ(),
			0.0f
		};

		ProxyInterface->SetVertexShaderConstant(VSHADER_FLASHLIGHT_POS_REGISTER, g_FlashLightPos, 1);
		ProxyInterface->SetVertexShaderConstant(VSHADER_CAMERA_POS_REGISTER, cameraPos, 1);

		ProxyInterface->SetVertexShader(isVColorGeometry ? vcolorVsHandle : windowVsHandle);

		ProxyInterface->SetPixelShader(windowPsHandle);

		// Set up sampler states
		ProxyInterface->SetTextureStageState(4, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
		ProxyInterface->SetTextureStageState(4, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
		ProxyInterface->SetTextureStageState(4, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		ProxyInterface->SetTextureStageState(4, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		ProxyInterface->SetTextureStageState(4, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		HRESULT hr = ProxyInterface->DrawIndexedPrimitive(Type, MinVertexIndex, NumVertices, startIndex, primCount);

		ProxyInterface->SetVertexShader(currVs);
		ProxyInterface->SetPixelShader(currPs);

		ProxyInterface->SetTexture(SPECULAR_LUT_TEXTURE_SLOT, savedTexture.GetPtr());

		return hr;
	}
	else if (currVs == MODEL_VSHADER_ORIGINAL && desc.Format == D3DFMT_DXT4 && flashlightPhase >= 0)
	{
		if (!g_SpecularLUT) {
			GenerateSpecularLUT(ProxyInterface);
		}

		// Assign specular highlight texture to slot 1
        IUnknownPtr<IDirect3DBaseTexture8> savedTexture;
		ProxyInterface->GetTexture(SPECULAR_LUT_TEXTURE_SLOT, savedTexture.ReleaseAndGetAddressOf());
		ProxyInterface->SetTexture(SPECULAR_LUT_TEXTURE_SLOT, g_SpecularLUT);
		// Set up sampler states
		ProxyInterface->SetTextureStageState(SPECULAR_LUT_TEXTURE_SLOT, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		ProxyInterface->SetTextureStageState(SPECULAR_LUT_TEXTURE_SLOT, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		ProxyInterface->SetTextureStageState(SPECULAR_LUT_TEXTURE_SLOT, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		ProxyInterface->SetTextureStageState(SPECULAR_LUT_TEXTURE_SLOT, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		ProxyInterface->SetTextureStageState(SPECULAR_LUT_TEXTURE_SLOT, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

        IUnknownPtr<IDirect3DBaseTexture8> savedTexture2;
		ProxyInterface->GetTexture(2, savedTexture2.ReleaseAndGetAddressOf());
		ProxyInterface->SetTexture(2, g_flashLightTexture_1F5F16C);
		ProxyInterface->SetTextureStageState(2, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
		ProxyInterface->SetTextureStageState(2, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
		ProxyInterface->SetTextureStageState(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		ProxyInterface->SetTextureStageState(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		ProxyInterface->SetTextureStageState(2, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		const float flashlightIntensity = GetFlashlightBrightnessRed() / 7.0f;
		const float specColorInt[4] = { specColor[0] * flashlightIntensity, specColor[1] * flashlightIntensity, specColor[2] * flashlightIntensity, specColor[3] };
		ProxyInterface->SetVertexShaderConstant(27, specColorInt, 1);

		float cameraPos[4] = {
			GetInGameCameraPosX(),
			GetInGameCameraPosY(),
			GetInGameCameraPosZ(),
			0.0f
		};

		float savedConstants[6 * 4] = {};

		float savedConstants2[4 * 4] = {};

		ProxyInterface->GetVertexShaderConstant(VSHADER_FLASHLIGHT_POS_REGISTER, savedConstants, 6);
		ProxyInterface->GetVertexShaderConstant(20, savedConstants2, 4);

		D3DXMATRIX viewMat = {};
		D3DXMATRIX projMat, viewProjTransMat;
		D3DXMATRIX viewMatInv = {};
		D3DXMATRIX viewMatInvTrans = {};
		ProxyInterface->GetTransform(D3DTS_VIEW, &viewMat);
		ProxyInterface->GetTransform(D3DTS_PROJECTION, &projMat);

		D3DXVECTOR4 dxFlashLightPos = {}, dxFlashLightPosIn = { g_FlashLightPos[0], g_FlashLightPos[1], g_FlashLightPos[2], 1.0f };
		D3DXVECTOR4 dxCameraPos = {}, dxCameraPosIn = { cameraPos[0], cameraPos[1], cameraPos[2], 1.0f };
		D3DXVec4Transform(&dxFlashLightPos, &dxFlashLightPosIn, &viewMat);
		D3DXVec4Transform(&dxCameraPos, &dxCameraPosIn, &viewMat);

		ProxyInterface->SetVertexShaderConstant(VSHADER_FLASHLIGHT_POS_REGISTER, &dxFlashLightPos.x, 1);

		D3DXMatrixMultiplyTranspose(&viewProjTransMat, &viewMat, &projMat);
		ProxyInterface->SetVertexShaderConstant(20, &viewProjTransMat, 4);

		D3DXMatrixInverse(&viewMatInv, nullptr, &viewMat);
		D3DXMatrixTranspose(&viewMatInvTrans, &viewMatInv);
		ProxyInterface->SetVertexShaderConstant(92, &viewMatInvTrans, 4);

		ProxyInterface->SetVertexShader(hospitalDoorVsHandle);

		ProxyInterface->SetPixelShader(hospitalDoorPsHandles[flashlightPhase]);

		HRESULT hr = ProxyInterface->DrawIndexedPrimitive(Type, MinVertexIndex, NumVertices, startIndex, primCount);

		ProxyInterface->SetVertexShader(currVs);
		ProxyInterface->SetPixelShader(currPs);
		ProxyInterface->SetTexture(SPECULAR_LUT_TEXTURE_SLOT, savedTexture.GetPtr());
        ProxyInterface->SetTexture(2, savedTexture2.GetPtr());

		ProxyInterface->SetVertexShaderConstant(20, savedConstants2, 4);
		ProxyInterface->SetVertexShaderConstant(VSHADER_FLASHLIGHT_POS_REGISTER, savedConstants, 6);

		return hr;
	}

	return -1;
}

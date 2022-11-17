/**
* Copyright (C) 2022 Elisha Riedlinger
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

#include "d3d8wrapper.h"
#include <shlwapi.h>
#include <chrono>
#include <ctime>
#include "Common\Utils.h"
#include "stb_image.h"
#include "stb_image_dds.h"
#include "stb_image_write.h"
#include "stb_image_resize.h"
#include "Patches/ModelID.h"

bool DeviceLost = false;
bool DisableShaderOnPresent = false;
bool IsInFullscreenImage = false;
bool IsInBloomEffect = false;
bool IsInFakeFadeout = false;
bool ClassReleaseFlag = false;
bool TextureSet = false;
DWORD TextureNum = 0;
Overlay OverlayRef;

struct SCREENSHOTSTRUCT
{
	bool Enabled = false;
	std::vector<BYTE> buffer;
	std::vector<BYTE> bufferRaw;
	std::wstring filename;
	DWORD size = 0;
	INT Pitch = 0;
	LONG Width = 0;
	LONG Height = 0;
};

std::vector<SCREENSHOTSTRUCT> ScreenshotVector;

HRESULT m_IDirect3DDevice8::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	Logging::LogDebug() << __FUNCTION__;

	if ((riid == IID_IDirect3DDevice8 || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	HRESULT hr = ProxyInterface->QueryInterface(riid, ppvObj);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppvObj, this);
	}

	return hr;
}

ULONG m_IDirect3DDevice8::AddRef()
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DDevice8::Release()
{
	Logging::LogDebug() << __FUNCTION__;

	ULONG count = ProxyInterface->Release();

	if (count == 0)
	{
		delete this;
	}

	return count;
}

HRESULT m_IDirect3DDevice8::Reset(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	Logging::LogDebug() << __FUNCTION__;

	DeviceLost = false;

	isInScene = false;

	pCurrentRenderTexture = nullptr;

	pInitialRenderTexture = nullptr;

	for (SURFACEVECTOR SurfaceStruct : SurfaceVector)
	{
		if (SurfaceStruct.RenderTarget)
		{
			ReleaseInterface(&SurfaceStruct.RenderTarget);
		}
		if (SurfaceStruct.SourceTarget)
		{
			SurfaceStruct.SourceTarget->QueryInterface(IID_ClearRenderTarget, nullptr);
		}
	}
	SurfaceVector.clear();

	if (pInRender)
	{
		ReleaseInterface(&pInRender);
	}

	if (pInSurface)
	{
		ReleaseInterface(&pInSurface, 2);
	}

	if (pInTexture)
	{
		ReleaseInterface(&pInTexture);
	}

	if (pShrunkSurface)
	{
		ReleaseInterface(&pShrunkSurface, 2);
	}

	if (pShrunkTexture)
	{
		ReleaseInterface(&pShrunkTexture);
	}

	if (pOutSurface)
	{
		ReleaseInterface(&pOutSurface, 2);
	}

	if (pOutTexture)
	{
		ReleaseInterface(&pOutTexture);
	}

	if (silhouetteRender)
	{
		ReleaseInterface(&silhouetteRender);
	}

	if (silhouetteSurface)
	{
		ReleaseInterface(&silhouetteSurface, 2);
	}

	if (silhouetteTexture)
	{
		ReleaseInterface(&silhouetteTexture);
	}

	OverlayRef.ResetFont();

	// Update presentation parameters
	UpdatePresentParameter(pPresentationParameters, nullptr, true);

	// Set AntiAliasing
	if (DeviceMultiSampleType)
	{
		D3DPRESENT_PARAMETERS d3dpp;
		CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
		d3dpp.BackBufferCount = (d3dpp.BackBufferCount) ? d3dpp.BackBufferCount : 1;

		// Update Present Parameter for Multisample
		UpdatePresentParameterForMultisample(&d3dpp, DeviceMultiSampleType);

		// Reset device
		if (SUCCEEDED(ProxyInterface->Reset(&d3dpp)))
		{
			return D3D_OK;
		}

		// If failed
		DeviceMultiSampleType = D3DMULTISAMPLE_NONE;
		Logging::Log() << __FUNCTION__ << " Error: MultiSample disabled!!!";
	}

	return ProxyInterface->Reset(pPresentationParameters);
}

HRESULT m_IDirect3DDevice8::EndScene()
{
	Logging::LogDebug() << __FUNCTION__;

	// Skip frames in specific cutscenes to prevent flickering
	if (RemoveEnvironmentFlicker)
	{
		if ((LastCutsceneID == 0x01 && SkipSceneCounter < 4 && (SkipSceneCounter || GetJamesPosX() != LastJamesPosX)) ||
			(LastCutsceneID == 0x03 && SkipSceneCounter < 1 && (SkipSceneCounter || GetJamesPosX() == 330.845f)) ||
			((LastCutsceneID == 0x15 || LastCutsceneID == 0x16) && SkipSceneCounter < 1 && (SkipSceneCounter || GetCutsceneID() != LastCutsceneID || (ClassReleaseFlag && !(GetCutscenePos() == *(float*)"\xAE\x01\x31\x46" && LastCameraPos == 0))) && !(GetCutsceneID() == 0x16 && LastCutsceneID == 0x15)) ||
			(LastCutsceneID == 0x16 && SkipSceneCounter < 4 && (SkipSceneCounter || (GetCutscenePos() != LastCameraPos && GetCutscenePos() == *(float*)"\x40\xA1\xA8\x45")) && GetCutsceneID() == 0x16) ||
			(LastCutsceneID == 0x4C && SkipSceneCounter < 1 && (SkipSceneCounter || GetCutsceneID() != LastCutsceneID)) ||
			(LastCutsceneID == 0x4D && SkipSceneCounter < 2 && (SkipSceneCounter || GetCutsceneID() != LastCutsceneID || ClassReleaseFlag)) ||
			(LastCutsceneID == 0x4D && SkipSceneCounter < 3 && (SkipSceneCounter || GetCutscenePos() != LastCameraPos) && GetCutscenePos() == *(float*)"\x59\xCC\x06\xC6" && GetCutsceneID() == 0x4D))
		{
			LOG_LIMIT(1, "Skipping frame during cutscene!");
			Logging::LogDebug() << __FUNCTION__ " frame - Counter " << SkipSceneCounter << " Release: " << ClassReleaseFlag << " CutsceneID: " << GetCutsceneID() << " LastCutsceneID: " << LastCutsceneID <<
				" CutsceneCameraPos: " << GetCutscenePos() << " LastCameraPos: " << LastCameraPos << " JamesPos: " << GetJamesPosX() << " LastJamesPos: " << LastJamesPosX;

			SkipSceneFlag = true;
			SkipSceneCounter++;

			return D3D_OK;
		}

		SkipSceneFlag = false;
		SkipSceneCounter = 0;
		LastCutsceneID = GetCutsceneID();
		LastCameraPos = GetCutscenePos();
		LastJamesPosX = GetJamesPosX();
	}

	// Reset flag for black pillar boxes
	if (!LastFrameFullscreenImage && !IsInFullscreenImage)
	{
		DontModifyClear = false;
	}

	EndSceneCounter++;

	return D3D_OK;
}

void m_IDirect3DDevice8::SetCursorPosition(THIS_ UINT XScreenSpace, UINT YScreenSpace, DWORD Flags)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetCursorPosition(XScreenSpace, YScreenSpace, Flags);
}

HRESULT m_IDirect3DDevice8::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface8 *pCursorBitmap)
{
	Logging::LogDebug() << __FUNCTION__;

	if (pCursorBitmap)
	{
		pCursorBitmap = static_cast<m_IDirect3DSurface8 *>(pCursorBitmap)->GetProxyInterface();
	}

	return ProxyInterface->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

BOOL m_IDirect3DDevice8::ShowCursor(BOOL bShow)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->ShowCursor(bShow);
}

HRESULT m_IDirect3DDevice8::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain8** ppSwapChain)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = D3DERR_INVALIDCALL;

	// Update presentation parameters
	UpdatePresentParameter(pPresentationParameters, nullptr, false);

	// Set AntiAliasing
	if (DeviceMultiSampleType)
	{
		D3DPRESENT_PARAMETERS d3dpp;
		CopyMemory(&d3dpp, pPresentationParameters, sizeof(D3DPRESENT_PARAMETERS));
		d3dpp.BackBufferCount = (d3dpp.BackBufferCount) ? d3dpp.BackBufferCount : 1;

		// Update Present Parameter for Multisample
		UpdatePresentParameterForMultisample(&d3dpp, DeviceMultiSampleType);

		// Create swap chain
		hr = ProxyInterface->CreateAdditionalSwapChain(&d3dpp, ppSwapChain);
	}

	if (FAILED(hr))
	{
		hr = ProxyInterface->CreateAdditionalSwapChain(pPresentationParameters, ppSwapChain);
	}

	if (SUCCEEDED(hr))
	{
		*ppSwapChain = new m_IDirect3DSwapChain8(*ppSwapChain, this);
	}

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to create swap chain! " <<
			(pPresentationParameters ? pPresentationParameters->BackBufferWidth : 0) << "x" <<
			(pPresentationParameters ? pPresentationParameters->BackBufferHeight : 0);
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::CreateCubeTexture(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture8** ppCubeTexture)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture);

	if (SUCCEEDED(hr) && ppCubeTexture)
	{
		*ppCubeTexture = new m_IDirect3DCubeTexture8(*ppCubeTexture, this);
	}

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to create cubed texture! " << EdgeLength;
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::CreateDepthStencilSurface(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8** ppSurface)
{
	Logging::LogDebug() << __FUNCTION__;

	MultiSample = DeviceMultiSampleType;

	HRESULT hr = ProxyInterface->CreateDepthStencilSurface(Width, Height, Format, MultiSample, ppSurface);

	if (SUCCEEDED(hr) && ppSurface)
	{
		*ppSurface = new m_IDirect3DSurface8(*ppSurface, this);
	}

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to create depth stencil surface! " << Width << "x" << Height;
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::CreateIndexBuffer(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer8** ppIndexBuffer)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer);

	if (SUCCEEDED(hr) && ppIndexBuffer)
	{
		*ppIndexBuffer = new m_IDirect3DIndexBuffer8(*ppIndexBuffer, this);
	}

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to create index buffer! " << Length;
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::CreateRenderTarget(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, IDirect3DSurface8** ppSurface)
{
	Logging::LogDebug() << __FUNCTION__;

	if (DeviceMultiSampleType)
	{
		Lockable = FALSE;
		MultiSample = DeviceMultiSampleType;
	}

	HRESULT hr = ProxyInterface->CreateRenderTarget(Width, Height, Format, MultiSample, Lockable, ppSurface);

	if (SUCCEEDED(hr) && ppSurface)
	{
		*ppSurface = new m_IDirect3DSurface8(*ppSurface, this);
	}

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to create render target! " << Width << "x" << Height;
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::CreateTexture(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8** ppTexture)
{
	Logging::LogDebug() << __FUNCTION__;

	// Fixes FMV issue with UAC prompt
	if (FixFMVResetIssue && Usage == D3DUSAGE_DYNAMIC && Format == D3DFMT_A8R8G8B8 && Pool == D3DPOOL_DEFAULT)
	{
		Usage = 0;
		Pool = D3DPOOL_MANAGED;
	}

	HRESULT hr = ProxyInterface->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture);

	if (SUCCEEDED(hr) && ppTexture)
	{
		IDirect3DTexture8 *pCreatedTexture = *ppTexture;
		*ppTexture = new m_IDirect3DTexture8(*ppTexture, this);

		if (!pInitialRenderTexture && Usage == D3DUSAGE_RENDERTARGET && Width == (UINT)BufferWidth && Height == (UINT)BufferHeight)
		{
			pInitialRenderTexture = *ppTexture;

			// If cached data exists then copy to render target
			if (PauseScreenFix && CachedSurfaceData.size() == (Width * 4) * Height)
			{
				IDirect3DSurface8 *pTmpSurface = nullptr;
				if (SUCCEEDED(ProxyInterface->CreateImageSurface(Width, Height, Format, &pTmpSurface)))
				{
					D3DLOCKED_RECT LockedRect = {};
					if (SUCCEEDED(pTmpSurface->LockRect(&LockedRect, nullptr, 0)))
					{
						memcpy(LockedRect.pBits, &CachedSurfaceData[0], LockedRect.Pitch * Height);
						pTmpSurface->UnlockRect();
					}
					IDirect3DSurface8 *pTmpTargetSurface = nullptr;
					if (SUCCEEDED(pCreatedTexture->GetSurfaceLevel(0, &pTmpTargetSurface)))
					{
						POINT PointDest = { 0, 0 };
						RECT Rect = { 0, 0, BufferWidth, BufferHeight };
						ProxyInterface->CopyRects(pTmpSurface, &Rect, 1, pTmpTargetSurface, &PointDest);
						pTmpTargetSurface->Release();
					}
					pTmpSurface->Release();
				}
			}
		}
	}

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to create texture! " << Width << "x" << Height;
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::CreateVertexBuffer(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer8** ppVertexBuffer)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer);

	if (SUCCEEDED(hr) && ppVertexBuffer)
	{
		*ppVertexBuffer = new m_IDirect3DVertexBuffer8(*ppVertexBuffer, this);
	}

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to create vertex buffer! " << Length;
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::CreateVolumeTexture(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture8** ppVolumeTexture)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture);

	if (SUCCEEDED(hr) && ppVolumeTexture)
	{
		*ppVolumeTexture = new m_IDirect3DVolumeTexture8(*ppVolumeTexture, this);
	}

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to create volume texture! " << Width << "x" << Height;
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::BeginStateBlock()
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->BeginStateBlock();
}

HRESULT m_IDirect3DDevice8::CreateStateBlock(THIS_ D3DSTATEBLOCKTYPE Type, DWORD* pToken)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->CreateStateBlock(Type, pToken);
}

HRESULT m_IDirect3DDevice8::ApplyStateBlock(THIS_ DWORD Token)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->ApplyStateBlock(Token);
}

HRESULT m_IDirect3DDevice8::CaptureStateBlock(THIS_ DWORD Token)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->CaptureStateBlock(Token);
}

HRESULT m_IDirect3DDevice8::DeleteStateBlock(THIS_ DWORD Token)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->DeleteStateBlock(Token);
}

HRESULT m_IDirect3DDevice8::EndStateBlock(THIS_ DWORD* pToken)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->EndStateBlock(pToken);
}

HRESULT m_IDirect3DDevice8::GetClipStatus(D3DCLIPSTATUS8 *pClipStatus)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetClipStatus(pClipStatus);
}

HRESULT m_IDirect3DDevice8::GetDisplayMode(THIS_ D3DDISPLAYMODE* pMode)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetDisplayMode(pMode);
}

HRESULT m_IDirect3DDevice8::GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetRenderState(State, pValue);
}

HRESULT m_IDirect3DDevice8::GetRenderTarget(THIS_ IDirect3DSurface8** ppRenderTarget)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->GetRenderTarget(ppRenderTarget);

	if (SUCCEEDED(hr) && ppRenderTarget)
	{
		*ppRenderTarget = ProxyAddressLookupTableD3d8->FindAddress<m_IDirect3DSurface8>(*ppRenderTarget);
	}

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to get render target!";
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetTransform(State, pMatrix);
}

HRESULT m_IDirect3DDevice8::SetClipStatus(CONST D3DCLIPSTATUS8 *pClipStatus)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetClipStatus(pClipStatus);
}

HRESULT m_IDirect3DDevice8::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	Logging::LogDebug() << __FUNCTION__;

	// Fix for 2D Fog and glow around the flashlight lens for Nvidia cards
	if (FogLayerFix && State == D3DRS_ZBIAS)
	{
		Value = (Value * 15) / 16;
	}

	// Restores self shadows
	if (EnableXboxShadows && State == D3DRS_STENCILPASS && Value == D3DSTENCILOP_REPLACE)
	{
		// Special handling for room 54
		if (GetCutsceneID() == 0x54 && (IsEnabledForCutscene54 || GetCutscenePos() == -19521.60742f))
		{
			IsEnabledForCutscene54 = true;
			Value = D3DSTENCILOP_ZERO; // Restore self shadows
		}
		else if (GetChapterID() == 0x01) // Born From a Wish
		{
			IsEnabledForCutscene54 = false;
			if (GetSpecializedLight1() != 0x01) // If not in a specialized lighting zone
			{
				if (GetRoomID() != 0x20 && GetRoomID() != 0x25 && GetRoomID() != 0x26) // Exclude Blue Creek hallways/staircase completely from restored self shadows
				{
					Value = D3DSTENCILOP_ZERO; // Restore self shadows
				}
			}
		}
		else // Main campaign
		{
			IsEnabledForCutscene54 = false;
			if (GetCutsceneID() == 0x4E || (GetSpecializedLight1() != 0x01 && GetSpecializedLight2() != 0x01))	// Exclude specialized lighting zone unless in specific cutscene
			{
				if (GetRoomID() != 0x9E) // Exclude Hotel Room 202-204 completely from restored self shadows
				{
					Value = D3DSTENCILOP_ZERO; // Restore self shadows
				}
			}
		}
	}

	// For blur frame flicker fix
	if (RemoveEffectsFlicker && State == D3DRS_TEXTUREFACTOR)
	{
		if (!OverrideTextureLoop && EndSceneCounter != 0)
		{
			OverrideTextureLoop = true;
			PresentFlag = true;
		}
		// Known values to exclude: 0x11000000, 0xFF000000, 0xC31E1E1E, B7222222, B9222222, BA202020, BA1E1E1E, BA1C1C1C, BA1A1A1A, BA171717, BA121212
		if (OverrideTextureLoop && PresentFlag && !((Value & 0xFF) == ((Value >> 8) & 0xFF) && ((Value >> 8) & 0xFF) == ((Value >> 16) & 0xFF)))
		{
			Value = 0xFFFFFFFF;
			IsInBloomEffect = true;
		}
	}

	return ProxyInterface->SetRenderState(State, Value);
}

HRESULT m_IDirect3DDevice8::SetRenderTarget(THIS_ IDirect3DSurface8* pRenderTarget, IDirect3DSurface8* pNewZStencil)
{
	Logging::LogDebug() << __FUNCTION__;

	// Check if surface should be copied
	if (ReplacedLastRenderTarget)
	{
		IDirect3DSurface8 *pCurrentTarget = nullptr;
		if (SUCCEEDED(ProxyInterface->GetRenderTarget(&pCurrentTarget)) && pCurrentTarget)
		{
			pCurrentTarget->Release();
			pCurrentTarget = ProxyAddressLookupTableD3d8->FindAddress<m_IDirect3DSurface8>(pCurrentTarget);

			IDirect3DSurface8 *pReplacedSurface = nullptr;
			if (SUCCEEDED(pCurrentTarget->QueryInterface(IID_GetReplacedInterface, (void**)&pReplacedSurface)) && pReplacedSurface)
			{
				UpdateSurface(pCurrentTarget, pReplacedSurface);
			}
		}
	}
	ReplacedLastRenderTarget = false;

	if (pRenderTarget)
	{
		IDirect3DSurface8 *pSurface = nullptr;
		if (SUCCEEDED(pRenderTarget->QueryInterface(IID_GetReplacedInterface, (void**)&pSurface)) && pSurface)
		{
			pRenderTarget = pSurface;
		}

		// Check if surface needs to be replaced
		pSurface = nullptr;
		if (SUCCEEDED(pRenderTarget->QueryInterface(IID_GetRenderTarget, (void**)&pSurface)) && pSurface)
		{
			ReplacedLastRenderTarget = true;
			pRenderTarget = pSurface;
		}

		// Get proxy interface
		pSurface = nullptr;
		if (SUCCEEDED(pRenderTarget->QueryInterface(IID_GetProxyInterface, (void**)&pSurface)) && pSurface)
		{
			pRenderTarget = pSurface;
		}
	}

	if (pNewZStencil)
	{
		// Get proxy interface
		IDirect3DSurface8 *pSurface = nullptr;
		if (SUCCEEDED(pNewZStencil->QueryInterface(IID_GetProxyInterface, (void**)&pSurface)) && pSurface)
		{
			pNewZStencil = pSurface;
		}
	}

	return ProxyInterface->SetRenderTarget(pRenderTarget, pNewZStencil);
}

HRESULT m_IDirect3DDevice8::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetTransform(State, pMatrix);
}

void m_IDirect3DDevice8::GetGammaRamp(THIS_ D3DGAMMARAMP* pRamp)
{
	Logging::LogDebug() << __FUNCTION__;

	ProxyInterface->GetGammaRamp(pRamp);
}

void m_IDirect3DDevice8::SetGammaRamp(THIS_ DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{
	Logging::LogDebug() << __FUNCTION__;

	// Don't enable shaders until the game calls SetGamma to make sure all the shader settings are initialized
	ShadersReady = EnableCustomShaders;

	if (ScreenMode != WINDOWED || (RestoreBrightnessSelector && d3d8to9))
	{
		ProxyInterface->SetGammaRamp(Flags, pRamp);
	}
}

HRESULT m_IDirect3DDevice8::DeletePatch(UINT Handle)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->DeletePatch(Handle);
}

HRESULT m_IDirect3DDevice8::DrawRectPatch(UINT Handle, CONST float *pNumSegs, CONST D3DRECTPATCH_INFO *pRectPatchInfo)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

HRESULT m_IDirect3DDevice8::DrawTriPatch(UINT Handle, CONST float *pNumSegs, CONST D3DTRIPATCH_INFO *pTriPatchInfo)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT m_IDirect3DDevice8::GetIndices(THIS_ IDirect3DIndexBuffer8** ppIndexData, UINT* pBaseVertexIndex)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->GetIndices(ppIndexData, pBaseVertexIndex);

	if (SUCCEEDED(hr) && ppIndexData)
	{
		*ppIndexData = ProxyAddressLookupTableD3d8->FindAddress<m_IDirect3DIndexBuffer8>(*ppIndexData);
	}

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to get index buffer!";
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::SetIndices(THIS_ IDirect3DIndexBuffer8* pIndexData, UINT BaseVertexIndex)
{
	Logging::LogDebug() << __FUNCTION__;

	if (pIndexData)
	{
		pIndexData = static_cast<m_IDirect3DIndexBuffer8 *>(pIndexData)->GetProxyInterface();
	}

	return ProxyInterface->SetIndices(pIndexData, BaseVertexIndex);
}

UINT m_IDirect3DDevice8::GetAvailableTextureMem()
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetAvailableTextureMem();
}

HRESULT m_IDirect3DDevice8::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetCreationParameters(pParameters);
}

HRESULT m_IDirect3DDevice8::GetDeviceCaps(D3DCAPS8 *pCaps)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetDeviceCaps(pCaps);
}

HRESULT m_IDirect3DDevice8::GetDirect3D(IDirect3D8** ppD3D9)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!ppD3D9)
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to get Direct3D!";
		return D3DERR_INVALIDCALL;
	}

	m_pD3D->AddRef();

	*ppD3D9 = m_pD3D;

	return D3D_OK;
}

HRESULT m_IDirect3DDevice8::GetRasterStatus(THIS_ D3DRASTER_STATUS* pRasterStatus)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetRasterStatus(pRasterStatus);
}

HRESULT m_IDirect3DDevice8::GetLight(DWORD Index, D3DLIGHT8 *pLight)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetLight(Index, pLight);
}

HRESULT m_IDirect3DDevice8::GetLightEnable(DWORD Index, BOOL *pEnable)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetLightEnable(Index, pEnable);
}

HRESULT m_IDirect3DDevice8::GetMaterial(D3DMATERIAL8 *pMaterial)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetMaterial(pMaterial);
}

HRESULT m_IDirect3DDevice8::LightEnable(DWORD LightIndex, BOOL bEnable)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->LightEnable(LightIndex, bEnable);
}

HRESULT m_IDirect3DDevice8::SetLight(DWORD Index, CONST D3DLIGHT8 *pLight)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetLight(Index, pLight);
}

HRESULT m_IDirect3DDevice8::SetMaterial(CONST D3DMATERIAL8 *pMaterial)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetMaterial(pMaterial);
}

HRESULT m_IDirect3DDevice8::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->MultiplyTransform(State, pMatrix);
}

HRESULT m_IDirect3DDevice8::ProcessVertices(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer8* pDestBuffer, DWORD Flags)
{
	Logging::LogDebug() << __FUNCTION__;

	if (pDestBuffer)
	{
		pDestBuffer = static_cast<m_IDirect3DVertexBuffer8 *>(pDestBuffer)->GetProxyInterface();
	}

	return ProxyInterface->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, Flags);
}

HRESULT m_IDirect3DDevice8::TestCooperativeLevel()
{
	Logging::LogDebug() << __FUNCTION__;

	if (DeviceLost)
	{
		return D3DERR_DEVICENOTRESET;
	}

	return ProxyInterface->TestCooperativeLevel();
}

HRESULT m_IDirect3DDevice8::GetCurrentTexturePalette(UINT *pPaletteNumber)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetCurrentTexturePalette(pPaletteNumber);
}

HRESULT m_IDirect3DDevice8::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY *pEntries)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT m_IDirect3DDevice8::SetCurrentTexturePalette(UINT PaletteNumber)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetCurrentTexturePalette(PaletteNumber);
}

HRESULT m_IDirect3DDevice8::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY *pEntries)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT m_IDirect3DDevice8::CreatePixelShader(THIS_ CONST DWORD* pFunction, DWORD* pHandle)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->CreatePixelShader(pFunction, pHandle);
}

HRESULT m_IDirect3DDevice8::GetPixelShader(THIS_ DWORD* pHandle)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetPixelShader(pHandle);
}

HRESULT m_IDirect3DDevice8::SetPixelShader(THIS_ DWORD Handle)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetPixelShader(Handle);
}

HRESULT m_IDirect3DDevice8::DeletePixelShader(THIS_ DWORD Handle)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->DeletePixelShader(Handle);
}

HRESULT m_IDirect3DDevice8::GetPixelShaderFunction(THIS_ DWORD Handle, void* pData, DWORD* pSizeOfData)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetPixelShaderFunction(Handle, pData, pSizeOfData);
}

HRESULT m_IDirect3DDevice8::Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
	Logging::LogDebug() << __FUNCTION__;

	// Skip frames in specific cutscenes to prevent flickering
	if (SkipSceneFlag)
	{
		return D3D_OK;
	}

	// Draw Overlays
	OverlayRef.DrawOverlays(ProxyInterface);

	// Endscene
	isInScene = false;
	ProxyInterface->EndScene();

	// Update in progress
	static bool ClearScreen = true;
	if (!m_StopThreadFlag && IsUpdating && !IsGetFrontBufferCalled)
	{
		if (ClearScreen)
		{
			ClearScreen = false;
			ProxyInterface->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
		}

		return D3D_OK;
	}
	ClearScreen = true;

	// Fix pause menu
	bool PauseMenuFlag = false;
	if (GetEventIndex() == 16)
	{
		if (PauseScreenFix && !InPauseMenu && pCurrentRenderTexture)
		{
			IDirect3DSurface8 *pCurrentRenderSurface = nullptr;
			if (SUCCEEDED(pCurrentRenderTexture->GetSurfaceLevel(0, &pCurrentRenderSurface)))
			{
				pCurrentRenderSurface->Release();
				FakeGetFrontBuffer(pCurrentRenderSurface);
				PauseMenuFlag = true;
			}
		}
		InPauseMenu = true;
	}
	else
	{
		InPauseMenu = false;
	}

	// Check if shader needs to be disabled
	if (EnableCustomShaders && !PauseMenuFlag)
	{
		// Get variables
		static BYTE LastEvent = 0;
		IsGetFrontBufferCalled = (GetTransitionState() == 1 || GetLoadingScreen() != 0) ? IsGetFrontBufferCalled : false;

		// Set shader disable flag
		DisableShaderOnPresent = (IsGetFrontBufferCalled || (PauseScreenFix && (GetEventIndex() == 16 || (LastEvent == 16 && IsSnapshotTextureSet))));

		// Reset variables
		LastEvent = GetEventIndex();
		IsSnapshotTextureSet = false;
	}

	HRESULT hr = D3D_OK;

	// Disable Transparency Supersampling
	if (SetSSAA)
	{
		ProxyInterface->SetRenderState((D3DRENDERSTATETYPE)D3DRS_ADAPTIVETESS_Y, D3DFMT_UNKNOWN);
	}

	// Present screen
	if (!PauseMenuFlag)
	{
		hr = ProxyInterface->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	}

	// Take screenshot
	if (TakeScreenShot)
	{
		if (!isInScene)
		{
			isInScene = true;
			ProxyInterface->BeginScene();
		}
		TakeScreenShot = false;
		CaptureScreenShot();
	}

	// Fix inventory snapshot in Hotel Employee Elevator Room
	if (HotelEmployeeElevatorRoomFlag)
	{
		if (pInitialRenderTexture && GetOnScreen() == 6)
		{
			if (!isInScene)
			{
				isInScene = true;
				ProxyInterface->BeginScene();
			}
			IDirect3DSurface8 *pSnapshotSurface = nullptr;
			if (SUCCEEDED(pInitialRenderTexture->GetSurfaceLevel(0, &pSnapshotSurface)))
			{
				pSnapshotSurface->Release();
				FakeGetFrontBuffer(pSnapshotSurface);
			}
		}
		HotelEmployeeElevatorRoomFlag = FALSE;
	}

	LastDrawPrimitiveUPStride = 0;

	// Shadow fading counter
	if (DrawingShadowsFlag)
	{
		ShadowFadingCounter++;
	}

	DrawingShadowsFlag = false;
	IsNoiseFilterVertexSet = false;

	// For blur frame flicker fix
	if (EndSceneCounter == 1)
	{
		OverrideTextureLoop = false;
		IsInBloomEffect = false;
	}

	// Reset pillarbox values
	PillarBoxLeft = 0.0f;
	PillarBoxRight = 0.0f;

	EndSceneCounter = 0;
	PresentFlag = false;

	return hr;
}

HRESULT m_IDirect3DDevice8::DrawIndexedPrimitive(THIS_ D3DPRIMITIVETYPE Type, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	Logging::LogDebug() << __FUNCTION__;

	// Drawing opaque map geometry and dynamic objects
	if (EnableXboxShadows && !shadowVolumeFlag)
	{
		// Change default states to be more like Xbox version
		ProxyInterface->SetRenderState(D3DRS_STENCILMASK, 0xFFFFFFFF);
		ProxyInterface->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFF);

		// Drawing just opaque map geometry
		DWORD stencilRef = 0;
		if (SUCCEEDED(ProxyInterface->GetRenderState(D3DRS_STENCILREF, &stencilRef)) && stencilRef == 0)
		{
			// Change default states to be more like Xbox version
			ProxyInterface->SetRenderState(D3DRS_STENCILENABLE, TRUE);
			ProxyInterface->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
			ProxyInterface->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_ZERO);
		}
	}

	// Exclude Woodside Room 208 TV static geometry from receiving shadows
	if (EnableSoftShadows && GetRoomID() == 0x18 && GetModelID() == ModelID::chr_item_noa)
	{
		DWORD stencilPass = 0;
		ProxyInterface->GetRenderState(D3DRS_STENCILPASS, &stencilPass);

		ProxyInterface->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

		HRESULT hr = ProxyInterface->DrawIndexedPrimitive(Type, MinVertexIndex, NumVertices, startIndex, primCount);

		ProxyInterface->SetRenderState(D3DRS_STENCILPASS, stencilPass);

		return hr;
	}
	// Exclude windows in Heaven's Night, Hotel 2F Room Hallway and Hotel Storeroom from receiving shadows
	else if (EnableSoftShadows &&
		((GetRoomID() == 0x0C && Type == D3DPT_TRIANGLESTRIP && MinVertexIndex == 0 && NumVertices == 18 && startIndex == 0 && primCount == 21) ||
		(GetRoomID() == 0x9F && Type == D3DPT_TRIANGLESTRIP && MinVertexIndex == 0 && NumVertices == 10 && startIndex == 0 && primCount == 10) ||
		(GetRoomID() == 0x94 && Type == D3DPT_TRIANGLESTRIP && MinVertexIndex == 0 && NumVertices == 8 && startIndex == 0 && primCount == 8)))
	{
		DWORD stencilPass, stencilRef = 0;

		// Backup renderstates
		ProxyInterface->GetRenderState(D3DRS_STENCILPASS, &stencilPass);
		ProxyInterface->GetRenderState(D3DRS_STENCILREF, &stencilRef);

		// Set states so we don't receive shadows
		ProxyInterface->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
		ProxyInterface->SetRenderState(D3DRS_STENCILREF, 1);

		HRESULT hr = ProxyInterface->DrawIndexedPrimitive(Type, MinVertexIndex, NumVertices, startIndex, primCount);

		// Restore renderstates
		ProxyInterface->SetRenderState(D3DRS_STENCILPASS, stencilPass);
		ProxyInterface->SetRenderState(D3DRS_STENCILREF, stencilRef);

		return hr;
	}
	// Exclude refrigerator interior in hospital from receiving shadows
	else if (EnableSoftShadows && GetRoomID() == 0x53 && Type == D3DPT_TRIANGLESTRIP && MinVertexIndex == 0 && NumVertices == 1037 && startIndex == 0 && primCount == 1580)
	{
		DWORD stencilPass = 0;

		// Backup renderstates
		ProxyInterface->GetRenderState(D3DRS_STENCILPASS, &stencilPass);

		// Set states so we don't receive shadows
		ProxyInterface->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

		HRESULT hr = ProxyInterface->DrawIndexedPrimitive(Type, MinVertexIndex, NumVertices, startIndex, primCount);

		// Restore renderstates
		ProxyInterface->SetRenderState(D3DRS_STENCILPASS, stencilPass);

		return hr;
	}

	return ProxyInterface->DrawIndexedPrimitive(Type, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT m_IDirect3DDevice8::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT IndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->DrawIndexedPrimitiveUP(PrimitiveType, MinIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT m_IDirect3DDevice8::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	Logging::LogDebug() << __FUNCTION__;

	// Set pillar boxes to black (removes game images from pillars)
	if (LastFrameFullscreenImage && !IsInFullscreenImage && GetRoomID() && !GetCutsceneID())
	{
		if (GetRoomID() == 0x08)
		{
			DontModifyClear = true;
		}
		return ProxyInterface->Clear(0x00, nullptr, D3DCLEAR_TARGET | D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0xFF, 0x00, 0x00, 0x00), 1.0f, 0x80);
	}
	// Set pillar boxes to black (removes street decals from West Town fullscreen images)
	else if (IsInFullscreenImage && PrimitiveType == D3DPT_TRIANGLESTRIP && PrimitiveCount == 2 && GetRoomID() == 0x08)
	{
		return D3D_OK;
	}

	// Drawing transparent dynamic objects (Character hair etc.)
	if (EnableXboxShadows && !shadowVolumeFlag)
	{
		// Change default states to be more like Xbox version
		ProxyInterface->SetRenderState(D3DRS_STENCILMASK, 0xFFFFFFFF);
		ProxyInterface->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFF);
	}

	// Disable shadow on the Labyrinth Valve
	if (EnableSoftShadows && GetCutsceneID() == 0x46 && PrimitiveType == D3DPT_TRIANGLELIST && PrimitiveCount > 496 && PrimitiveCount < 536)
	{
		return D3D_OK;
	}
	// Top Down Shadow
	else if (EnableSoftShadows && ((GetRoomID() == 0x02 || GetRoomID() == 0x24 || GetRoomID() == 0x8F || GetRoomID() == 0x90) || GetCutsceneID() == 0x5A))
	{
		DWORD stencilPass = 0;
		ProxyInterface->GetRenderState(D3DRS_STENCILPASS, &stencilPass);

		if (stencilPass == D3DSTENCILOP_INCR)
		{
			DWORD stencilFunc = 0;
			ProxyInterface->GetRenderState(D3DRS_STENCILFUNC, &stencilFunc);

			ProxyInterface->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
			ProxyInterface->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESS);

			HRESULT hr = ProxyInterface->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);

			ProxyInterface->SetRenderState(D3DRS_STENCILPASS, stencilPass);
			ProxyInterface->SetRenderState(D3DRS_STENCILFUNC, stencilFunc);

			return hr;
		}
	}

	DWORD stencilPass = 0;
	ProxyInterface->GetRenderState(D3DRS_STENCILPASS, &stencilPass);

	if (EnableXboxShadows && stencilPass == D3DSTENCILOP_INCR && !shadowVolumeFlag)
	{
		// Drawing shadow volumes, toggle flag on
		shadowVolumeFlag = true;

		IDirect3DSurface8 *backbufferColor, *backbufferDepthStencil = nullptr;

		// Backup backbuffer and depth/stencil buffer
		if (SUCCEEDED(ProxyInterface->GetRenderTarget(&backbufferColor)) && backbufferColor)
		{
			backbufferColor->Release();
		}
		if (SUCCEEDED(ProxyInterface->GetDepthStencilSurface(&backbufferDepthStencil)) && backbufferDepthStencil)
		{
			backbufferDepthStencil->Release();
		}

		// DrawPrimitive trashes vertex buffer stream 0, back it up
		IDirect3DVertexBuffer8 *pStream0 = nullptr;
		UINT stream0Stride = 0;
		if (SUCCEEDED(ProxyInterface->GetStreamSource(0, &pStream0, &stream0Stride)) && pStream0)
		{
			pStream0->Release();
		}

		// Error checking
		if (!CheckSilhouetteTexture() || !backbufferColor || !backbufferDepthStencil || !pStream0)
		{
			return D3DERR_INVALIDCALL;
		}

		// Temporarily swap to new color buffer (keep original depth/stencil)
		ProxyInterface->SetRenderTarget(((silhouetteRender) ? silhouetteRender : silhouetteSurface), backbufferDepthStencil);
		ProxyInterface->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		CUSTOMVERTEX_DIF fullscreenQuad[] =
		{
			{0.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(255, 255, 255, 255)},
			{0.0f, (float)BufferHeight, 0.0f, 1.0f, D3DCOLOR_ARGB(255, 255, 255, 255)},
			{(float)BufferWidth, 0.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(255, 255, 255, 255)},
			{(float)BufferWidth, (float)BufferHeight, 0.0f, 1.0f, D3DCOLOR_ARGB(255, 255, 255, 255)}
		};

		// Bias coords to align correctly to screen space
		for (int i = 0; i < 4; i++)
		{
			fullscreenQuad[i].x -= 0.5f;
			fullscreenQuad[i].y -= 0.5f;
		}

		// Backup alpha blend and alpha test enable, disable both
		DWORD alphaBlend, alphaTest = 0;
		ProxyInterface->GetRenderState(D3DRS_ALPHABLENDENABLE, &alphaBlend);
		ProxyInterface->GetRenderState(D3DRS_ALPHATESTENABLE, &alphaTest);
		ProxyInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		ProxyInterface->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

		// Discard all pixels but James' silhouette
		DWORD stencilFunc = 0;
		ProxyInterface->GetRenderState(D3DRS_STENCILFUNC, &stencilFunc);
		ProxyInterface->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);

		// Backup FVF, use pre-transformed vertices and diffuse color
		DWORD vshader = 0;
		ProxyInterface->GetVertexShader(&vshader);
		ProxyInterface->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

		// Backup cullmode, disable culling so we don't have to worry about winding order
		DWORD cullMode = 0;
		ProxyInterface->GetRenderState(D3DRS_CULLMODE, &cullMode);
		ProxyInterface->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		// Draw James' silhouette to our texture
		ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, fullscreenQuad, 20);

		// Restore state
		ProxyInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, alphaBlend);
		ProxyInterface->SetRenderState(D3DRS_ALPHATESTENABLE, alphaTest);
		ProxyInterface->SetStreamSource(0, pStream0, stream0Stride);
		ProxyInterface->SetRenderState(D3DRS_STENCILFUNC, stencilFunc);
		ProxyInterface->SetVertexShader(vshader);
		ProxyInterface->SetRenderState(D3DRS_CULLMODE, cullMode);

		// Bring back our original color buffer, clear stencil buffer like Xbox
		ProxyInterface->SetRenderTarget(backbufferColor, backbufferDepthStencil);
		if (silhouetteRender)
		{
			UpdateSurface(silhouetteRender, silhouetteSurface);
		}
		ProxyInterface->Clear(0, NULL, D3DCLEAR_STENCIL, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0x80);
	}

	return ProxyInterface->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT m_IDirect3DDevice8::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	Logging::LogDebug() << __FUNCTION__;

	LastDrawPrimitiveUPStride += VertexStreamZeroStride;

	// Remove cursor during fade
	if (EnableCustomShaders && PrimitiveType == D3DPT_TRIANGLESTRIP && PrimitiveCount == 2 && VertexStreamZeroStride == 24 &&
		(GetTransitionState() == 1 || GetTransitionState() == 2 || GetTransitionState() == 3 || (GetEventIndex() == 7 && LastDrawPrimitiveUPStride == 2024)))
	{
		CUSTOMVERTEX_TEX1 *pVertex = (CUSTOMVERTEX_TEX1*)pVertexStreamZeroData;

		float Width = pVertex[1].x - pVertex[0].x;
		float Height = pVertex[2].y - pVertex[1].y;

		float ComputeWidth = truncf(BufferHeight * (4.0f / 3.0f)) * (15.0f / 256.0f);
		float ComputeHeight = BufferHeight / 16.0f;

		if (pVertex[0].z == 0.010f && pVertex[0].rhw == 1.0f &&
			pVertex[0].u == 0.0f && pVertex[0].v == 0.0f &&
			pVertex[1].u == 1.0f && pVertex[1].v == 0.0f &&
			pVertex[2].u == 0.0f && pVertex[2].v == 1.0f &&
			pVertex[3].u == 1.0f && pVertex[3].v == 1.0f &&
			Width > ComputeWidth - 0.1f && Width < ComputeWidth + 0.1f &&
			Height > ComputeHeight - 0.1f && Height < ComputeHeight + 0.1f)
		{
			return D3D_OK;
		}
	}

	// Remove red cross in inventory snapshot in Hotel Employee Elevator Room
	if (DisableRedCrossInCutScenes && HotelEmployeeElevatorRoomFlag && GetOnScreen() == 6 && PrimitiveType == D3DPT_TRIANGLESTRIP && PrimitiveCount == 2 && VertexStreamZeroStride == 24 &&
		((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[0].z == 0.01f && ((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[0].rhw == 1.0f)
	{
		return D3D_OK;
	}

	// Fix bowling cutscene fading
	if (GetCutsceneID() == 0x19 && PrimitiveType == D3DPT_TRIANGLELIST && PrimitiveCount == 2 && VertexStreamZeroStride == 28 && pVertexStreamZeroData &&
		((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[0].z == 0.01f && ((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[1].z == 0.01f && ((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[2].z == 0.01f)
	{
		IsInFakeFadeout = true;

		for (const DWORD &x : { 1, 2, 4 })
		{
			FullScreenFadeout[x].x = (float)BufferWidth - 0.5f;
		}

		for (int x = 0; x < 6; x++)
		{
			FullScreenFadeout[x].y = ((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[x].y;
			FullScreenFadeout[x].color = ((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[x].color;
		}

		pVertexStreamZeroData = FullScreenFadeout;
	}
	// Detect fullscreen images for fixing pillar box color
	else if (SetBlackPillarBoxes && PrimitiveType == D3DPT_TRIANGLELIST && PrimitiveCount == 2 && VertexStreamZeroStride == 28 && pVertexStreamZeroData &&
		((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[0].z == 0.01f && ((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[1].z == 0.01f && ((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[2].z == 0.01f)
	{
		// Get fullscreen image left and right values
		if (!IsInFullscreenImage)
		{
			PillarBoxLeft = ((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[0].x;
			PillarBoxRight = ((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[1].x;
			PillarBoxTop = ((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[1].y;
			PillarBoxBottom = ((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[2].y;
		}
		// Clip artifacts that protrude into pillarbox
		else if (PillarBoxLeft && PillarBoxRight && GetRoomID() && !GetCutsceneID() && (GetOnScreen() == 4 || GetOnScreen() == 5))
		{
			// Clip green player marker
			if (pVertexStreamZeroData && ((((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[1].x != ((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[2].x ||
				((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[3].x != ((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[5].x)))
			{
				bool SkipDrawing = true;
				bool DrawPillarBoxes = false;
				for (int x = 0; x < 6; x++)
				{
					if (((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[x].x > PillarBoxLeft && ((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[x].x < PillarBoxRight)
					{
						SkipDrawing = false;
					}
					if (((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[x].x < PillarBoxLeft || ((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[x].x > PillarBoxRight)
					{
						DrawPillarBoxes = true;
					}
				}
				if (SkipDrawing)
				{
					return D3D_OK;
				}
				// Add vertex to cover green marker that protrudes into the pillar box
				if (DrawPillarBoxes)
				{
					// Draw green marker
					ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, pVertexStreamZeroData, 28);

					// Add left and right black pillar boxes
					ProxyInterface->SetTexture(0, BlankTexture);
					ProxyInterface->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_TEX1);

					// Set top and bottom vertex
					for (const DWORD &x : { 0, 1, 3 })
					{
						PillarBoxVertex[x].y = PillarBoxTop;
					}
					for (const DWORD &x : { 2, 4, 5 })
					{
						PillarBoxVertex[x].y = PillarBoxBottom;
					}

					// Set vertex for right pillar box and draw primitive
					for (const DWORD &x : { 0, 3, 5 })
					{
						PillarBoxVertex[x].x = PillarBoxRight;
					}
					for (const DWORD &x : { 1, 2, 4 })
					{
						PillarBoxVertex[x].x = (float)BufferWidth;
					}
					ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, PillarBoxVertex, 24);

					// Set vertex for left pillar box and draw primitive
					for (const DWORD &x : { 0, 3, 5 })
					{
						PillarBoxVertex[x].x = 0.0f;
					}
					for (const DWORD &x : { 1, 2, 4 })
					{
						PillarBoxVertex[x].x = PillarBoxLeft;
					}
					return ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, PillarBoxVertex, 24);
				}
			}
			// Clip other artifacts
			else if (pVertexStreamZeroData)
			{
				// Artifact is completely in the pillarbox so don't draw it
				if (((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[1].x < PillarBoxLeft || ((CUSTOMVERTEX_DIF_TEX1*)pVertexStreamZeroData)[0].x > PillarBoxRight)
				{
					return D3D_OK;
				}

				// Copy vertex
				memcpy(FullScreenArtifact, pVertexStreamZeroData, sizeof(FullScreenArtifact));
				pVertexStreamZeroData = FullScreenArtifact;

				// Update vertex left
				if (FullScreenArtifact[0].x < PillarBoxLeft)
				{
					float TexWidth = FullScreenArtifact[1].u - FullScreenArtifact[0].u;
					float VertexWidth = FullScreenArtifact[1].x - FullScreenArtifact[0].x;
					float NewVertexWidth = FullScreenArtifact[1].x - PillarBoxLeft;

					float TextLeft = FullScreenArtifact[0].u + (TexWidth * (1.0f - (NewVertexWidth / VertexWidth)));
					for (const DWORD &x : { 0, 3, 5 })
					{
						FullScreenArtifact[x].x = PillarBoxLeft;
						FullScreenArtifact[x].u = TextLeft;
					}
				}

				// Update vertex right
				if (FullScreenArtifact[1].x > PillarBoxRight)
				{
					float TexWidth = FullScreenArtifact[1].u - FullScreenArtifact[0].u;
					float VertexWidth = FullScreenArtifact[1].x - FullScreenArtifact[0].x;
					float NewVertexWidth = PillarBoxRight - FullScreenArtifact[0].x;

					float TextRight = FullScreenArtifact[1].u - (TexWidth * (1.0f - (NewVertexWidth / VertexWidth)));
					for (const DWORD &x : { 1, 2, 4 })
					{
						FullScreenArtifact[x].x = PillarBoxRight;
						FullScreenArtifact[x].u = TextRight;
					}
				}
			}
		}

		IsInFullscreenImage = true;
	}
	// Set pillar boxes to black (removes fog from West Town fullscreen images)
	else if (IsInFullscreenImage && PrimitiveType == D3DPT_TRIANGLEFAN && PrimitiveCount == 4 && VertexStreamZeroStride == 24 && GetRoomID() == 0x08)
	{
		return D3D_OK;
	}
	// Set pillar boxes to black (removes noise filter from fullscreen images and limits noise filter size for FMVs)
	else if (PrimitiveType == D3DPT_TRIANGLESTRIP && PrimitiveCount == 2 && VertexStreamZeroStride == 24 && pVertexStreamZeroData &&
		((CUSTOMVERTEX_TEX1*)pVertexStreamZeroData)[0].z == 0.0f && ((CUSTOMVERTEX_TEX1*)pVertexStreamZeroData)[1].z == 0.0f)
	{
		if (IsInFullscreenImage)
		{
			// Remove noise filter from fullscreen images
			return D3D_OK;
		}
		else if (IsNoiseFilterVertexSet)
		{
			// Limit noise filter vertex to size of FMV
			pVertexStreamZeroData = FMVVertex;
		}
	}

	// Get the vertex of the FMV
	if (WidescreenFix && PrimitiveType == D3DPT_TRIANGLESTRIP && PrimitiveCount == 2 && VertexStreamZeroStride == 24 && LastDrawPrimitiveUPStride == VertexStreamZeroStride && pVertexStreamZeroData &&
		(GetEventIndex() == 7 || GetEventIndex() == 15))
	{
		IsNoiseFilterVertexSet = true;
		memcpy_s(FMVVertex, sizeof(FMVVertex), pVertexStreamZeroData, sizeof(CUSTOMVERTEX_TEX1) * 4);
	}

	// Draw Self/Soft Shadows
	DWORD stencilRef;
	if (SUCCEEDED(ProxyInterface->GetRenderState(D3DRS_STENCILREF, &stencilRef)) && stencilRef == 129)
	{
		if (EnableXboxShadows)
		{
			// Done drawing shadow volumes, toggle flag off
			shadowVolumeFlag = false;

			// DrawPrimitiveUP trashes vertex buffer stream 0, back it up
			IDirect3DVertexBuffer8 *pStream0 = nullptr;
			UINT stream0Stride = 0;
			if (SUCCEEDED(ProxyInterface->GetStreamSource(0, &pStream0, &stream0Stride)) && pStream0)
			{
				pStream0->Release();
			}

			// Error checking
			if (!CheckSilhouetteTexture() || !pStream0)
			{
				return D3DERR_INVALIDCALL;
			}

			// Backup current texture
			IDirect3DBaseTexture8 *pTexture = nullptr;
			if (SUCCEEDED(ProxyInterface->GetTexture(0, &pTexture)) && pTexture)
			{
				pTexture->Release();
			}

			// Bind our texture of James' silhouette
			ProxyInterface->SetTexture(0, silhouetteTexture);

			CUSTOMVERTEX_TEX1 fullscreenQuad[] =
			{
				{0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
				{0.0f, (float)BufferHeight, 0.0f, 1.0f, 0.0f, 1.0f},
				{(float)BufferWidth, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
				{(float)BufferWidth, (float)BufferHeight, 0.0f, 1.0f, 1.0f, 1.0f}
			};

			// Bias coords to align correctly to screen space
			for (int i = 0; i < 4; i++)
			{
				fullscreenQuad[i].x -= 0.5f;
				fullscreenQuad[i].y -= 0.5f;
			}

			// Backup alpha blend and alpha test enable, disable alpha blend, enable alpha test
			DWORD alphaBlend, alphaTest = 0;
			ProxyInterface->GetRenderState(D3DRS_ALPHABLENDENABLE, &alphaBlend);
			ProxyInterface->GetRenderState(D3DRS_ALPHATESTENABLE, &alphaTest);
			ProxyInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			ProxyInterface->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);

			// Backup FVF, use pre-transformed vertices and texture coords
			DWORD vshader = 0;
			ProxyInterface->GetVertexShader(&vshader);
			ProxyInterface->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_TEX1);

			// Backup cullmode, disable culling so we don't have to worry about winding order
			DWORD cullMode = 0;
			ProxyInterface->GetRenderState(D3DRS_CULLMODE, &cullMode);
			ProxyInterface->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

			// Backup stencil func, use D3DCMP_ALWAYS to stop stencil interfering with alpha test
			DWORD stencilFunc = 0;
			ProxyInterface->GetRenderState(D3DRS_STENCILFUNC, &stencilFunc);
			ProxyInterface->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);

			// Backup stencil pass, so that James' silhouette clears that portion of the stencil buffer
			DWORD stencilPass = 0;
			ProxyInterface->GetRenderState(D3DRS_STENCILPASS, &stencilPass);
			ProxyInterface->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_ZERO);

			// Backup color write value, we only want to touch the stencil buffer
			DWORD colorWrite = 0;
			ProxyInterface->GetRenderState(D3DRS_COLORWRITEENABLE, &colorWrite);
			ProxyInterface->SetRenderState(D3DRS_COLORWRITEENABLE, 0); // Comment me for debug fun

			// Use texture color and alpha
			DWORD colorArg1, alphaArg1 = 0;
			ProxyInterface->GetTextureStageState(0, D3DTSS_COLORARG1, &colorArg1);
			ProxyInterface->GetTextureStageState(0, D3DTSS_ALPHAARG1, &alphaArg1);
			ProxyInterface->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			ProxyInterface->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

			// Clear James' silhouette from the stencil buffer
			ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, fullscreenQuad, 24);

			// Backup alpha blend and alpha test enable, disable alpha blend, enable alpha test
			ProxyInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, alphaBlend);
			ProxyInterface->SetRenderState(D3DRS_ALPHATESTENABLE, alphaTest);
			ProxyInterface->SetStreamSource(0, pStream0, stream0Stride);
			ProxyInterface->SetVertexShader(vshader);
			ProxyInterface->SetRenderState(D3DRS_CULLMODE, cullMode);
			ProxyInterface->SetRenderState(D3DRS_STENCILFUNC, stencilFunc);
			ProxyInterface->SetRenderState(D3DRS_STENCILPASS, stencilPass);
			ProxyInterface->SetRenderState(D3DRS_COLORWRITEENABLE, colorWrite);
			ProxyInterface->SetTextureStageState(0, D3DTSS_COLORARG1, colorArg1);
			ProxyInterface->SetTextureStageState(0, D3DTSS_ALPHAARG1, alphaArg1);

			// Unbind silhouette texture, just to be safe
			ProxyInterface->SetTexture(0, pTexture);
		}

		if (EnableSoftShadows)
		{
			return DrawSoftShadows();
		}
	}

	// Fix drawing line when using '0xFE' byte in '.mes' files
	if (FixDrawingTextLine && pVertexStreamZeroData)
	{
		if (PrimitiveType == D3DPT_LINELIST && PrimitiveCount == 3 && VertexStreamZeroStride == 20) {
			ProxyInterface->DrawPrimitiveUP(PrimitiveType, 1, pVertexStreamZeroData, VertexStreamZeroStride);
			PrimitiveType = D3DPT_TRIANGLESTRIP;
			PrimitiveCount = 2;
			pVertexStreamZeroData = (void *)((BYTE *)pVertexStreamZeroData + VertexStreamZeroStride * 2);
		}
	}

	if ((DeviceMultiSampleType || FixGPUAntiAliasing) && pVertexStreamZeroData)
	{
		DWORD Handle = 0;
		ProxyInterface->GetVertexShader(&Handle);
		if (Handle == D3DFVF_XYZRHW)
		{
			CUSTOMVERTEX *vert = (CUSTOMVERTEX*)pVertexStreamZeroData;
			if (PrimitiveType == D3DPT_TRIANGLELIST &&
				vert[0].x == 0.0f && vert[0].y == 0.0f && vert[0].z == 0.0f && vert[0].rhw == 1.0f &&
				vert[1].x == (float)BufferWidth && vert[1].y == 0.0f && vert[1].z == 0.0f && vert[1].rhw == 1.0f &&
				vert[2].x == (float)BufferWidth && vert[2].y == (float)BufferHeight && vert[2].z == 0.0f && vert[2].rhw == 1.0f &&
				vert[3].x == 0.0f && vert[3].y == 0.0f && vert[3].z == 0.0f && vert[3].rhw == 1.0f &&
				vert[4].x == (float)BufferWidth && vert[4].y == (float)BufferHeight && vert[4].z == 0.0f && vert[4].rhw == 1.0f &&
				vert[5].x == 0.0f && vert[5].y == (float)BufferHeight && vert[5].z == 0.0f && vert[5].rhw == 1.0f)
			{
				for (int x = 0; x < 6; x++)
				{
					vert[x].x -= 0.5f;
					vert[x].y -= 0.5f;
					vert[x].z = 0.01f;
				}
			}
			else if (PrimitiveType == D3DPT_TRIANGLESTRIP &&
				vert[0].x == 0.0f && vert[0].y == 0.0f && vert[0].z == 0.0f && vert[0].rhw == 1.0f &&
				vert[1].x == (float)BufferWidth && vert[1].y == 0.0f && vert[1].z == 0.0f && vert[1].rhw == 1.0f &&
				vert[2].x == 0.0f && vert[2].y == (float)BufferHeight && vert[2].z == 0.0f && vert[2].rhw == 1.0f &&
				vert[3].x == (float)BufferWidth && vert[3].y == (float)BufferHeight && vert[3].z == 0.0f && vert[3].rhw == 1.0f)
			{
				for (int x = 0; x < 4; x++)
				{
					vert[x].x -= 0.5f;
					vert[x].y -= 0.5f;
					vert[x].z = 0.01f;
				}
			}
		}
		else if (Handle == (D3DFVF_XYZRHW | D3DFVF_TEX4))
		{
			CUSTOMVERTEX_TEX4 *vert = (CUSTOMVERTEX_TEX4*)pVertexStreamZeroData;
			if (PrimitiveType == D3DPT_TRIANGLESTRIP &&
				vert[0].x == 0.0f && vert[0].y == 0.0f && vert[0].z == 0.0f && vert[0].rhw == 1.0f &&
				vert[1].x == (float)BufferWidth && vert[1].y == 0.0f && vert[1].z == 0.0f && vert[1].rhw == 1.0f &&
				vert[2].x == 0.0f && vert[2].y == (float)BufferHeight && vert[2].z == 0.0f && vert[2].rhw == 1.0f &&
				vert[3].x == (float)BufferWidth && vert[3].y == (float)BufferHeight && vert[3].z == 0.0f && vert[3].rhw == 1.0f)
			{
				for (int x = 0; x < 4; x++)
				{
					vert[x].x -= 0.5f;
					vert[x].y -= 0.5f;
					vert[x].z = 0.01f;
				}
			}
		}
		// Fix 1 pixel gap in cutscene letterboxes
		else if (PrimitiveType == D3DPT_TRIANGLESTRIP && PrimitiveCount == 2 && VertexStreamZeroStride == 20)
		{
			CUSTOMVERTEX_DIF *vert = (CUSTOMVERTEX_DIF*)pVertexStreamZeroData;
			if (vert[0].x == 0.0f && vert[2].x == 0.0f && (vert[1].x == (float)BufferWidth || vert[0].y == (float)BufferHeight))
			{
				for (int x = 0; x < 4; x++)
				{
					vert[x].x -= 0.5f;
					vert[x].y -= 0.5f;
				}
			}
		}
	}

	return ProxyInterface->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT m_IDirect3DDevice8::BeginScene()
{
	Logging::LogDebug() << __FUNCTION__;

	if (EndSceneCounter == 0)
	{
		// Skip frames in specific cutscenes to prevent flickering
		if (SkipSceneFlag == true)
		{
			return D3D_OK;
		}

		ClassReleaseFlag = false;
		LastFrameFullscreenImage = IsInFullscreenImage;
		IsInFullscreenImage = false;

		// Enable Xbox shadows
		if (EnableSoftShadows)
		{
			EnableXboxShadows = !((GetRoomID() == 0x02 || GetRoomID() == 0x24 || GetRoomID() == 0x8F || GetRoomID() == 0x90) || GetCutsceneID() == 0x5A);
		}

		// Hotel Water Visual Fixes
		if (HotelWaterFix)
		{
			RunHotelWater();
		}

		// Change James' spawn point after the cutscene ends
		if (ChangeClosetSpawn)
		{
			RunClosetSpawn();
		}

		// RPT Apartment Closet Cutscene Fix
		if (ClosetCutsceneFix)
		{
			RunClosetCutscene();
		}

		// RPT Hospital Elevator Stabbing Animation Fix
		if (HospitalChaseFix)
		{
			RunHospitalChase();
		}

		// Hang on Esc Fix
		if (FixHangOnEsc)
		{
			RunHangOnEsc();
		}

		// Fix infinite rumble in pause menu
		if (RestoreVibration)
		{
			RunInfiniteRumbleFix();
		}

		// Fix draw distance in forest with chainsaw logs and Eddie boss meat cold room
		if (IncreaseDrawDistance)
		{
			RunDynamicDrawDistance();
		}

		// Lighting Transition fix
		if (LightingTransitionFix)
		{
			RunLightingTransition();
		}

		// Game save fix
		if (GameLoadFix)
		{
			RunGameLoad();
		}

		// FixSaveBGImage
		if (FixSaveBGImage)
		{
			RunSaveBGImage();
		}

		// Increase blood size
		if (IncreaseBlood)
		{
			RunBloodSize();
		}

		// Fix Fog volume in Hotel Room 312
		if (RestoreSpecialFX)
		{
			RunHotelRoom312FogVolumeFix();
		}

		// Disable shadow in specific cutscenes
		if (EnableSoftShadows)
		{
			RunShadowCutscene();
		}

		// Scale special FX based on resolution
		if (RestoreSpecialFX)
		{
			RunSpecialFXScale(BufferHeight);
		}

		// Scale the inner glow of the flashlight
		if (PS2FlashlightBrightness)
		{
			RunInnerFlashlightGlow(BufferHeight);
		}

		// Tree Lighting fix
		if (LightingFix)
		{
			RunTreeColor();
		}

		// Lighting patch
		if (RoomLightingFix)
		{
			RunRoomLighting();
		}

		// Fix rotating Mannequin glitch
		if (WoodsideRoom205Fix)
		{
			RunRotatingMannequin();
		}

		// Update fog speed
		if (FogSpeedFix)
		{
			RunFogSpeed();
		}

		// Fix flashlight at end of failed clock push cutscene
		if (FixAptClockFlashlight)
		{
			RunFlashlightClockPush();
		}

		// Bowling cutscene fading
		if (IsInFakeFadeout && GetCutsceneID() != 0x19)
		{
			IsInFakeFadeout = false;
		}
	}

	if (!isInScene)
	{
		isInScene = true;
		ProxyInterface->BeginScene();
	}

	if (EndSceneCounter == 0)
	{
		// Set MultiSample
		if (DeviceMultiSampleType)
		{
			ProxyInterface->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
		}

		// Set Transparency Supersampling
		if (SetSSAA)
		{
			ProxyInterface->SetRenderState((D3DRENDERSTATETYPE)D3DRS_ADAPTIVETESS_Y, MAKEFOURCC('S', 'S', 'A', 'A'));
		}
	}

	return D3D_OK;
}

HRESULT m_IDirect3DDevice8::GetStreamSource(THIS_ UINT StreamNumber, IDirect3DVertexBuffer8** ppStreamData, UINT* pStride)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->GetStreamSource(StreamNumber, ppStreamData, pStride);

	if (SUCCEEDED(hr) && ppStreamData)
	{
		*ppStreamData = ProxyAddressLookupTableD3d8->FindAddress<m_IDirect3DVertexBuffer8>(*ppStreamData);
	}

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to get stream source!";
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::SetStreamSource(THIS_ UINT StreamNumber, IDirect3DVertexBuffer8* pStreamData, UINT Stride)
{
	Logging::LogDebug() << __FUNCTION__;

	if (pStreamData)
	{
		pStreamData = static_cast<m_IDirect3DVertexBuffer8 *>(pStreamData)->GetProxyInterface();
	}

	return ProxyInterface->SetStreamSource(StreamNumber, pStreamData, Stride);
}

HRESULT m_IDirect3DDevice8::GetBackBuffer(THIS_ UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface8** ppBackBuffer)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->GetBackBuffer(iBackBuffer, Type, ppBackBuffer);

	if (SUCCEEDED(hr) && ppBackBuffer)
	{
		*ppBackBuffer = ProxyAddressLookupTableD3d8->FindAddress<m_IDirect3DSurface8>(*ppBackBuffer);
	}

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to get backbuffer!";
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::GetDepthStencilSurface(IDirect3DSurface8** ppZStencilSurface)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->GetDepthStencilSurface(ppZStencilSurface);

	if (SUCCEEDED(hr) && ppZStencilSurface)
	{
		*ppZStencilSurface = ProxyAddressLookupTableD3d8->FindAddress<m_IDirect3DSurface8>(*ppZStencilSurface);
	}

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to get stencil surface!";
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::GetTexture(DWORD Stage, IDirect3DBaseTexture8** ppTexture)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->GetTexture(Stage, ppTexture);

	if (SUCCEEDED(hr) && ppTexture && *ppTexture)
	{
		switch ((*ppTexture)->GetType())
		{
		case D3DRTYPE_TEXTURE:
			*ppTexture = ProxyAddressLookupTableD3d8->FindAddress<m_IDirect3DTexture8>(*ppTexture);
			break;
		case D3DRTYPE_VOLUMETEXTURE:
			*ppTexture = ProxyAddressLookupTableD3d8->FindAddress<m_IDirect3DVolumeTexture8>(*ppTexture);
			break;
		case D3DRTYPE_CUBETEXTURE:
			*ppTexture = ProxyAddressLookupTableD3d8->FindAddress<m_IDirect3DCubeTexture8>(*ppTexture);
			break;
		default:
			Logging::Log() << __FUNCTION__ << " Error: Failed to get texture type!";
			return D3DERR_INVALIDCALL;
		}
	}

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to get texture!";
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetTextureStageState(Stage, Type, pValue);
}

HRESULT m_IDirect3DDevice8::SetTexture(DWORD Stage, IDirect3DBaseTexture8 *pTexture)
{
	Logging::LogDebug() << __FUNCTION__;

	if (Stage == 0)
	{
		TextureNum = 0;
	}

	if (pTexture)
	{
		switch (pTexture->GetType())
		{
		case D3DRTYPE_TEXTURE:
			D3DSURFACE_DESC Desc;
			if (Stage == 0 && SUCCEEDED(static_cast<m_IDirect3DTexture8 *>(pTexture)->GetLevelDesc(0, &Desc)))
			{
				if (pCurrentRenderTexture == pTexture)
				{
					IsSnapshotTextureSet = true;
				}
				TextureNum = ((Desc.Format & 0x00FFFFFF) == MAKEFOURCC('D', 'X', 'T', 0)) ? (BYTE)(Desc.Format >> 24) - 48 : 0;
				if (PauseScreenFix && Desc.Usage == D3DUSAGE_RENDERTARGET)
				{
					pCurrentRenderTexture = static_cast<m_IDirect3DTexture8 *>(pTexture);
				}
			}
			pTexture = static_cast<m_IDirect3DTexture8 *>(pTexture)->GetProxyInterface();
			break;
		case D3DRTYPE_VOLUMETEXTURE:
			pTexture = static_cast<m_IDirect3DVolumeTexture8 *>(pTexture)->GetProxyInterface();
			break;
		case D3DRTYPE_CUBETEXTURE:
			pTexture = static_cast<m_IDirect3DCubeTexture8 *>(pTexture)->GetProxyInterface();
			break;
		default:
			return D3DERR_INVALIDCALL;
		}
		TextureSet = true;	// Used for FakeGetFrontBuffer()
	}

	// Fix for the white shader issue
	if (WhiteShaderFix && Stage == 0 && !pTexture)
	{
		pTexture = BlankTexture;
	}

	return ProxyInterface->SetTexture(Stage, pTexture);
}

HRESULT m_IDirect3DDevice8::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	Logging::LogDebug() << __FUNCTION__;

	// Setup Anisotropy Filtering
	if (AnisotropyFlag && (Type == D3DTSS_MAXANISOTROPY || ((Type == D3DTSS_MINFILTER || Type == D3DTSS_MAGFILTER) && Value == D3DTEXF_LINEAR)))
	{
		AnisotropyFlag = false;

		D3DCAPS8 Caps;
		ZeroMemory(&Caps, sizeof(D3DCAPS8));
		if (SUCCEEDED(ProxyInterface->GetDeviceCaps(&Caps)))
		{
			MaxAnisotropy = (AnisotropicFiltering == 1) ? Caps.MaxAnisotropy : min((DWORD)AnisotropicFiltering, Caps.MaxAnisotropy);
		}

		if (MaxAnisotropy && SUCCEEDED(ProxyInterface->SetTextureStageState(Stage, D3DTSS_MAXANISOTROPY, MaxAnisotropy)))
		{
			Logging::Log() << "Setting Anisotropy Filtering at " << MaxAnisotropy << "x";
		}
		else
		{
			MaxAnisotropy = 0;

			Logging::Log() << "Failed to enable Anisotropy Filtering!";
		}
	}

	// Enable Anisotropic Filtering
	if (MaxAnisotropy)
	{
		if (Type == D3DTSS_MAXANISOTROPY)
		{
			if (SUCCEEDED(ProxyInterface->SetTextureStageState(Stage, D3DTSS_MAXANISOTROPY, MaxAnisotropy)))
			{
				return D3D_OK;
			}
		}
		else if (Stage != 0 && (Type == D3DTSS_MINFILTER || Type == D3DTSS_MAGFILTER) && Value == D3DTEXF_LINEAR)
		{
			if (SUCCEEDED(ProxyInterface->SetTextureStageState(Stage, D3DTSS_MAXANISOTROPY, MaxAnisotropy)) &&
				SUCCEEDED(ProxyInterface->SetTextureStageState(Stage, Type, D3DTEXF_ANISOTROPIC)))
			{
				return D3D_OK;
			}
		}
	}

	return ProxyInterface->SetTextureStageState(Stage, Type, Value);
}

HRESULT m_IDirect3DDevice8::UpdateSurface(IDirect3DSurface8* pSourceSurface, IDirect3DSurface8* pDestSurface)
{
	if (!pSourceSurface || !pDestSurface || pSourceSurface == pDestSurface)
	{
		return D3DERR_INVALIDCALL;
	}

	if (pSourceSurface)
	{
		// Get proxy interface
		IDirect3DSurface8 *pSurface = nullptr;
		if (SUCCEEDED(pSourceSurface->QueryInterface(IID_GetProxyInterface, (void**)&pSurface)) && pSurface)
		{
			pSourceSurface = pSurface;
		}
	}
	if (pDestSurface)
	{
		// Get proxy interface
		IDirect3DSurface8 *pSurface = nullptr;
		if (SUCCEEDED(pDestSurface->QueryInterface(IID_GetProxyInterface, (void**)&pSurface)) && pSurface)
		{
			pDestSurface = pSurface;
		}
	}

	D3DSURFACE_DESC desc;
	pSourceSurface->GetDesc(&desc);
	RECT rect = { 0, 0, (LONG)desc.Width, (LONG)desc.Height };
	POINT point = { 0, 0 };

	return ProxyInterface->CopyRects(pSourceSurface, &rect, 1, pDestSurface, &point);
}

HRESULT m_IDirect3DDevice8::UpdateTexture(IDirect3DBaseTexture8 *pSourceTexture, IDirect3DBaseTexture8 *pDestinationTexture)
{
	Logging::LogDebug() << __FUNCTION__;

	if (pSourceTexture)
	{
		switch (pSourceTexture->GetType())
		{
		case D3DRTYPE_TEXTURE:
			pSourceTexture = static_cast<m_IDirect3DTexture8 *>(pSourceTexture)->GetProxyInterface();
			break;
		case D3DRTYPE_VOLUMETEXTURE:
			pSourceTexture = static_cast<m_IDirect3DVolumeTexture8 *>(pSourceTexture)->GetProxyInterface();
			break;
		case D3DRTYPE_CUBETEXTURE:
			pSourceTexture = static_cast<m_IDirect3DCubeTexture8 *>(pSourceTexture)->GetProxyInterface();
			break;
		default:
			return D3DERR_INVALIDCALL;
		}
	}
	if (pDestinationTexture)
	{
		switch (pDestinationTexture->GetType())
		{
		case D3DRTYPE_TEXTURE:
			pDestinationTexture = static_cast<m_IDirect3DTexture8 *>(pDestinationTexture)->GetProxyInterface();
			break;
		case D3DRTYPE_VOLUMETEXTURE:
			pDestinationTexture = static_cast<m_IDirect3DVolumeTexture8 *>(pDestinationTexture)->GetProxyInterface();
			break;
		case D3DRTYPE_CUBETEXTURE:
			pDestinationTexture = static_cast<m_IDirect3DCubeTexture8 *>(pDestinationTexture)->GetProxyInterface();
			break;
		default:
			return D3DERR_INVALIDCALL;
		}
	}

	return ProxyInterface->UpdateTexture(pSourceTexture, pDestinationTexture);
}

HRESULT m_IDirect3DDevice8::ValidateDevice(DWORD *pNumPasses)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->ValidateDevice(pNumPasses);
}

HRESULT m_IDirect3DDevice8::GetClipPlane(DWORD Index, float *pPlane)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetClipPlane(Index, pPlane);
}

HRESULT m_IDirect3DDevice8::SetClipPlane(DWORD Index, CONST float *pPlane)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetClipPlane(Index, pPlane);
}

HRESULT m_IDirect3DDevice8::Clear(DWORD Count, CONST D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	Logging::LogDebug() << __FUNCTION__;

	// Change first Clear call to match Xbox version
	if (EnableXboxShadows && Flags == (D3DCLEAR_TARGET | D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER) && Color == D3DCOLOR_ARGB(124, 0, 0, 0))
	{
		return ProxyInterface->Clear(Count, pRects, Flags, Color, Z, 0);
	}

	// Set pillar boxes to black
	if (LastFrameFullscreenImage && !DontModifyClear && Count == 0x00 && pRects == nullptr && Flags == (D3DCLEAR_TARGET | D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER) && Z == 1.0f && Stencil == 0x80)
	{
		Color = D3DCOLOR_ARGB(0xFF, 0x00, 0x00, 0x00);
	}

	return ProxyInterface->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT m_IDirect3DDevice8::GetViewport(D3DVIEWPORT8 *pViewport)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetViewport(pViewport);
}

HRESULT m_IDirect3DDevice8::SetViewport(CONST D3DVIEWPORT8 *pViewport)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetViewport(pViewport);
}

HRESULT m_IDirect3DDevice8::CreateVertexShader(THIS_ CONST DWORD* pDeclaration, CONST DWORD* pFunction, DWORD* pHandle, DWORD Usage)
{
	Logging::LogDebug() << __FUNCTION__;

	constexpr BYTE WallFixShaderCode[4][13] = {
		{ 0x0b, 0x00, 0x08, 0x80, 0x01, 0x00, 0xff, 0x80, 0x0e, 0x00, 0x00, 0xa0, 0x06 },		// Shader code:  max r11.w, r1.w, c14.x
		{ 0x0b, 0x00, 0x08, 0x80, 0x01, 0x00, 0xff, 0x80, 0x0e, 0x00, 0xff, 0xa0, 0x06 },		// Shader code:  max r11.w, r1.w, c14.w
		{ 0x0b, 0x00, 0x08, 0x80, 0x0a, 0x00, 0xff, 0x80, 0x0e, 0x00, 0x00, 0xa0, 0x06 },		// Shader code:  max r11.w, r10.w, c14.x
		{ 0x0b, 0x00, 0x08, 0x80, 0x0a, 0x00, 0xff, 0x80, 0x0e, 0x00, 0xff, 0xa0, 0x06 } };		// Shader code:  max r11.w, r10.w, c14.w

	if (FixMissingWallChunks && pFunction)
	{
		if (*pFunction >= D3DVS_VERSION(1, 0) && *pFunction <= D3DVS_VERSION(1, 1))
		{
			ReplaceMemoryBytes((void*)WallFixShaderCode[0], (void*)WallFixShaderCode[1], sizeof(WallFixShaderCode[0]), (DWORD)pFunction, 0x400, 1);
			ReplaceMemoryBytes((void*)WallFixShaderCode[2], (void*)WallFixShaderCode[3], sizeof(WallFixShaderCode[2]), (DWORD)pFunction, 0x400, 1);
		}
	}

	constexpr BYTE HalogenLightFixShaderCode[4][24] = {
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x07, 0x00, 0xe4, 0x90, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xd0, 0x05, 0x00, 0xe4, 0x90, 0x01 },			// Shader code:  mov oT0.xy, v7\n    mov oD0, v5
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x07, 0x00, 0xe4, 0x90, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xd0, 0x0e, 0x00, 0xff, 0xa0, 0x01 },			// Shader code:  mov oT0.xy, v7\n    mov oD0, c14.w
		{ 0xc0, 0x0e, 0x00, 0x00, 0xa0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x01, 0x00, 0xe4, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03 },			// Shader code:  mov oFog, c14.x\n   mov oPos, r1
		{ 0x80, 0x01, 0x00, 0xe4, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x01, 0x00, 0xe4, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03 }};		// Shader code:  mov r1, r1\n        mov oPos, r1

	if (HalogenLightFix && pFunction)
	{
		if (*pFunction >= D3DVS_VERSION(1, 0) && *pFunction <= D3DVS_VERSION(1, 1))
		{
			ReplaceMemoryBytes((void*)HalogenLightFixShaderCode[0], (void*)HalogenLightFixShaderCode[1], sizeof(HalogenLightFixShaderCode[0]), (DWORD)pFunction, 0x400, 1);
			ReplaceMemoryBytes((void*)HalogenLightFixShaderCode[2], (void*)HalogenLightFixShaderCode[3], sizeof(HalogenLightFixShaderCode[2]), (DWORD)pFunction, 0x400, 1);
		}
	}

	return ProxyInterface->CreateVertexShader(pDeclaration, pFunction, pHandle, Usage);
}

HRESULT m_IDirect3DDevice8::GetVertexShader(THIS_ DWORD* pHandle)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetVertexShader(pHandle);
}

HRESULT m_IDirect3DDevice8::SetVertexShader(THIS_ DWORD Handle)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetVertexShader(Handle);
}

HRESULT m_IDirect3DDevice8::DeleteVertexShader(THIS_ DWORD Handle)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->DeleteVertexShader(Handle);
}

HRESULT m_IDirect3DDevice8::GetVertexShaderDeclaration(THIS_ DWORD Handle, void* pData, DWORD* pSizeOfData)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetVertexShaderDeclaration(Handle, pData, pSizeOfData);
}

HRESULT m_IDirect3DDevice8::GetVertexShaderFunction(THIS_ DWORD Handle, void* pData, DWORD* pSizeOfData)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetVertexShaderFunction(Handle, pData, pSizeOfData);
}

HRESULT m_IDirect3DDevice8::SetPixelShaderConstant(THIS_ DWORD Register, CONST void* pConstantData, DWORD ConstantCount)
{
	Logging::LogDebug() << __FUNCTION__;

	// We want to skip the first call to SetPixelShaderConstant when fixing Specular highlights and only adjust the second
	if(SpecularFix && SpecularFlag == 1)
	{
		auto pConstants = reinterpret_cast<const float*>(pConstantData);
		float constants[3] = { pConstants[0], pConstants[1], pConstants[2] };

		if (constants[0] != 0.0f || constants[1] != 0.0f || constants[2] != 0.0f)
		{
			ModelID modelID = GetModelID();

			if (IsJames(modelID)) // James
			{
				if (InSpecialLightZone)
				{
					// 75% if in a special lighting zone
					constants[0] = 0.75f;
					constants[1] = 0.75f;
					constants[2] = 0.75f;
				}
				else
				{
					// Default to 25% specularity
					constants[0] = 0.25f;
					constants[1] = 0.25f;
					constants[2] = 0.25f;
				}
			}
			else if (IsMariaExcludingEyes(modelID)) // Maria, but not her eyes
			{
				if (!UseFakeLight || InSpecialLightZone)
				{
					// 20% If in a special lighting zone and/or flashlight is on
					constants[0] = 0.20f;
					constants[1] = 0.20f;
					constants[2] = 0.20f;
				}
				else
				{
					// Default to 5% specularity
					constants[0] = 0.05f;
					constants[1] = 0.05f;
					constants[2] = 0.05f;
				}
			}
			else if (IsMariaEyes(modelID)) // Maria's Eyes
			{
				// 50% specularity
				constants[0] = 0.50f;
				constants[1] = 0.50f;
				constants[2] = 0.50f;
			}
			else if (modelID == ModelID::chr_bos_bos) // Final boss
			{
				// 25% specularity
				constants[0] = 0.25f;
				constants[1] = 0.25f;
				constants[2] = 0.25f;
			}
			else if ((modelID == ModelID::chr_agl_agl || modelID == ModelID::chr_agl_ragl) && GetCurrentMaterialIndex() == 3) // Angela's eyes
			{
				if (UseFakeLight && !InSpecialLightZone && GetCutsceneID() != 0x53)
				{
					// 25% specularity if flashlight is off and not in special light zone or in cutscene 0x53
					constants[0] = 0.25f;
					constants[1] = 0.25f;
					constants[2] = 0.25f;
				}
				else
				{
					// Default to 50% specularity
					constants[0] = 0.50f;
					constants[1] = 0.50f;
					constants[2] = 0.50f;
				}
			}
			else if (modelID == ModelID::chr_mry_mry) // Mary (Healthy)
			{
				// 50% specularity
				constants[0] = 0.50f;
				constants[1] = 0.50f;
				constants[2] = 0.50f;
			}
			else // Everything else
			{
				if (!UseFakeLight || InSpecialLightZone)
				{
					// 40% If in a special lighting zone and/or flashlight is on
					constants[0] = 0.40f;
					constants[1] = 0.40f;
					constants[2] = 0.40f;
				}
				else
				{
					// Default to 15% specularity
					constants[0] = 0.15f;
					constants[1] = 0.15f;
					constants[2] = 0.15f;
				}
			}
		}

		SpecularFlag--;
		return ProxyInterface->SetPixelShaderConstant(Register, &constants, ConstantCount);
	}

	if (SpecularFix && SpecularFlag > 0)
		SpecularFlag--;

	return ProxyInterface->SetPixelShaderConstant(Register, pConstantData, ConstantCount);
}

HRESULT m_IDirect3DDevice8::GetPixelShaderConstant(THIS_ DWORD Register, void* pConstantData, DWORD ConstantCount)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetPixelShaderConstant(Register, pConstantData, ConstantCount);
}

HRESULT m_IDirect3DDevice8::SetVertexShaderConstant(THIS_ DWORD Register, CONST void* pConstantData, DWORD ConstantCount)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->SetVertexShaderConstant(Register, pConstantData, ConstantCount);
}

HRESULT m_IDirect3DDevice8::GetVertexShaderConstant(THIS_ DWORD Register, void* pConstantData, DWORD ConstantCount)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetVertexShaderConstant(Register, pConstantData, ConstantCount);
}

HRESULT m_IDirect3DDevice8::ResourceManagerDiscardBytes(THIS_ DWORD Bytes)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->ResourceManagerDiscardBytes(Bytes);
}

HRESULT m_IDirect3DDevice8::CreateImageSurface(THIS_ UINT Width, UINT Height, D3DFORMAT Format, IDirect3DSurface8** ppSurface)
{
	Logging::LogDebug() << __FUNCTION__;

	HRESULT hr = ProxyInterface->CreateImageSurface(Width, Height, Format, ppSurface);

	if (SUCCEEDED(hr) && ppSurface)
	{
		*ppSurface = new m_IDirect3DSurface8(*ppSurface, this);
	}

	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Error: Failed to create image surface! " << Width << "x" << Height;
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::CopyRects(THIS_ IDirect3DSurface8* pSourceSurface, CONST RECT* pSourceRectsArray, UINT cRects, IDirect3DSurface8* pDestinationSurface, CONST POINT* pDestPointsArray)
{
	Logging::LogDebug() << __FUNCTION__;

	if (pSourceSurface)
	{
		pSourceSurface = static_cast<m_IDirect3DSurface8 *>(pSourceSurface)->GetProxyInterface();
	}

	if (pDestinationSurface)
	{
		pDestinationSurface = static_cast<m_IDirect3DSurface8 *>(pDestinationSurface)->GetProxyInterface();
	}

	return ProxyInterface->CopyRects(pSourceSurface, pSourceRectsArray, cRects, pDestinationSurface, pDestPointsArray);
}

HRESULT m_IDirect3DDevice8::GetFrontBuffer(THIS_ IDirect3DSurface8* pDestSurface)
{
	Logging::LogDebug() << __FUNCTION__;

	// Update in progress
	if (!m_StopThreadFlag && IsUpdating)
	{
		return D3D_OK;
	}

	IsGetFrontBufferCalled = true;

	// Fix inventory snapshot in Hotel Employee Elevator Room
	if (PauseScreenFix && GetRoomID() == 0x9D && GetOnScreen() == 0x04)
	{
		HotelEmployeeElevatorRoomFlag = TRUE;
	}

	return FakeGetFrontBuffer(pDestSurface);
}

HRESULT m_IDirect3DDevice8::GetFrontBufferFromGDI(THIS_ BYTE* lpBuffer, size_t Size)
{
	// Capture Silent Hill 2 window data
	HWND hDeviceWnd = (ScreenMode == EXCLUSIVE_FULLSCREEN) ? GetDesktopWindow() : DeviceWindow;
	HDC hWindowDC = GetDC(hDeviceWnd);
	HDC hCaptureDC = CreateCompatibleDC(hWindowDC);
	HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hWindowDC, BufferWidth, BufferHeight);
	HGDIOBJ hObject = SelectObject(hCaptureDC, hCaptureBitmap);

	// Blt window data to bitmap
	BOOL rBlt = BitBlt(hCaptureDC, 0, 0, BufferWidth, BufferHeight, hWindowDC, 0, 0, SRCCOPY | CAPTUREBLT);

	// Copy bitmap to cached buffer
	LONG rBitmap = GetBitmapBits(hCaptureBitmap, Size, lpBuffer);

	// Release DC
	DeleteObject(hCaptureBitmap);
	DeleteDC(hCaptureDC);
	ReleaseDC(hDeviceWnd, hWindowDC);

	// Return any errors
	if (!hObject || !rBlt || !rBitmap)
	{
		LOG_ONCE(__FUNCTION__ << " Error: Could not get front buffer from GDI.");
		return D3DERR_INVALIDCALL;
	}

	return D3D_OK;
}

HRESULT m_IDirect3DDevice8::GetFrontBufferFromDirectX(THIS_ BYTE* lpBuffer, size_t Size)
{
	// Check buffer size
	if ((LONG)Size < (BufferWidth * 4) * BufferHeight)
	{
		LOG_ONCE(__FUNCTION__ << " Error: Buffer size is not large enough.");
		return D3DERR_INVALIDCALL;
	}

	// Get primary monitor resolution
	LONG ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	LONG ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	if (MonitorFromWindow(DeviceWindow, MONITOR_DEFAULTTONEAREST) != MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTONEAREST))
	{
		LOG_ONCE(__FUNCTION__ << " Error: Cannot get front buffer data from DirectX on non-primary monitor.");
		return D3DERR_INVALIDCALL;
	}

	// Location of client window
	LONG Left = 0;
	LONG Top = 0;

	// Get location of client window if not in exclusive fullscreen mode
	if (ScreenMode != EXCLUSIVE_FULLSCREEN)
	{
		RECT RectSrc = { 0, 0, BufferWidth, BufferHeight };
		RECT rcClient = { 0, 0, BufferWidth, BufferHeight };
		if (!GetWindowRect(DeviceWindow, &RectSrc) || !GetClientRect(DeviceWindow, &rcClient))
		{
			LOG_ONCE(__FUNCTION__ << " Error: Could not get window or client rect for front buffer.");
			return D3DERR_INVALIDCALL;
		}
		int border_thickness = ((RectSrc.right - RectSrc.left) - rcClient.right) / 2;
		int top_border = (RectSrc.bottom - RectSrc.top) - rcClient.bottom - border_thickness;
		RectSrc.left += border_thickness;
		RectSrc.top += top_border;
		RectSrc.right = RectSrc.left + rcClient.right;
		RectSrc.bottom = RectSrc.top + rcClient.bottom;
		Left = RectSrc.left;
		Top = RectSrc.top;
	}

	HRESULT hr = D3DERR_INVALIDCALL;

	// Create new surface to hold data
	IDirect3DSurface8 *pSrcSurface = nullptr;
	if (SUCCEEDED(ProxyInterface->CreateImageSurface(ScreenWidth, ScreenHeight, D3DFMT_A8R8G8B8, &pSrcSurface)))
	{
		// Get front buffer data
		if (SUCCEEDED(ProxyInterface->GetFrontBuffer(pSrcSurface)))
		{
			// Lock destination surface
			D3DLOCKED_RECT LockedRect = {};
			if (SUCCEEDED(pSrcSurface->LockRect(&LockedRect, nullptr, 0)))
			{
				// Copy surface to cached buffer
				if (LockedRect.Pitch == BufferWidth * 4 && ScreenHeight == BufferHeight && Top == 0 && Left == 0)
				{
					memcpy(lpBuffer, LockedRect.pBits, LockedRect.Pitch * BufferHeight);
				}
				else
				{
					BYTE *pBuffer = (BYTE*)((DWORD)LockedRect.pBits + Top * LockedRect.Pitch + Left * 4);
					DWORD EndBuffer = (DWORD)LockedRect.pBits + (LockedRect.Pitch * ScreenHeight);
					LONG Pitch = BufferWidth * 4;
					for (int x = 0; x < BufferHeight && (DWORD)pBuffer + Pitch <= EndBuffer; x++)
					{
						memcpy(&lpBuffer[x * Pitch], pBuffer, Pitch);
						pBuffer += LockedRect.Pitch;
					}
				}
				hr = D3D_OK;
				pSrcSurface->UnlockRect();
			}
		}
		else
		{
			LOG_ONCE(__FUNCTION__ << " Error: Failed to get DirectX front buffer data.");
		}

		// Release surface
		pSrcSurface->Release();
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::FakeGetFrontBuffer(THIS_ IDirect3DSurface8* pDestSurface)
{
	Logging::LogDebug() << __FUNCTION__;

	if (!pDestSurface)
	{
		return D3DERR_INVALIDCALL;
	}

	pDestSurface = static_cast<m_IDirect3DSurface8 *>(pDestSurface)->GetProxyInterface();

	// Update cached buffer size
	if ((LONG)CachedSurfaceData.size() != (BufferWidth * 4) * BufferHeight)
	{
		CachedSurfaceData.resize((BufferWidth * 4) * BufferHeight);
	}

	bool SkipGetFrontBuffer = false;

	// Detect if GDI will work for getting front buffer data
	if (FrontBufferControl == AUTO_BUFFER)
	{
		if (FAILED(GetFrontBufferFromGDI(&CachedSurfaceData[0], CachedSurfaceData.size())))
		{
			FrontBufferControl = BUFFER_FROM_DIRECTX;
		}

		if (FrontBufferControl == AUTO_BUFFER)
		{
			static bool TestedBlackScreen = false;

			// Check black (or solid color) screen before texture is set
			if (!TextureSet)
			{
				// Get front buffer data from DirectX
				std::vector<BYTE> TempSurfaceData;
				TempSurfaceData.resize(CachedSurfaceData.size());
				if (FAILED(GetFrontBufferFromDirectX(&TempSurfaceData[0], TempSurfaceData.size())))
				{
					FrontBufferControl = BUFFER_FROM_GDI;
					SkipGetFrontBuffer = true;
				}
				else
				{
					bool SolidColorFlag = false;
					bool DataMatchesFlag = false;
					for (DWORD x = 0; x < CachedSurfaceData.size(); x = x + 4)
					{
						// Test for a solid screen
						if (CachedSurfaceData[0] != CachedSurfaceData[0 + x] && CachedSurfaceData[1] != CachedSurfaceData[1 + x] && CachedSurfaceData[2] != CachedSurfaceData[2 + x])
						{
							SolidColorFlag = true;
						}
						// Test if GDI matches DirectX
						if (CachedSurfaceData[0 + x] != TempSurfaceData[0 + x] && CachedSurfaceData[1 + x] != TempSurfaceData[1 + x] && CachedSurfaceData[2 + x] != TempSurfaceData[2 + x])
						{
							DataMatchesFlag = true;
						}
						if (SolidColorFlag && DataMatchesFlag)
						{
							// Found that GDI does not show a solid screen and does not match DirectX, meaning that GDI cannot capture game screen correctly
							// Need to use DirectX for front buffer
							FrontBufferControl = BUFFER_FROM_DIRECTX;
							break;
						}
						*(DWORD*)&CachedSurfaceData[x] = 0x00000000;  // Set the pixel to black
					}
					TestedBlackScreen = true;
				}
			}

			// Check for white (or solid color) screen after texture is set
			if (TextureSet)
			{
				bool Found = false;
				if (TestedBlackScreen)
				{
					for (DWORD x = 0; x < CachedSurfaceData.size(); x = x + 4)
					{
						if (CachedSurfaceData[0] != CachedSurfaceData[0 + x] && CachedSurfaceData[1] != CachedSurfaceData[1 + x] && CachedSurfaceData[2] != CachedSurfaceData[2 + x])
						{
							// Found non-solid color screen, meaning GDI correctly captured the game screen
							Found = true;
							break;
						}
					}
				}
				if (Found)
				{
					// Ok to use GDI for front buffer
					FrontBufferControl = BUFFER_FROM_GDI;
					SkipGetFrontBuffer = true;
				}
				else
				{
					// Need to use DirectX for front buffer
					FrontBufferControl = BUFFER_FROM_DIRECTX;
				}
			}
		}
	}

	// Use GDI to get front buffer data
	if (FrontBufferControl == BUFFER_FROM_GDI && !SkipGetFrontBuffer)
	{
		LOG_ONCE(__FUNCTION__ << " Using GDI to get front buffer data.");

		HRESULT hr = GetFrontBufferFromGDI(&CachedSurfaceData[0], CachedSurfaceData.size());

		if (FAILED(hr))
		{
			return hr;
		}
	}

	// Use DirectX to get front buffer data by calling the real GetFrontBuffer()
	if (FrontBufferControl == BUFFER_FROM_DIRECTX && !SkipGetFrontBuffer)
	{
		LOG_ONCE(__FUNCTION__ << " Using DirectX to get front buffer data.");

		HRESULT hr = GetFrontBufferFromDirectX(&CachedSurfaceData[0], CachedSurfaceData.size());

		if (FAILED(hr))
		{
			return hr;
		}
	}

	HRESULT hr = D3D_OK;

	// Lock destination surface
	D3DLOCKED_RECT LockedRect = {};
	if (SUCCEEDED(pDestSurface->LockRect(&LockedRect, nullptr, 0)))
	{
		// Copy data to surface
		memcpy(LockedRect.pBits, &CachedSurfaceData[0], LockedRect.Pitch * BufferHeight);
		pDestSurface->UnlockRect();
	}
	else
	{
		// Create new surface to hold data
		IDirect3DSurface8 *pSrcSurface = nullptr;
		if (SUCCEEDED(ProxyInterface->CreateImageSurface(BufferWidth, BufferHeight, D3DFMT_A8R8G8B8, &pSrcSurface)))
		{
			// Lock destination surface
			if (SUCCEEDED(pSrcSurface->LockRect(&LockedRect, nullptr, 0)))
			{
				// Copy data to new surface
				memcpy(LockedRect.pBits, &CachedSurfaceData[0], LockedRect.Pitch * BufferHeight);
				pSrcSurface->UnlockRect();

				// Copy data to destination surface
				POINT PointDest = { 0, 0 };
				RECT Rect = { 0, 0, BufferWidth, BufferHeight };
				hr = ProxyInterface->CopyRects(pSrcSurface, &Rect, 1, pDestSurface, &PointDest);
			}

			// Release surface
			pSrcSurface->Release();
		}
	}

	return hr;
}

HRESULT m_IDirect3DDevice8::GetInfo(THIS_ DWORD DevInfoID, void* pDevInfoStruct, DWORD DevInfoStructSize)
{
	Logging::LogDebug() << __FUNCTION__;

	return ProxyInterface->GetInfo(DevInfoID, pDevInfoStruct, DevInfoStructSize);
}

void m_IDirect3DDevice8::AddSurfaceToVector(m_IDirect3DSurface8 *pSourceTarget, IDirect3DSurface8 *pRenderTarget)
{
	if (pSourceTarget && pRenderTarget)
	{
		// Store surface
		SURFACEVECTOR SurfaceStruct = { pSourceTarget , pRenderTarget };
		SurfaceVector.push_back(SurfaceStruct);
	}
}

HRESULT m_IDirect3DDevice8::DrawSoftShadows()
{
	DrawingShadowsFlag = true;
	SetShadowFading();

	// Variables for soft shadows
	const DWORD SHADOW_OPACITY = (ShadowMode == SHADOW_FADING_NONE) ? GetShadowOpacity() : (GetShadowOpacity() * ShadowFadingIntensity) / 100;
	const float SHADOW_DIVISOR = round(((float)BufferHeight / 720.0f) + 0.5f);	// Round number to prevent shadow from being misplaced
	const int BLUR_PASSES = 4;

	IDirect3DSurface8 *pBackBuffer = nullptr, *pStencilBuffer = nullptr;

	const float screenW = (float)BufferWidth;
	const float screenH = (float)BufferHeight;

	const float screenWs = (float)BufferWidth / SHADOW_DIVISOR;
	const float screenHs = (float)BufferHeight / SHADOW_DIVISOR;

	// Original geometry vanilla game uses
	CUSTOMVERTEX_DIF shadowRectDiffuse[] =
	{
		{0.0f, 0.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(SHADOW_OPACITY, 0, 0, 0)},
		{0.0f, screenH, 0.0f, 1.0f, D3DCOLOR_ARGB(SHADOW_OPACITY, 0, 0, 0)},
		{screenW, 0.0f, 0.0f, 1.0f, D3DCOLOR_ARGB(SHADOW_OPACITY, 0, 0, 0)},
		{screenW, screenH, 0.0f, 1.0f, D3DCOLOR_ARGB(SHADOW_OPACITY, 0, 0, 0)}
	};

	// No need for diffuse color, used to render from texture, requires texture coords
	CUSTOMVERTEX_TEX1 shadowRectUV_Small[] =
	{
		{0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
		{0.0f, screenHs, 0.0f, 1.0f, 0.0f, 1.0f},
		{screenWs, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
		{screenWs, screenHs, 0.0f, 1.0f, 1.0f, 1.0f}
	};

	// Bias coords to align correctly to screen space
	for (int i = 0; i < 4; i++)
	{
		shadowRectUV_Small[i].x -= 0.5f;
		shadowRectUV_Small[i].y -= 0.5f;
	}

	CUSTOMVERTEX_TEX1 shadowRectUV[] =
	{
		{0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
		{0.0f, screenH, 0.0f, 1.0f, 0.0f, 1.0f},
		{screenW, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
		{screenW, screenH, 0.0f, 1.0f, 1.0f, 1.0f}
	};

	// Create our intermediate render targets/textures only once
	if (!pInTexture)
	{
		if (SUCCEEDED(ProxyInterface->CreateTexture((UINT)screenW, (UINT)screenH, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pInTexture)) && pInTexture)
		{
			pInTexture->GetSurfaceLevel(0, &pInSurface);
		}
	}

	if (!pInRender && CopyRenderTarget)
	{
		ProxyInterface->CreateRenderTarget((UINT)screenW, (UINT)screenH, D3DFMT_A8R8G8B8, DeviceMultiSampleType, FALSE, &pInRender);
	}

	if (!pShrunkTexture)
	{
		if (SUCCEEDED(ProxyInterface->CreateTexture((UINT)screenWs, (UINT)screenHs, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pShrunkTexture)) && pShrunkTexture)
		{
			pShrunkTexture->GetSurfaceLevel(0, &pShrunkSurface);
		}
	}

	if (!pOutTexture)
	{
		if (SUCCEEDED(ProxyInterface->CreateTexture((UINT)screenWs, (UINT)screenHs, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pOutTexture)) && pOutTexture)
		{
			pOutTexture->GetSurfaceLevel(0, &pOutSurface);
		}
	}

	if (!pInTexture || !pShrunkTexture || !pOutTexture ||
		!pInSurface || !pShrunkSurface || !pOutSurface)
	{
		return D3DERR_INVALIDCALL;
	}

	// Backup current state
	D3DSTATE state;
	BackupState(&state);

	// Textures will be scaled bi-linearly
	ProxyInterface->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	ProxyInterface->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);

	// Turn off alpha blending
	ProxyInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	ProxyInterface->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	// Back up current render target (backbuffer) and stencil buffer
	if (SUCCEEDED(ProxyInterface->GetRenderTarget(&pBackBuffer)) && pBackBuffer)
	{
		pBackBuffer->Release();
	}
	if (SUCCEEDED(ProxyInterface->GetDepthStencilSurface(&pStencilBuffer)) && pStencilBuffer)
	{
		pStencilBuffer->Release();
	}

	// Swap to new render target, maintain old stencil buffer and draw shadows
	ProxyInterface->SetRenderTarget(((pInRender) ? pInRender : pInSurface), pStencilBuffer);
	ProxyInterface->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	HRESULT hr = ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, shadowRectDiffuse, 20);

	// Draw to a scaled down buffer first
	ProxyInterface->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_TEX1);
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	ProxyInterface->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	ProxyInterface->SetRenderTarget(pShrunkSurface, NULL);
	if (pInRender)
	{
		UpdateSurface(pInRender, pInSurface);
	}
	ProxyInterface->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	ProxyInterface->SetTexture(0, pInTexture);

	ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, shadowRectUV_Small, 24);

	// Create 4 full-screen quads, each offset a tiny amount diagonally in each direction
	CUSTOMVERTEX_TEX1 blurUpLeft[] =
	{
		{    0.0f,     0.0f, 0.0f, 1.0f,    0.0f - (0.5f / screenWs), 0.0f - (0.5f / screenHs)},
		{    0.0f, screenHs, 0.0f, 1.0f,    0.0f - (0.5f / screenWs), 1.0f - (0.5f / screenHs)},
		{screenWs,     0.0f, 0.0f, 1.0f,    1.0f - (0.5f / screenWs), 0.0f - (0.5f / screenHs)},
		{screenWs, screenHs, 0.0f, 1.0f,    1.0f - (0.5f / screenWs), 1.0f - (0.5f / screenHs)}
	};

	CUSTOMVERTEX_TEX1 blurDownLeft[] =
	{
		{    0.0f,     0.0f, 0.0f, 1.0f,    0.0f - (0.5f / screenWs), 0.0f + (0.5f / screenHs)},
		{    0.0f, screenHs, 0.0f, 1.0f,    0.0f - (0.5f / screenWs), 1.0f + (0.5f / screenHs)},
		{screenWs,     0.0f, 0.0f, 1.0f,    1.0f - (0.5f / screenWs), 0.0f + (0.5f / screenHs)},
		{screenWs, screenHs, 0.0f, 1.0f,    1.0f - (0.5f / screenWs), 1.0f + (0.5f / screenHs)}
	};

	CUSTOMVERTEX_TEX1 blurUpRight[] =
	{
		{    0.0f,     0.0f, 0.0f, 1.0f,    0.0f + (0.5f / screenWs), 0.0f - (0.5f / screenHs)},
		{    0.0f, screenHs, 0.0f, 1.0f,    0.0f + (0.5f / screenWs), 1.0f - (0.5f / screenHs)},
		{screenWs,     0.0f, 0.0f, 1.0f,    1.0f + (0.5f / screenWs), 0.0f - (0.5f / screenHs)},
		{screenWs, screenHs, 0.0f, 1.0f,    1.0f + (0.5f / screenWs), 1.0f - (0.5f / screenHs)}
	};

	CUSTOMVERTEX_TEX1 blurDownRight[] =
	{
		{    0.0f,     0.0f, 0.0f, 1.0f,    0.0f + (0.5f / screenWs), 0.0f + (0.5f / screenHs)},
		{    0.0f, screenHs, 0.0f, 1.0f,    0.0f + (0.5f / screenWs), 1.0f + (0.5f / screenHs)},
		{screenWs,     0.0f, 0.0f, 1.0f,    1.0f + (0.5f / screenWs), 0.0f + (0.5f / screenHs)},
		{screenWs, screenHs, 0.0f, 1.0f,    1.0f + (0.5f / screenWs), 1.0f + (0.5f / screenHs)}
	};

	// Bias coords to align correctly to screen space
	for (int j = 0; j < 4; j++)
	{
		blurUpLeft[j].x -= 0.5f;
		blurUpLeft[j].y -= 0.5f;

		blurDownLeft[j].x -= 0.5f;
		blurDownLeft[j].y -= 0.5f;

		blurUpRight[j].x -= 0.5f;
		blurUpRight[j].y -= 0.5f;

		blurDownRight[j].x -= 0.5f;
		blurDownRight[j].y -= 0.5f;

		// Undo biasing for blur step
		shadowRectUV_Small[j].x += 0.5f;
		shadowRectUV_Small[j].y += 0.5f;
	}

	// Perform fixed function blur
	for (int i = 0; i < BLUR_PASSES; i++)
	{
		ProxyInterface->SetRenderTarget(pOutSurface, NULL);
		ProxyInterface->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
		ProxyInterface->SetTexture(0, pShrunkTexture);

		// Should probably be combined into one
		ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, blurUpLeft, 24);
		ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, blurDownLeft, 24);
		ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, blurUpRight, 24);
		ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, blurDownRight, 24);

		ProxyInterface->SetRenderTarget(pShrunkSurface, NULL);
		ProxyInterface->SetTexture(0, pOutTexture);

		ProxyInterface->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
		ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, shadowRectUV_Small, 24);
	}

	// Return to backbuffer but without stencil buffer
	ProxyInterface->SetRenderTarget(pBackBuffer, NULL);
	ProxyInterface->SetTexture(0, pShrunkTexture);

	// Set up alpha-blending for final draw back to scene
	ProxyInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	ProxyInterface->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	ProxyInterface->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// Bias coords to align correctly to screen space
	for (int i = 0; i < 4; i++)
	{
		shadowRectUV[i].x -= 0.5f;
		shadowRectUV[i].y -= 0.5f;
	}

	ProxyInterface->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, shadowRectUV, 24);

	// Return to original render target and stencil buffer
	ProxyInterface->SetRenderTarget(pBackBuffer, pStencilBuffer);

	RestoreState(&state);

	return hr;
}

void m_IDirect3DDevice8::BackupState(D3DSTATE *state)
{
	ProxyInterface->GetTextureStageState(0, D3DTSS_MAGFILTER, &state->magFilter);
	ProxyInterface->GetTextureStageState(0, D3DTSS_MINFILTER, &state->minFilter);
	ProxyInterface->GetTextureStageState(0, D3DTSS_COLORARG1, &state->colorArg1);
	ProxyInterface->GetTextureStageState(0, D3DTSS_ALPHAARG1, &state->alphaArg1);

	ProxyInterface->GetRenderState(D3DRS_ALPHABLENDENABLE, &state->alphaBlendEnable);	// Doesn't really need to be backed up
	ProxyInterface->GetRenderState(D3DRS_ALPHATESTENABLE, &state->alphaTestEnable);
	ProxyInterface->GetRenderState(D3DRS_SRCBLEND, &state->srcBlend);
	ProxyInterface->GetRenderState(D3DRS_DESTBLEND, &state->destBlend);

	if (SUCCEEDED(ProxyInterface->GetTexture(0, &state->stage0)) && state->stage0)		// Could use a later stage instead of backing up
	{
		state->stage0->Release();
	}
	else
	{
		state->stage0 = nullptr;
	}

	ProxyInterface->GetVertexShader(&state->vertexShader);

	if (SUCCEEDED(ProxyInterface->GetStreamSource(0, &state->pStream0, &state->stream0Stride)) && state->pStream0)
	{
		state->pStream0->Release();
	}
	else
	{
		state->pStream0 = nullptr;
	}
}

void m_IDirect3DDevice8::RestoreState(D3DSTATE *state)
{
	ProxyInterface->SetTextureStageState(0, D3DTSS_MAGFILTER, state->magFilter);
	ProxyInterface->SetTextureStageState(0, D3DTSS_MINFILTER, state->minFilter);
	ProxyInterface->SetTextureStageState(0, D3DTSS_COLORARG1, state->colorArg1);
	ProxyInterface->SetTextureStageState(0, D3DTSS_ALPHAARG1, state->alphaArg1);

	ProxyInterface->SetRenderState(D3DRS_ALPHABLENDENABLE, state->alphaBlendEnable);
	ProxyInterface->SetRenderState(D3DRS_ALPHATESTENABLE, state->alphaTestEnable);
	if (state->pStream0)
	{
		ProxyInterface->SetStreamSource(0, state->pStream0, state->stream0Stride);
	}
	ProxyInterface->SetRenderState(D3DRS_SRCBLEND, state->srcBlend);
	ProxyInterface->SetRenderState(D3DRS_DESTBLEND, state->destBlend);

	if (state->stage0)
	{
		ProxyInterface->SetTexture(0, state->stage0);
	}

	ProxyInterface->SetVertexShader(state->vertexShader);
}

template <typename T>
void m_IDirect3DDevice8::ReleaseInterface(T** ppInterface, UINT ReleaseRefNum)
{
	if (ppInterface && *ppInterface && ReleaseRefNum)
	{
		UINT ref = 0;
		do {
			ref = (*ppInterface)->Release();
		} while (--ReleaseRefNum && ref);

		// Add Error: checking
		if (ref || ReleaseRefNum)
		{
			LOG_LIMIT(100, __FUNCTION__ << " Error: failed to release interface! " << ref);
		}

		*ppInterface = nullptr;
	}
}

// Create texture for James' silhouette if it doesn't exist
bool m_IDirect3DDevice8::CheckSilhouetteTexture()
{
	if (!silhouetteTexture)
	{
		if (SUCCEEDED(ProxyInterface->CreateTexture(BufferWidth, BufferHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &silhouetteTexture)) && silhouetteTexture)
		{
			silhouetteTexture->GetSurfaceLevel(0, &silhouetteSurface);
		}
	}
	if (!silhouetteRender && CopyRenderTarget)
	{
		ProxyInterface->CreateRenderTarget(BufferWidth, BufferHeight, D3DFMT_A8R8G8B8, DeviceMultiSampleType, FALSE, &silhouetteRender);
	}
	return (silhouetteTexture && silhouetteSurface);
}

// Get shadow opacity
DWORD m_IDirect3DDevice8::GetShadowOpacity()
{
	// Main scenario
	if (GetChapterID() == 0x00)
	{
		switch (GetRoomID())
		{
		case 0x0F:
			return 40;
		case 0xA2:
		case 0xBD:
			return 50;
		case 0x89:
			return 70;
		case 0x0B:
		case 0x21:
			return 110;
		case 0xBF:
			return 128;
		}
	}
	// Born From a Wish chapter
	else if (GetChapterID() == 0x01)
	{
		switch (GetRoomID())
		{
		case 0x27:
			return 30;
		case 0xC2:
			return 60;
		case 0x0C:
		case 0x0D:
		case 0x20:
		case 0x21:
		case 0x25:
		case 0x26:
		case 0xC0:
		case 0xC1:
		case 0xC3:
		case 0xC4:
		case 0xC5:
		case 0xC6:
		case 0xC8:
		case 0xC9:
		case 0xCA:
		case 0xCB:
		case 0xCC:
		case 0xCD:
		case 0xCE:
		case 0xCF:
		case 0xD0:
		case 0xD1:
		case 0xD2:
		case 0xD5:
			return 90;
		case 0xC7:
			return 128;
		}
	}

	// Default opacity
	return 170;
}

// Get shadow fading intensity
DWORD m_IDirect3DDevice8::GetShadowIntensity()
{
	return (DWORD)(((GetFlashlightBrightnessRed() + GetFlashlightBrightnessGreen() + GetFlashlightBrightnessBlue()) * 33.3333333f) / 7.0f);
}

// Check if shadow fading needs to be done and set the shadow intensity
void m_IDirect3DDevice8::SetShadowFading()
{
	// Check room ID to see if shadow fading should be enabled
	bool EnableShadowFading = true;
	for (const DWORD &Room : { 0x32, 0x39, 0x42, 0x89, 0xA2, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBD })
	{
		if (GetRoomID() == Room)
		{
			EnableShadowFading = false;
			break;
		}
	}

	// Fade shadows in after turning flashlight on
	if (EnableShadowFading && (LastFlashlightSwitch == 0x00 || ShadowMode != SHADOW_FADING_NONE) && GetFlashlightSwitch() == 0x01)
	{
		if (ShadowMode != SHADOW_FADING_IN)
		{
			ShadowFadingCounter = 0;
		}
		ShadowMode = SHADOW_FADING_IN;
		ShadowFadingIntensity = GetShadowIntensity();	// Intesity is increased by around 3 each frame
		if (ShadowFadingIntensity == 100)				// Exit once intensity reaches 100
		{
			LastFlashlightSwitch = GetFlashlightSwitch();
			ShadowMode = SHADOW_FADING_NONE;
		}
	}
	// Fade shadows out after turning flashlight off
	else if (EnableShadowFading && GetFlashlightSwitch() == 0x00 && GetFlashLightRender() == 0x01)
	{
		if (ShadowMode != SHADOW_FADING_OUT)
		{
			ShadowFadingCounter = 0;
		}
		ShadowMode = SHADOW_FADING_OUT;
		ShadowFadingIntensity = GetShadowIntensity();	// Intesity is decreased by around 15 each frame
	}
	// Refade in other shadows after turning flashlight off
	else if (EnableShadowFading && (GetFlashlightSwitch() != LastFlashlightSwitch || ShadowMode == SHADOW_FADING_OUT || ShadowMode == SHADOW_REFADING))
	{
		if (ShadowMode != SHADOW_REFADING)
		{
			ShadowFadingCounter = 0;
			ShadowFadingIntensity = 0;
		}
		ShadowMode = SHADOW_REFADING;
		ShadowFadingIntensity = min(100, ShadowFadingCounter / 2);	// Intesity is increased by 1 every other frame
		if (ShadowFadingIntensity == 100)							// Exit once intensity reaches 100
		{
			LastFlashlightSwitch = GetFlashlightSwitch();
			ShadowMode = SHADOW_FADING_NONE;
		}
	}
	// No shadow fading effects
	else
	{
		ShadowMode = SHADOW_FADING_NONE;
		LastFlashlightSwitch = GetFlashlightSwitch();
		ShadowFadingIntensity = LastFlashlightSwitch * 100;
	}
}

DWORD WINAPI SaveScreenshotFile(LPVOID)
{
	// Wait for new screenshot
	while (!m_StopThreadFlag)
	{
		if (ScreenshotVector.size() && ScreenshotVector[0].Enabled)
		{
			if (ScreenshotVector[0].filename.size() && ScreenshotVector[0].bufferRaw.size() && ScreenshotVector[0].Width && ScreenshotVector[0].Height)
			{
				ScreenshotVector[0].buffer.resize(ScreenshotVector[0].size);
				BYTE *bufferIn = &ScreenshotVector[0].bufferRaw[0];
				BYTE *bufferOut = &ScreenshotVector[0].buffer[0];

				// Transcode buffer into RGB for image conversion
				for (int y = 0; y < ScreenshotVector[0].Height; y++)
				{
					for (int x = 0; x < ScreenshotVector[0].Width; x++)
					{
						DWORD loc = x * 4;
						bufferOut[3] = 0xFF;				// Alpha - bufferIn[loc + 3];
						bufferOut[0] = bufferIn[loc + 2];	// Red
						bufferOut[1] = bufferIn[loc + 1];	// Green
						bufferOut[2] = bufferIn[loc + 0];	// Blue
						bufferOut += 4;
					}
					bufferIn += ScreenshotVector[0].Pitch;
				}

				// Write PNG buffer to disk
				if (FILE *file; _wfopen_s(&file, ScreenshotVector[0].filename.c_str(), L"wb") == 0)
				{
					const auto write_callback = [](void *context, void *data, int size) {
						fwrite(data, 1, size, static_cast<FILE *>(context));
					};

					Logging::Log() << "Saving screenshot to " << ScreenshotVector[0].filename.c_str() << " ...";

					stbi_write_png_to_func(write_callback, file, ScreenshotVector[0].Width, ScreenshotVector[0].Height, 4, &ScreenshotVector[0].buffer[0], 0);

					fclose(file);
				}
				else
				{
					LOG_LIMIT(3, __FUNCTION__ << " Error creating screenshot file!");
				}
			}
			else
			{
				LOG_LIMIT(3, __FUNCTION__ << " Error with data in screenshot vector!");
			}
			// Remove item from vector
			ScreenshotVector.erase(ScreenshotVector.begin());
		}
		Sleep(100);
	}

	return S_OK;
}

void m_IDirect3DDevice8::CaptureScreenShot()
{
	Logging::LogDebug() << __FUNCTION__;

	// Get BackBuffer
	IDirect3DSurface8 *pDestSurface = nullptr;
	if (FAILED(GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pDestSurface)))
	{
		LOG_LIMIT(3, __FUNCTION__ << " Failed to get back buffer!");
		return;
	}

	// Lock surface, read it into a memory buffer and add it to a queue (vector)
	D3DLOCKED_RECT LockedRect = {};
	if (SUCCEEDED(pDestSurface->LockRect(&LockedRect, nullptr, D3DLOCK_READONLY)) && LockedRect.pBits)
	{
		// Set variables
		SCREENSHOTSTRUCT scElement;
		ScreenshotVector.push_back(scElement);
		ScreenshotVector.back().Width = BufferWidth;
		ScreenshotVector.back().Height = BufferHeight;
		ScreenshotVector.back().size = LockedRect.Pitch * BufferHeight;
		ScreenshotVector.back().Pitch = LockedRect.Pitch;
		ScreenshotVector.back().bufferRaw.resize(ScreenshotVector.back().size);

		// Copy memory
		memcpy(&ScreenshotVector.back().bufferRaw[0], LockedRect.pBits, ScreenshotVector.back().size);

		// Unlock surface
		pDestSurface->UnlockRect();

		// Get current time and date
		const std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		tm tm; localtime_s(&tm, &t);

		// Get file name
		char timestamp[21];
		sprintf_s(timestamp, " %.4d-%.2d-%.2d %.2d-%.2d-%.2d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		std::string name("Screenshot" + std::string(timestamp) + ".png");

		// Get Silent Hill 2 folder
		wchar_t path[MAX_PATH] = {};
		bool ret = GetSH2FolderPath(path, MAX_PATH);
		wchar_t* pdest = wcsrchr(path, '\\');
		if (ret && pdest)
		{
			*pdest = '\0';
			wcscat_s(path, MAX_PATH, L"\\imgs\\");
			if (!PathFileExists(path))
			{
				CreateDirectory(path, nullptr);
			}
		}

		// File name
		ScreenshotVector.back().filename.assign(path + std::wstring(name.begin(), name.end()));

		// Enable item in vector
		ScreenshotVector.back().Enabled = true;
	}
	else
	{
		LOG_LIMIT(3, __FUNCTION__ << " Failed to lock back buffer!");
	}

	// Release surface
	pDestSurface->Release();

	return;
}

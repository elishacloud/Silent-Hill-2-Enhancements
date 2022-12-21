/**
* Copyright (C) 2014 Patrick Mours. All rights reserved.
* License: https://github.com/crosire/reshade#license
*
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

#include "d3d9wrapper.h"

extern bool DisableShaderOnPresent;

bool ShadersReady = false;
DWORD GammaLevel = 3;

HRESULT m_IDirect3DDevice9::QueryInterface(REFIID riid, void** ppvObj)
{
	if ((riid == IID_IDirect3DDevice9 || riid == IID_IUnknown) && ppvObj)
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG m_IDirect3DDevice9::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DDevice9::Release()
{
	const ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		// Release remaining references to this device
		_buffer_detection.reset(true);
		_auto_depthstencil.reset();
		_implicit_swapchain->Release();

		delete this;
	}

	return ref;
}

HRESULT m_IDirect3DDevice9::TestCooperativeLevel()
{
	return ProxyInterface->TestCooperativeLevel();
}

UINT m_IDirect3DDevice9::GetAvailableTextureMem()
{
	return ProxyInterface->GetAvailableTextureMem();
}

HRESULT m_IDirect3DDevice9::EvictManagedResources()
{
	return ProxyInterface->EvictManagedResources();
}

HRESULT m_IDirect3DDevice9::GetDirect3D(IDirect3D9 **ppD3D9)
{
	return ProxyInterface->GetDirect3D(ppD3D9);
}

HRESULT m_IDirect3DDevice9::GetDeviceCaps(D3DCAPS9 *pCaps)
{
	return ProxyInterface->GetDeviceCaps(pCaps);
}

HRESULT m_IDirect3DDevice9::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE *pMode)
{
	if (iSwapChain != 0)
	{
		Logging::Log() << "Access to multi-head swap chain at index " << iSwapChain << " is unsupported.";
		return D3DERR_INVALIDCALL;
	}

	return _implicit_swapchain->GetDisplayMode(pMode);
}

HRESULT m_IDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	return ProxyInterface->GetCreationParameters(pParameters);
}

HRESULT m_IDirect3DDevice9::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9 *pCursorBitmap)
{
	return ProxyInterface->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

void m_IDirect3DDevice9::SetCursorPosition(int X, int Y, DWORD Flags)
{
	return ProxyInterface->SetCursorPosition(X, Y, Flags);
}

BOOL m_IDirect3DDevice9::ShowCursor(BOOL bShow)
{
	return ProxyInterface->ShowCursor(bShow);
}

HRESULT m_IDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain9 **ppSwapChain)
{
	Logging::Log() << "Redirecting " << "IDirect3DDevice9::CreateAdditionalSwapChain" << '(' << "this = " << this << ", pPresentationParameters = " << pPresentationParameters << ", ppSwapChain = " << ppSwapChain << ')' << " ...";

	if (pPresentationParameters == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	com_ptr<IDirect3D9> d3d;
	ProxyInterface->GetDirect3D(&d3d);
	D3DDEVICE_CREATION_PARAMETERS cp = {};
	ProxyInterface->GetCreationParameters(&cp);

	D3DPRESENT_PARAMETERS pp = *pPresentationParameters;
	d3d.reset();

	const HRESULT hr = ProxyInterface->CreateAdditionalSwapChain(&pp, ppSwapChain);
	// Update output values (see https://docs.microsoft.com/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-createadditionalswapchain)
	pPresentationParameters->BackBufferWidth = pp.BackBufferWidth;
	pPresentationParameters->BackBufferHeight = pp.BackBufferHeight;
	pPresentationParameters->BackBufferFormat = pp.BackBufferFormat;
	pPresentationParameters->BackBufferCount = pp.BackBufferCount;

	if (FAILED(hr))
	{
		Logging::Log() << "IDirect3DDevice9::CreateAdditionalSwapChain" << " failed with error code " << (D3DERR)hr << '!';
		return hr;
	}

	IDirect3DDevice9 *const device = ProxyInterface;
	IDirect3DSwapChain9 *const swapchain = *ppSwapChain;
	assert(swapchain != nullptr);

	// Retrieve present parameters here again, to get correct values for 'BackBufferWidth' and 'BackBufferHeight'
	// They may otherwise still be set to zero (which is valid for creation)
	swapchain->GetPresentParameters(&pp);

	const auto runtime = std::make_shared<reshade::d3d9::runtime_d3d9>(device, swapchain, &_buffer_detection);
	if (!runtime->on_init(pp))
	{
		Logging::Log() << "Failed to initialize Direct3D 9 runtime environment on runtime " << runtime.get() << '.';
	}

	AddRef(); // Add reference which is released when the swap chain is destroyed (see 'm_IDirect3DSwapChain9::Release')

	const auto swapchain_proxy = new m_IDirect3DSwapChain9(this, swapchain, runtime);

	_additional_swapchains.push_back(swapchain_proxy);
	*ppSwapChain = swapchain_proxy;

	Logging::LogDebug() << "Returning IDirect3DSwapChain9 object: " << swapchain_proxy << '.';

	return D3D_OK;
}

HRESULT m_IDirect3DDevice9::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9 **ppSwapChain)
{
	if (iSwapChain != 0)
	{
		Logging::Log() << "Access to multi-head swap chain at index " << iSwapChain << " is unsupported.";
		return D3DERR_INVALIDCALL;
	}

	if (ppSwapChain == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	_implicit_swapchain->AddRef();
	*ppSwapChain = _implicit_swapchain;

	return D3D_OK;
}

UINT m_IDirect3DDevice9::GetNumberOfSwapChains()
{
	return 1;
}

HRESULT m_IDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	Logging::Log() << "Redirecting " << "IDirect3DDevice9::Reset" << '(' << "this = " << this << ", pPresentationParameters = " << pPresentationParameters << ')' << " ...";

	if (pPresentationParameters == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	isInScene = false;

	com_ptr<IDirect3D9> d3d;
	ProxyInterface->GetDirect3D(&d3d);
	D3DDEVICE_CREATION_PARAMETERS cp = {};
	ProxyInterface->GetCreationParameters(&cp);

	D3DPRESENT_PARAMETERS pp = *pPresentationParameters;
	d3d.reset();

	const auto runtime = _implicit_swapchain->_runtime;
	runtime->on_reset();

	_buffer_detection.reset(true);
	_auto_depthstencil.reset();

	const HRESULT hr = ProxyInterface->Reset(&pp);
	// Update output values (see https://docs.microsoft.com/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-reset)
	pPresentationParameters->BackBufferWidth = pp.BackBufferWidth;
	pPresentationParameters->BackBufferHeight = pp.BackBufferHeight;
	pPresentationParameters->BackBufferFormat = pp.BackBufferFormat;
	pPresentationParameters->BackBufferCount = pp.BackBufferCount;

	if (FAILED(hr))
	{
		Logging::Log() << "IDirect3DDevice9::Reset" << " failed with error code " << (D3DERR)hr << '!';
		return hr;
	}

	_implicit_swapchain->GetPresentParameters(&pp);

	if (!runtime->on_init(pp))
	{
		Logging::Log() << "Failed to recreate Direct3D 9 runtime environment on runtime " << runtime.get() << '.';
	}

	// Reload auto depth-stencil surface
	if (pp.EnableAutoDepthStencil)
	{
		ProxyInterface->GetDepthStencilSurface(&_auto_depthstencil);
		SetDepthStencilSurface(_auto_depthstencil.get());
	}

	return hr;
}

HRESULT m_IDirect3DDevice9::Present(const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion)
{
	bool SkipScene = false;
	// Only call into runtime if the entire surface is presented, to avoid partial updates messing up effects and the GUI
	if (ShadersReady && !DisableShaderOnPresent && m_IDirect3DSwapChain9::is_presenting_entire_surface(pSourceRect, hDestWindowOverride))
	{
		SkipScene = _implicit_swapchain->_runtime->get_gamma();
		_implicit_swapchain->_runtime->on_present();
	}

	// Endscene
	isInScene = false;
	ProxyInterface->EndScene();

	_buffer_detection.reset(false);

	if (SkipScene)
	{
		Logging::Log() << __FUNCTION__ << " Skipping frame after gamma change!";
		return D3D_OK;
	}

	return ProxyInterface->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT m_IDirect3DDevice9::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9 **ppBackBuffer)
{
	if (iSwapChain != 0)
	{
		Logging::Log() << "Access to multi-head swap chain at index " << iSwapChain << " is unsupported.";
		return D3DERR_INVALIDCALL;
	}

	return _implicit_swapchain->GetBackBuffer(iBackBuffer, Type, ppBackBuffer);
}

HRESULT m_IDirect3DDevice9::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS *pRasterStatus)
{
	if (iSwapChain != 0)
	{
		Logging::Log() << "Access to multi-head swap chain at index " << iSwapChain << " is unsupported.";
		return D3DERR_INVALIDCALL;
	}

	return _implicit_swapchain->GetRasterStatus(pRasterStatus);
}

HRESULT m_IDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs)
{
	return ProxyInterface->SetDialogBoxMode(bEnableDialogs);
}

void m_IDirect3DDevice9::SetGammaRamp(UINT iSwapChain, DWORD Flags, const D3DGAMMARAMP *pRamp)
{
	if (iSwapChain != 0 || !pRamp)
	{
		Logging::Log() << "Access to multi-head swap chain at index " << iSwapChain << " is unsupported.";
		return;
	}

	if (RestoreBrightnessSelector)
	{
		memcpy(&Ramp, pRamp, sizeof(D3DGAMMARAMP));

		GammaLevel = (pRamp->red[127] == 19018) ? 0 :
			(pRamp->red[127] == 22873) ? 1 :
			(pRamp->red[127] == 28013) ? 2 :
			(pRamp->red[127] == 32639) ? 3 :
			(pRamp->red[127] == 35466) ? 4 :
			(pRamp->red[127] == 39321) ? 5 :
			(pRamp->red[127] == 45746) ? 6 :
			(pRamp->red[127] == 56026) ? 7 : 3;

		const auto runtime = _implicit_swapchain->_runtime;

		runtime->reset_gamma(true);

		return;
	}

	return ProxyInterface->SetGammaRamp(0, Flags, pRamp);
}

void m_IDirect3DDevice9::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP *pRamp)
{
	if (iSwapChain != 0 || !pRamp)
	{
		Logging::Log() << "Access to multi-head swap chain at index " << iSwapChain << " is unsupported.";
		return;
	}

	if (RestoreBrightnessSelector)
	{
		memcpy(pRamp, &Ramp, sizeof(D3DGAMMARAMP));
		return;
	}

	return ProxyInterface->GetGammaRamp(0, pRamp);
}

HRESULT m_IDirect3DDevice9::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9 **ppTexture, HANDLE *pSharedHandle)
{
	return ProxyInterface->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT m_IDirect3DDevice9::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9 **ppVolumeTexture, HANDLE *pSharedHandle)
{
	return ProxyInterface->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
}

HRESULT m_IDirect3DDevice9::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9 **ppCubeTexture, HANDLE *pSharedHandle)
{
	return ProxyInterface->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
}

HRESULT m_IDirect3DDevice9::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9 **ppVertexBuffer, HANDLE *pSharedHandle)
{
	// Need to allow buffer for use in software vertex processing, since application uses software and not hardware processing, but device was created with both
	if (UseSoftwareRendering)
	{
		Usage |= D3DUSAGE_SOFTWAREPROCESSING;
	}

	return ProxyInterface->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

HRESULT m_IDirect3DDevice9::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9 **ppIndexBuffer, HANDLE *pSharedHandle)
{
	if (UseSoftwareRendering)
	{
		Usage |= D3DUSAGE_SOFTWAREPROCESSING;
	}

	return ProxyInterface->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT m_IDirect3DDevice9::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9 **ppSurface, HANDLE *pSharedHandle)
{
	return ProxyInterface->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
}

HRESULT m_IDirect3DDevice9::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9 **ppSurface, HANDLE *pSharedHandle)
{
	return ProxyInterface->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
}

HRESULT m_IDirect3DDevice9::UpdateSurface(IDirect3DSurface9 *pSourceSurface, const RECT *pSourceRect, IDirect3DSurface9 *pDestinationSurface, const POINT *pDestPoint)
{
	return ProxyInterface->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

HRESULT m_IDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9 *pSourceTexture, IDirect3DBaseTexture9 *pDestinationTexture)
{
	return ProxyInterface->UpdateTexture(pSourceTexture, pDestinationTexture);
}

HRESULT m_IDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9 *pRenderTarget, IDirect3DSurface9 *pDestSurface)
{
	return ProxyInterface->GetRenderTargetData(pRenderTarget, pDestSurface);
}

HRESULT m_IDirect3DDevice9::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9 *pDestSurface)
{
	if (iSwapChain != 0)
	{
		Logging::Log() << "Access to multi-head swap chain at index " << iSwapChain << " is unsupported.";
		return D3DERR_INVALIDCALL;
	}

	return _implicit_swapchain->GetFrontBufferData(pDestSurface);
}

HRESULT m_IDirect3DDevice9::StretchRect(IDirect3DSurface9 *pSourceSurface, const RECT *pSourceRect, IDirect3DSurface9 *pDestSurface, const RECT *pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	return ProxyInterface->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT m_IDirect3DDevice9::ColorFill(IDirect3DSurface9 *pSurface, const RECT *pRect, D3DCOLOR color)
{
	return ProxyInterface->ColorFill(pSurface, pRect, color);
}

HRESULT m_IDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9 **ppSurface, HANDLE *pSharedHandle)
{
	return ProxyInterface->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

HRESULT m_IDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9 *pRenderTarget)
{
	return ProxyInterface->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT m_IDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9 **ppRenderTarget)
{
	return ProxyInterface->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}

HRESULT m_IDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9 *pNewZStencil)
{
	_buffer_detection.on_set_depthstencil(pNewZStencil);

	return ProxyInterface->SetDepthStencilSurface(pNewZStencil);
}

HRESULT m_IDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9 **ppZStencilSurface)
{
	const HRESULT hr = ProxyInterface->GetDepthStencilSurface(ppZStencilSurface);

	if (SUCCEEDED(hr))
	{
		assert(ppZStencilSurface != nullptr);

		_buffer_detection.on_get_depthstencil(*ppZStencilSurface);
	}

	return hr;
}

HRESULT m_IDirect3DDevice9::BeginScene()
{
	if (!isInScene)
	{
		isInScene = true;
		ProxyInterface->BeginScene();
	}

	return D3D_OK;
}

HRESULT m_IDirect3DDevice9::EndScene()
{
	return D3D_OK;
}

HRESULT m_IDirect3DDevice9::Clear(DWORD Count, const D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	// Skip partial clears, since buffer detection logic replaces entire depth-stencil surface and therefore may otherwise break rendering after those
	if (Flags != D3DCLEAR_TARGET && (Count == 0 || (pRects->x1 == 0 && pRects->y1 == 0)))
	{
		_buffer_detection.on_clear_depthstencil(Flags);
	}

	return ProxyInterface->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT m_IDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX *pMatrix)
{
	return ProxyInterface->SetTransform(State, pMatrix);
}

HRESULT m_IDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix)
{
	return ProxyInterface->GetTransform(State, pMatrix);
}

HRESULT m_IDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX *pMatrix)
{
	return ProxyInterface->MultiplyTransform(State, pMatrix);
}

HRESULT m_IDirect3DDevice9::SetViewport(const D3DVIEWPORT9 *pViewport)
{
	return ProxyInterface->SetViewport(pViewport);
}

HRESULT m_IDirect3DDevice9::GetViewport(D3DVIEWPORT9 *pViewport)
{
	return ProxyInterface->GetViewport(pViewport);
}

HRESULT m_IDirect3DDevice9::SetMaterial(const D3DMATERIAL9 *pMaterial)
{
	return ProxyInterface->SetMaterial(pMaterial);
}

HRESULT m_IDirect3DDevice9::GetMaterial(D3DMATERIAL9 *pMaterial)
{
	return ProxyInterface->GetMaterial(pMaterial);
}

HRESULT m_IDirect3DDevice9::SetLight(DWORD Index, const D3DLIGHT9 *pLight)
{
	return ProxyInterface->SetLight(Index, pLight);
}

HRESULT m_IDirect3DDevice9::GetLight(DWORD Index, D3DLIGHT9 *pLight)
{
	return ProxyInterface->GetLight(Index, pLight);
}

HRESULT m_IDirect3DDevice9::LightEnable(DWORD Index, BOOL Enable)
{
	return ProxyInterface->LightEnable(Index, Enable);
}

HRESULT m_IDirect3DDevice9::GetLightEnable(DWORD Index, BOOL *pEnable)
{
	return ProxyInterface->GetLightEnable(Index, pEnable);
}

HRESULT m_IDirect3DDevice9::SetClipPlane(DWORD Index, const float *pPlane)
{
	return ProxyInterface->SetClipPlane(Index, pPlane);
}

HRESULT m_IDirect3DDevice9::GetClipPlane(DWORD Index, float *pPlane)
{
	return ProxyInterface->GetClipPlane(Index, pPlane);
}

HRESULT m_IDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	return ProxyInterface->SetRenderState(State, Value);
}

HRESULT m_IDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue)
{
	return ProxyInterface->GetRenderState(State, pValue);
}

HRESULT m_IDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9 **ppSB)
{
	return ProxyInterface->CreateStateBlock(Type, ppSB);
}

HRESULT m_IDirect3DDevice9::BeginStateBlock()
{
	return ProxyInterface->BeginStateBlock();
}

HRESULT m_IDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9 **ppSB)
{
	return ProxyInterface->EndStateBlock(ppSB);
}

HRESULT m_IDirect3DDevice9::SetClipStatus(const D3DCLIPSTATUS9 *pClipStatus)
{
	return ProxyInterface->SetClipStatus(pClipStatus);
}

HRESULT m_IDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9 *pClipStatus)
{
	return ProxyInterface->GetClipStatus(pClipStatus);
}

HRESULT m_IDirect3DDevice9::GetTexture(DWORD Stage, IDirect3DBaseTexture9 **ppTexture)
{
	return ProxyInterface->GetTexture(Stage, ppTexture);
}

HRESULT m_IDirect3DDevice9::SetTexture(DWORD Stage, IDirect3DBaseTexture9 *pTexture)
{
	return ProxyInterface->SetTexture(Stage, pTexture);
}

HRESULT m_IDirect3DDevice9::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue)
{
	return ProxyInterface->GetTextureStageState(Stage, Type, pValue);
}

HRESULT m_IDirect3DDevice9::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	return ProxyInterface->SetTextureStageState(Stage, Type, Value);
}

HRESULT m_IDirect3DDevice9::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD *pValue)
{
	return ProxyInterface->GetSamplerState(Sampler, Type, pValue);
}

HRESULT m_IDirect3DDevice9::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	return ProxyInterface->SetSamplerState(Sampler, Type, Value);
}

HRESULT m_IDirect3DDevice9::ValidateDevice(DWORD *pNumPasses)
{
	return ProxyInterface->ValidateDevice(pNumPasses);
}

HRESULT m_IDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber, const PALETTEENTRY *pEntries)
{
	return ProxyInterface->SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT m_IDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY *pEntries)
{
	return ProxyInterface->GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT m_IDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber)
{
	return ProxyInterface->SetCurrentTexturePalette(PaletteNumber);
}

HRESULT m_IDirect3DDevice9::GetCurrentTexturePalette(UINT *PaletteNumber)
{
	return ProxyInterface->GetCurrentTexturePalette(PaletteNumber);
}
HRESULT m_IDirect3DDevice9::SetScissorRect(const RECT *pRect)
{
	return ProxyInterface->SetScissorRect(pRect);
}

HRESULT m_IDirect3DDevice9::GetScissorRect(RECT *pRect)
{
	return ProxyInterface->GetScissorRect(pRect);
}

HRESULT m_IDirect3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	return ProxyInterface->SetSoftwareVertexProcessing(bSoftware);
}

BOOL m_IDirect3DDevice9::GetSoftwareVertexProcessing()
{
	return ProxyInterface->GetSoftwareVertexProcessing();
}

HRESULT m_IDirect3DDevice9::SetNPatchMode(float nSegments)
{
	return ProxyInterface->SetNPatchMode(nSegments);
}

float m_IDirect3DDevice9::GetNPatchMode()
{
	return ProxyInterface->GetNPatchMode();
}

HRESULT m_IDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	_buffer_detection.on_draw(PrimitiveType, PrimitiveCount);

	return ProxyInterface->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT m_IDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount)
{
	_buffer_detection.on_draw(PrimitiveType, PrimitiveCount);

	return ProxyInterface->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, StartIndex, PrimitiveCount);
}

HRESULT m_IDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	_buffer_detection.on_draw(PrimitiveType, PrimitiveCount);

	return ProxyInterface->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT m_IDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, const void *pIndexData, D3DFORMAT IndexDataFormat, const void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	_buffer_detection.on_draw(PrimitiveType, PrimitiveCount);

	return ProxyInterface->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT m_IDirect3DDevice9::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9 *pDestBuffer, IDirect3DVertexDeclaration9 *pVertexDecl, DWORD Flags)
{
	return ProxyInterface->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT m_IDirect3DDevice9::CreateVertexDeclaration(const D3DVERTEXELEMENT9 *pVertexElements, IDirect3DVertexDeclaration9 **ppDecl)
{
	return ProxyInterface->CreateVertexDeclaration(pVertexElements, ppDecl);
}

HRESULT m_IDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9 *pDecl)
{
	return ProxyInterface->SetVertexDeclaration(pDecl);
}

HRESULT m_IDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9 **ppDecl)
{
	return ProxyInterface->GetVertexDeclaration(ppDecl);
}

HRESULT m_IDirect3DDevice9::SetFVF(DWORD FVF)
{
	return ProxyInterface->SetFVF(FVF);
}

HRESULT m_IDirect3DDevice9::GetFVF(DWORD *pFVF)
{
	return ProxyInterface->GetFVF(pFVF);
}

HRESULT m_IDirect3DDevice9::CreateVertexShader(const DWORD *pFunction, IDirect3DVertexShader9 **ppShader)
{
	return ProxyInterface->CreateVertexShader(pFunction, ppShader);
}

HRESULT m_IDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9 *pShader)
{
	return ProxyInterface->SetVertexShader(pShader);
}

HRESULT m_IDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9 **ppShader)
{
	return ProxyInterface->GetVertexShader(ppShader);
}

HRESULT m_IDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister, const float *pConstantData, UINT Vector4fCount)
{
	return ProxyInterface->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT m_IDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister, float *pConstantData, UINT Vector4fCount)
{
	return ProxyInterface->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT m_IDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister, const int *pConstantData, UINT Vector4iCount)
{
	return ProxyInterface->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT m_IDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister, int *pConstantData, UINT Vector4iCount)
{
	return ProxyInterface->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT m_IDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister, const BOOL *pConstantData, UINT  BoolCount)
{
	return ProxyInterface->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT m_IDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister, BOOL *pConstantData, UINT BoolCount)
{
	return ProxyInterface->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT m_IDirect3DDevice9::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9 *pStreamData, UINT OffsetInBytes, UINT Stride)
{
	return ProxyInterface->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT m_IDirect3DDevice9::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9 **ppStreamData, UINT *OffsetInBytes, UINT *pStride)
{
	return ProxyInterface->GetStreamSource(StreamNumber, ppStreamData, OffsetInBytes, pStride);
}

HRESULT m_IDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber, UINT Divider)
{
	return ProxyInterface->SetStreamSourceFreq(StreamNumber, Divider);
}

HRESULT m_IDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber, UINT *Divider)
{
	return ProxyInterface->GetStreamSourceFreq(StreamNumber, Divider);
}

HRESULT m_IDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9 *pIndexData)
{
	return ProxyInterface->SetIndices(pIndexData);
}

HRESULT m_IDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9 **ppIndexData)
{
	return ProxyInterface->GetIndices(ppIndexData);
}

HRESULT m_IDirect3DDevice9::CreatePixelShader(const DWORD *pFunction, IDirect3DPixelShader9 **ppShader)
{
	return ProxyInterface->CreatePixelShader(pFunction, ppShader);
}

HRESULT m_IDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9 *pShader)
{
	return ProxyInterface->SetPixelShader(pShader);
}

HRESULT m_IDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9 **ppShader)
{
	return ProxyInterface->GetPixelShader(ppShader);
}

HRESULT m_IDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister, const float *pConstantData, UINT Vector4fCount)
{
	return ProxyInterface->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT m_IDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister, float *pConstantData, UINT Vector4fCount)
{
	return ProxyInterface->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT m_IDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister, const int *pConstantData, UINT Vector4iCount)
{
	return ProxyInterface->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT m_IDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister, int *pConstantData, UINT Vector4iCount)
{
	return ProxyInterface->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT m_IDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister, const BOOL *pConstantData, UINT  BoolCount)
{
	return ProxyInterface->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT m_IDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister, BOOL *pConstantData, UINT BoolCount)
{
	return ProxyInterface->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT m_IDirect3DDevice9::DrawRectPatch(UINT Handle, const float *pNumSegs, const D3DRECTPATCH_INFO *pRectPatchInfo)
{
	return ProxyInterface->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

HRESULT m_IDirect3DDevice9::DrawTriPatch(UINT Handle, const float *pNumSegs, const D3DTRIPATCH_INFO *pTriPatchInfo)
{
	return ProxyInterface->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT m_IDirect3DDevice9::DeletePatch(UINT Handle)
{
	return ProxyInterface->DeletePatch(Handle);
}

HRESULT m_IDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9 **ppQuery)
{
	return ProxyInterface->CreateQuery(Type, ppQuery);
}

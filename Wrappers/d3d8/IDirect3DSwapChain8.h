#pragma once

class m_IDirect3DSwapChain8 : public IDirect3DSwapChain8, public AddressLookupTableObject
{
private:
	LPDIRECT3DSWAPCHAIN8 ProxyInterface;
	m_IDirect3DDevice8* m_pDevice;

public:
	m_IDirect3DSwapChain8(LPDIRECT3DSWAPCHAIN8 pSwapChain8, m_IDirect3DDevice8* pDevice) : ProxyInterface(pSwapChain8), m_pDevice(pDevice)
	{
		m_pDevice->ProxyAddressLookupTable->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DSwapChain8() {}

	LPDIRECT3DSWAPCHAIN8 GetProxyInterface() { return ProxyInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DSwapChain8 methods ***/
	STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
	STDMETHOD(GetBackBuffer)(THIS_ UINT BackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface8** ppBackBuffer);
};

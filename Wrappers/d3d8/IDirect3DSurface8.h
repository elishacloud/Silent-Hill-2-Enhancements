#pragma once

class m_IDirect3DSurface8 : public IDirect3DSurface8, public AddressLookupTableObject
{
private:
	LPDIRECT3DSURFACE8 ProxyInterface;
	m_IDirect3DDevice8* m_pDevice;

public:
	m_IDirect3DSurface8(LPDIRECT3DSURFACE8 pSurface8, m_IDirect3DDevice8* pDevice) : ProxyInterface(pSurface8), m_pDevice(pDevice)
	{
		m_pDevice->ProxyAddressLookupTable->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DSurface8() {}

	LPDIRECT3DSURFACE8 GetProxyInterface() { return ProxyInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DSurface8 methods ***/
	STDMETHOD(GetDevice)(THIS_ IDirect3DDevice8** ppDevice);
	STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid);
	STDMETHOD(GetContainer)(THIS_ REFIID riid, void** ppContainer);
	STDMETHOD(GetDesc)(THIS_ D3DSURFACE_DESC *pDesc);
	STDMETHOD(LockRect)(THIS_ D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
	STDMETHOD(UnlockRect)(THIS);
};

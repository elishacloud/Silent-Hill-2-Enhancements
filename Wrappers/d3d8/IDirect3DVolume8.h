#pragma once

class m_IDirect3DVolume8 : public IDirect3DVolume8, public AddressLookupTableObject
{
private:
	LPDIRECT3DVOLUME8 ProxyInterface;
	m_IDirect3DDevice8* m_pDevice;

public:
	m_IDirect3DVolume8(LPDIRECT3DVOLUME8 pVolume8, m_IDirect3DDevice8* pDevice) : ProxyInterface(pVolume8), m_pDevice(pDevice)
	{
		m_pDevice->ProxyAddressLookupTable->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DVolume8() {}

	LPDIRECT3DVOLUME8 GetProxyInterface() { return ProxyInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DVolume8 methods ***/
	STDMETHOD(GetDevice)(THIS_ IDirect3DDevice8** ppDevice);
	STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid);
	STDMETHOD(GetContainer)(THIS_ REFIID riid, void** ppContainer);
	STDMETHOD(GetDesc)(THIS_ D3DVOLUME_DESC *pDesc);
	STDMETHOD(LockBox)(THIS_ D3DLOCKED_BOX * pLockedVolume, CONST D3DBOX* pBox, DWORD Flags);
	STDMETHOD(UnlockBox)(THIS);
};

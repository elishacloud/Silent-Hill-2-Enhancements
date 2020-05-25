#pragma once

class m_IDirect3DIndexBuffer8 : public IDirect3DIndexBuffer8, public AddressLookupTableD3d8Object
{
private:
	LPDIRECT3DINDEXBUFFER8 ProxyInterface;
	m_IDirect3DDevice8* m_pDevice;

public:
	m_IDirect3DIndexBuffer8(LPDIRECT3DINDEXBUFFER8 pBuffer8, m_IDirect3DDevice8* pDevice) : ProxyInterface(pBuffer8), m_pDevice(pDevice)
	{
		Logging::LogDebug() << "Creating device " << __FUNCTION__ << "(" << this << ")";

		m_pDevice->ProxyAddressLookupTableD3d8->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DIndexBuffer8()
	{
		Logging::LogDebug() << __FUNCTION__ << "(" << this << ")" << " deleting device!";
	}

	LPDIRECT3DINDEXBUFFER8 GetProxyInterface() { return ProxyInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DResource8 methods ***/
	STDMETHOD(GetDevice)(THIS_ IDirect3DDevice8** ppDevice);
	STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid);
	STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew);
	STDMETHOD_(DWORD, GetPriority)(THIS);
	STDMETHOD_(void, PreLoad)(THIS);
	STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS);
	STDMETHOD(Lock)(THIS_ UINT OffsetToLock, UINT SizeToLock, BYTE** ppbData, DWORD Flags);
	STDMETHOD(Unlock)(THIS);
	STDMETHOD(GetDesc)(THIS_ D3DINDEXBUFFER_DESC *pDesc);
};

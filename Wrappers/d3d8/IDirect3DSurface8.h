#pragma once

class m_IDirect3DSurface8 : public IDirect3DSurface8, public AddressLookupTableD3d8Object
{
private:
	LPDIRECT3DSURFACE8 ProxyInterface;
	m_IDirect3DDevice8* m_pDevice;
	IDirect3DSurface8* RenderInterface = nullptr;
	IDirect3DSurface8* ReplacedInterface = nullptr;

	bool IsTextureRenderTarget = false;

	// For fake emulated locking
	bool IsLocked = false;
	std::vector<byte> surfaceArray;

public:
	m_IDirect3DSurface8(LPDIRECT3DSURFACE8 pSurface8, m_IDirect3DDevice8* pDevice) : ProxyInterface(pSurface8), m_pDevice(pDevice)
	{
		Logging::LogDebug() << "Creating device " << __FUNCTION__ << "(" << this << ")";

		m_pDevice->ProxyAddressLookupTableD3d8->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DSurface8()
	{
		Logging::LogDebug() << __FUNCTION__ << "(" << this << ")" << " deleting device!";
	}

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

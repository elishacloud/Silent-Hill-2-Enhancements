#pragma once

class m_IDirect3DVolumeTexture8 : public IDirect3DVolumeTexture8, public AddressLookupTableObject
{
private:
	LPDIRECT3DVOLUMETEXTURE8 ProxyInterface;
	m_IDirect3DDevice8* m_pDevice;

public:
	m_IDirect3DVolumeTexture8(LPDIRECT3DVOLUMETEXTURE8 pTexture8, m_IDirect3DDevice8* pDevice) : ProxyInterface(pTexture8), m_pDevice(pDevice)
	{
		m_pDevice->ProxyAddressLookupTable->SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DVolumeTexture8() {}

	LPDIRECT3DVOLUMETEXTURE8 GetProxyInterface() { return ProxyInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DBaseTexture8 methods ***/
	STDMETHOD(GetDevice)(THIS_ IDirect3DDevice8** ppDevice);
	STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid);
	STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew);
	STDMETHOD_(DWORD, GetPriority)(THIS);
	STDMETHOD_(void, PreLoad)(THIS);
	STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS);
	STDMETHOD_(DWORD, SetLOD)(THIS_ DWORD LODNew);
	STDMETHOD_(DWORD, GetLOD)(THIS);
	STDMETHOD_(DWORD, GetLevelCount)(THIS);
	STDMETHOD(GetLevelDesc)(THIS_ UINT Level, D3DVOLUME_DESC *pDesc);
	STDMETHOD(GetVolumeLevel)(THIS_ UINT Level, IDirect3DVolume8** ppVolumeLevel);
	STDMETHOD(LockBox)(THIS_ UINT Level, D3DLOCKED_BOX* pLockedVolume, CONST D3DBOX* pBox, DWORD Flags);
	STDMETHOD(UnlockBox)(THIS_ UINT Level);
	STDMETHOD(AddDirtyBox)(THIS_ CONST D3DBOX* pDirtyBox);
};

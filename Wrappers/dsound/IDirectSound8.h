#pragma once

class m_IDirectSound8 : public IDirectSound8, public AddressLookupTableDsoundObject
{
private:
	LPDIRECTSOUND8 ProxyInterface;

	// Helper functions
	void InitDevice();
	void ReleaseDevice();

public:
	m_IDirectSound8(LPDIRECTSOUND8 pSound8) : ProxyInterface(pSound8)
	{
		Logging::LogDebug() << "Creating device " << __FUNCTION__ << "(" << this << ")";

		InitDevice();

		ProxyAddressLookupTableDsound.SaveAddress(this, ProxyInterface);
	}
	~m_IDirectSound8()
	{
		Logging::LogDebug() << __FUNCTION__ << "(" << this << ")" << " deleting device!";

		ReleaseDevice();

		ProxyAddressLookupTableDsound.DeleteAddress(this);
	}

	LPDIRECTSOUND8 GetProxyInterface() { return ProxyInterface; }

	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ _In_ REFIID, _Outptr_ LPVOID*);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	// IDirectSound methods
	STDMETHOD(CreateSoundBuffer)(THIS_ _In_ LPCDSBUFFERDESC pcDSBufferDesc, _Out_ LPDIRECTSOUNDBUFFER *ppDSBuffer, _Pre_null_ LPUNKNOWN pUnkOuter);
	STDMETHOD(GetCaps)(THIS_ _Out_ LPDSCAPS pDSCaps);
	STDMETHOD(DuplicateSoundBuffer)(THIS_ _In_ LPDIRECTSOUNDBUFFER pDSBufferOriginal, _Out_ LPDIRECTSOUNDBUFFER *ppDSBufferDuplicate);
	STDMETHOD(SetCooperativeLevel)(THIS_ HWND hwnd, DWORD dwLevel);
	STDMETHOD(Compact)(THIS);
	STDMETHOD(GetSpeakerConfig)(THIS_ _Out_ LPDWORD pdwSpeakerConfig);
	STDMETHOD(SetSpeakerConfig)(THIS_ DWORD dwSpeakerConfig);
	STDMETHOD(Initialize)(THIS_ _In_opt_ LPCGUID pcGuidDevice);

	// IDirectSound8 methods
	STDMETHOD(VerifyCertification)(THIS_ _Out_ LPDWORD pdwCertified);

	// Helper functions
	HRESULT CreateWAVSoundBuffer(const char* filePath, m_IDirectSoundBuffer8** ppDSBuffer);

	// static functions
	static HRESULT ParseWavFile(const char* filePath, DSBUFFERDESC& dsbd, WAVEFORMATEX& waveFormat, std::vector<char>& AudioBuffer);
	static HRESULT PlayWavFile(const char* filePath, DWORD BifferID);
	static HRESULT StopWavFile(DWORD BifferID);
	static void ReleaseSoundBuffer(DWORD BifferID);
};

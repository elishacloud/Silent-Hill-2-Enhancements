#pragma once
#if XAUDIO2

#define XAUDIO2_BUFFER_SIZE		32768
#define XAUDIO2_BUFFER_HALF		(XAUDIO2_BUFFER_SIZE / 2)

class SndObjXAudio : public SndObjBase
{
public:
	SndObjXAudio()
	{
		pVoice = nullptr;
		flip = 0;
		memset(&info, 0, sizeof(info));

		Release();
	}
	virtual ~SndObjXAudio()
	{
		Release();
	}

	virtual void CreateBuffer(CriFileStream* stream);

	virtual void Release();

	virtual void Play();
	virtual int  Stop();
	virtual void Update();

	virtual void SendData();
	virtual void SetVolume(int vol);

private:
	IXAudio2SourceVoice *pVoice;
	BYTE buffer[XAUDIO2_BUFFER_SIZE];
	XAUDIO2_BUFFER info;
	int flip;

	// simple callback override for signaling
	class XAudio2Callback : public IXAudio2VoiceCallback
	{
	public:
		XAudio2Callback() :
			hEndEvent(CreateEventA(nullptr, FALSE, FALSE, nullptr))
		{}
		~XAudio2Callback() { CloseHandle(hEndEvent); }

		virtual void _stdcall OnStreamEnd() {}
		virtual void _stdcall OnVoiceProcessingPassEnd() {}
		virtual void _stdcall OnVoiceProcessingPassStart(UINT32 SamplesRequired) {}
		virtual void _stdcall OnBufferEnd(void* pBufferContext) { SetEvent(hEndEvent); }	// signal that we need more data
		virtual void _stdcall OnBufferStart(void* pBufferContext) {}
		virtual void _stdcall OnLoopEnd(void* pBufferContext) {}
		virtual void _stdcall OnVoiceError(void* pBufferContext, HRESULT Error) {}

		HANDLE hEndEvent;
	};

	XAudio2Callback cb;
};

void adxs_SetupXAudio(void* pXA);

#endif

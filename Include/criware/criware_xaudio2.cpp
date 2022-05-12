#include "criware.h"

#if XAUDIO2

IXAudio2 *XA;

static HMODULE xdll = nullptr;
static HRESULT(__stdcall* XAudio2CreateEx)(_Outptr_ IXAudio2** ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor);

static IXAudio2* pXAudio2;
static IXAudio2MasteringVoice* pMasterVoice;

// XAudio 2.9 dll loader
static int Xaudio2_loader()
{
	if (xdll)
		return S_OK;

#if _DEBUG
	xdll = LoadLibraryW(L"xaudio2_9d.dll");
#else
	xdll = LoadLibraryW(L"xaudio2_9.dll");
#endif
	// if it's not in the game folder, go for system32
	if (!xdll)
	{
		wchar_t path[MAX_PATH], path_w32[MAX_PATH];
		GetSystemDirectoryW(path_w32, MAX_PATH);
		wsprintf(path, L"%s\\xaudio2_9.dll", path_w32);
		xdll = LoadLibraryW(path);
		if (!xdll)
			return S_FALSE;
	}

	// grab XAudio2 main procedure and we're good
	XAudio2CreateEx = (HRESULT(__stdcall*)(IXAudio2**, UINT32, XAUDIO2_PROCESSOR))GetProcAddress(xdll, "XAudio2Create");
	return S_OK;
}

extern SndObjBase* sound_obj_tbl[SOUND_MAX_OBJ];

void adxs_SetupXAudio(void* pXA)
{
	XA = (IXAudio2*)pXA;
	for (int i = 0; i < SOUND_MAX_OBJ; i++)
		sound_obj_tbl[i] = new SndObjXAudio();
}

void SndObjXAudio::CreateBuffer(CriFileStream* stream)
{
	str = stream;

	fmt.cbSize = sizeof(WAVEFORMATEX);
	fmt.nSamplesPerSec = stream->sample_rate;
	fmt.nBlockAlign = (WORD)(2 * stream->channel_count);
	fmt.nChannels = (WORD)stream->channel_count;
	fmt.wBitsPerSample = 16;
	fmt.wFormatTag = WAVE_FORMAT_PCM;
	fmt.nAvgBytesPerSec = stream->sample_rate * 2 * stream->channel_count;

	info.AudioBytes = XAUDIO2_BUFFER_HALF;
	info.PlayBegin = 0;
	info.PlayLength = XAUDIO2_BUFFER_HALF / fmt.nBlockAlign;
	info.pContext = this;
	flip = 0;

	if (FAILED(XA->CreateSourceVoice(&pVoice, &fmt, 0, 1.f, &cb)))
		ADXD_Error(__FUNCTION__, "Can't create source voice.");

	// send two whole buffers to fill the initial queue
	SendData();
	SendData();
	ResetEvent(cb.hEndEvent);

	Play();

	used = 1;
	offset = 0;
	offset_played = 0;
}

void SndObjXAudio::Release()
{
	if (pVoice)
	{
		pVoice->DestroyVoice();
		pVoice = nullptr;

		SndObjBase::Release();
	}
}

void SndObjXAudio::Play()
{
	if (used)
	{
		if (adx && adx->set_volume)
		{
			SetVolume(adx->volume);
			adx->set_volume = 0;
		}

		if (pVoice)
		{
			ResetEvent(cb.hEndEvent);
			pVoice->Start();
		}
	}
}

int SndObjXAudio::Stop()
{
	if (stopping) return;

	stopping = 1;
	if (pVoice)
	{
		//pVoice->FlushSourceBuffers();
		pVoice->Stop();
		ResetEvent(cb.hEndEvent);
	}
}

void SndObjXAudio::Update()
{
	// inactive objects need to do nothing
	if (used == 0) return;

	if (pVoice && stopped == 0)
	{
		// if this stream is not set to loop we need to stop streaming when it's done playing
		if (loops == 0)
		{
			// signal that decoding is done
			if (adx->state != ADXT_STAT_DECEND && str->sample_index >= str->loop_end_index)
				adx->state = ADXT_STAT_DECEND;
		}

		// check if the volume needs to be changed
		if (adx && adx->set_volume)
		{
			SetVolume(adx->volume);
			adx->set_volume = 0;
		}

		WaitForSingleObject(cb.hEndEvent, INFINITE);	// stay locked until the callback requires more data

		SendData();
	}
}

void SndObjXAudio::SendData()
{
	BYTE *b = &buffer[flip * XAUDIO2_BUFFER_HALF];
	info.pAudioData = b;

	if (stopping)
		memset(b, 0, XAUDIO2_BUFFER_HALF);
	else
	{
		// prepare the necessary data for half the buffer
		
		auto needed = str->Decode((int16_t*)b, XAUDIO2_BUFFER_HALF / fmt.nBlockAlign, str->loop_enabled);
		if (needed)
		{
			// fill trail with silence, just for ADX (WAV does it internally)
			needed *= fmt.nBlockAlign;
			memset(&b[XAUDIO2_BUFFER_HALF - needed], 0, needed);
		}
	}

	// send to queue
	pVoice->SubmitSourceBuffer(&info);
	flip ^= 1;
}

void SndObjXAudio::SetVolume(int vol)
{
	if (vol < -1000)
		vol = -1000;

	volume = vol;

	if (pVoice)
	{
		float v = XAudio2DecibelsToAmplitudeRatio(vol / 10.f);
		pVoice->SetVolume(v);
	}
}

#endif

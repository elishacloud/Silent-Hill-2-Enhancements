/*
* Copyright (C) 2022 Gemini
* ===============================================================
* DirectSound8 interface
* ---------------------------------------------------------------
* Generates sound buffers and takes care of playback.
* ===============================================================
*/
#include "criware.h"

#if !XAUDIO2

LPDIRECTSOUND8 pDS8;

#define BUFFER_SIZE		(32 * 1024)
#define BUFFER_HALF		(BUFFER_SIZE / 2)
#define BUFFER_QUART	(BUFFER_HALF / 2)

#ifdef _DEBUG
static int buffer_cnt = 0;
#endif

void adxs_SetupDSound(LPDIRECTSOUND8 pDS)
{
	pDS8 = pDS;

	for (int i = 0; i < SOUND_MAX_OBJ; i++)
		sound_obj_tbl[i] = new SndObjDSound();
}

static const char* get_ds_error(HRESULT err)
{
	switch (err)
	{
	case DSERR_ALLOCATED: return "DSERR_ALLOCATED";
	case DSERR_CONTROLUNAVAIL: return "DSERR_CONTROLUNAVAIL";
	case DSERR_BADFORMAT: return "DSERR_BADFORMAT";
	case DSERR_INVALIDPARAM: return "DSERR_INVALIDPARAM";
	case DSERR_NOAGGREGATION: return "DSERR_NOAGGREGATION";
	case DSERR_OUTOFMEMORY: return "DSERR_OUTOFMEMORY";
	case DSERR_UNINITIALIZED: return "DSERR_UNINITIALIZED";
	case DSERR_UNSUPPORTED: return "DSERR_UNSUPPORTED";
	case DSERR_BUFFERLOST: return "DSERR_BUFFERLOST";
	case DSERR_INVALIDCALL: return "DSERR_INVALIDCALL";
	case DSERR_PRIOLEVELNEEDED: return "DSERR_PRIOLEVELNEEDED";
	}

	return "DSERR_UNKNOWN";
}

#define DS_CALL_CATCH(x, caption, error)		{ HRESULT hr = (x); \
	if(FAILED(hr)) ADXD_Error(caption, error, get_ds_error(hr)); \
	} \

void SndObjDSound::CreateBuffer(CriFileStream* stream)
{
	str = stream;

	fmt.cbSize = sizeof(WAVEFORMATEX);
	fmt.nSamplesPerSec = stream->sample_rate;
	fmt.nBlockAlign = (WORD)(2 * stream->channel_count);
	fmt.nChannels = (WORD)stream->channel_count;
	fmt.wBitsPerSample = 16;
	fmt.wFormatTag = WAVE_FORMAT_PCM;
	fmt.nAvgBytesPerSec = stream->sample_rate * 2 * stream->channel_count;

	DSBUFFERDESC desc = { 0 };
	desc.dwSize = sizeof(desc);
	desc.lpwfxFormat = &fmt;
	desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_LOCSOFTWARE;
	desc.dwBufferBytes = BUFFER_SIZE;
	DS_CALL_CATCH((pDS8->CreateSoundBuffer(&desc, &pBuf, nullptr)), __FUNCTION__, "Couldn't create DirectSound buffer (%s).");

#ifdef _DEBUG
	ADXD_Log("Allocating, %d buffers so far\n", ++buffer_cnt);
#endif

	Fill(BUFFER_SIZE);

	used = 1;
	offset = 0;
	offset_played = 0;
	stopped = 1;
}

void SndObjDSound::Play()
{
	if (used)
	{
		if (adx && adx->set_volume)
		{
			SetVolume(adx->volume);
			adx->set_volume = 0;
		}

		if (pBuf)
		{
			pBuf->Play(0, 0, DSBPLAY_LOOPING);
			stopped = 0;
		}
	}
}

void SndObjDSound::Stop()
{
	// this is inactive or stopped already
	if (used == 0) return;
	if (stopped == 1) return;

	if (pBuf)
	{
		pBuf->Stop();

		//DWORD st;
		//do { pBuf->GetStatus(&st); } while (st & DSBSTATUS_PLAYING);

		stopped = 1;
	}
}

void SndObjDSound::Update()
{
	// inactive objects need to do nothing
	if (used)
	{
		if (pBuf && stopped == 0)
		{
			// if this stream is not set to loop we need to stop streaming when it's done playing
			if (loops == 0)
			{
				// signal that decoding is done
				if (adx->state != ADXT_STAT_DECEND && str->sample_index >= str->loop_end_index)
					adx->state = ADXT_STAT_DECEND;
				// signal that playback is done and stop filling the buffer
				if (GetPlayedSamples() >= str->loop_end_index)
				{
					Stop();
					adx->state = ADXT_STAT_PLAYEND;
				}
			}

			// check if the volume needs to be changed
			if (adx && adx->set_volume)
			{
				SetVolume(adx->volume);
				adx->set_volume = 0;
			}

			SendData();
		}
	}
}

void SndObjDSound::SendData()
{
	u_long pos = GetPosition(),
		add = 0;

	if (pos - offset < 0)
		add = BUFFER_SIZE;
	if (pos + add - offset > BUFFER_HALF + 16)
	{
		Fill(BUFFER_QUART);

		u_long total = offset + BUFFER_QUART;
		offset = total;
		if (BUFFER_SIZE <= total)
			offset = total - BUFFER_SIZE;
		offset_played += BUFFER_QUART;
	}
}

void SndObjDSound::SetVolume(int vol)
{
	if (vol < -1000)
		vol = -1000;

	volume = vol;
	if (used && pBuf)
		pBuf->SetVolume(vol * 10);
}

void SndObjDSound::Release()
{
	if (used)
	{
		if (stopped == 0)
			Stop();

		if (pBuf)
		{
#ifdef _DEBUG
			ADXD_Log("Dellocating, %d buffers so far\n", --buffer_cnt);
#endif

			pBuf->Release();
			pBuf = nullptr;
		}

		SndObjBase::Release();
	}
}

// -----------------------------
// non virtual methods
u_long SndObjDSound::GetPosition()
{
	DWORD pos;
	pBuf->GetCurrentPosition(&pos, nullptr);

	return pos;
}

u_long SndObjDSound::GetPlayedSamples()
{
	return (offset_played + GetPosition()) / fmt.nBlockAlign;
}

int SndObjDSound::GetStatus()
{
	if (used == 0 || pBuf == nullptr)
		return DSOS_UNUSED;

	DWORD status;
	pBuf->GetStatus(&status);

	if (status & DSBSTATUS_LOOPING)
		return DSOS_LOOPING;
	if (status & DSBSTATUS_PLAYING)
		return DSOS_PLAYING;
	if (status & DSBSTATUS_TERMINATED)
		return DSOS_ENDED;

	return DSOS_ENDED;
}

void SndObjDSound::Fill(u_long size)
{
	short *ptr1, *ptr2;
	DWORD bytes1, bytes2;

	DS_CALL_CATCH((pBuf->Lock(offset, size, (LPVOID*)&ptr1, &bytes1, (LPVOID*)&ptr2, &bytes2, 0)), __FUNCTION__, "Couldn't lock DirectSound buffer (%s)");

	// just fill with silence if we're stopping or the data was previously over
	if (adx->state != ADXT_STAT_PLAYING)
	{
		ADXD_Log(__FUNCTION__ ": sending silence...\n");
		memset(ptr1, 0, bytes1);
	}
	else
	{
		auto needed = str->Decode(ptr1, bytes1 / fmt.nBlockAlign, loops);

		if (loops == 0)
		{
			if (needed)
			{
				// fill trail with silence, just for ADX (WAV does it internally)
				needed *= fmt.nBlockAlign;
				memset(&ptr1[(bytes1 - needed) / 2], 0, needed);
			}
		}
	}

	DS_CALL_CATCH((pBuf->Unlock(ptr1, bytes1, ptr2, bytes2)), __FUNCTION__, "Couldn't unlock DirectSound buffer (%s)");
}

#endif

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

#define BUFFER_SIZE		32768
#define BUFFER_HALF		(BUFFER_SIZE / 2)
#define BUFFER_QUART	(BUFFER_HALF / 2)

void adxs_SetupDSound(LPDIRECTSOUND8 pDS)
{
	pDS8 = pDS;

	for (int i = 0; i < SOUND_MAX_OBJ; i++)
		sound_obj_tbl[i] = new SndObjDSound();
}

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
	if (FAILED(pDS8->CreateSoundBuffer(&desc, &pBuf, nullptr)))
		ADXD_Error(__FUNCTION__, "Can't create buffer.");

	Fill(BUFFER_SIZE);

	used = 1;
	offset = 0;
	offset_played = 0;
}

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

void SndObjDSound::Play()
{
	if (used)
	{
		if (adx && adx->set_volume)
		{
			SetVolume(adx->volume);
			adx->set_volume = 0;
		}

		if(pBuf)
			pBuf->Play(0, 0, DSBPLAY_LOOPING);
	}
}

int SndObjDSound::Stop()
{
	// this is inactive or stopped already
	if (used == 0) return 1 ;
	if (stopped == 1) return 1;

	if (pBuf)
	{
		if (stopping == 0)
		{
			// queue stop to directsound
			stopping = 1;
			pBuf->Stop();
		}

		// check if directsound is done
		int s = GetStatus();
		if (s == DSOS_LOOPING || s == DSOS_PLAYING)
		{
			ADXD_Log(__FUNCTION__ ": still playing, can't release...\n");
			return 0;	// still playing
		}
		
		// ok, we're done!
		stopped = 1;
		return 1;
	}

	return 0;
}

void SndObjDSound::Update()
{
	ADX_lock();
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
	ADX_unlock();
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

void SndObjDSound::Lock(u_long size)
{
	if (FAILED(pBuf->Lock(offset, size, (LPVOID*)&ptr1, &bytes1, (LPVOID*)&ptr2, &bytes2, 0)))
		ADXD_Error(__FUNCTION__, "Can't lock.");
}

void SndObjDSound::Unlock()
{
	if (FAILED(pBuf->Unlock(ptr1, bytes1, ptr2, bytes2)))
		ADXD_Error(__FUNCTION__, "Can't unlock.");
}

void SndObjDSound::Fill(u_long size)
{
	Lock(size);
	// just fill with silence if we're stopping or the data was previously over
	if (stopping || adx->state == ADXT_STAT_DECEND)
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
	Unlock();
}

void SndObjDSound::Release()
{
	if (used && pBuf)
	{
		pBuf->Release();
		pBuf = nullptr;

		ptr1 = nullptr;
		bytes1 = 0;
		ptr2 = nullptr;
		bytes2 = 0;

		SndObjBase::Release();
	}
}

#endif

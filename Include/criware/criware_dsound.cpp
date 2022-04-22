/*
* Copyright (C) 2022 Gemini
* ===============================================================
* DirectSound interface for buffers
* ---------------------------------------------------------------
* Just an interface with DirectSound8 to generate buffers and
* some helpers.
* ===============================================================
*/
#include "criware.h"

LPDIRECTSOUND8 pDS8;

#define BUFFER_SIZE		32768
#define BUFFER_HALF		(BUFFER_SIZE / 2)
#define BUFFER_QUART	(BUFFER_HALF / 2)

void adxs_SetupDSound(LPDIRECTSOUND8 pDS)
{
	pDS8 = pDS;

	for (int i = 0; i < SOUND_MAX_OBJ; i++)
	{
		SndObjDSound* n = new SndObjDSound();
		sound_obj_tbl[i] = n;
		sound_obj_tbl[i]->used = 0;
	}
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
	desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY | DSBCAPS_LOCSOFTWARE;
	desc.dwBufferBytes = BUFFER_SIZE;
	if (FAILED(pDS8->CreateSoundBuffer(&desc, &pBuf, nullptr)))
		MessageBoxA(nullptr, "error", __FUNCTION__, MB_OK);

	DWORD bytes1, bytes2;
	short* ptr1, * ptr2;
	trans_lock = 1;
	pBuf->Lock(0, BUFFER_SIZE, (LPVOID*)&ptr1, &bytes1, (LPVOID*)&ptr2, &bytes2, 0);
#if 1
	auto needed = stream->Decode(ptr1, bytes1 / fmt.nBlockAlign, loops);
	if (needed && loops == 0)
	{
		needed *= fmt.nBlockAlign;
		memset(&ptr1[(bytes1 - needed) / 2], 0, needed);
	}
#else
	memset(ptr1, 0, bytes1);
#endif
	pBuf->Unlock(ptr1, bytes1, ptr2, bytes2);
	trans_lock = 0;

	used = 1;
	offset = 0;
}

u_long SndObjDSound::GetPosition()
{
	DWORD pos;
	pBuf->GetCurrentPosition(&pos, nullptr);

	return pos;
}

void SndObjDSound::SendData()
{
	u_long pos = GetPosition(),
		add = 0;

	if (pos - offset < 0)
		add = BUFFER_SIZE;
	if (pos + add - offset > BUFFER_HALF + 16)
	{
		DWORD bytes1, bytes2;
		short* ptr1, * ptr2;

		trans_lock = 1;
		pBuf->Lock(offset, BUFFER_QUART, (LPVOID*)&ptr1, &bytes1, (LPVOID*)&ptr2, &bytes2, 0);
		// just fill with silence if we're stopping
		if (stopping)
		{
#if _DEBUG
			OutputDebugStringA(__FUNCTION__ ": sending silence...\n");
#endif
			memset(ptr1, 0, bytes1);
		}
		else
		{
			auto needed = str->Decode(ptr1, bytes1 / fmt.nBlockAlign, loops);
			if (needed && loops == 0)
			{
				// fill trail with silence
				needed *= fmt.nBlockAlign;
				memset(&ptr1[(bytes1 - needed) / 2], 0, needed);
			}
		}
		pBuf->Unlock(ptr1, bytes1, ptr2, bytes2);
		trans_lock = 0;

		u_long total = offset + BUFFER_QUART;
		offset = total;
		if (BUFFER_SIZE <= total)
			offset = total - BUFFER_SIZE;
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
#if _DEBUG
			OutputDebugStringA(__FUNCTION__ ": still playing, can't release...\n");
#endif
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
	// inactive objects need to do nothing
	if (used == 0) return;

	if (pBuf && stopped == 0)
	{
		// if it's not set to loop we need to release the object when it's done playing
		if (loops == 0 && str->sample_index >= str->loop_end_index)
		{
			Stop();
			if (cbPlayEnd)
				cbPlayEnd(cbPlayContext);
			//Release();
		}
		else
		{
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

void SndObjDSound::Release()
{
	if (used && pBuf)
	{
		while (trans_lock)
			;	// wait if it's transferring data in the thread
		pBuf->Release();
		pBuf = nullptr;

		SndObjBase::Release();
	}
}

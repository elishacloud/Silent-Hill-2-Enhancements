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

#define MAX_OBJ			32
#define BUFFER_SIZE		32768
#define BUFFER_HALF		(BUFFER_SIZE / 2)
#define BUFFER_QUART	(BUFFER_HALF / 2)

static SndObj obj_tbl[MAX_OBJ];

void ds_SetupSound(LPDIRECTSOUND8 pDS)
{
	pDS8 = pDS;

	for (int i = 0; i < MAX_OBJ; i++)
		obj_tbl[i].used = 0;
}

SndObj* ds_FindObj()
{
	for (int i = 0; MAX_OBJ; i++)
	{
		if (obj_tbl[i].used == 0)
			return &obj_tbl[i];
	}

	return nullptr;
}

void ds_Update()
{
	for (int i = 0; i < MAX_OBJ; i++)
	{
		auto obj = &obj_tbl[i];
		if (obj->used == 0) continue;
		if (obj->pBuf && obj->stopped == 0)
		{
			if (obj->loops == 0 && obj->str->sample_index >= obj->str->loop_end_index)
			{
				obj->Stop();
				if (obj->cbPlayEnd)
					obj->cbPlayEnd(obj->cbPlayContext);
			}
			else
			{
				if (obj->adx && obj->adx->set_volume)
				{
					obj->SetVolume(obj->adx->volume);
					obj->adx->set_volume = 0;
				}
				obj->SendData();
			}
		}
	}
}

void SndObj::CreateBuffer(CriFileStream* stream)
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
	desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | /*DSBCAPS_CTRL3D |*/ DSBCAPS_CTRLFREQUENCY /*| DSBCAPS_GETCURRENTPOSITION2*/ /*| DSBCAPS_LOCSOFTWARE*/;
	desc.dwBufferBytes = BUFFER_SIZE;
	if (FAILED(pDS8->CreateSoundBuffer(&desc, &pBuf, nullptr)))
		MessageBoxA(nullptr, "error", __FUNCTION__, MB_OK);

	DWORD bytes1, bytes2;
	short* ptr1, * ptr2;
	trans_lock = 1;
	pBuf->Lock(0, BUFFER_SIZE, (LPVOID*)&ptr1, &bytes1, (LPVOID*)&ptr2, &bytes2, 0);
	auto needed = stream->Decode(ptr1, bytes1 / fmt.nBlockAlign, loops);
	if (needed && loops == 0)
	{
		needed *= fmt.nBlockAlign;
		memset(&ptr1[(bytes1 - needed) / 2], 0, needed);
	}
	pBuf->Unlock(ptr1, bytes1, ptr2, bytes2);
	trans_lock = 0;

	used = 1;
	offset = 0;
}

u_long SndObj::GetPosition()
{
	DWORD pos;
	pBuf->GetCurrentPosition(&pos, nullptr);

	return pos;
}

void SndObj::SendData()
{
	trans_lock = 1;

	u_long add = 0, pos;
	const DWORD snd_dwBytes = BUFFER_QUART;

	pos = GetPosition();

	if (pos - offset < 0)
		add = BUFFER_SIZE;
	if (pos + add - offset > 2 * snd_dwBytes + 16)
	{
		DWORD bytes1, bytes2;
		short* ptr1, * ptr2;

		pBuf->Lock(offset, snd_dwBytes, (LPVOID*)&ptr1, &bytes1, (LPVOID*)&ptr2, &bytes2, 0);
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

		auto total = snd_dwBytes + offset;
		offset = total;
		if (BUFFER_SIZE <= total)
			offset = total - BUFFER_SIZE;
	}

	trans_lock = 0;
}

void SndObj::SetVolume(int vol)
{
	if (vol < -1000)
		vol = -1000;

	volume = vol;
	if (used && pBuf)
		pBuf->SetVolume(vol * 10);
}

void SndObj::Play()
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

int SndObj::Stop()
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

int SndObj::GetStatus()
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

void SndObj::Release()
{
	if (used && pBuf)
	{
		while (trans_lock);	// wait if it's transferring data in the thread
		pBuf->Release();
		memset(this, 0, sizeof(*this));
	}
}

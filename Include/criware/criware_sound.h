#pragma once
#include "criware_adx.h"

enum DSOBJ_STATE
{
	DSOS_UNUSED,
	DSOS_PLAYING,
	DSOS_LOOPING,
	DSOS_ENDED
};

#define SOUND_MAX_OBJ			32

typedef void (*SndCbPlayEnd)(LPVOID);

class SndObjBase
{
public:
	SndObjBase()
	{
		Release();
	}
	virtual ~SndObjBase()
	{
		Release();
	}

	virtual void CreateBuffer(CriFileStream* stream)
	{
		UNREFERENCED_PARAMETER(stream);
	}

	virtual void Release()
	{
		offset = 0;
		offset_played = 0;
		used = 0;
		loops = 0;
		stopped = 0;
		volume = 0;
		memset(&fmt, 0, sizeof(fmt));
		str = nullptr;
		adx = nullptr;
	}

	virtual void Play() {}
	virtual void Stop() {}
	virtual void Update() {}

	virtual void SendData() {}
	virtual void SetVolume(int vol)
	{
		UNREFERENCED_PARAMETER(vol);
	}

	u_long offset,
		offset_played;
	u_long used : 1,
		loops : 1,
		stopped : 1;
	int volume;
	WAVEFORMATEX fmt;
	CriFileStream* str;
	ADXT_Object* adx;
};

SndObjBase* adxs_FindObj();
void adxs_Clear(SndObjBase* obj);
void adxs_Release();

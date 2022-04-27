#pragma once
#include "criware_adx.h"

enum DSOBJ_STATE
{
	DSOS_UNUSED,
	DSOS_PLAYING,
	DSOS_LOOPING,
	DSOS_ENDED
};

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

#pragma warning(suppress: 4100)
	virtual void CreateBuffer(CriFileStream* stream) {}

	virtual void Release()
	{
		offset = 0;
		offset_played = 0;
		used = 0;
		loops = 0;
		stopped = 0;
		stopping = 0;
		volume = 0;
		memset(&fmt, 0, sizeof(fmt));
		str = nullptr;
		adx = nullptr;
	}

	virtual void Play() {}
	virtual int  Stop() { return 1; }
	virtual void Update() {}

	virtual void SendData() {}
#pragma warning(suppress: 4100)
	virtual void SetVolume(int vol) {}

	u_long offset,
		offset_played;
	u_long used : 1,
		loops : 1,
		stopped : 1,
		stopping : 1;
	int volume;
	WAVEFORMATEX fmt;
	CriFileStream* str;
	ADXT_Object* adx;
};

SndObjBase* adxs_FindObj();
void adxs_Update();
void adxs_Release();

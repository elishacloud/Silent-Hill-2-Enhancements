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

	virtual void CreateBuffer(CriFileStream* stream) {}

	virtual void Release()
	{
		offset = 0;
		used = 0;
		loops = 0;
		stopped = 0;
		trans_lock = 0;
		stopping = 0;
		volume = 0;
		memset(&fmt, 0, sizeof(fmt));
		str = nullptr;
		adx = nullptr;

		cbPlayEnd = nullptr;
		cbPlayContext = nullptr;
	}

	virtual void Play() {}
	virtual int  Stop() { return 1; }
	virtual void Update() {}

	virtual void SendData() {}
	virtual void SetVolume(int vol) {}

	void SetEndCallback(SndCbPlayEnd cb, LPVOID ctx)
	{
		cbPlayEnd = cb;
		cbPlayContext = ctx;
	}

	SndCbPlayEnd cbPlayEnd;
	LPVOID cbPlayContext;

	u_long offset;
	u_long used : 1,
		loops : 1,
		stopped : 1,
		trans_lock : 1,		// failsafe for locking data transfers
		stopping : 1;
	int volume;
	WAVEFORMATEX fmt;
	CriFileStream* str;
	ADXT_Object* adx;
};

SndObjBase* adxs_FindObj();
void adxs_Update();

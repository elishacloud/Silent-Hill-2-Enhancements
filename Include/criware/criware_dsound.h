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

class SndObj
{
public:
	SndObj() : offset(0),
		used(0),
		loops(0),
		stopped(0),
		trans_lock(0),
		stopping(0),
		volume(0),
		pBuf(nullptr),
		str(nullptr),
		fmt{ 0 },
		cbPlayEnd(nullptr),
		cbPlayContext(nullptr)
	{}
	~SndObj()
	{}

	void CreateBuffer(CriFileStream* stream);
	void SetEndCallback(SndCbPlayEnd cb, LPVOID ctx)
	{
		cbPlayEnd = cb;
		cbPlayContext = ctx;
	}

	void Play();
	int  Stop();

	u_long GetPosition();
	int GetStatus();
	void SendData();
	void SetVolume(int vol);
	void Release();

	u_long offset;
	u_long used : 1,
		loops : 1,
		stopped : 1,
		trans_lock : 1,		// failsafe for locking data transfers
		stopping : 1;
	int volume;
	WAVEFORMATEX fmt;
	LPDIRECTSOUNDBUFFER pBuf;
	CriFileStream* str;
	ADXT_Object* adx;

	SndCbPlayEnd cbPlayEnd;
	LPVOID cbPlayContext;
};

void ds_SetupSound(LPDIRECTSOUND8 pDS);
SndObj* ds_FindObj();
void ds_Update();

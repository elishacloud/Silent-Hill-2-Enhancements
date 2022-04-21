#pragma once

#define SOUND_MAX_OBJ			32

class SndObjDSound : public SndObjBase
{
public:
	SndObjDSound() : pBuf(nullptr)
	{}
	virtual ~SndObjDSound()
	{}

	virtual void CreateBuffer(CriFileStream* stream);

	virtual void Play();
	virtual int  Stop();
	virtual void Update();

	virtual void SendData();
	virtual void SetVolume(int vol);
	virtual void Release();

	u_long GetPosition();
	int GetStatus();

	LPDIRECTSOUNDBUFFER pBuf;
};

extern SndObjBase* sound_obj_tbl[SOUND_MAX_OBJ];

void adxs_SetupDSound(LPDIRECTSOUND8 pDS);

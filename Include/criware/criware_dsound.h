#pragma once

#define SOUND_MAX_OBJ			32

class SndObjDSound : public SndObjBase
{
public:
	SndObjDSound() : pBuf(nullptr)
	{
		Release();
	}
	virtual ~SndObjDSound()
	{
		Release();
	}

	virtual void CreateBuffer(CriFileStream* stream);

	virtual void Play();
	virtual int  Stop();
	virtual void Update();

	virtual void SendData();
	virtual void SetVolume(int vol);
	virtual void Release();

private:
	u_long GetPosition();
	u_long GetPlayedSamples();
	int GetStatus();

	void Lock(u_long size);
	void Unlock();
	void Fill(u_long size);

	LPDIRECTSOUNDBUFFER pBuf;
	short *ptr1, *ptr2;
	DWORD bytes1, bytes2;
};

extern SndObjBase* sound_obj_tbl[SOUND_MAX_OBJ];

void adxs_SetupDSound(LPDIRECTSOUND8 pDS);

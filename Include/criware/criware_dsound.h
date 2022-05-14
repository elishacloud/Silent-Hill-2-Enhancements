#pragma once

#if !XAUDIO2

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
	virtual void Stop();
	virtual void Update();

	virtual void SendData();
	virtual void SetVolume(int vol);
	virtual void Release();

private:
	u_long GetPosition();
	u_long GetPlayedSamples();
	int GetStatus();

	void Fill(u_long size);

	LPDIRECTSOUNDBUFFER pBuf;
};

extern SndObjBase* sound_obj_tbl[SOUND_MAX_OBJ];

void adxs_SetupDSound(LPDIRECTSOUND8 pDS);

#endif

/*
* Copyright (C) 2022 Gemini
* ===============================================================
* Main ADX interface
* ---------------------------------------------------------------
* Exposes the original functions used in Criware's ADX library,
* helpful for hooking to the game's code.
* ===============================================================
*/
#include "criware.h"

ADXFIC_Object* ADXFIC_Create(const char* dname, int mode, char *work, int wksize)
{
	return adx_ficCreate(dname);
}

void ADXFIC_Destroy(ADXFIC_Object* obj)
{
	delete obj;
}

u_long ADXFIC_GetNumFiles(ADXFIC_Object* obj)
{
	return obj->files.size();
}

const char* ADXFIC_GetFileName(ADXFIC_Object* obj, u_long index)
{
	return obj->files[index].filename.c_str();
}

// no need to fill these, it's just some leftovers from the XBOX library
void ADXWIN_SetupDvdFs(void*) {}
void ADXWIN_ShutdownDvdFs() {}

void ADXWIN_SetupSound(LPDIRECTSOUND8 pDS8)
{
	ds_SetupSound(pDS8);
}

// initializes the threads used by the ADX server
// in our case it just creates one thread running in parallel with the game
void ADXM_SetupThrd(int* priority /* ignored */)
{
	ADX_lock_init();
	server_create();
}

// destroys the ADX threads
void ADXM_ShutdownThrd()
{
	server_destroy();
	ADX_lock_close();
}

// leave empty
void ADXM_ExecMain() {}

int ADXF_LoadPartitionNw(int ptid, const char *filename, void *ptinfo, void *nfile)
{
	asf_LoadPartitionNw(ptid, filename, ptinfo, nfile);

	return 1;
}

// AFS partition read status, just pretent it's done
int ADXF_GetPtStat(int)
{
	return ADXF_STAT_READEND;
}

// returns an ADXT_STAT value
int ADXT_GetStat(ADXT_Object* obj)
{
	if(obj)
		return obj->state;

	return ADXT_STAT_STOP;
}

void ADXT_SetOutVol(ADXT_Object *obj, int volume)
{
	if (obj)
	{
		obj->volume = volume;
		// if there's no dsound object queue a volume change
		if (obj->obj == nullptr)
			obj->set_volume = 1;
		// otherwise just update the object directly
		else obj->obj->SetVolume(volume);
	}
}

// Starts to play ADX file with partition ID and file ID
void ADXT_StartAfs(ADXT_Object* obj, int patid, int fid)
{
	if (obj == nullptr)
		return;
	
	if (obj->state != ADXT_STAT_STOP)
		ADXT_Stop(obj);
	asf_StartAfs(obj, patid, fid);
}

void ADXT_Stop(ADXT_Object* obj)
{
	if(obj)
		obj->Release();
}

void ADXT_StartFname(ADXT_Object* obj, const char* fname)
{
	if (obj == nullptr)
		return;

	if (obj->state != ADXT_STAT_STOP)
		ADXT_Stop(obj);

	ADXStream *stream;
	OpenADX(fname, &stream);

	obj->state = ADXT_STAT_PLAYING;
	obj->stream = stream;
	obj->obj = ds_FindObj();
	obj->obj->loops = true;
	obj->obj->adx = obj;

	obj->obj->CreateBuffer(stream);
	obj->obj->Play();
}

// initializes the "talk" module, whatever that does
void ADXT_Init()
{}

// theoretically destroys all active talk handles
void ADXT_Finish()
{}

// Creation of an ADXT handle
ADXT_Object* ADXT_Create(int maxch, void* work, u_long work_size)
{
	ADXT_Object* obj = new ADXT_Object;
	obj->maxch = maxch;
	obj->work = work;
	obj->work_size = work_size;

	return obj;
}

// Destroy an ADXT handle
void ADXT_Destroy(ADXT_Object* adxt)
{
	if(adxt)
		delete adxt;
}

// leave empty
int AIXP_Init() { return 0; }

// leave empty
void AIXP_ExecServer() {}

AIXP_Object* AIXP_Create(int maxntr, int maxnch, void* work, int worksize)
{
	AIXP_Object* obj = new AIXP_Object;

	for (int i = 0; i < _countof(obj->adxt); i++)
	{
		obj->adxt[i].maxch = maxnch;
		obj->adxt[i].work_size = worksize;
		obj->adxt[i].work = work;
	}

	return obj;
}

void AIXP_Destroy(AIXP_Object* obj)
{
	if(obj)
		delete obj;
}

void AIXP_Stop(AIXP_Object* obj)
{
	if(obj)
		obj->Release();
}

// set if this should loop
void AIXP_SetLpSw(AIXP_Object* obj, int sw)
{
	if (obj == nullptr)
		return;

	for(int i = 0; i < obj->stream_no; i++)
		obj->adxt[i].obj->loops = sw ? 1 : 0;
}

void AIXP_StartFname(AIXP_Object* obj, const char* fname, void* atr)
{
	if (obj == nullptr)
		return;

	aix_start(obj, fname);
}

ADXT_Object* AIXP_GetAdxt(AIXP_Object* obj, int trno)
{
	if (obj == nullptr)
		return nullptr;

	return &obj->adxt[trno];
}

int AIXP_GetStat(AIXP_Object* obj)
{
	if(obj)
		return obj->state;

	return AIXP_STAT_STOP;
}

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
	UNREFERENCED_PARAMETER(mode);
	UNREFERENCED_PARAMETER(work);
	UNREFERENCED_PARAMETER(wksize);

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

// directsound setup
void ADXWIN_SetupSound(LPDIRECTSOUND8 pDS8)
{
	adxs_SetupDSound(pDS8);
}

// close directsound
void ADXWIN_ShutdownSound()
{
	adxs_Release();
}

// initialize the threads used by the ADX server
void ADXM_SetupThrd(int* priority /* ignored */)
{
	UNREFERENCED_PARAMETER(priority);

	ADX_lock_init();
	server_create();
}

// destroy the ADX threads
void ADXM_ShutdownThrd()
{
	server_destroy();
	ADX_lock_close();
}

// leave empty
// this is called inside the performance thread, but it's useless
int ADXM_ExecMain()
{
	return 1;
}

// initialize AFS partition
int ADXF_LoadPartitionNw(int ptid, const char *filename, void *ptinfo, void *nfile)
{
	afs_LoadPartitionNw(ptid, filename, ptinfo, nfile);

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
	if (obj)
		return obj->state;

	return -1;
}

// send volume signal or just set volume in real time
void ADXT_SetOutVol(ADXT_Object *obj, int volume)
{
	if (obj)
	{
		obj->volume = volume;
		// if there's no sound object yet, queue a volume change
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
	afs_StartAfs(obj, patid, fid);
}

// clear ADXT handle
void ADXT_Stop(ADXT_Object* obj)
{
	if(obj)
		obj->Reset();
}

// Starts to play ADX with filename
void ADXT_StartFname(ADXT_Object* obj, const char* fname)
{
	if (obj == nullptr)
		return;

	if (obj->state != ADXT_STAT_STOP)
		ADXT_Stop(obj);
	adx_StartFname(obj, fname);
}

// initialize the "talk" module, whatever that does
void ADXT_Init()
{}

// theoretically destroys all active talk handles
void ADXT_Finish()
{}

// creation of an ADXT handle
ADXT_Object* ADXT_Create(int maxch, void* work, u_long work_size)
{
	ADXT_Object* obj = new ADXT_Object;
	obj->maxch = (u_short)maxch;
	obj->work = work;
	obj->work_size = work_size;

	return obj;
}

// destroy an ADXT handle
void ADXT_Destroy(ADXT_Object* adxt)
{
	if(adxt)
		delete adxt;
}

// leave empty
void AIXP_Init() {}

// leave empty
void AIXP_ExecServer() {}

// create a demuxer handle
AIXP_Object* AIXP_Create(int maxntr, int maxnch, void* work, int worksize)
{
	UNREFERENCED_PARAMETER(maxntr);

	AIXP_Object* obj = new AIXP_Object;

	for (int i = 0; i < _countof(obj->adxt); i++)
	{
		obj->adxt[i].maxch = (u_short)maxnch;
		obj->adxt[i].work_size = worksize;
		obj->adxt[i].work = work;
	}

	return obj;
}

// deallocate demuxer
void AIXP_Destroy(AIXP_Object* obj)
{
	if (obj)
	{
		AIXP_Stop(obj);
		delete obj;
	}
}

// release internal states of demuxer
void AIXP_Stop(AIXP_Object* obj)
{
	if(obj)
		obj->Release();
}

// set if AIX should loop
void AIXP_SetLpSw(AIXP_Object* obj, int sw)
{
	if (obj == nullptr)
		return;

	for(int i = 0; i < obj->stream_no; i++)
		obj->adxt[i].obj->loops = sw ? 1 : 0;
}

// start playing AIX with a filename
void AIXP_StartFname(AIXP_Object* obj, const char* fname, void* atr)
{
	UNREFERENCED_PARAMETER(atr);

	if (obj == nullptr)
		return;

	aix_start(obj, fname);
}

// retrieve an ADXT handle from demuxer
ADXT_Object* AIXP_GetAdxt(AIXP_Object* obj, int trno)
{
	if (obj == nullptr)
		return nullptr;

	return &obj->adxt[trno];
}

// demuxer state
int AIXP_GetStat(AIXP_Object* obj)
{
	if(obj)
		return obj->state;

	return AIXP_STAT_STOP;
}

#pragma once

#ifdef _NVWA
#include "..\nvwa\debug_new.h"
#endif

#include <windows.h>
#include <dsound.h>
#include <vector>
#include <string>

#define	ADXF_STAT_STOP		(1)		// Idling
#define ADXF_STAT_READING	(2)		// During data read-in
#define ADXF_STAT_READEND	(3)		// Data read-in end
#define ADXF_STAT_ERROR		(4)		// Read-in error outbreak state

#define	ADXT_STAT_STOP		(0)		// Idling
#define ADXT_STAT_DECINFO	(1)		// Getting header information
#define ADXT_STAT_PREP		(2)		// During playback preparation
#define ADXT_STAT_PLAYING	(3)		// During decode and playback
#define ADXT_STAT_DECEND	(4)		// Decode end
#define ADXT_STAT_PLAYEND	(5)		// Play end
#define ADXT_STAT_ERROR		(6)		// Read-in error outbreak state

#define	AIXP_STAT_STOP		(0)		// Idling
#define AIXP_STAT_PREP		(1)		// During playback preparation
#define AIXP_STAT_PLAYING	(2)		// During decode and playback
#define AIXP_STAT_PLAYEND	(3)		// Play end
#define AIXP_STAT_ERROR		(4)		// Read-in error outbreak state

//-------------------------------------------
// big endian helpers
class BE16
{
public:
	__inline WORD w() { return (d[0] << 8) | (d[1]); }
	__inline short s() { return (short)w(); }

	operator BYTE()  { return w() & 0xff; }
	operator WORD()  { return w(); }
	operator DWORD() { return w(); }
	operator char()  { return s() & 0xff; }
	operator short() { return s(); }
	operator int()   { return s(); }
private:
	BYTE d[2];
};

class BE32
{
public:
	__inline DWORD dw() { return (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | (d[3]); }
	__inline int i() { return (int)dw(); }

	operator BYTE()  { return dw() & 0xff; }
	operator WORD()  { return dw() & 0xffff; }
	operator DWORD() { return dw(); }
	operator char()  { return i() & 0xff; }
	operator short() { return i() & 0xffff; }
	operator int()   { return i(); }
private:
	BYTE d[4];
};

#include "criware_server.h"
#include "criware_file.h"
#include "criware_sound.h"
#include "criware_dsound.h"
#include "criware_adx.h"
#include "criware_aix.h"
#include "criware_adxfic.h"
#include "criware_afs.h"

//-------------------------------------------
// main module exposed methods

// windows exclusive
void ADXWIN_SetupDvdFs(void* = nullptr);
void ADXWIN_ShutdownDvdFs();
void ADXWIN_SetupSound(LPDIRECTSOUND8 pDS8);
void ADXWIN_ShutdownSound();

// threads
void ADXM_SetupThrd(int* = nullptr);
void ADXM_ShutdownThrd();
int  ADXM_ExecMain();

// hierarchy interface
ADXFIC_Object* ADXFIC_Create(const char* dname, int mode, char* work, int wksize);
void ADXFIC_Destroy(ADXFIC_Object* obj);
u_long ADXFIC_GetNumFiles(ADXFIC_Object* obj);
const char* ADXFIC_GetFileName(ADXFIC_Object* obj, u_long index);

// partition interface
int ADXF_LoadPartitionNw(int ptid, const char* filename, void* ptinfo, void* nfile);
int ADXF_GetPtStat(int);

// adx talk interface
void ADXT_Init();
void ADXT_Finish();
ADXT_Object* ADXT_Create(int maxch, void* work, u_long work_size);
void ADXT_Stop(ADXT_Object* obj);
int  ADXT_GetStat(ADXT_Object* obj);
void ADXT_StartFname(ADXT_Object* obj, const char* fname);
void ADXT_SetOutVol(ADXT_Object* obj, int);
void ADXT_StartAfs(ADXT_Object* obj, int patid, int fid);

// aix parsing interface
void AIXP_Init();
void AIXP_Stop(AIXP_Object *obj);
void AIXP_ExecServer();
void AIXP_Destroy(AIXP_Object *obj);
AIXP_Object* AIXP_Create(int maxntr, int maxnch, void* work, int worksize);
void AIXP_SetLpSw(AIXP_Object *obj, int sw);
void AIXP_StartFname(AIXP_Object *obj, const char *fname, void *atr);
ADXT_Object* AIXP_GetAdxt(AIXP_Object *obj, int trno);
int  AIXP_GetStat(AIXP_Object *obj);


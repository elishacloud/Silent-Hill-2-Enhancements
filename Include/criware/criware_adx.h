#pragma once

//-------------------------------------------
typedef struct ADX_headerV3
{
	BE16 magic;					// 0
	BE16 copyright_offset;		// 2
	BYTE encoding,				// 4
		block_size,				// 5
		sample_bitdepth,		// 6
		channel_count;			// 7
	BE32 sample_rate,			// 8
		total_samples;			// C
	BE16 highpass_frequency;	// 10
	BYTE version,				// 12
		flags;					// 13
	BE16 loop_align_samples,	// 14
		loop_enabled;			// 16
	BE32 loop_enabled2,			// 18
		loop_sample_begin,		// 1C
		loop_byte_begin,		// 20
		loop_sample_end,		// 24
		loop_byte_end,			// 28
		dummy0,					// 2C
		dummy1,					// 30
		dummy2,					// 34
		dummy3,					// 38
		dummy4;					// 3C
} ADX_headerV3;

typedef struct ADX_headerAIX
{
	BE16 magic;					// 0
	BE16 copyright_offset;		// 2
	BYTE encoding,				// 4
		block_size,				// 5
		sample_bitdepth,		// 6
		channel_count;			// 7
	BE32 sample_rate,			// 8
		total_samples;			// C
	BE16 highpass_frequency;	// 10
	BYTE version,				// 12
		flags;					// 13
} ADX_header_AIX;

typedef struct ADX_headerV4
{
	BE16 magic;					// 0
	BE16 copyright_offset;		// 2
	BYTE encoding,				// 4
		block_size,				// 5
		sample_bitdepth,		// 6
		channel_count;			// 7
	BE32 sample_rate,			// 8
		total_samples;			// C
	BE16 highpass_frequency;	// 10
	BYTE version,				// 12
		flags;					// 13
	BE16 loop_align_samples,	// 14
		dummy0;					// 16
	BE32 dummy1,				// 18
		dummy2,					// 1C
		dummy3,					// 20
		loop_enabled,			// 24
		loop_sample_begin,		// 28
		loop_byte_begin,		// 2C
		loop_sample_end,		// 30
		loop_byte_end,			// 34
		dummy4,					// 38
		dummy5;					// 3C
} ADX_headerV4;

class SndObjBase;	// forward declaration

class ADXT_Object
{
public:
	ADXT_Object();
	~ADXT_Object();

	void Release();

	void Suspend();
	void Resume();

	CriFileStream* stream;
	SndObjBase* obj;
	int volume,
		state;
	//
	u_long work_size;
	void* work;
	u_short maxch;
	// state flags
	u_short is_aix : 1,
		is_blocking : 1,
		set_volume : 1,
	// threading
		th_suspended : 1,
		th_exit : 1;
	HANDLE th;

private:
	void Thread();
	static DWORD WINAPI thread(LPVOID param);
};

void ADXDEC_SetCoeff(CriFileStream* adx);
unsigned ADXDEC_Decode(CriFileStream* adx, short* buffer, unsigned samples_needed, bool looping_enabled);

int OpenADX(const char* filename, ADXStream** obj);
int OpenADX(ADXStream* adx);

void adx_StartFname(ADXT_Object* obj, const char* fname);

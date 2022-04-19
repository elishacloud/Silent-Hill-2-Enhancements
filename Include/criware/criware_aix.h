#pragma once
//-------------------------------------------
// generic header
typedef struct AIX_ENRTY
{
	BE32 frequency;
	BE32 channels;
} AIX_ENTRY;

typedef struct AIX_CHUNK
{
	BYTE magic[3],			// 00 'AIX'
		type;				// 03 F, P, E
	BE32 next;				// 04 offset to next header
} AIX_CHUNK;

typedef struct AIX_HEADER
{
	DWORD magic;			// 00 'AIXF'
	BE32 next;				// 04 offset to next header
	BE32 unk8,				// 08
		unkC,				// 0C sector alignment?
		unk10,				// 10
		unk14,				// 14
		unk18,				// 18
		unk1C,				// 1C
		unk20,				// 20
		data_size,			// 24 amount of interleaved ADX data
		total_samples;		// 28
	BE32 frequency,			// 2C frequency for all streams
		unk30,				// 30
		unk34,				// 34
		unk38,				// 38
		unk3C;				// 3C
	BYTE stream_count,			// 40 number of interleaved streams
		unk41[3];			// 41
	BE32 unk44;				// 44 no idea, always zero
	AIX_ENTRY entries[759];	// 48 supplementary stream data
} AIX_HEADER;

// file header
typedef struct AIXF_HEADER
{
	BYTE magic[4];	// AIXF
} AIXF_HEADER;

// properties header
typedef struct AIXP_HEADER
{
	BYTE stream_id,
		out_channels;
	BE16 size;
	BE32 frames;
} AIXP_HEADER;

// end header
typedef struct AIXE_HEADER
{
	BYTE magic[4];	// AIXE
} AIXE_HEADER;

class AIXP_Object
{
public:
	AIXP_Object() : stream_no(0),
		aix(nullptr),
		state(AIXP_STAT_STOP),
		fname(nullptr),
		th(0)
	{
	}
	~AIXP_Object()
	{
		Release();
	}

	void Release();

	int stream_no,
		state;
	AIX_Demuxer* aix;
	ADXT_Object adxt[8];
	HANDLE th;		// initial caching thread
	const char* fname;
};

int OpenAIX(const char* filename, AIX_Demuxer** obj);

void aix_start(AIXP_Object* obj, const char* fname);

/*
*/
#pragma once
#define ADXFIC_USE_HASHES	0

#if ADXFIC_USE_HASHES
#include "..\xxhash.h"
#endif

typedef struct ADX_Entry
{
#if ADXFIC_USE_HASHES
	XXH64_hash_t hash;
#endif
	std::string filename;
	DWORD size;
} ADX_Entry;

typedef struct ADXFIC_Object
{
	std::vector<ADX_Entry> files;
} ADXFIC_Object;

ADXFIC_Object* adx_ficCreate(const char* dname);

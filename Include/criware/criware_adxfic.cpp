/*
* Copyright (C) 2022 Gemini
* ===============================================================
* Directory module
* ---------------------------------------------------------------
* Pretty bare bone, game just uses it to enlist files inside the
* data\sound\adx folder and subfolders.
* ===============================================================
*/
#include "criware.h"
#include <algorithm>

static void list_data_folder(const char* path, ADXFIC_Object& dir)
{
	WIN32_FIND_DATAA data;

	char filter[MAX_PATH];
	sprintf_s(filter, sizeof(filter), "%s\\*.*", path);

	HANDLE hFind = FindFirstFileA(filter, &data);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// ignore this folder and previous folder values
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && data.cFileName[0] == '.')
				continue;

			// full file path
			sprintf_s(filter, "%s\\%s", path, data.cFileName);

			// it's a folder, go one level deeper
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				list_data_folder(filter, dir);
			// it's a regular file, add to queue
			else
			{
				HANDLE fp = CreateFileA(filter, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (fp != INVALID_HANDLE_VALUE)
				{
					ADX_Entry entry;
					entry.filename = filter;
					entry.size = GetFileSize(fp, nullptr);
#if ADXFIC_USE_HASHES
					entry.hash = XXH64(entry.filename.c_str(), entry.filename.size(), 0);
#endif
					CloseHandle(fp);
					dir.files.push_back(entry);
				}
			}

		} while (FindNextFileA(hFind, &data));
		FindClose(hFind);
	}
}

#if ADXFIC_USE_HASHES
int quickfind(ADX_Entry* f, size_t size, XXH64_hash_t hash)
{
	int first = 0,
		last = (int)size - 1,
		middle = last / 2;

	while (first <= last)
	{
		if (f[middle].hash < hash)
			first = middle + 1;
		else if (f[middle].hash == hash)
			return middle;
		else last = middle - 1;

		middle = (first + last) / 2;
	}

	return -1;
}
#endif

ADXFIC_Object* adx_ficCreate(const char *dname)
{
	ADXFIC_Object* dir = new ADXFIC_Object;
	// list all the files
	list_data_folder(dname, *dir);
#if ADXFIC_USE_HASHES
	// sort entries by hashes for binary find
	std::sort(dir->files.begin(), dir->files.end(), [](ADX_Entry& a, ADX_Entry& b) { return a.hash < b.hash; });
#endif

	return dir;
}

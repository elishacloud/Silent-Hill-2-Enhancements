#include <windows.h>
#include <fstream>

using namespace std;

constexpr BYTE WAVList[] = {
	0x4C, 0x49, 0x53, 0x54 };

constexpr BYTE WAVMetadata[] = {
	0x4C, 0x49, 0x53, 0x54,
	0x1C, 0x00, 0x00, 0x00,
	0x49, 0x4E, 0x46, 0x4F,
	0x49, 0x43, 0x4F, 0x50,
	0x10, 0x00, 0x00, 0x00,
	0x28, 0x43, 0x29, 0x31,
	0x39, 0x39, 0x37, 0x20,
	0x4B, 0x4F, 0x4E, 0x41,
	0x4D, 0x49, 0x2E, 0x00,
	0x00, 0x00 };

char t_xFileName[MAX_PATH] = { '\0' };

static char* GetFileName(DWORD FileNum)
{
	char FileName[10] = { '\0' };
	sprintf_s(FileName, "%d", FileNum);
	if (strlen(FileName) == 1)
	{
		FileName[3] = '\0';
		FileName[2] = FileName[0];
		FileName[1] = '0';
		FileName[0] = '0';
	}
	else if (strlen(FileName) == 2)
	{
		FileName[3] = '\0';
		FileName[2] = FileName[1];
		FileName[1] = FileName[0];
		FileName[0] = '0';
	}

	sprintf_s(t_xFileName, "sddata%s.wav", FileName);
	return t_xFileName;
}

int main(int argc, char** argv)
{
	printf("MERGEBIN 1.0 by Elisha Riedlinger\n\n");

	// Vars
	ofstream myfile;
	ifstream infile;
	DWORD aIndex = 0;
	char myPath[MAX_PATH] = { '\0' };
	char folder[MAX_PATH] = { '\0' };

	// Arguments
	if (argc == 1)
	{
		printf("usage: %s <source folder>\n", argv[0]);
		printf("No folder path found using current folder!\n");
		strcpy_s(folder, ".\\");
	}
	else if (argc != 2)
	{
		printf("usage: %s <source folder>\n", argv[0]);
		return 1;
	}
	else
	{
		strcpy_s(folder, argv[1]);
		strcat_s(folder, "\\");
	}

	// Open file
	sprintf_s(myPath, "%ssddata.bin", folder);
	myfile.open(myPath, ios::binary | ios::out);
	if (!myfile.is_open())
	{
		printf("error opening %s\n", myPath);
		return 1;
	}

	if (myfile.is_open())
	{
		// Open file
		DWORD FileCounter = 0;

		// Start loop
		for (DWORD FileCounter = 0; FileCounter < 417; FileCounter++)
		{
			// Open file
			sprintf_s(myPath, "%s%s", folder, GetFileName(FileCounter));
			infile.open(myPath, ios::binary | ios::in | ios::ate);
			if (!infile.is_open())
			{
				printf("error opening %s\n", myPath);
				return 1;
			}

			// Get file size
			DWORD size = (DWORD)infile.tellg();
			aIndex += size;
			//printf("sddata%d.wav -> %d\n", FileCounter, aIndex);
			printf("%d - ", FileCounter);

			// Read bytes until metadata location
			char * memblock;
			memblock = new char[size + 1];
			ZeroMemory(memblock, size + 1);
			infile.seekg(0, ios::beg);
			infile.read(memblock, size);

			// Close file
			infile.close();

			// Remove metadata
			for (size_t i = 0; i < size; i += 2)
			{
				int locMetadata = memcmp(WAVList, &memblock[i], 4);
				if (locMetadata == 0)
				{
					size = i;
				}
			}
			if (memblock[size - 1] == 0x01 && memblock[size - 2] == memblock[size - 3])
			{
				size -= 2;
			}

			// Write byte to file
			myfile.write(memblock, size);

			// Write metadata
			if (FileCounter >= 350 && FileCounter <= 393)
			{
				char metablock[2];
				metablock[0] = memblock[size - 1];
				metablock[1] = 0x01;
				myfile.write(metablock, 2);
			}
			else
			{
				myfile.write((char*)WAVMetadata, sizeof(WAVMetadata));
			}

			// Flush data
			myfile.flush();

			// Delete array
			delete[] memblock;
		}
	}

	// Close file
	myfile.close();

	printf("\n\nCompleted!\n");

	return 0;
}

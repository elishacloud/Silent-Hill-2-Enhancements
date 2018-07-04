#include <windows.h>
#include <fstream>

using namespace std;

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
	ofstream outfile;
	ifstream myfile;
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
	myfile.open(myPath, ios::binary | ios::in | ios::ate);
	if (!myfile.is_open())
	{
		printf("error opening %s\n", myPath);
		return 1;
	}

	// Get file bytes
	DWORD size = (DWORD)myfile.tellg();
	myfile.seekg(0, ios::beg);
	char * memblock;
	memblock = new char[size];
	myfile.read(memblock, size);
	myfile.close();

	// Open file
	DWORD FileCounter = 0;
	sprintf_s(myPath, "%s%s", folder, GetFileName(FileCounter));
	outfile.open(myPath, ios::binary | ios::out);
	if (!outfile.is_open())
	{
		printf("error creating %s\n", myPath);
		return 1;
	}

	// Start loop
	for (DWORD x = 0; x < size; x++)
	{
		// Check if new file should be created
		if (x + 5 < size && x > 5)
		{
			if (memblock[x] == 'R' &&
				memblock[x + 1] == 'I' &&
				memblock[x + 2] == 'F' &&
				memblock[x + 3] == 'F')
			{
				// Close old file
				outfile.flush();
				outfile.close();

				// Start new file
				FileCounter++;
				sprintf_s(myPath, "%s%s", folder, GetFileName(FileCounter));
				outfile.open(myPath, ios::binary | ios::out);
				if (!outfile.is_open())
				{
					printf("error creating %s\n", myPath);
					return 1;
				}
				printf("%d - ", FileCounter);
			}
		}

		// Write byte to file
		outfile.write(&memblock[x], 1);
	}

	// Delete array
	delete[] memblock;

	printf("\n\nCompleted!\n");

	return 0;
}

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

			// Write byte to file
			myfile.write(memblock, size);

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

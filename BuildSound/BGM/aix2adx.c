#include <stdio.h>
#include <string.h>

// AIX2ADX 0.1
// by hcs

// get 16-bit big endian value
int get16bit(unsigned char* p)
{
	return (p[0] << 8) | p[1];
}

// get 32-bit big endian value
int get32bit(unsigned char* p)
{
	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

int main(int argc, char ** argv) {
	FILE * infile, *outfile = NULL;
	int channel, chancount, frame, samplerate, size;
	int goround, searchstart, eos = 0;
	int channeltocopy, curaix, nextaix, done, i;
	char buf[0x24], namebase[256], filename[256 + 4];
	char *t;

	// Default print statement
	printf("AIX2ADX 0.1 by hcs\n");

	// CLI help message
	if (argc != 2) { printf("usage: %s AIXFILE.AIX\n", argv[0]); return 1; }

	// Open file
	infile = fopen(argv[1], "rb");
	if (!infile) { printf("error opening %s\n", argv[1]); return 1; }

	// Generate namebase
	t = strrchr(argv[1], '\\');
	if (!t) t = argv[1];
	else t++;
	for (i = 0; t < strrchr(argv[1], '.'); t++, i++) namebase[i] = *t;
	namebase[i] = '\0';

	for (goround = 0; goround >= 0; goround++)
	{
		chancount = -1;

		searchstart = eos;

		for (channeltocopy = 0; chancount < 0 || channeltocopy < chancount; channeltocopy++)
		{
			fseek(infile, searchstart, SEEK_SET);
			curaix = searchstart;
			done = 0;

			while (!done)
			{
				if (fread(buf, 8, 1, infile) != 1) { channeltocopy = chancount = 0; goround = -2; break; }

				if (memcmp(buf, "AIX", 3)) { printf("malformed AIX header at %08x (bad signature)\n", curaix); return 1; }

				nextaix = curaix + 8 + get32bit(buf + 0x4);

				switch (buf[3]) {
				case 'F':
					printf("file header\n");
					break;
				case 'E':
					printf("end of section\n");
					done = 1;
					eos = nextaix;
					break;
				case 'P':
					fread(buf, 8, 1, infile);
					channel = buf[0];
					chancount = buf[1];
					frame = get32bit(buf + 4);
					size = get16bit(buf + 2);

					if (channel == channeltocopy)
					{
						if (!outfile)
						{
							sprintf(filename, "%s%02d%03d.adx", namebase, goround, channel);
							outfile = fopen(filename, "wb");
							if (!outfile) { printf("error opening %s\n", filename); return 1; }
						}
						printf("goround #%d\tchannel #%d/%d\tframe #%d\tsize: %#08x\n", goround, channel + 1, chancount, frame, size);
						for (i = 0; i < size; i++)
						{
							fread(buf, 1, 1, infile);
							fwrite(buf, 1, 1, outfile);
						}
					}
					break;
				default:
					printf("malformed AIX header at %08x (bad type)\n", curaix);
				}

				fseek(infile, nextaix, SEEK_SET);
				curaix = nextaix;
			} // while (!done)

			if (outfile) { fclose(outfile); outfile = NULL; }

		} // for (channeltocopy)

	} // for (goround)

	return 0;
}

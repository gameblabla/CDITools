/*
****************************************************************
* Pads a file to a specified size using a specified byte.
****************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef	unsigned char	BYTE;
typedef	unsigned short	WORD;
typedef	unsigned long	DWORD;
typedef	BYTE			BOOL;

#ifndef	TRUE
#define	TRUE			1
#endif
#ifndef	FALSE
#define	FALSE			0
#endif

BYTE	*Buffer	= NULL;

void Usage();

int main(int argc, char *argv[])
{
	DWORD	Length;
	BYTE	Filler;

	printf("Pad File.\n");

	if (argc < 4)
	{
		Usage();
	}

	Filler	= (BYTE)atol(argv[1]);

	FILE	*Handle	= fopen(argv[3], "rb");

	if (NULL == Handle)
	{
		printf("Unable to open file.\n");

		exit(1);
	}

	fseek(Handle, 0, SEEK_END);

	Length	= ftell(Handle);

	rewind(Handle);

	Buffer	= malloc(atol(argv[2]));

	if (NULL == Buffer)
	{
		fclose(Handle);

		printf("Unable to allocate memory.\n");

		exit(1);
	}

	memset(Buffer, Filler, atol(argv[2]));

	fread(Buffer, 1, Length, Handle);
	fclose(Handle);

	Handle	= fopen(argv[3], "wb");

	if (NULL == Handle)
	{
		printf("Unable to open rom.\n");

		exit(1);
	}

	fwrite(Buffer, 1, atol(argv[2]), Handle);
	fclose(Handle);
	
	free(Buffer);

	return	0;
}

void Usage()
{
	printf("Command syntax is: PadFile PadByte PadSize Infile.bin\n");

	exit(1);
}

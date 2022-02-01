/*****************************************************************
* Extract ADPCM from a AIFF file (CD-I)
*****************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BYTE uint8_t
#define DWORD uint32_t
#define WORD int32_t
#define LONG int32_t
#define BOOL uint8_t
#define FALSE 0
#define TRUE 1

DWORD	ADPCMLength;
BYTE	*ADPCMBuffer;
WORD	SampleChannels;
DWORD	SampleCount;
WORD	SampleSize;
WORD	SampleRate;

#define	WORDSwap(type) (((type >> 8) & 0x00FF) | ((type << 8)  & 0xFF00))
#define	DWORDSwap(type) (((type >> 24) & 0x000000FF) | ((type >> 8) & 0x0000FF00) | ((type << 8)  & 0x00FF0000) | ((type << 24) & 0xFF000000))

void Usage();
void DecodeADPCM(BYTE *Buffer);

int main(int argc, char *argv[])
{
	printf("Extract ADPCM (CD-I).\n");

	if (argc < 3)
	{
		Usage();
	}

	FILE	*Handle	= fopen(argv[1], "rb");

	if (NULL == Handle)
	{
		printf("Unable to open input file\n");
	
		exit(1);
	}

	fseek(Handle, 0, SEEK_END);

	DWORD	Length	= ftell(Handle);
	rewind(Handle);

	BYTE	*Buffer	= malloc(Length);

	if (NULL == Buffer)
	{
		printf("Unable to allocate memory for the input buffer\n");
	
		exit(1);
	}

	memset(Buffer, 0, Length);

	fread(Buffer, 1, Length, Handle);

	fclose(Handle);

	DecodeADPCM(Buffer);

	if (ADPCMLength % 128 != 0)
	{
		ADPCMLength	-= ADPCMLength % 128;
		ADPCMLength	+= 128;
	}
	
	Handle	= fopen(argv[2], "wb");

	if (NULL == Handle)
	{
		printf("Unable to open output file\n");
	
		exit(1);
	}

	fwrite(ADPCMBuffer, 1, ADPCMLength, Handle);

	fclose(Handle);

	if (Buffer != NULL)
	{
		free(Buffer);
	}
	
	if (ADPCMBuffer != NULL) free(ADPCMBuffer);

	printf("Output Length %d Groups %d Sectors %d\n", ADPCMLength, ADPCMLength / 128, ADPCMLength / 2304);

	return	0;
}

void Usage()
{
	printf("Command syntax is: ExtractADPCM InFile.acm Outfile.acm\n");

	exit(1);
}

void DecodeADPCM(BYTE *Buffer)
{
	DWORD	Chunk;
	DWORD	DataSize	= 1;
	DWORD	Length;
	
	while (DataSize > 0)
	{
		Chunk	= *(DWORD *)Buffer;
		/* Gameblabla : Possibly noy endian safe */
		switch (Chunk)
		{
			case 'MROF':						
				Buffer		+= 4;				

				DataSize	= DWORDSwap(*(DWORD *)Buffer);			

				Buffer		+= 4;

				break;
	
			case 'FFIA':
				Buffer		+= 4;				
				DataSize	-= 4;
				
				break;
		
			case 'MMOC':
				Buffer		+= 4;				
				DataSize	-= 4;

				Length		= DWORDSwap(*(DWORD *)Buffer);
				
				Buffer		+= 4;
				DataSize	-= 4;

				SampleChannels	= WORDSwap(*(WORD *)Buffer);
				Buffer		+= 2;
				DataSize	-= 2;

				SampleCount	= DWORDSwap(*(DWORD *)Buffer);
				Buffer		+= 4;
				DataSize	-= 4;

				SampleSize	= WORDSwap(*(WORD *)Buffer);
				Buffer		+= 2;
				DataSize	-= 2;

				/* Skip WORD */
				Buffer		+= 2;
				DataSize	-= 2;

				SampleRate	= WORDSwap(*(WORD *)Buffer);
				Buffer		+= 2;
				DataSize	-= 2;

				/* Skip last 6 bytes */
				Buffer		+= 6;
				DataSize	-= 6;

				break;

			case 'MCPA':
				Buffer		+= 4;				
				DataSize	-= 4;

				ADPCMLength	= DWORDSwap(*(DWORD *)Buffer) - 8;
				
				Buffer		+= 4;
				DataSize	-= 4;
								
				ADPCMBuffer	= malloc(ADPCMLength);

				if (NULL == ADPCMBuffer)
				{
					printf("Unable to allocate memory for buffer\n");

					exit(1);
				}

				memset(ADPCMBuffer, 0, ADPCMLength);

				memcpy(ADPCMBuffer, Buffer + 8, ADPCMLength);

				Buffer		+= (ADPCMLength + 8);
				DataSize	-= (ADPCMLength + 8);
		}
	}
}

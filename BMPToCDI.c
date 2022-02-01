/*
****************************************************************
* BMP to CD-I 4/7 bit converter. (Paletted)
****************************************************************
*/

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

#define PAL_SIZE	(128 + 2)
#define	DWORDSwap(type) (((type >> 24) & 0x000000FF) | ((type >> 8) & 0x0000FF00) | ((type << 8)  & 0x00FF0000) | ((type << 24) & 0xFF000000))
#define BANKSIZE 384 * 240

uint32_t	Width;
uint32_t	Height;
uint32_t	NumColors;
uint32_t	BankIndex;
uint8_t	Bank[BANKSIZE];
uint32_t	Palette[PAL_SIZE];
uint8_t	BMPBuffer[1024 * 1024];
uint8_t	BMPPalette[768];
char	FilePal[32];

void Usage();
BOOL Convert(char *Filename);
void WriteBitmap(char *Filename);
BOOL LoadBMP(char *Filename, BYTE *Buffer, BYTE *Palette);

void fskip(FILE *fp, int num_bytes)
{
   int i;
   for (i=0; i<num_bytes; i++)
      fgetc(fp);
}


int main(int argc, char *argv[])
{
	printf("BMP to CD-I 4/7 bit converter.\n");

	if (argc < 5)
	{
		Usage();
	}

	memset(Palette, 0, PAL_SIZE * 4);

	if ('B' == argv[1][0])
	{
		BankIndex	= 2;
	}

	else
	{
		BankIndex	= 0;
	}

	NumColors	= atol(argv[2]);

	if (NumColors != 16 && NumColors != 128)
	{
		printf("Colors must be 16 or 128.\n");

		exit(1);
	}

	memset(Bank, 0, BANKSIZE);

	if (argc > 5)
	{
		strcpy(FilePal, argv[5]);

		if (strlen(FilePal) <= 0)
		{
			printf("Palette will not be saved.\n");
		}
	}

	else
	{
		memset(FilePal, 0, sizeof(FilePal));
	}

	if (FALSE == Convert(argv[3]))
	{
		printf("Unable to extract tiles.\n");

		exit(1);
	}

	WriteBitmap(argv[4]);

	return	0;
}

void Usage()
{
	printf("Command syntax is: BMPToCDI Bank Colors Infile.bmp Outfile.bin [Palette]\n");

	exit(1);
}

BOOL Convert(char *Filename)
{
	BYTE	*Buffer	= Bank;
	BYTE	*Image	= BMPBuffer;
	DWORD YLoop, XLoop;

	if (FALSE == LoadBMP(Filename, BMPBuffer, BMPPalette))
	{
		return	FALSE;
	}

	for (YLoop = 0; YLoop < Height; YLoop++)
	{
		for (XLoop = 0; XLoop < Width / 2; XLoop++)
		{
			if (16 == NumColors)
			{
				*Buffer	= (*Image & 0x0F) << 4;
				Image++;
				
				*Buffer	|= (*Image & 0x0F);
				
				Image++;
				Buffer++;
			}

			else
			{
				*Buffer	= *Image;
				
				Buffer++;
				Image++;

				*Buffer	= *Image;
				
				Buffer++;
				Image++;
			}
		}
	}

	return	TRUE;
}

void WriteBitmap(char *Filename)
{
	FILE	*Handle;
	DWORD	Index	= 0;
	DWORD Loop;
	BYTE	Red, Green, Blue;
	
	for (Loop = 0; Loop < NumColors; Loop++)
	{
		if (0 == Loop || 64 == Loop)
		{
			Palette[Index]	= DWORDSwap((0xC3000000 | BankIndex));
					
			Index++;
			BankIndex++;
		}

		Red		= BMPPalette[Loop * 3];
		Green	= BMPPalette[Loop * 3 + 1];
		Blue	= BMPPalette[Loop * 3 + 2];

		Palette[Index]	= DWORDSwap((0x80000000 | ((Loop % 64) << 24) | ((DWORD)Red  << 16) | ((DWORD)Green << 8) | (DWORD)Blue));

		Index++;
	}

	if (strlen(FilePal) > 0)
	{
		Handle	= fopen(FilePal, "wb");

		if (Handle != NULL)
		{
			if (128 == NumColors)
			{
				fwrite(Palette, 4, (NumColors + 2), Handle);
			}

			else
			{
				fwrite(Palette, 4, (NumColors + 1), Handle);
			}

			fclose(Handle);
		}
	}

	Handle	= fopen(Filename, "wb");

	if (Handle != NULL)
	{
		fwrite(Bank, 1, Width * Height, Handle);
		fclose(Handle);
	}
	else
	{
		printf("Can't write file %s\n", Filename);
	}
}


BOOL LoadBMP(char *Filename, BYTE *buu, BYTE *pall)
{
	FILE *fp;
	long long index;
	long x;
	int num_colors;
  
	/* open the file */
	if ((fp = fopen(Filename,"rb")) == NULL)
	{
		printf("Error opening file %s.\n",Filename);
		return 0;
	}
  
	/* check to see if it is a valid bitmap file */
	if (fgetc(fp)!='B' || fgetc(fp)!='M')
	{
		fclose(fp);
		printf("%s is not a bitmap file.\n",Filename);
		return 0;
	}
	
	/* read in the Width and Height of the image, and the
	number of colors used; ignore the rest */
	fskip(fp,16);
	fread(&Width, sizeof(uint16_t), 1, fp);
	fskip(fp,2);
	fread(&Height,sizeof(uint16_t), 1, fp);
	fskip(fp,22);
	fread(&num_colors,sizeof(uint16_t), 1, fp);
	fskip(fp,6);

	/* assume we are working with an 8-bit file */
	if (num_colors==0) num_colors=256;

	/* try to allocate memory */
	if ((buu = malloc((Width*Height))) == NULL)
	{
		fclose(fp);
		printf("Error allocating memory for file %s.\n",Filename);
		return 0;
	}
  
	/* read the palette information */
	for(index=0;index<num_colors;index++)
	{
		pall[index * 3]		= fgetc(fp);
		pall[index * 3 + 1]	= fgetc(fp);
		pall[index * 3 + 2]	= fgetc(fp);
		fgetc(fp);
	}
	
	/* read the bitmap */
	for(index = (Height-1)*Width; index >= 0;index-=Width)
	{
		for(x = 0; x < Width; x++)
		{
			buu[(int)((index+x))]=fgetc(fp);
		}
	}
	fclose(fp);
  
	return 1;
}


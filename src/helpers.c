#include "helpers.h"

boolean debugStop = FALSE;

byte convertAsciiByteToPetsciiByte(byte* toConvert)
{
	if (*toConvert == ASCIIDASH)
	{
		*toConvert = PETSCIIDash;
	}
	else if (*toConvert >= ASCIIA && *toConvert <= ASCIIZ)
	{
		*toConvert = *toConvert + DIFF_ASCII_PETSCII_CAPS;
	}
	else if (*toConvert >= ASCIIa && *toConvert <= ASCIIz)
	{
		*toConvert = *toConvert + DIFF_ASCII_PETSCII_LOWER;
	}
}

void trampoline_0(fnTrampoline_0 func, byte bank)
{
	byte previousRamBank = RAM_BANK;

	RAM_BANK = bank;

	func();

	RAM_BANK = previousRamBank;
}


void trampoline_1pp(fnTrampoline_1BytePointerPointer func, byte** data, byte bank)
{
	byte previousRamBank = RAM_BANK;

	RAM_BANK = bank;
	func(data);

	RAM_BANK = previousRamBank;
}

boolean trampoline_1pRetbool(fnTrampoline_1BytePointerPointerRetBool func, byte** data, byte bank)
{
	byte returnVal;
	byte previousRamBank = RAM_BANK;
	RAM_BANK = bank;
	returnVal = func(data);
	RAM_BANK = previousRamBank;

	return returnVal;
}

void trampoline_1Int(fnTrampoline_1Int func, int data, byte bank)
{
	byte previousRamBank = RAM_BANK;
	RAM_BANK = bank;
	func(data);
	RAM_BANK = previousRamBank;
}

void trampoline_2Int(fnTrampoline_2Int func, int data1, int data2, byte bank)
{
	byte previousRamBank = RAM_BANK;
	RAM_BANK = bank;
	func(data1, data2);
	RAM_BANK = previousRamBank;
}

void trampoline_3Int(fnTrampoline_3Int func, int data1, int data2, int data3, int bank)
{
	byte previousRamBank = RAM_BANK;
	RAM_BANK = bank;
	func(data1, data2, data3);
	RAM_BANK = previousRamBank;
}

char* strcpyBanked(char* dest, const char* src, byte bank)
{
	char* result;
	byte previousRamBank = RAM_BANK;

	RAM_BANK = bank;

    strcpy(dest, src);

	RAM_BANK = previousRamBank;
	
	return result;
}

void copyStringFromBanked(char* src, char* dest, int start, int chunk, byte sourceBank)
{
	int i;
	byte previousRamBank = RAM_BANK;

	RAM_BANK = sourceBank;

	for (i = start; i - start < chunk && (i == 0 || src[i - 1] != '\0'); i++)
	{
		dest[i - start] = src[i];
	}

	RAM_BANK = previousRamBank;
}

int sprintfBanked(const char* buffer, byte bank, char const* const format,  ...) {
	va_list list;
	int result;
	byte previousRamBank = RAM_BANK;

	RAM_BANK = bank;

	va_start(list, format);
	result = sprintf(buffer, list);

	va_end(list);

	RAM_BANK = previousRamBank;
}

void setLogicDirectory(AGIFilePosType* newLogicDirectory, AGIFilePosType* logicDirectoryLocation)
{
	byte previousRamBank = RAM_BANK;

	RAM_BANK = DIRECTORY_BANK;

	*logicDirectoryLocation = *newLogicDirectory;

	RAM_BANK = previousRamBank;
}





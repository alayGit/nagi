#include "helpers.h"

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

void trampoline_1(fnTrampoline_1 func, void* data, byte bank)
{
	byte previousRamBank = RAM_BANK;
	RAM_BANK = bank;
	func(data);
	RAM_BANK = previousRamBank;
}


byte trampoline_1b(fnTrampoline_1b func, void* data, byte bank)
{
	byte returnVal;
	byte previousRamBank = RAM_BANK;
	RAM_BANK = bank;
	returnVal = func(data);
	RAM_BANK = previousRamBank;

	return returnVal;
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



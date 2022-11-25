#include "helpers.h"

byte convertAsciiByteToPetsciiByte(byte* toConvert)
{
	if (*toConvert >= ASCIIA && *toConvert <= ASCIIZ)
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

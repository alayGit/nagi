#include "helpers.h"

byte convertAsciiByteToPetsciiByte(char* toConvert)
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

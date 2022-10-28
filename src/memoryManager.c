#include "memoryManager.h"

short _allocationArraySize;
short _tinyStart;
short _extraSmallStart;
short _smallSmart;
short _mediumStart;
short _mediumLargeStart;
short _largeStart;
short _extraLargeStart;
short _allocationArraySize;
short _allocationArrayStart;

void memoryMangerInit()
{
	_allocationArraySize = MAX_TINY + MAX_EXTRA_SMALL + MAX_SMALL + MAX_MEDIUM + MAX_MEDIUM_LARGE + MAX_EXTRA_LARGE;
	_tinyStart = MANAGED_START;
	_extraSmallStart = _tinyStart + (TINY_SIZE * MAX_TINY);
	_smallSmart = _extraSmallStart + (EXTRA_SMALL_SIZE * MAX_EXTRA_SMALL);
	_mediumStart = _smallSmart + (SMALL_SIZE * MAX_SMALL);
	_mediumLargeStart = _mediumStart + (MEDIUM_SIZE * MAX_MEDIUM);
	_largeStart = _mediumLargeStart + (MEDIUM_LARGE_SIZE * MAX_EXTRA_LARGE);
	_extraLargeStart = _largeStart + (LARGE_SIZE * MAX_LARGE);
	_allocationArrayStart = _extraLargeStart + (EXTRA_LARGE_SIZE * MAX_EXTRA_LARGE);

	memset(BANK_RAM + _allocationArrayStart, NULL, _allocationArraySize);
}

short getSearchStart(int size)
{
	int searchStart = 0;
	if (size <= TINY_SIZE)
	{
		searchStart = _tinyStart;
	}
	else if (size <= EXTRA_SMALL_SIZE)
	{
		searchStart = _extraSmallStart;
	}
	else if (size <= _smallSmart)
	{
		searchStart = _smallSmart;
	}
	else if (size <= _mediumStart)
	{
		searchStart = _mediumStart;
	}
	else if (size <= _mediumLargeStart)
	{
		searchStart = _mediumLargeStart;
	}
	else if (size <= _largeStart)
	{
		searchStart = _largeStart;
	}
	else if (size <= _extraLargeStart)
	{
		searchStart = _extraLargeStart;
	}
	else {
		printf("Attempting to allocate a string of size %d, this is bigger then allowed", size);
		exit(0);
	}

	return searchStart;
}


byte* alloc(int size)
{
	short i,j;
	short searchStart = getSearchStart(size);
	byte potentialEmptyByte = NULL;

	for (i = searchStart; i < _allocationArraySize * sizeof(byte*); i = i + sizeof(byte*))
	{
		potentialEmptyByte = BANK_RAM[_allocationArrayStart + i];

		if (potentialEmptyByte == NULL)
		{
			byte* memoryLocation = BANK_RAM + i;
			byte** ptrMemoryLocation = &memoryLocation;

			for (j = sizeof(byte**) - 1; j >= 0; j--)
			{
				BANK_RAM[_allocationArrayStart + i + j] = (byte) *(ptrMemoryLocation + j);
			}
		}
	}
}

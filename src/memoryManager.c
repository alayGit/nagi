#include "memoryManager.h"

int _allocationArraySize;
int _tinyStart;
int _extraSmallStart;
int _smallSmart;
int _mediumStart;
int _mediumLargeStart;
int _largeStart;
int _extraLargeStart;
int _allocationArraySize;
int _allocationArrayStart;

#ifdef _MSC_VER //Used for testing under windows
byte* banked;
#define BANKED_RAM banked
#endif 



void memoryMangerInit()
{
#ifdef _MSC_VER
	banked = (byte*) malloc(512000);
#define BANK_RAM banked
#endif // _MSC_VER


	_allocationArraySize = MAX_TINY + MAX_EXTRA_SMALL + MAX_SMALL + MAX_MEDIUM + MAX_MEDIUM_LARGE + MAX_LARGE + MAX_EXTRA_LARGE;
	_tinyStart = MANAGED_START;
	_extraSmallStart = _tinyStart + (TINY_SIZE * MAX_TINY);
	_smallSmart = _extraSmallStart + (EXTRA_SMALL_SIZE * MAX_EXTRA_SMALL);
	_mediumStart = _smallSmart + (SMALL_SIZE * MAX_SMALL);
	_mediumLargeStart = _mediumStart + (MEDIUM_SIZE * MAX_MEDIUM);
	_largeStart = _mediumLargeStart + (MEDIUM_LARGE_SIZE * MAX_MEDIUM_LARGE);
	_extraLargeStart = _largeStart + (LARGE_SIZE * MAX_LARGE);
	_allocationArrayStart = _extraLargeStart + (EXTRA_LARGE_SIZE * MAX_EXTRA_LARGE);

	memset(BANK_RAM + _allocationArrayStart, NULL, _allocationArraySize * sizeof(byte*));
}

int getSearchStart(int size)
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


byte* banked_alloc(int size)
{
	short i,j;
	int searchStart = getSearchStart(size);
	byte potentialEmptyByte = NULL;
	boolean found = FALSE;
	byte* memoryLocation = NULL;
	byte** ptrMemoryLocation;


	for (i = searchStart; i < _allocationArraySize * sizeof(byte*) && !found; i = i + sizeof(byte*))
	{
		potentialEmptyByte = BANK_RAM[_allocationArrayStart + i];

		if (potentialEmptyByte == NULL)
		{
			memoryLocation = BANK_RAM + i;
			ptrMemoryLocation = &memoryLocation;

			for (j = sizeof(byte**) - 1; j >= 0; j--)
			{
				byte* addressByte = ((byte*)ptrMemoryLocation)[j];
				BANK_RAM[_allocationArrayStart + i + j] = (byte)addressByte;
			}

			found = TRUE;
		}
	}
	return memoryLocation;
}

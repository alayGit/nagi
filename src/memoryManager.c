#include "memoryManager.h"

int _allocationArraySize;
int _tinyStart;
int _extraSmallStart;
int _smallSmart;
int _mediumStart;
int _mediumLargeStart;
int _largeStart;
int _extraLargeStart;
int _allocationArrayStart;

#ifdef _MSC_VER //Used for testing under windows
byte* banked;
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

	//printf("memset at %d %d\n", &BANK_RAM[_allocationArrayStart], _allocationArrayStart);
	memset(BANK_RAM + _allocationArrayStart, NULL, _allocationArraySize * sizeof(byte*));
}

int getSearchStart(int size)
{
	int searchStart = 0;
	searchStart = MANAGED_START;
	
	if (size > TINY_SIZE)
	{
		searchStart+= MAX_TINY;
	}
	if (size > EXTRA_SMALL_SIZE)
	{
		searchStart += MAX_EXTRA_SMALL;
	}
	if (size >= SMALL_SIZE)
	{
		searchStart += MAX_SMALL;
	}
	if (size > MEDIUM_SIZE)
	{
		searchStart += MAX_MEDIUM;
	}
	if (size > LARGE_SIZE)
	{
		searchStart+= MAX_LARGE;
	}
	if (size > EXTRA_LARGE_SIZE) {
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
	
	for (i = searchStart * sizeof(byte*); i < _allocationArraySize * sizeof(byte*) && !found; i = i + sizeof(byte*))
	{
		
		potentialEmptyByte = BANK_RAM[_allocationArrayStart + i];

		printf("The byte is %d", potentialEmptyByte);
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

byte* banked_dealloc(byte* ptr)
{
	short i = 0;
	short j = 0;
	boolean found = FALSE;
	byte* potentialDelete = NULL;
	
	for (i = 0; i < _allocationArraySize * sizeof(byte*) && !found; i = i + sizeof(byte*))
	{
		memcpy(&potentialDelete, BANK_RAM + _allocationArrayStart + i, sizeof(byte*));
		
		if (potentialDelete == ptr)
		{
			for (j = 0; j < sizeof(byte*); j++)
			{
				BANK_RAM[_allocationArrayStart + i + j] = NULL;
			}
		}
	}
}

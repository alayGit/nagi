#include "memoryManager.h"
#define VERBOSE
MemoryArea* _memoryAreas;
int _noSegments;


#ifdef _MSC_VER //Used for testing under windows
byte* banked;
#endif 

void initSegments(byte segOrder, byte noBanks, int segmentSize, byte noSegments, byte firstBank)
{
	if (segOrder > 0)
	{
		_memoryAreas[segOrder].start = _memoryAreas[segOrder - 1].start + _memoryAreas[segOrder - 1].noSegments;
		//printf("The address is %p \n", _segments[segOrder].start);
	}
	else {
		_memoryAreas[segOrder].start = &BANK_RAM[0];
	}

	_memoryAreas[segOrder].firstBank = firstBank;
	
	_memoryAreas[segOrder].noBanks = noBanks;
	
	_memoryAreas[segOrder].segmentSize = segmentSize;
	
	_memoryAreas[segOrder].noSegments = noSegments;

	//printf("Segments: banks %p, noBanks %d, segmentSize %d, allocationArray %p, noSegments %d\n", _segments[segOrder].banks, _segments[segOrder].noBanks, _segments[segOrder].segmentSize, _segments[segOrder].allocationArray, _segments[segOrder].noSegments);
}

byte getFirstSegment(byte size)
{
	byte i;
	byte result = 0;

#ifdef  __CX16__
	byte previousRamBank = RAM_BANK;
	
	RAM_BANK = ALLOCATION_BANK;
#endif //  __CX16__

	for (i = 0; i < size; i++)
	{
		result += _memoryAreas[i].noSegments;
	}

	return result;

#ifdef  __CX16__
		RAM_BANK = previousRamBank;
#endif //  __CX16__
}

void initDynamicMemory()
{
#ifdef _MSC_VER
	banked = (byte*)malloc(512000);
#endif // _MSC_VER
#ifdef  __CX16__
	byte previousRamBank = RAM_BANK;
	RAM_BANK = ALLOCATION_BANK;
#endif //  __CX16__

	_noSegments = TINY_NO_SEGMENTS + EXTRA_SMALL_NO_SEGMENTS + SMALL_NO_SEGMENTS + MEDIUM_NO_SEGMENTS + LARGE_NO_SEGMENTS;

	_memoryAreas = malloc(sizeof(MemoryArea) * NO_SIZES);

	initSegments(TINY_SEG_ORDER, TINY_NO_BANKS, TINY_SIZE, TINY_NO_SEGMENTS, TINY_FIRST_BANK);
	initSegments(EXTRA_SMALL_SEG_ORDER, EXTRA_SMALL_NO_BANKS, EXTRA_SMALL_SIZE, EXTRA_SMALL_NO_SEGMENTS, EXTRA_SMALL_FIRST_BANK);
	initSegments(SMALL_SEG_ORDER, SMALL_NO_BANKS, SMALL_SIZE, SMALL_NO_SEGMENTS, SMALL_FIRST_BANK);
	initSegments(MEDIUM_SEG_ORDER, MEDIUM_NO_BANKS, MEDIUM_SIZE, MEDIUM_NO_SEGMENTS, MEDIUM_FIRST_BANK);
	initSegments(LARGE_SEG_ORDER, LARGE_NO_BANKS, LARGE_SIZE, LARGE_NO_SEGMENTS, LARGE_FIRST_BANK);

	memset(_memoryAreas[0].start, 0, _noSegments);

#ifdef  __CX16__
	RAM_BANK = previousRamBank;
#endif //  __CX16__
}

void bankedRamInit()
{
#define FILE_NAME_LENGTH 13
	int i, j = 0;
	FILE* fp;
	char fileName[FILE_NAME_LENGTH];
	byte previousRamBank = RAM_BANK;

	unsigned char fileByte;
	int bankRamSizes[NO_CODE_BANKS] = {
		(int) _BANKRAM01_SIZE__,
		(int) _BANKRAM02_SIZE__,
		(int)_BANKRAM03_SIZE__,
		(int)_BANKRAM04_SIZE__
	};


	for (i = 0; i < NO_CODE_BANKS; i++)
	{
		sprintf(fileName, "agi.cx16.0%d", i + 1);

		RAM_BANK = i + 1;
		if ((fp = fopen(fileName, "rb")) != NULL) {

#ifdef VERBOSE
			printf("Loading file %s\n", fileName);
#endif // VERBOSE
			fgetc(fp);
			fgetc(fp);

			for (j = 0; j < bankRamSizes[i]; j++) {
				fileByte = (byte) fgetc(fp);
						

				BANK_RAM[j] = fileByte;
				
#ifdef VERBOSE
				if (j < 5)
				{
					printf("%d Bank ram [%d] is now %p. The file byte is %p \n", i + 1, j, *(BANK_RAM + j), fileByte);
				}
#endif // VERBOSE
			}
			fclose(fp);
		}
		else {
			printf("Cannot find file");
		}
	}
}

void memoryMangerInit()
{
	initDynamicMemory();
	bankedRamInit();
}



byte* banked_alloc(int size, byte* bank)
{

	byte i, j;
	byte* result = 0;
	byte* allocationByte = 0;
#ifdef  __CX16__
	byte previousRamBank = RAM_BANK;
	RAM_BANK = ALLOCATION_BANK;
#endif //  __CX16__

	for (i = 0; i < NO_SIZES && !result; i++)
	{
		if (size <= _memoryAreas[i].segmentSize)
		{
			for (j = 0; j < _memoryAreas[i].noSegments && !result; j++)
			{
				allocationByte = _memoryAreas[i].start + j;

				if (!*(allocationByte))
				{
					*allocationByte = TRUE;
					*bank = (byte) ((j * _memoryAreas[i].segmentSize) / BANK_SIZE + _memoryAreas[i].firstBank);
					result = (_memoryAreas[i].segmentSize * j) % BANK_SIZE + &BANK_RAM[0];
				}
			}
		}
	}

#ifdef  __CX16__
	RAM_BANK = previousRamBank;
#endif //  __CX16__

	return result;
}

boolean banked_dealloc(byte* ptr, byte bank)
{
	byte i;
	byte size = 0;
	byte segment = 0;
	byte result = FALSE;

#ifdef  __CX16__
	byte previousRamBank = RAM_BANK;
	RAM_BANK = ALLOCATION_BANK;
#endif //  __CX16__
	for (i = NO_SIZES - 1; i >= 0 && !size; i--)
	{
		if (bank >= _memoryAreas[i].firstBank) {
			size = i;
		}
	}

	segment = ((ptr - &BANK_RAM[0]) / _memoryAreas[i].segmentSize);

	if (BANK_RAM[segment])
	{
		BANK_RAM[segment] = FALSE;

		result = TRUE;
	}

#ifdef  __CX16__
	RAM_BANK = previousRamBank;
#endif //  __CX16__	
	return result;
}

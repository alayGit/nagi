#include "memoryManager.h"

Segment* _segments;



#ifdef _MSC_VER //Used for testing under windows
byte* banked;
#endif 

void initSegments(byte segOrder, byte noBanks, int segmentSize, byte noSegments, byte firstBank)
{
	if (segOrder > 0)
	{
		_segments[segOrder].start = _segments[segOrder - 1].start + _segments[segOrder - 1].noSegments;
		//printf("The address is %p \n", _segments[segOrder].start);
	}
	else {
		_segments[segOrder].start = &BANK_RAM[0];
	}

	_segments[segOrder].firstBank = firstBank;
	
	_segments[segOrder].noBanks = noBanks;
	
	_segments[segOrder].segmentSize = segmentSize;
	_segments[segOrder].allocationArray = malloc(noSegments);
	memset(_segments[segOrder].allocationArray, FALSE, noSegments);
	
	_segments[segOrder].noSegments = noSegments;

	//printf("Segments: banks %p, noBanks %d, segmentSize %d, allocationArray %p, noSegments %d\n", _segments[segOrder].banks, _segments[segOrder].noBanks, _segments[segOrder].segmentSize, _segments[segOrder].allocationArray, _segments[segOrder].noSegments);
}

void memoryMangerInit()
{
#ifdef _MSC_VER
	banked = (byte*) malloc(512000);
#define BANK_RAM banked
#endif // _MSC_VER


	_segments = malloc(sizeof(Segment) * NO_SIZES);

	initSegments(TINY_SEG_ORDER, TINY_NO_BANKS, TINY_SIZE, TINY_NO_SEGMENTS, TINY_FIRST_BANK);
	initSegments(EXTRA_SMALL_SEG_ORDER, EXTRA_SMALL_NO_BANKS, EXTRA_SMALL_SIZE, EXTRA_SMALL_NO_SEGMENTS, EXTRA_SMALL_FIRST_BANK);
	initSegments(SMALL_SEG_ORDER, SMALL_NO_BANKS, SMALL_SIZE, SMALL_NO_SEGMENTS, SMALL_FIRST_BANK);
	initSegments(MEDIUM_SEG_ORDER, MEDIUM_NO_BANKS, MEDIUM_SIZE, MEDIUM_NO_SEGMENTS, MEDIUM_FIRST_BANK);
	initSegments(LARGE_SEG_ORDER, LARGE_NO_BANKS, LARGE_SIZE, LARGE_NO_SEGMENTS, LARGE_FIRST_BANK);
}

byte* banked_alloc(int size, byte* bank)
{
	byte i,j;
	byte* result = 0;
	for (i = 0; i < NO_SIZES && !result; i++)
	{
		if (size <= _segments[i].segmentSize)
		{
			for (j = 0; j < _segments[i].noSegments && !result; j++)
			{
				if (!_segments[i].allocationArray[j])
				{
					_segments[i].allocationArray[j] == TRUE;
					*bank = (byte) (j * _segments[i].segmentSize) / 8000 + _segments[i].firstBank;
					result = _segments[i].segmentSize * j + &BANK_RAM[0];
				}
			}
		}
	}

	return result;
}

byte* banked_dealloc(byte* ptr)
{
	
}

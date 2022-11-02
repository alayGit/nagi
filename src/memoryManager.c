#include "memoryManager.h"

Segment* _segments;

byte _tinyBanks[] = {0};
byte _extraSmallBanks[] = {1};
byte _smallBanks[] = {3,4,5,6,7,8,9};
byte _mediumBanks[] = { 10,11,12,13,14,15,16,17,19 };
byte _largeBanks[] = { 19,20,21,22,23,24 };


#ifdef _MSC_VER //Used for testing under windows
byte* banked;
#endif 

void initSegments(byte segOrder, byte noBanks, int segmentSize, byte noSegments, byte* banks)
{
	_segments[segOrder].banks = malloc(noBanks);
	memcpy(_segments[segOrder].banks, banks, noBanks);
	
	_segments[segOrder].noBanks = noBanks;
	
	_segments[segOrder].segmentSize = segmentSize;
	_segments[segOrder].allocationArray = malloc(noSegments);
	memset(_segments[segOrder].allocationArray, FALSE, noSegments);
	
	_segments[segOrder].noSegments = noSegments;

	printf("Segments: banks %p, noBanks %d, segmentSize %d, allocationArray %p, noSegments %d\n", _segments[segOrder].banks, _segments[segOrder].noBanks, _segments[segOrder].segmentSize, _segments[segOrder].allocationArray, _segments[segOrder].noSegments);
}

void memoryMangerInit()
{
#ifdef _MSC_VER
	banked = (byte*) malloc(512000);
#define BANK_RAM banked
#endif // _MSC_VER


	_segments = malloc(sizeof(Segment) * NO_SIZES);

	initSegments(TINY_SEG_ORDER, TINY_NO_BANKS, TINY_SIZE, TINY_NO_SEGMENTS, &_tinyBanks[0]);
	initSegments(EXTRA_SMALL_SEG_ORDER, EXTRA_SMALL_NO_BANKS, EXTRA_SMALL_SIZE, EXTRA_SMALL_NO_SEGMENTS, &_extraSmallBanks[0]);
	initSegments(SMALL_SEG_ORDER, SMALL_NO_BANKS, SMALL_SIZE, SMALL_NO_SEGMENTS, &_smallBanks[0]);
	initSegments(MEDIUM_SEG_ORDER, MEDIUM_NO_BANKS, MEDIUM_SIZE, MEDIUM_NO_SEGMENTS, &_mediumBanks[0]);
	initSegments(LARGE_SEG_ORDER, LARGE_NO_BANKS, LARGE_SIZE, LARGE_NO_SEGMENTS, &_largeBanks[0]);
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
					*bank = (byte) (j * _segments[i].segmentSize) / 8000 + _segments[i].banks[0];
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

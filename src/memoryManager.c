#include "memoryManager.h"

Segment* _segments;

#ifdef _MSC_VER //Used for testing under windows
byte* banked;
#endif 

void initSegments(byte segOrder, byte noBanks, int segmentSize, byte noSegments)
{
	_segments[segOrder].banks = malloc(noBanks);
	_segments[segOrder].noBanks = noBanks;
	_segments[segOrder].segmentSize = segmentSize;
	_segments[segOrder].allocationArray = malloc(noSegments);
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

	initSegments(TINY_SEG_ORDER, TINY_NO_BANKS, TINY_SIZE, TINY_NO_SEGMENTS);
	initSegments(EXTRA_SMALL_SEG_ORDER, EXTRA_SMALL_NO_BANKS, EXTRA_SMALL_SIZE, EXTRA_SMALL_NO_SEGMENTS);
	initSegments(SMALL_SEG_ORDER, SMALL_NO_BANKS, SMALL_SIZE, SMALL_NO_SEGMENTS);
	initSegments(MEDIUM_SEG_ORDER, MEDIUM_NO_BANKS, MEDIUM_SIZE, MEDIUM_NO_SEGMENTS);
	initSegments(LARGE_SEG_ORDER, LARGE_NO_BANKS, LARGE_SIZE, LARGE_NO_SEGMENTS);
}

byte* banked_alloc(int size)
{
	
}

byte* banked_dealloc(byte* ptr)
{
	
}

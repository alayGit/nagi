#ifndef _MEMORYMANAGER_H_
#define _MEMORYMANAGER_H_

#include "general.h"

#ifndef _MSC_VER
#include <cx16.h>
#endif // !_MSC_VER

#define TINY_SEG_ORDER  0
#define EXTRA_SMALL_SEG_ORDER  1
#define SMALL_SEG_ORDER  2
#define MEDIUM_SEG_ORDER  3
#define LARGE_SEG_ORDER  4

#define TINY_NO_SEGMENTS  80
#define EXTRA_SMALL_NO_SEGMENTS 50
#define SMALL_NO_SEGMENTS 30
#define MEDIUM_NO_SEGMENTS 20
#define LARGE_NO_SEGMENTS 5

#define TINY_SIZE  100
#define EXTRA_SMALL_SIZE 160
#define SMALL_SIZE 1500
#define MEDIUM_SIZE 3200
#define LARGE_SIZE 8000

#define TINY_NO_BANKS  1
#define EXTRA_SMALL_NO_BANKS 1
#define SMALL_NO_BANKS 7
#define MEDIUM_NO_BANKS 9
#define LARGE_NO_BANKS 6

#define TINY_FIRST_BANK  0
#define EXTRA_SMALL_FIRST_BANK 1
#define SMALL_FIRST_BANK 3
#define MEDIUM_FIRST_BANK 10
#define LARGE_FIRST_BANK 19

#define NO_SIZES 5


typedef struct {          /* DIR entry structure */
	byte firstBank;
	byte noBanks;
	int segmentSize;
	boolean* allocationArray;
	byte noSegments;
	byte* start;
} Segment;

void memoryMangerInit();
byte* banked_alloc(int size, byte* bank);
byte* banked_dealloc(byte* ptr);
void initSegments(byte segOrder, byte noBanks, int segmentSize, byte noSegments, byte firstBank);

#endif


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

#define TINY_NO_SEGMENTS  50
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
#define EXTRA_SMALL_NO_BANKS 2
#define SMALL_NO_BANKS 6
#define MEDIUM_NO_BANKS 8
#define LARGE_NO_BANKS 5

#define NO_SIZES 5

typedef struct {          /* DIR entry structure */
	byte* banks;
	byte noBanks;
	int segmentSize;
	byte* allocationArray;
	byte noSegments;
} Segment;

void memoryMangerInit();
byte* banked_alloc(int size);
byte* banked_dealloc(byte* ptr);
void initSegments(byte segOrder, byte noBanks, int segmentSize, byte noSegments);

#endif


#include "general.h"

#ifndef _MSC_VER
#include <cx16.h>
#endif // !_MSC_VER



#define MANAGED_START 0

#define MAX_TINY  50
#define MAX_EXTRA_SMALL 50
#define MAX_SMALL 30
#define MAX_MEDIUM 20
#define MAX_MEDIUM_LARGE 5
#define MAX_LARGE 3
#define MAX_EXTRA_LARGE 3

#define TINY_SIZE  100
#define EXTRA_SMALL_SIZE 150
#define SMALL_SIZE 1500
#define MEDIUM_SIZE 2800
#define MEDIUM_LARGE_SIZE 5000
#define LARGE_SIZE 15000
#define EXTRA_LARGE_SIZE 32000

void memoryMangerInit();
byte* alloc(int size);
int getSearchStart(int size);


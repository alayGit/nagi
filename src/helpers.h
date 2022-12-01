#ifndef _HELPERS_H_
#define _HELPERS_H_
#include "general.h"
#include "memoryManager.h"
#include <stdarg.h>
#include <cx16.h>
#include <string.h>

#define ASCIIDASH 95
#define ASCIIA 65
#define ASCIIZ 90
#define ASCIIa 97
#define ASCIIz 122

#define PETSCIIA 193
#define PETSCIIZ 218
#define PETSCIIa 65
#define PETSCIIz 90
#define PETSCIISpace 32
#define	PETSCIIPercent 37
#define PETSCIIDash 228

#define DIFF_ASCII_PETSCII_CAPS -128
#define DIFF_ASCII_PETSCII_LOWER -32

typedef void (*fnTrampoline_1)(void* data);
typedef byte (*fnTrampoline_1b)(void* data);

byte convertAsciiByteToPetsciiByte(byte* toConvert);

extern void trampoline_1(fnTrampoline_1 func, void* data, byte bank);
extern byte trampoline_1b(fnTrampoline_1b func, void* data, byte bank);

extern char* strcpyBanked(char* dest, const char* src, byte bank);

extern void copyStringFromBanked(char* src, char* dest, int start, int chunk, byte sourceBank);

extern int sprintfBanked(const char* buffer, byte bank, char const* const format, ...);

extern void setLogicDirectory(AGIFilePosType* newLogicDirectory, AGIFilePosType* logicDirectoryLocation);

#endif

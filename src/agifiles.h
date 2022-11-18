/**************************************************************************
** AGIFILES.H
**************************************************************************/

#ifndef _AGIFILES_H_
#define _AGIFILES_H_

#include <string.h>
#include <cx16.h>
#include "memoryManager.h"
#include "helpers.h"

#define  LOGIC    0
#define  PICTURE  1
#define  VIEW     2
#define  SOUND    3

#define  EMPTY  0xFFFFF   /* Empty DIR entry */

typedef struct {          /* DIR entry structure */
   char *fileName;
   unsigned long filePos;
} AGIFilePosType;

typedef struct {          /* AGI data file structure */
   unsigned int totalSize;
   unsigned int codeSize;
   byte* code;
   byte codeBank;
   byte messageBank;
   unsigned int noMessages;
   byte** messagePointers;
   byte* messageData;
} AGIFile;

extern AGIFilePosType logdir[256], picdir[256], viewdir[256], snddir[256];
extern int numLogics, numPictures, numViews, numSounds;

void initFiles();
void loadAGIDirs();
void loadAGIFile(int resType, AGIFilePosType* location, AGIFile *AGIData);

#endif  /* _AGIFILES_H_ */

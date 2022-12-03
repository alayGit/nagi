/**************************************************************************
** LOGIC.H
**************************************************************************/

#ifndef _LOGIC_H_
#define _LOGIC_H_

#include <stdlib.h>
#include "helpers.h"

typedef struct {
	word codeSize;
	byte* logicCode;
	byte numMessages;
	byte** messages;
	byte codeBank;
	byte messageBank;

} LOGICFile;

typedef struct {
	boolean loaded;
	word entryPoint;
	word currentPoint;
	LOGICFile* data;
	byte dataBank;
} LOGICEntry;

extern LOGICEntry* logics;

void initLogics();
void loadLogicFile(byte logFileNum);
void discardLogicFile(byte logFileNum);

extern void getLogicDirectory(AGIFilePosType* returnedLogicDirectory, AGIFilePosType* logicDirectoryLocation);
extern void getLogicFile(LOGICFile* logicFile, byte logicFileNo);
extern void getLogicEntry(LOGICEntry* logicEntry, byte logicFileNo);
extern void setLogicEntry(LOGICEntry* logicEntry, byte logicFileNo);


#endif  /* _LOGIC_H_ */

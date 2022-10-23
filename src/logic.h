/**************************************************************************
** LOGIC.H
**************************************************************************/

#ifndef _LOGIC_H_
#define _LOGIC_H_

#include <stdlib.h>


typedef struct {
   word codeSize;
   byte *logicCode;
   byte numMessages;
   byte **messages;
} LOGICFile;

typedef struct {
   boolean loaded;
   word entryPoint;
   word currentPoint;
   LOGICFile *data;
} LOGICEntry;

extern LOGICEntry logics[];

void initLogics();
void loadLogicFile(int logFileNum);
void discardLogicFile(int logFileNum);


#endif  /* _LOGIC_H_ */

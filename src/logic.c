/**************************************************************************
** LOGIC.C
**
** These functions are used to load LOGIC files into a structure of type
** LOGICFile. It uses the general function LoadAGIFile to load the data
** from the VOL file itself.
**
** (c) 1997 Lance Ewing - Original code (2 July 97)
**                        Changed (26 Aug 97)
**************************************************************************/
#define VERBOSE
#include <string.h>

#include "agifiles.h"
#include "general.h"
#include "logic.h"
#include "memoryManager.h"

/* The logics array is the array that holds all the information about the
** logic files. A boolean flag determines whether the logic is loaded or
** not. If it isn't loaded, then the data is not in memory. */
LOGICEntry* logics = (LOGICEntry*)&BANK_RAM[LOGIC_ENTRY_START];

/***************************************************************************
** initLogics
**
** Purpose: To initialize the array that holds information on the loaded
** logic files and also to load LOGIC.0 into that array. All other logics
** are marked as unloaded. Make sure you call initFiles() before calling
** this function.
***************************************************************************/
void initLogics()
{
	int i;

	for (i = 0; i < 256; i++) {
		logics[i].loaded = FALSE;
		logics[i].entryPoint = 0;
		logics[i].currentPoint = 0;
		logics[i].data = NULL;
	}

	loadLogicFile(0);
}


/**************************************************************************
** loadLogicFile
**
** Purpose: To load a LOGIC file, decode the messages, and store in a
** suitable structure.
**************************************************************************/
void loadLogicFile(int logFileNum)
{
	AGIFile tempAGI;
	AGIFilePosType agiFilePosType;
	LOGICEntry logicEntry;
	LOGICFile* logicData;

	int dummy;
	byte previousRamBank = RAM_BANK;

	RAM_BANK = DIRECTORY_BANK;
	agiFilePosType = logdir[logFileNum];
	RAM_BANK = previousRamBank;

#ifdef VERBOSE
	printf("\n%d Retrieved file num %d, Offset %lu\n", logFileNum, agiFilePosType.filePos);
#endif // VERBOSE

	discardLogicFile(logFileNum);
	/* Load LOGIC file, calculate logic code length, and copy
	** logic code into tempLOGIC. */


#ifdef VERBOSE
	printf("Loading Logic %d\n", logFileNum);
#endif // VERBOSE

	loadAGIFile(LOGIC, &agiFilePosType, &tempAGI);

	RAM_BANK = LOGIC_FILE_BANK;
	logicData = &((LOGICFile*)&BANK_RAM[LOGIC_FILE_BANK])[logFileNum];
	logicData->codeBank = tempAGI.codeBank;
	logicData->codeSize = tempAGI.codeSize;
	logicData->logicCode = tempAGI.code;
	logicData->messageBank = tempAGI.messageBank;
	logicData->messages = (byte**)tempAGI.messagePointers;
	logicData->numMessages = tempAGI.noMessages;

#ifdef VERBOSE
	printf("The codebank is %d, the code size is %d, the messageBank is %d, \n and the number of messages is %d, the code pointer is non zero and matched against temp agi %d the message pointer is non zero and matches temp agi %d \n",
		logicData->codeBank, logicData->codeSize, logicData->messageBank, logicData->numMessages
		, logicData->logicCode == tempAGI.code && logicData->logicCode
		, logicData->messages == (byte**)tempAGI.messagePointers && logicData->messages
	);
#endif // VERBOSE
	RAM_BANK = LOGIC_ENTRY_BANK;

	logics[logFileNum].data = logicData;
	logicEntry = logics[logFileNum];

	logics[logFileNum].loaded = TRUE;

	RAM_BANK = previousRamBank;
}

/**************************************************************************
** discardLogicFile
**
** Purpose: To deallocate all the memory associated with a LOGICFile
** struct. This function can only be used with dynamically allocated
** LOGICFile structs which is what I plan to use.
**************************************************************************/
void discardLogicFile(int logFileNum)
{
	byte previousRamBank = RAM_BANK;
	int messNum;
	LOGICFile* logicData;

	RAM_BANK = LOGIC_ENTRY_BANK;
	if (logics[logFileNum].loaded) {
		logicData = logics[logFileNum].data;
		if (logics[logFileNum].loaded && !banked_dealloc((byte*)logicData->messages, logicData->messageBank))
		{
			printf("Failed to deallocate messages for logic %d in bank %d", logFileNum, logicData->codeBank);
		}

		if (logics[logFileNum].loaded && !banked_dealloc((byte*)logicData->logicCode, logicData->codeBank))
		{
			printf("Failed to deallocate messages for logic %d in bank %d", logFileNum, logicData->codeBank);
		}

		logics[logFileNum].loaded = FALSE;
	}
	RAM_BANK = previousRamBank;
}

void testLogic()
{
	initFiles();
	initLogics();
	loadLogicFile(0);
	discardLogicFile(0);
}

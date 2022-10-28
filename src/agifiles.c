/***************************************************************************
** AGIFILES.C
**
** Routines to handle AGI file system. These functions should enable you
** to load individual LOGIC, VIEW, PIC, and SOUND files into memory. The
** data is stored in a structure of type AGIFile. There is no code that
** is specific to the above types of data file though.
**
** (c) 1997 Lance Ewing
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <cbm.h>
#include <dbg.h>

#include "general.h"
#include "agifiles.h"
#include "decomp.h"

#define  AVIS_DURGAN  "Avis Durgan" //https://www.liquisearch.com/what_is_avis_durgan
#define FILE_OPEN_ADDRESS 2

AGIFilePosType logdir[256], picdir[256], viewdir[256], snddir[256];
int numLogics, numPictures, numViews, numSounds;
boolean version3 = FALSE;
char gameSig[10] = "";

/***************************************************************************
** initFiles
**
** Purpose: To load the AGI resource directories and to determine whether
** the AGI files are for an AGIv3 game or for an AGIv2 game. It will also
** initialize the game signature in the case of a version 3 game.
***************************************************************************/
void initFiles()
{
	loadGameSig(gameSig);
	if (strlen(gameSig) > 0) version3 = TRUE;
	loadAGIDirs();
}

byte cbm_openForSeeking(const char* fileName)
{
	int i;
	char s[200];
	const char* OPEN_FLAGS = ",S,R";

	byte lfn = SEQUENTIAL_LFN;
	byte dev = 8;
	byte sec_addr = FILE_OPEN_ADDRESS;

	char* fileNameAndFlags = malloc(strlen(fileName) + strlen(OPEN_FLAGS) + 1);
	sprintf(fileNameAndFlags, "%s%s", fileName, OPEN_FLAGS);

	cbm_open(lfn, dev, sec_addr, fileNameAndFlags);

	free(fileNameAndFlags);

	return lfn;
}

byte cbm_getSeekedByte()
{
	cbm_k_chkin(2);

	return cbm_k_chrin();
}

int8_t cx16_fseek(uint8_t channel, uint32_t offset) {
	int8_t result = 0, status = 0, chkin = 0;
	int i;
#define SETNAM 0xFFBD
	static struct cmd {
		char p;
		uint8_t lfn;
		uint32_t offset;
	} cmd;

	printf("Attempting to seek at channel %d offset %lu\n", channel, offset);

	// open command channel to DOS and send P command.
	// P u8 u32 (no spaces) u8 is LFN to seek(), and u32 = offset.
	cmd.p = 'p';
	cmd.lfn = channel;
	cmd.offset = offset;
	// can't call cbm_open because the P command is binary and may
	// contain zeros, which C would interpret as null terminator.
	//
	// Roll-your-own inline asm call to SETNAM:
	__asm__("lda #6");
	__asm__("ldx #<%v", cmd);
	__asm__("ldy #>%v", cmd);
	__asm__("jsr %w", SETNAM);
	cbm_k_setlfs(15, 8, 15);
	cbm_k_open(); // this sends the CMD bytes..

	cbm_k_close(15); // close the command channel

	return 0;
	// TODO: ERROR HANDLING!!!!!
}

/***************************************************************************
** loadAGIDir
**
** Purpose: To read in an individual AGI directory file. This function
** should only be called by loadAGIDirs() below.
***************************************************************************/
void loadAGIDir(int dirNum, char* fName, int* count)
{
	FILE* fp;
	unsigned char byte1, byte2, byte3;
	unsigned char address[4] = { 0,0,0,0 };
	AGIFilePosType tempPos;
	int value;


	if ((fp = fopen(fName, "rb")) == NULL) {
		printf("Could not find file : %s.\n", fName);
		printf("Make sure you are in an AGI version 2 game directory.\n");
		exit(0);
	}

	while (!feof(fp)) {
		byte1 = fgetc(fp);
		byte2 = fgetc(fp);
		byte3 = fgetc(fp);

		tempPos.fileName = (char*)malloc(10);
		sprintf(tempPos.fileName, "vol.%d", ((byte1 & 0xF0) >> 4));

		address[3] = 0;
		address[2] = byte1 & 0x0F;
		address[1] = byte2;
		address[0] = byte3;


		memcpy(&tempPos.filePos, &address[0], 4);

		value = tempPos.filePos;

		switch (dirNum) {
		case 0: logdir[*count] = tempPos; break;
		case 1: picdir[*count] = tempPos; break;
		case 2: viewdir[*count] = tempPos; break;
		case 3: snddir[*count] = tempPos; break;
		}

		(*count)++;
	}

	fclose(fp);
}

/***************************************************************************
** loadAGIv3Dir
***************************************************************************/
void loadAGIv3Dir()
{
	//FILE *dirFile;
	//unsigned char dirName[15], *marker, *dirData, *endPos, *dataPos;
	//int resType=0, resNum=0, dirLength;
	//AGIFilePosType tempPos;

	//sprintf(dirName, "%sDIR", gameSig);
	//if ((dirFile = fopen(dirName, "rb")) == NULL) {
	//   printf("File not found : %s\n", dirName);
	//   exit(1);
	//}

	////fseek(dirFile, 0, SEEK_END);
	//dirLength = //ftell(dirFile);
	////fseek(dirFile, 0, SEEK_SET);
	//dirData = (char *)malloc(sizeof(char)*dirLength);
	//fread(dirData, sizeof(char), dirLength, dirFile);
	//fclose(dirFile);
	//marker = dirData;

	//for (resType=0, marker=dirData; resType<4; resType++, marker+=2) {
	//   dataPos = dirData + (*marker + *(marker+1)*256);
	//   endPos = ((resType<3)? (dirData + (*(marker+2) + *(marker+3)*256))
	//      :(dirData+dirLength));
	//   resNum = 0;
	//   for (; dataPos<endPos; dataPos+=3, resNum++) {
	//      tempPos.fileName = (char *)malloc(10);
	//      sprintf(tempPos.fileName, "%sVOL.%d", gameSig,
	//         ((dataPos[0] & 0xF0) >> 4));
	//      tempPos.filePos = ((long)((dataPos[0] & 0x0F) << 16) +
	//                         (long)((dataPos[1] & 0xFF) << 8) +
	//                         (long)(dataPos[2] & 0xFF));

	//      switch (resType) {
	//         case 0: logdir[resNum] = tempPos; break;
	//         case 1: picdir[resNum] = tempPos; break;
	//         case 2: viewdir[resNum] = tempPos; break;
	//         case 3: snddir[resNum] = tempPos; break;
	//      }
	//   }
	//   if (resNum > 256) {
	//      printf("Error loading directory file.\n");
	//      printf("Too many resources.\n");
	//      exit(1);
	//   }

	//   switch (resType) {
	//      case 0: numLogics = resNum; break;
	//      case 1: numPictures = resNum; break;
	//      case 2: numViews = resNum; break;
	//      case 3: numSounds = resNum; break;
	//   }
	//}

	//free(dirData);
}

/***************************************************************************
** loadAGIDirs
**
** Purpose: To read the AGI directory files LOGDIR, PICDIR, VIEWDIR, and
** SNDDIR and store the information in a usable format. This function must
** be called once at the start of the interpreter.
***************************************************************************/
void loadAGIDirs()
{
	numLogics = numPictures = numViews = numSounds = 0;
	if (version3) {
		loadAGIv3Dir();
	}
	else {
		loadAGIDir(0, "logdir", &numLogics);
		loadAGIDir(1, "picdir", &numPictures);
		loadAGIDir(2, "viewdir", &numViews);
		loadAGIDir(3, "snddir", &numSounds);
		printf("Index Load Complete\n");
	}
}

#define  NORMAL     0
#define  ALTERNATE  1

/**************************************************************************
** convertPic
**
** Purpose: To convert an AGIv3 resource to the AGIv2 format.
**************************************************************************/
void convertPic(unsigned char* input, unsigned char* output, int dataLen)
{
	unsigned char data, oldData, outData;
	int mode = NORMAL, i = 0;

	while (i++ < dataLen) {
		data = *input++;

		if (mode == ALTERNATE)
			outData = ((data & 0xF0) >> 4) + ((oldData & 0x0F) << 4);
		else
			outData = data;

		if ((outData == 0xF0) || (outData == 0xF2)) {
			*output++ = outData;
			if (mode == NORMAL) {
				data = *input++;
				*output++ = ((data & 0xF0) >> 4);
				mode = ALTERNATE;
			}
			else {
				*output++ = (data & 0x0F);
				mode = NORMAL;
			}
		}
		else
			*output++ = outData;

		oldData = data;
	}
}

/**************************************************************************
** loadAGIFile
**
** Purpose: To read an AGI data file out of a VOL file and store the data
** and data size into the AGIFile structure whose pointer is passed in.
** This function handles AGIv2 and AGIv3. It makes sure that the resources
** are converted to a common format to deal with the differences between
** the two versions. The conversions needed are as follows:
**
**  - All encrypted LOGIC message sections are decrypted. This includes
**    uncompressed AGIv3 LOGICs as well as AGIv2 LOGICs.
**  - AGIv3 PICTUREs are decompressed.
**
** In both cases the format that is easier to deal with is returned.
**************************************************************************/
void loadAGIFile(int resType, AGIFilePosType* location, AGIFile* AGIData)
{
	short compSize, startPos, endPos, numMess, avisPos = 0, i;
	unsigned char byte1, byte2, volNum, * compBuf, * fileData;
	byte actualSig1, actualSig2;
	byte lfn;
	byte currentByte;
	boolean signatureValidationPassed;

	const byte EXPECT_SIG_1 = 0x12;
	const byte EXPECTED_SIG_2 = 0x34;

	printf("----Attempting to open %s for seeking data\n", location->fileName);

	if (location->filePos == EMPTY) {
		printf("Could not find requested AGI file.\n");
		printf("This could indicate problems with your game data files\n");
		printf("or there may be something wrong with MEKA.\n");
		exit(0);
	}

	lfn = cbm_openForSeeking(location->fileName);

	cx16_fseek(FILE_OPEN_ADDRESS, location->filePos);

	cbm_read(SEQUENTIAL_LFN, &currentByte, 1);
	signatureValidationPassed = currentByte == 0x12;

	cbm_read(SEQUENTIAL_LFN, &currentByte, 1);
	signatureValidationPassed = signatureValidationPassed & currentByte == 0x34;

	if (!signatureValidationPassed) {  /* All AGI data files start with 0x1234 */
		printf("Data error reading %s.\n", location->fileName);
		printf("The requested AGI file did not have a signature.\n");
		printf("Check if your game files are corrupt.\n");
		exit(0);
	}

	printf("Passed Signature Validation\n");

	cbm_read(SEQUENTIAL_LFN, &currentByte, 1);
	volNum = currentByte;

	cbm_read(SEQUENTIAL_LFN, &currentByte, 1);
	byte1 = currentByte;

	cbm_read(SEQUENTIAL_LFN, &currentByte, 1);
	byte2 = currentByte;

	AGIData->size = (unsigned int)(byte1)+(unsigned int)(byte2 << 8);
	AGIData->data = BANK_RAM;
	//memset(AGIData->data, 0, AGIData->size);

	printf("AGIData data is %p\n and the size is %d\n", AGIData->data, AGIData->size);

	printf("volNum:%d byte1:%p, byte2:%p, size:%d\n", volNum, byte1, byte2, (unsigned int)(byte1)+(unsigned int)(byte2 << 8));

	printf("Attempting to read data of size %d\n", AGIData->size);

	//printf("The address of banked ram is %p and it holds %p", BANK_RAM, *BANK_RAM);
	
	cbm_read(SEQUENTIAL_LFN, AGIData->data, AGIData->size);
	printf("The address of agi data is %p and it holds %p\n", AGIData->data, *AGIData->data);
	printf("The address of banked ram is %p and it holds %p\n", BANK_RAM, *BANK_RAM);

	printf("Size of byte pointer %d", sizeof(byte*));

	for (i = 0; i < 1000000; i++);

	exit(0);

	if (resType == LOGIC) {
		/* Decrypt message section */
		fileData = AGIData->data;
		startPos = *fileData + (*(fileData + 1)) * 256 + 2;
		numMess = fileData[startPos];
		endPos = fileData[startPos + 1] + fileData[startPos + 2] * 256;
		fileData += (startPos + 3);
		startPos = (numMess * 2) + 0;

		for (i = startPos; i < endPos; i++)
			fileData[i] ^= AVIS_DURGAN[avisPos++ % 11];
	}



			//if (version3) {
			//   byte1 = fgetc(fp);
			//   byte2 = fgetc(fp);
			//   compSize = (unsigned int)(byte1) + (unsigned int)(byte2 << 8);
			//   compBuf = (unsigned char *)malloc((compSize+10)*sizeof(char));
			//   fread(compBuf, sizeof(char), compSize, fp);

			//   //initLZW();

			//   if (volNum & 0x80) {
			//      convertPic(compBuf, AGIData->data, compSize);
			//   }
			//   else if (AGIData->size == compSize) {  /* Not compressed */
			//      memcpy(AGIData->data, compBuf, compSize);

			//      if (resType == LOGIC) {
			//         /* Uncompressed AGIv3 logic files have their message sections
			//            encrypted, so we decrypt it here */
			//         fileData = AGIData->data;
			//         startPos = *fileData + (*(fileData+1))*256 + 2;
			//         numMess = fileData[startPos];
			//         endPos = fileData[startPos+1] + fileData[startPos+2]*256;
			//         fileData += (startPos + 3);
			//         startPos = (numMess * 2) + 0;

			//         for (i=startPos; i<endPos; i++)
			   //          fileData[i] ^= AVIS_DURGAN[avisPos++ % 11];
			//      }

			//      free(compBuf);
			//   }
			//   else {
			//      expand(compBuf, AGIData->data, AGIData->size);
			//   }

			//   free(compBuf);
			//   //closeLZW();
			//}
			//else {
			//   fread(AGIData->data, AGIData->size, 1, fp);
			//   if (resType == LOGIC) {
			//      /* Decrypt message section */
			//      fileData = AGIData->data;
			//      startPos = *fileData + (*(fileData+1))*256 + 2;
			//      numMess = fileData[startPos];
			//      endPos = fileData[startPos+1] + fileData[startPos+2]*256;
			//      fileData += (startPos + 3);
			//      startPos = (numMess * 2) + 0;

			//      for (i=startPos; i<endPos; i++)
			   //       fileData[i] ^= AVIS_DURGAN[avisPos++ % 11];
			//   }
			//}

			cbm_close(lfn);
	}

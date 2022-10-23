#ifndef _STUB_H
#define _STUB_H
#include "general.h"

//Picture
void discardPictureFile(int picFileNum);
void initPicture();
void initPictures();
void closePicture();

//Sound
void discardSoundFile(int soundNum);
void initSound();

//View
typedef struct {
	byte width;
	byte height;
	byte transparency;
	BITMAP* bmp;
} Cel;

void initViews();
void initObjects();
void resetViews();
void discardObjects();

typedef struct {
	byte numberOfCels;
	Cel* cels;
} Loop;

typedef struct {
	boolean loaded;
	byte numberOfLoops;
	Loop* loops;
	char* description;
} View;

typedef struct {
	byte stepTime;
	byte stepTimeCount;
	word xPos;
	word yPos;
	byte currentView;
	View* viewData;             /* This pointer points to the loaded view */
	byte currentLoop;
	byte numberOfLoops;
	Loop* loopData;             /* ditto */
	byte currentCel;
	byte numberOfCels;
	Cel* celData;              /* ditto */
	BITMAP* bgPic;             /* Storage for background behind drawn view */
	BITMAP* bgPri;
	word bgX;                  /* Position to place background bmp */
	word bgY;
	word xsize;
	word ysize;
	byte stepSize;
	byte cycleTime;
	byte cycleTimeCount;
	byte direction;
	byte motion;
	byte cycleStatus;
	byte priority;
	word flags;
	byte param1;
	byte param2;
	byte param3;
	byte param4;
} ViewTable;


extern ViewTable viewtab[];

void discardView(byte viewNum);

void calcObjMotion();

//Logic 
extern byte directions[9];

//allgero
extern BITMAP* screen;
void clear(BITMAP* bitmap);
void rectfill(BITMAP* bmp, int x1, int y1, int x2, int y2, int color);

//Picture
extern boolean statusLineDisplayed, inputLineDisplayed;
void initAGIScreen();
void initPalette();


//Graphics
void drawBigString(BITMAP* scn, char* data, int x, int y, int foreColour, int backColour);
#endif

//Parserf
void pollKeyboard();
void initEvents();

//Commander
void executeLogic(int logNum);
void freeMenuItems();

//Object
void loadObjectFile();

//Words
void loadWords();
void discardWords();

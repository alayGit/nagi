#include "stub.h"

//A temporary file so that references to yet to be imported files can resolved

//Allegro
void clear_to_color(BITMAP* bitmap, int color)
{
}

void stretch_blit(BITMAP* s, BITMAP* d, int s_x, int s_y, int s_w, int s_h, int d_x, int d_y, int d_w, int d_h)
{

}

void show_mouse(BITMAP* bmp) {

}

int readkey()
{
	return 0;
}

int keypressed()
{
	return 0;
}

void blit(BITMAP* source, BITMAP* dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height)
{
}

void clear_keybuf()
{

}

BITMAP* create_bitmap(int width, int height)
{

}

void destroy_bitmap(BITMAP* bitmap)
{
}

int do_menu(MENU* menu, int x, int y)
{
	return 0;
}

void clear(BITMAP* bitmap)
{
}

void rectfill(BITMAP* bmp, int x1, int y1, int x2, int y2, int color)
{
}

void lookupWords(char* inputLine)
{

}


//Picture

int min_print_line = 1, user_input_line = 23, status_line_num = 0;

boolean okToShowPic;
int screenMode;

PictureFile loadedPictures[256];

BITMAP* picture;
BITMAP* priority;
BITMAP* control;
BITMAP* agi_screen;      /* This is a subbitmap of the screen */

int numInputWords, inputWords[10];
char wordText[10][80];

void showPicture()
{

}
void showDebugPri()
{

}

void discardPictureFile(int picFileNum) {

}

void initPicture() {

}

void initPictures() {

}

void initSound() {

}

void initAGIScreen()
{
}

void initPalette()
{

}

boolean statusLineDisplayed, inputLineDisplayed;

void discardSoundFile(int soundNum)
{

}

void closePicture() {

}

void drawPic(byte* data, int pLen, boolean okToClearScreen) {

}

void loadPictureFile(int picFileNum) {

}

void loadSoundFile(int soundNum) {

}

//Graphics
void drawString(BITMAP* scn, char* data, int x, int y, int foreColour, int backColour)
{

}

void drawChar(BITMAP* scn, byte charNum, int x, int y, int foreColour, int backColour)
{

}

void drawBigString(BITMAP* scn, char* data, int x, int y, int foreColour, int backColour)
{

}

//Directions
byte directions[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//allgero
BITMAP* screen;
void rect(BITMAP* bmp, int x1, int y1, int x2, int y2, int color)
{

}

void stretch_sprite(BITMAP* bmp, BITMAP* sprite, int x, int y, int w, int h)
{

}

//Parser
boolean haveKey = FALSE;
char cursorChar = '_';
byte keyState[256], asciiState[256];
int lastKey;

void pollKeyboard()
{

}

void initEvents()
{

}

void getString(char* promptStr, char* returnStr, int x, int y, int l) {

}

//Object
void loadObjectFile() {

}

void discardObjects() {

}

//Words
void loadWords()
{

}

void discardWords()
{

}

//Sound
int soundEndFlag;

boolean checkForEnd = FALSE;
extern SoundFile loadedSounds[];
extern boolean checkForEnd;
extern int soundEndFlag;

void stop_midi()
{
}


//objects
int numObjects;
//objectType* objects;

//Text
void printInBoxBig2(char* theString) {

}

void printInBoxBig(char* theString, int x, int y, int lineLen)
{
}

void printInBox(char* theString) {

}

//AGI Codes
agiCommandType testCommands[NUM_TEST_COMMANDS] = {
 { "", 0, 0x00 },
 { "equaln", 2, 0x80 },
 { "equalv", 2, 0xC0 },
 { "lessn", 2, 0x80 },
 { "lessv", 2, 0xC0 },
 { "greatern", 2, 0x80 },
 { "greaterv", 2, 0xC0 },
 { "isset", 1, 0x00 },
 { "issetv", 1, 0x80 },
 { "has", 1, 0x00 },
 { "obj.in.room", 2, 0x40 },
 { "posn", 5, 0x00 },
 { "controller", 1, 0x00 },
 { "have.key", 0, 0x00 },
 { "said", 0, 0x00 },                /* Not 0 args. Has variable number. */
 { "compare.strings", 2, 0x00 },
 { "obj.in.box", 5, 0x00 },
 { "center.posn", 5, 0x00 },
 { "right.posn", 5, 0x00 }
};

//unknown
byte* key;

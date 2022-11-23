/***************************************************************************
** commands.c
**
** These functions are used to execute the LOGIC files. The function
** executeLogic() is called by the main AGI interpret function in order to
** execute LOGIC.0 for each cycle of interpretation. Most of the other
** functions correspond almost exactly with the AGI equivalents.
**
** (c) 1997, 1998 Lance Ewing - Initial code (30 Aug 97)
**                              Additions (10 Jan 98) (4-5 Jul 98)
***************************************************************************/


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <cx16.h>

#include "commands.h"
#include "general.h"
#include "logic.h"
#include "memoryManager.h"
#include "stub.h"

#define HIGHEST_BANK1_FUNC 40


#define  PLAYER_CONTROL   0
#define  PROGRAM_CONTROL  1
#define CODE_WINDOW_SIZE 10
#define VERBOSE

//#define  DEBUG

extern byte var[256];
extern boolean flag[256];
extern char string[12][40];
extern int newRoomNum;
extern boolean hasEnteredNewRoom, exitAllLogics;
extern byte horizon;
extern int controlMode;

//extern int picFNum;   // Just a debug variable. Delete at some stage!!

int currentLog, agi_bg = 1, agi_fg = 16;
extern char cursorChar;
boolean oldQuit = FALSE;

/* MENU data */
#define MAX_MENU_SIZE 20
int numOfMenus = 0;
MENU the_menu[MAX_MENU_SIZE] = {
   { NULL, NULL, NULL }
};

void executeLogic(int logNum);
void freeMenuItems();

#pragma code-name (push, "BANKRAM01");
/****************************************************************************
** addLogLine
****************************************************************************/
void b1AddLogLine(char* message)
{
    FILE* fp;

    if ((fp = fopen("log.txt", "a")) == NULL) {
        fprintf(stderr, "Error opening log file.");
        return;
    }

    fprintf(fp, "%s\n", message);

    fclose(fp);
}

/***************************************************************************
** //lprintf
**
** This function behaves exactly the same as printf except that it writes
** the output to the log file using addLogLine().
***************************************************************************/
int b1Lprintf(char* fmt, ...)
{
    /*va_list args;
    char tempStr[10];

    va_start(args, fmt);
    vsprintf(tempStr, fmt, args);
    va_end(args);

    addLogLine(tempStr);*/

    //printf("Implement lprinf");

    return 0;
}

/* TEST COMMANDS */

boolean b1Equaln(byte** data) // 2, 0x80 
{
    int varVal, value;

    varVal = var[*(*data)++];
    value = *(*data)++;
    return (varVal == value);
}

boolean b1Equalv(byte** data) // 2, 0xC0 
{
    int varVal1, varVal2;

    varVal1 = var[*(*data)++];
    varVal2 = var[*(*data)++];
    return (varVal1 == varVal2);
}

boolean b1Lessn(byte** data) // 2, 0x80 
{
    int varVal, value;

    varVal = var[*(*data)++];
    value = *(*data)++;
    return (varVal < value);
}

boolean b1Lessv(byte** data) // 2, 0xC0 
{
    int varVal1, varVal2;

    varVal1 = var[*(*data)++];
    varVal2 = var[*(*data)++];
    return (varVal1 < varVal2);
}

boolean b1Greatern(byte** data) // 2, 0x80 
{
    int varVal, value;

    varVal = var[*(*data)++];
    value = *(*data)++;
        
    return (varVal > value);
}

boolean b1Greaterv(byte** data) // 2, 0xC0 
{
    int varVal1, varVal2;

    varVal1 = var[*(*data)++];
    varVal2 = var[*(*data)++];
    return (varVal1 > varVal2);
}

boolean b1Isset(byte** data) // 1, 0x00 
{
    return (flag[*(*data)++]);
}

boolean b1Issetv(byte** data) // 1, 0x80 
{
    return (flag[var[*(*data)++]]);
}

boolean b1Has(byte** data) // 1, 0x00 
{
    return (objects[*(*data)++].roomNum == 255);
}

boolean b1Obj_in_room(byte** data) // 2, 0x40 
{
    int objNum, varNum;

    objNum = *(*data)++;
    varNum = var[*(*data)++];
    return (objects[objNum].roomNum == varNum);
}

boolean b1Posn(byte** data) // 5, 0x00 
{
    int objNum, x1, y1, x2, y2;

    objNum = *(*data)++;
    x1 = *(*data)++;
    y1 = *(*data)++;
    x2 = *(*data)++;
    y2 = *(*data)++;

    return ((viewtab[objNum].xPos >= x1) && (viewtab[objNum].yPos >= y1)
        && (viewtab[objNum].xPos <= x2) && (viewtab[objNum].yPos <= y2));
}

boolean b1Controller(byte** data) // 1, 0x00 
{
    int eventNum = *(*data)++, retVal = 0;

    /* Some events can be activated by menu input or key input. */

    /* Following code detects key presses at the current time */
    switch (events[eventNum].type) {
    case ASCII_KEY_EVENT:
        if (events[eventNum].activated) {
            events[eventNum].activated = FALSE;
            return TRUE;
        }
        return (asciiState[events[eventNum].eventID]);
    case SCAN_KEY_EVENT:
        if (events[eventNum].activated) {
            events[eventNum].activated = FALSE;
            return TRUE;
        }
        if ((events[eventNum].eventID < 59) &&
            (asciiState[0] == 0)) return FALSE;   /* ALT Combinations */
        return (keyState[events[eventNum].eventID]);
    case MENU_EVENT:
        retVal = events[eventNum].activated;
        events[eventNum].activated = 0;
        return (retVal);
    default:
        return (FALSE);
    }
}

boolean b1Have_key() // 0, 0x00
{
    /* return (TRUE); */
    /* return (haveKey); */
    /* return (keypressed() || haveKey); */
    if (haveKey && key[lastKey]) return TRUE;
    return keypressed();
}

/* The said() command is in parser.h
boolean said(byte **data)
{

}
*/

boolean b1Compare_strings(byte** data) // 2, 0x00 
{
    int s1, s2;

    s1 = *(*data)++;
    s2 = *(*data)++;
    if (strcmp(string[s1], string[s2]) == 0) return TRUE;
    return FALSE;
}

boolean b1Obj_in_box(byte** data) // 5, 0x00 
{
    int objNum, x1, y1, x2, y2;

    objNum = *(*data)++;
    x1 = *(*data)++;
    y1 = *(*data)++;
    x2 = *(*data)++;
    y2 = *(*data)++;

    return ((viewtab[objNum].xPos >= x1) &&
        (viewtab[objNum].yPos >= y1) &&
        ((viewtab[objNum].xPos + viewtab[objNum].xsize - 1) <= x2) &&
        (viewtab[objNum].yPos <= y2));
}

boolean b1Center_posn(byte** data) // 5, 0x00 }
{
    int objNum, x1, y1, x2, y2;

    objNum = *(*data)++;
    x1 = *(*data)++;
    y1 = *(*data)++;
    x2 = *(*data)++;
    y2 = *(*data)++;

    return (((viewtab[objNum].xPos + (viewtab[objNum].xsize / 2)) >= x1) &&
        (viewtab[objNum].yPos >= y1) &&
        ((viewtab[objNum].xPos + (viewtab[objNum].xsize / 2)) <= x2) &&
        (viewtab[objNum].yPos <= y2));
}

boolean b1Right_posn(byte** data) // 5, 0x00
{
    int objNum, x1, y1, x2, y2;

    objNum = *(*data)++;
    x1 = *(*data)++;
    y1 = *(*data)++;
    x2 = *(*data)++;
    y2 = *(*data)++;

    return (((viewtab[objNum].xPos + viewtab[objNum].xsize - 1) >= x1) &&
        (viewtab[objNum].yPos >= y1) &&
        ((viewtab[objNum].xPos + viewtab[objNum].xsize - 1) <= x2) &&
        (viewtab[objNum].yPos <= y2));
}



/* ACTION COMMANDS */

void b1Increment(byte** data) // 1, 0x80 
{
    if (var[*(*data)] < 0xFF)
        var[*(*data)]++;
    (*data)++;

    /* var[*(*data)++]++;  This one doesn't check bounds */
}

void b1Decrement(byte** data) // 1, 0x80 
{
    if (var[*(*data)] > 0)
        var[*(*data)]--;
    (*data)++;

    /* var[*(*data)++]--;  This one doesn't check bounds */
}

void b1Assignn(byte** data) // 2, 0x80 
{
    int varNum, value;

    varNum = *(*data)++;
    value = *(*data)++;
    var[varNum] = value;
}

void b1Assignv(byte** data) // 2, 0xC0 
{
    int var1, var2;

    var1 = *(*data)++;
    var2 = *(*data)++;
    var[var1] = var[var2];
}

void b1Addn(byte** data) // 2, 0x80 
{
    int varNum, value;

    varNum = *(*data)++;
    value = *(*data)++;
    var[varNum] += value;
}

void b1Addv(byte** data) // 2, 0xC0 
{
    int var1, var2;

    var1 = *(*data)++;
    var2 = *(*data)++;
    var[var1] += var[var2];
}

void b1Subn(byte** data) // 2, 0x80 
{
    int varNum, value;

    varNum = *(*data)++;
    value = *(*data)++;
    var[varNum] -= value;
}

void b1Subv(byte** data) // 2, 0xC0 
{
    int var1, var2;

    var1 = *(*data)++;
    var2 = *(*data)++;
    var[var1] -= var[var2];
}

void b1Lindirectv(byte** data) // 2, 0xC0 
{
    int var1, var2;

    var1 = *(*data)++;
    var2 = *(*data)++;
    var[var[var1]] = var[var2];
}

void b1Rindirect(byte** data) // 2, 0xC0 
{
    int var1, var2;

    var1 = *(*data)++;
    var2 = *(*data)++;
    var[var1] = var[var[var2]];
}

void b1Lindirectn(byte** data) // 2, 0x80 
{
    int varNum, value;

    varNum = *(*data)++;
    value = *(*data)++;
    var[var[varNum]] = value;
}

void b1Set(byte** data) // 1, 0x00 
{
    flag[*(*data)++] = TRUE;
}

void b1Reset(byte** data) // 1, 0x00 
{
    flag[*(*data)++] = FALSE;
}

void b1Toggle(byte** data) // 1, 0x00 
{
    int f = *(*data)++;

    flag[f] = (flag[f] ? FALSE : TRUE);
}

void b1Set_v(byte** data) // 1, 0x80 
{
    flag[var[*(*data)++]] = TRUE;
}

void b1Reset_v(byte** data) // 1, 0x80 
{
    flag[var[*(*data)++]] = FALSE;
}

void b1Toggle_v(byte** data) // 1, 0x80 
{
    int f = var[*(*data)++];

    flag[f] = (flag[f] ? FALSE : TRUE);
}

void b1New_room(byte** data) // 1, 0x00 
{
    /* This function is handled in meka.c */
    newRoomNum = *(*data)++;
    hasEnteredNewRoom = TRUE;
}

void b1New_room_v(byte** data) // 1, 0x80 
{
    /* This function is handled in meka.c */
    newRoomNum = var[*(*data)++];
    hasEnteredNewRoom = TRUE;
}

void b1Load_logics(byte** data) // 1, 0x00 
{
    loadLogicFile(*(*data)++);
}

void b1Load_logics_v(byte** data) // 1, 0x80 
{
    loadLogicFile(var[*(*data)++]);
}

void b1Call(byte** data) // 1, 0x00 
{
    executeLogic(*(*data)++);
}

void b1Call_v(byte** data) // 1, 0x80 
{
    executeLogic(var[*(*data)++]);
}

void b1Load_pic(byte** data) // 1, 0x80 
{
    loadPictureFile(var[*(*data)++]);
}

void b1Draw_pic(byte** data) // 1, 0x80 
{
    int pNum;

    pNum = var[*(*data)++];
    //picFNum = pNum;  // Debugging. Delete at some stage!!!
    drawPic(loadedPictures[pNum].data, loadedPictures[pNum].size, TRUE);
}

void b1Show_pic(byte** data) // 0, 0x00 
{
    okToShowPic = TRUE;   /* Says draw picture with next object update */
    /*stretch_blit(picture, working_screen, 0, 0, 160, 168, 0, 20, 640, 336);*/
    showPicture();
}

void b1Discard_pic(byte** data) // 1, 0x80 
{
    discardPictureFile(var[*(*data)++]);
}

void b1Overlay_pic(byte** data) // 1, 0x80 
{
    int pNum;

    pNum = var[*(*data)++];
    drawPic(loadedPictures[pNum].data, loadedPictures[pNum].size, FALSE);
}

void b1Show_pri_screen(byte** data) // 0, 0x00 
{
    //showPriority();
    showDebugPri();
    //getch();
    //while (!keypressed()) { /* Wait for key */ }
}

/************************** VIEW ACTION COMMANDS **************************/

void b1Load_view(byte** data) // 1, 0x00 
{
    loadViewFile(*(*data)++);
}

void b1Load_view_v(byte** data) // 1, 0x80 
{
    loadViewFile(var[*(*data)++]);
}

void b1Discard_view(byte** data) // 1, 0x00 
{
    discardView(*(*data)++);
}

void b1Animate_obj(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    //viewtab[entryNum].flags |= (ANIMATED | UPDATE | CYCLING);
    viewtab[entryNum].flags = (ANIMATED | UPDATE | CYCLING);
    /* Not sure about CYCLING */
    /* Not sure about whether these two are set to zero */
    viewtab[entryNum].motion = 0;
    viewtab[entryNum].cycleStatus = 0;
    viewtab[entryNum].flags |= MOTION;
    if (entryNum != 0) viewtab[entryNum].direction = 0;
}

void b1Unanimate_all(byte** data) // 0, 0x00 
{
    int entryNum;

    /* Mark all objects as unanimated and not drawn */
    for (entryNum = 0; entryNum < TABLESIZE; entryNum++)
        viewtab[entryNum].flags &= ~(ANIMATED | DRAWN);
}

void b1Draw(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= (DRAWN | UPDATE);   /* Not sure about update */
    setCel(entryNum, viewtab[entryNum].currentCel);
    drawObject(entryNum);
}

void b1Erase(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~DRAWN;
}

void b1Position(byte** data) // 3, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].xPos = *(*data)++;
    viewtab[entryNum].yPos = *(*data)++;
    /* Need to check that it hasn't been draw()n yet. */
}

void b1Position_v(byte** data) // 3, 0x60 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].xPos = var[*(*data)++];
    viewtab[entryNum].yPos = var[*(*data)++];
    /* Need to check that it hasn't been draw()n yet. */
}

void b1Get_posn(byte** data) // 3, 0x60 
{
    int entryNum;

    entryNum = *(*data)++;
    var[*(*data)++] = viewtab[entryNum].xPos;
    var[*(*data)++] = viewtab[entryNum].yPos;
}

void b1Reposition(byte** data) // 3, 0x60 
{
    int entryNum, dx, dy;

    entryNum = *(*data)++;
    dx = (signed char)var[*(*data)++];
    dy = (signed char)var[*(*data)++];
    viewtab[entryNum].xPos += dx;
    viewtab[entryNum].yPos += dy;
}

#pragma code-name (pop)
#pragma code-name (push, "BANKRAM02")

void set_view(byte** data) // 2, 0x00 
{
    int entryNum, viewNum;

    entryNum = *(*data)++;
    viewNum = *(*data)++;
    addViewToTable(entryNum, viewNum);
}

void set_view_v(byte** data) // 2, 0x40 
{
    int entryNum, viewNum;

    entryNum = *(*data)++;
    viewNum = var[*(*data)++];
    addViewToTable(entryNum, viewNum);
}

void set_loop(byte** data) // 2, 0x00 
{
    int entryNum, loopNum;

    entryNum = *(*data)++;
    loopNum = *(*data)++;
    setLoop(entryNum, loopNum);
    setCel(entryNum, 0);
}

void set_loop_v(byte** data) // 2, 0x40 
{
    int entryNum, loopNum;

    entryNum = *(*data)++;
    loopNum = var[*(*data)++];
    setLoop(entryNum, loopNum);
    setCel(entryNum, 0);
}

void fix_loop(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= FIXLOOP;
}

void release_loop(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~FIXLOOP;
}

void set_cel(byte** data) // 2, 0x00 
{
    int entryNum, celNum;

    entryNum = *(*data)++;
    celNum = *(*data)++;
    setCel(entryNum, celNum);
}

void set_cel_v(byte** data) // 2, 0x40 
{
    int entryNum, celNum;

    entryNum = *(*data)++;
    celNum = var[*(*data)++];
    setCel(entryNum, celNum);
}

void last_cel(byte** data) // 2, 0x40 
{
    int entryNum, varNum;

    entryNum = *(*data)++;
    varNum = *(*data)++;
    var[varNum] = viewtab[entryNum].numberOfCels - 1;
}

void current_cel(byte** data) // 2, 0x40 
{
    int entryNum, varNum;

    entryNum = *(*data)++;
    varNum = *(*data)++;
    var[varNum] = viewtab[entryNum].currentCel;
}

void current_loop(byte** data) // 2, 0x40 
{
    int entryNum, varNum;

    entryNum = *(*data)++;
    varNum = *(*data)++;
    var[varNum] = viewtab[entryNum].currentLoop;
}

void current_view(byte** data) // 2, 0x40 
{
    int entryNum, varNum;

    entryNum = *(*data)++;
    varNum = *(*data)++;
    var[varNum] = viewtab[entryNum].currentView;
}

void number_of_loops(byte** data) // 2, 0x40 
{
    int entryNum, varNum;

    entryNum = *(*data)++;
    varNum = *(*data)++;
    var[varNum] = viewtab[entryNum].numberOfLoops;
}

void set_priority(byte** data) // 2, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].priority = *(*data)++;
    viewtab[entryNum].flags |= FIXEDPRIORITY;
}

void set_priority_v(byte** data) // 2, 0x40 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].priority = var[*(*data)++];
    viewtab[entryNum].flags |= FIXEDPRIORITY;
}

void release_priority(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~FIXEDPRIORITY;
}

void get_priority(byte** data) // 2, 0x40 
{
    int entryNum, varNum;

    entryNum = *(*data)++;
    varNum = *(*data)++;
    var[varNum] = viewtab[entryNum].priority;
}

void stop_update(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~UPDATE;
}

void start_update(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= UPDATE;
}

void force_update(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    /* Do immediate update here. Call update(entryNum) */
    updateObj(entryNum);
}

void ignore_horizon(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= IGNOREHORIZON;
}

void observe_horizon(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~IGNOREHORIZON;
}

void set_horizon(byte** data) // 1, 0x00 
{
    horizon = *(*data)++;
}

void object_on_water(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= ONWATER;
}

void object_on_land(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= ONLAND;
}

void object_on_anything(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~(ONWATER | ONLAND);
}

void ignore_objs(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= IGNOREOBJECTS;
}

void observe_objs(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~IGNOREOBJECTS;
}


void distance(byte** data) // 3, 0x20 
{
    int o1, o2, varNum, x1, y1, x2, y2;

    o1 = *(*data)++;
    o2 = *(*data)++;
    varNum = *(*data)++;
    /* Check that both objects are on screen here. If they aren't
    ** then 255 should be returned. */
    if (!((viewtab[o1].flags & DRAWN) && (viewtab[o2].flags & DRAWN))) {
        var[varNum] = 255;
        return;
    }
    x1 = viewtab[o1].xPos;
    y1 = viewtab[o1].yPos;
    x2 = viewtab[o2].xPos;
    y2 = viewtab[o2].yPos;
    var[varNum] = abs(x1 - x2) + abs(y1 - y2);
}

void stop_cycling(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~CYCLING;
}

void start_cycling(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= CYCLING;
}

void normal_cycle(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].cycleStatus = 0;
}

void end_of_loop(byte** data) // 2, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].param1 = *(*data)++;
    viewtab[entryNum].cycleStatus = 1;
    viewtab[entryNum].flags |= (UPDATE | CYCLING);
}

void reverse_cycle(byte** data) // 1, 0x00
{
    int entryNum;

    entryNum = *(*data)++;
    /* Store the other parameters here */

    viewtab[entryNum].cycleStatus = 3;
}

void reverse_loop(byte** data) // 2, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].param1 = *(*data)++;
    viewtab[entryNum].cycleStatus = 2;
    viewtab[entryNum].flags |= (UPDATE | CYCLING);
}

void cycle_time(byte** data) // 2, 0x40 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].cycleTime = var[*(*data)++];
}

void stop_motion(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~MOTION;
    viewtab[entryNum].direction = 0;
    viewtab[entryNum].motion = 0;
}

void start_motion(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= MOTION;
    viewtab[entryNum].motion = 0;        /* Not sure about this */
}

void step_size(byte** data) // 2, 0x40 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].stepSize = var[*(*data)++];
}

void step_time(byte** data) // 2, 0x40 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].stepTime = var[*(*data)++];
}

void move_obj(byte** data) // 5, 0x00 
{
    int entryNum;
    byte stepVal;

    entryNum = *(*data)++;
    viewtab[entryNum].param1 = *(*data)++;
    viewtab[entryNum].param2 = *(*data)++;
    viewtab[entryNum].param3 = viewtab[entryNum].stepSize;  /* Save stepsize */
    stepVal = *(*data)++;
    if (stepVal > 0) viewtab[entryNum].stepSize = stepVal;
    viewtab[entryNum].param4 = *(*data)++;
    viewtab[entryNum].motion = 3;
    viewtab[entryNum].flags |= MOTION;
}

void move_obj_v(byte** data) // 5, 0x70 
{
    int entryNum;
    byte stepVal;

    entryNum = *(*data)++;
    viewtab[entryNum].param1 = var[*(*data)++];
    viewtab[entryNum].param2 = var[*(*data)++];
    viewtab[entryNum].param3 = viewtab[entryNum].stepSize;  /* Save stepsize */
    stepVal = var[*(*data)++];
    if (stepVal > 0) viewtab[entryNum].stepSize = stepVal;
    viewtab[entryNum].param4 = *(*data)++;
    viewtab[entryNum].motion = 3;
    viewtab[entryNum].flags |= MOTION;
}

void follow_ego(byte** data) // 3, 0x00 
{
    int entryNum, stepVal, flagNum;

    entryNum = *(*data)++;
    stepVal = *(*data)++;
    flagNum = *(*data)++;
    viewtab[entryNum].param1 = viewtab[entryNum].stepSize;
    /* Might need to put 'if (stepVal != 0)' */
    //viewtab[entryNum].stepSize = stepVal;
    viewtab[entryNum].param2 = flagNum;
    viewtab[entryNum].motion = 2;
    viewtab[entryNum].flags |= MOTION;
}

void wander(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].motion = 1;
    viewtab[entryNum].flags |= MOTION;
}

void normal_motion(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].motion = 0;
    viewtab[entryNum].flags |= MOTION;
}

void set_dir(byte** data) // 2, 0x40 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].direction = var[*(*data)++];
}

void get_dir(byte** data) // 2, 0x40 
{
    int entryNum;

    entryNum = *(*data)++;
    var[*(*data)++] = viewtab[entryNum].direction;
}

void ignore_blocks(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= IGNOREBLOCKS;
}

void observe_blocks(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~IGNOREBLOCKS;
}

void block(byte** data) // 4, 0x00 
{
    /* Is this used anywhere? - Not implemented at this stage */
    *data += 4;
}

void unblock(byte** data) // 0, 0x00 
{

}

void get(byte** data) // 1, 00 
{
    objects[*(*data)++].roomNum = 255;
}

void get_v(byte** data) // 1, 0x80 
{
    objects[var[*(*data)++]].roomNum = 255;
}

void drop(byte** data) // 1, 0x00 
{
    objects[*(*data)++].roomNum = 0;
}

void put(byte** data) // 2, 0x00 
{
    int objNum, room;

    objNum = *(*data)++;
    room = *(*data)++;
    objects[objNum].roomNum = room;
}

void put_v(byte** data) // 2, 0x40 
{
    int objNum, room;

    objNum = *(*data)++;
    room = var[*(*data)++];
    objects[objNum].roomNum = room;
}

void get_room_v(byte** data) // 2, 0xC0 
{
    int objNum, room;

    objNum = var[*(*data)++];
    var[*(*data)++] = objects[objNum].roomNum;
}

void load_sound(byte** data) // 1, 0x00 
{
    int soundNum;

    soundNum = *(*data)++;
    loadSoundFile(soundNum);
}

void play_sound(byte** data) // 2, 00  sound() renamed to avoid clash
{
    int soundNum;

    soundNum = *(*data)++;
    soundEndFlag = *(*data)++;
    /* playSound(soundNum); */
    flag[soundEndFlag] = TRUE;
}

void stop_sound(byte** data) // 0, 0x00 
{
    checkForEnd = FALSE;
    stop_midi();
}

#pragma code-name (pop)
#pragma code-name (push, "BANKRAM03")

int getNum(char* inputString, int* i)
{
    char tempString[80], strPos = 0;

    while (inputString[*i] == ' ') { *i++; }
    if ((inputString[*i] < '0') && (inputString[*i] > '9')) return 0;
    while ((inputString[*i] >= '0') && (inputString[*i] <= '9')) {
        tempString[strPos++] = inputString[(*i)++];
    }
    tempString[strPos] = 0;

    (*i)--;
    return (atoi(tempString));
}

boolean charIsIn(char testChar, char* testString)
{
    int i;

    for (i = 0; i < strlen(testString); i++) {
        if (testString[i] == testChar) return TRUE;
    }

    return FALSE;
}

void processString(char* inputString, char* outputString)
{
    int i, strPos = 0, tempNum, widthNum, count;
    char* numString;
    char* temp;

    numString = (char*)malloc(80);
    temp = (char*)malloc(256);

    outputString[0] = 0;

    for (i = 0; i < strlen(inputString); i++) {
        if (inputString[i] == '%') {
            i++;
            switch (inputString[i++]) {
                /* %% isn't actually supported */
                //case '%': sprintf(outputString, "%s%%", outputString); break;
            case 'v':
                tempNum = getNum(inputString, &i);
                if (inputString[i + 1] == '|') {
                    i += 2;
                    widthNum = getNum(inputString, &i);
                    sprintf(numString, "%d", var[tempNum]);
                    for (count = strlen(numString); count < widthNum; count++) {
                        sprintf(outputString, "%s0", outputString);
                    }
                    sprintf(outputString, "%s%d", outputString, var[tempNum]);
                }
                else
                    sprintf(outputString, "%s%d", outputString, var[tempNum]);
                break;
            case 'm':
                tempNum = getNum(inputString, &i);
                sprintf(outputString, "%s%s", outputString,
                    logics[currentLog].data->messages[tempNum - 1]);
                break;
            case 'g':
                tempNum = getNum(inputString, &i);
                sprintf(outputString, "%s%s", outputString, logics[0].data->messages[tempNum - 1]);
                break;
            case 'w':
                tempNum = getNum(inputString, &i);
                sprintf(outputString, "%s%s", outputString, wordText[tempNum]);
                break;
            case 's':
                tempNum = getNum(inputString, &i);
                sprintf(outputString, "%s%s", outputString, string[tempNum]);
                break;
            default: /* ignore the second character */
                break;
            }
        }
        else {
            sprintf(outputString, "%s%c", outputString, inputString[i]);
        }
    }

    /* Recursive part to make sure all % formatting codes are dealt with */
    if (charIsIn('%', outputString)) {
        strcpy(temp, outputString);
        processString(temp, outputString);
    }

    free(numString);
    free(temp);
}

void print(byte** data) // 1, 00 
{
    char* tempString = (char*)malloc(256);
    BITMAP* temp;

    show_mouse(NULL);
    temp = create_bitmap(640, 336);
    blit(agi_screen, temp, 0, 0, 0, 0, 640, 336);
    show_mouse(screen);
    while (key[KEY_ENTER] || key[KEY_ESC]) { /* Wait */ }
    processString(logics[currentLog].data->messages[(*(*data)++) - 1], tempString);
    printInBoxBig(tempString, -1, -1, 30);
    while (!key[KEY_ENTER] && !key[KEY_ESC]) { /* Wait */ }
    while (key[KEY_ENTER] || key[KEY_ESC]) { clear_keybuf(); }
    show_mouse(NULL);
    blit(temp, agi_screen, 0, 0, 0, 0, 640, 336);
    show_mouse(screen);
    destroy_bitmap(temp);

    free(tempString);
}

void print_v(byte** data) // 1, 0x80 
{
    char* tempString = (char*)malloc(256);
    BITMAP* temp;

    show_mouse(NULL);
    temp = create_bitmap(640, 336);
    blit(agi_screen, temp, 0, 0, 0, 0, 640, 336);
    while (key[KEY_ENTER] || key[KEY_ESC]) { /* Wait */ }
    processString(logics[currentLog].data->messages[(var[*(*data)++]) - 1], tempString);
    //printf("Warning Print In Bigbox Not Implemented Implement This");
    //printInBoxBig2(tempString, -1, -1, 30);
    while (!key[KEY_ENTER] && !key[KEY_ESC]) { /* Wait */ }
    while (key[KEY_ENTER] || key[KEY_ESC]) { clear_keybuf(); }
    blit(temp, agi_screen, 0, 0, 0, 0, 640, 336);
    show_mouse(screen);
    destroy_bitmap(temp);

    free(tempString);
}

void display(byte** data) // 3, 0x00 
{
    int row, col, messNum;
    char* tempString = malloc(256);

    col = *(*data)++;
    row = *(*data)++;
    messNum = *(*data)++;
    processString(logics[currentLog].data->messages[messNum - 1], tempString);
    drawBigString(screen, tempString, row * 16, 20 + (col * 16), agi_fg, agi_bg);
    /*lprintf("info: display() %s, fg: %d bg: %d row: %d col: %d",
       tempString, agi_fg, agi_bg, row, col);*/

    free(tempString);
}

void display_v(byte** data) // 3, 0xE0 
{
    int row, col, messNum;
    char* tempString = (char*)malloc(256);

    col = var[*(*data)++];
    row = var[*(*data)++];
    messNum = var[*(*data)++];
    //drawString(picture, logics[currentLog].data->messages[messNum-1],
    //   row*8, col*8, agi_fg, agi_bg);
    processString(logics[currentLog].data->messages[messNum - 1], tempString);
    drawBigString(screen, tempString, row * 16, 20 + (col * 16), agi_fg, agi_bg);
    /*lprintf("info: display.v() %s, foreground: %d background: %d",
       tempString, agi_fg, agi_bg);*/

    free(tempString);
}

void clear_lines(byte** data) // 3, 0x00 
{
    int boxColour, startLine, endLine;

    startLine = *(*data)++;
    endLine = *(*data)++;
    boxColour = *(*data)++;
    if ((screenMode == AGI_GRAPHICS) && (boxColour > 0)) boxColour = 15;
    boxColour++;
    show_mouse(NULL);
    rectfill(agi_screen, 0, startLine * 16, 639, (endLine * 16) + 15, boxColour);
    show_mouse(screen);
}

void text_screen(byte** data) // 0, 0x00 
{
    screenMode = AGI_TEXT;
    /* Do something else here */
    inputLineDisplayed = FALSE;
    statusLineDisplayed = FALSE;
    clear(screen);
}

void graphics(byte** data) // 0, 0x00 
{
    screenMode = AGI_GRAPHICS;
    /* Do something else here */
    inputLineDisplayed = TRUE;
    statusLineDisplayed = TRUE;
    okToShowPic = TRUE;
    clear(screen);
}

void set_cursor_char(byte** data) // 1, 0x00 
{
    char* temp = (char*)malloc(256);

    processString(logics[currentLog].data->messages[(*(*data)++) - 1], temp);
    cursorChar = temp[0];

    free(temp);
}

void set_text_attribute(byte** data) // 2, 0x00 
{
    agi_fg = (*(*data)++) + 1;
    agi_bg = (*(*data)++) + 1;
}

void shake_screen(byte** data) // 1, 0x00 
{
    (*data)++;  /* Ignore this for now. */
}

void configure_screen(byte** data) // 3, 0x00 
{
    min_print_line = *(*data)++;
    user_input_line = *(*data)++;
    status_line_num = *(*data)++;
}

void status_line_on(byte** data) // 0, 0x00 
{
    statusLineDisplayed = TRUE;
}

void status_line_off(byte** data) // 0, 0x00 
{
    statusLineDisplayed = FALSE;
}

void set_string(byte** data) // 2, 0x00 
{
    int stringNum, messNum;

    stringNum = *(*data)++;
    messNum = *(*data)++;
    strcpy(string[stringNum], logics[currentLog].data->messages[messNum - 1]);
}

void get_string(byte** data) // 5, 0x00 
{
    int strNum, messNum, row, col, l;

    strNum = *(*data)++;
    messNum = *(*data)++;
    col = *(*data)++;
    row = *(*data)++;
    l = *(*data)++;
    getString(logics[currentLog].data->messages[messNum - 1], string[strNum], row, col, l);
}

void word_to_string(byte** data) // 2, 0x00 
{
    int stringNum, wordNum;

    stringNum = *(*data)++;
    wordNum = *(*data)++;
    strcpy(string[stringNum], wordText[wordNum]);
}

void parse(byte** data) // 1, 0x00 
{
    int stringNum;

    stringNum = *(*data)++;
    lookupWords(string[stringNum]);
}

#pragma code-name (pop)
#pragma code-name (push, "BANKRAM04")

void get_num(byte** data) // 2, 0x40 
{
    int messNum, varNum;
    char temp[80];

    messNum = *(*data)++;
    varNum = *(*data)++;
    getString(logics[currentLog].data->messages[messNum - 1], temp, 1, 23, 3);
    var[varNum] = atoi(temp);
}

void prevent_input(byte** data) // 0, 0x00 
{
    inputLineDisplayed = FALSE;
    /* Do something else here */
}

void accept_input(byte** data) // 0, 0x00 
{
    inputLineDisplayed = TRUE;
    /* Do something else here */
}

void set_key(byte** data) // 3, 0x00 
{
    int asciiCode, scanCode, eventCode;
    char* tempStr = (char*)malloc(256);

    asciiCode = *(*data)++;
    scanCode = *(*data)++;
    eventCode = *(*data)++;

    /* Ignore cases which have both values set for now. They seem to behave
    ** differently than normal and often specify controllers that have
    ** already been defined.
    */
    if (scanCode && asciiCode) return;

    if (scanCode) {
        events[eventCode].type = SCAN_KEY_EVENT;
        events[eventCode].eventID = scanCode;
        events[eventCode].asciiValue = asciiCode;
        events[eventCode].scanCodeValue = scanCode;
        events[eventCode].activated = FALSE;
    }
    else if (asciiCode) {
        events[eventCode].type = ASCII_KEY_EVENT;
        events[eventCode].eventID = asciiCode;
        events[eventCode].asciiValue = asciiCode;
        events[eventCode].scanCodeValue = scanCode;
        events[eventCode].activated = FALSE;
    }

    free(tempStr);
}

void add_to_pic(byte** data) // 7, 0x00 
{
    int viewNum, loopNum, celNum, x, y, priNum, baseCol;

    viewNum = *(*data)++;
    loopNum = *(*data)++;
    celNum = *(*data)++;
    x = *(*data)++;
    y = *(*data)++;
    priNum = *(*data)++;
    baseCol = *(*data)++;

    addToPic(viewNum, loopNum, celNum, x, y, priNum, baseCol);
}

void add_to_pic_v(byte** data) // 7, 0xFE 
{
    int viewNum, loopNum, celNum, x, y, priNum, baseCol;

    viewNum = var[*(*data)++];
    loopNum = var[*(*data)++];
    celNum = var[*(*data)++];
    x = var[*(*data)++];
    y = var[*(*data)++];
    priNum = var[*(*data)++];
    baseCol = var[*(*data)++];

    addToPic(viewNum, loopNum, celNum, x, y, priNum, baseCol);
}

void status(byte** data) // 0, 0x00 
{
    /* Inventory */
    // set text mode
    // if flag 13 is set then allow selection and store selection in var[25]
    var[25] = 255;
}

void save_game(byte** data) // 0, 0x00 
{
    /* Not supported yet */
}

void restore_game(byte** data) // 0, 0x00 
{
    /* Not supported yet */
}

void init_disk(byte** data) // 0, 0x00 
{
    /* Not supported. Seems to be an old command. */
}

void restart_game(byte** data) // 0, 0x00 
{
    int i;

    /* Not supported yet */
    for (i = 0; i < 256; i++) {
        flag[i] = FALSE;
        var[i] = 0;
    }
    var[24] = 0x29;
    var[26] = 3;
    var[8] = 255;     /* Number of free 256 byte pages of memory */

    newRoomNum = 0;
    hasEnteredNewRoom = TRUE;
    freeMenuItems();
}

void show_obj(byte** data) // 1, 0x00 
{
    int objectNum;

    objectNum = *(*data)++;
    /* Not supported yet */
}

void random_num(byte** data) // 3, 0x20  random() renamed to avoid clash
{
    int startValue, endValue;

    startValue = *(*data)++;
    endValue = *(*data)++;
    var[*(*data)++] = (rand() % ((endValue - startValue) + 1)) + startValue;
}

void program_control(byte** data) // 0, 0x00 
{
    controlMode = PROGRAM_CONTROL;
}

void player_control(byte** data) // 0, 0x00 
{
    controlMode = PLAYER_CONTROL;
}

void obj_status_v(byte** data) // 1, 0x80 
{
    int objectNum;

    objectNum = var[*(*data)++];
    /* Not supported yet */

    /* showView(viewtab[objectNum].currentView); */
    showObjectState(objectNum);
}

void quit(byte** data) // 1, 0x00                     /* 0 args for AGI version 2_089 */
{
    int quitType, ch;

    quitType = ((!oldQuit) ? *(*data)++ : 0);
    if (quitType == 1) /* Immediate quit */
        exit(0);
    else { /* Prompt for exit */
        printInBoxBig("Press ENTER to quit.\nPress ESC to keep playing.", -1, -1, 30);
        do {
            ch = (readkey() >> 8);
        } while ((ch != KEY_ESC) && (ch != KEY_ENTER));
        if (ch == KEY_ENTER) exit(0);
        showPicture();
    }
}

void show_mem(byte** data) // 0, 0x00 
{
    /* Ignore */
}

void pause(byte** data) // 0, 0x00 
{
    while (key[KEY_ENTER]) { /* Wait */ }
    printInBoxBig("      Game paused.\nPress ENTER to continue.", -1, -1, 30);
    while (!key[KEY_ENTER]) { /* Wait */ }
    showPicture();
    okToShowPic = TRUE;
}

void echo_line(byte** data) // 0, 0x00 
{

}

void cancel_line(byte** data) // 0, 0x00 
{
    /*currentInputStr[0]=0;
    strPos=0;*/
}

void init_joy(byte** data) // 0, 0x00 
{
    /* Not important at this stage */
}

void toggle_monitor(byte** data) // 0, 0x00 
{
    /* Not important */
}

void version(byte** data) // 0, 0x00 
{
    while (key[KEY_ENTER] || key[KEY_ESC]) { /* Wait */ }
    printInBoxBig("MEKA AGI Interpreter\n    Version 1.0", -1, -1, 30);
    while (!key[KEY_ENTER] && !key[KEY_ESC]) { /* Wait */ }
    showPicture();
    okToShowPic = TRUE;
}

void script_size(byte** data) // 1, 0x00 
{
    (*data)++;  /* Ignore the script size. Not important for this interpreter */
}

void set_game_id(byte** data) // 1, 0x00 
{
    (*data)++;  /* Ignore the game ID. Not important */
}

void log(byte** data) // 1, 0x00 
{
    (*data)++;  /* Ignore log message. Not important */
}

void set_scan_start(byte** data) // 0, 0x00 
{
    /* currentPoint is set in executeLogic() */
    logics[currentLog].entryPoint = logics[currentLog].currentPoint + 1;
    /* Does it return() at this point, or does it execute to the end?? */
}

void reset_scan_start(byte** data) // 0, 0x00 
{
    logics[currentLog].entryPoint = 0;
}

void reposition_to(byte** data) // 3, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].xPos = *(*data)++;
    viewtab[entryNum].yPos = *(*data)++;
}

void reposition_to_v(byte** data) // 3, 0x60 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].xPos = var[*(*data)++];
    viewtab[entryNum].yPos = var[*(*data)++];
}

void trace_on(byte** data) // 0, 0x00 
{
    /* Ignore at this stage */
}

void trace_info(byte** data) // 3, 0x00 
{
    *data += 3;  /* Ignore trace information at this stage. */
}

void print_at(byte** data) // 4, 0x00           /* 3 args for AGI versions before */
{
    char* tempString = (char*)malloc(256);
    BITMAP* temp;
    int messNum, x, y, l;

    messNum = *(*data)++;
    x = *(*data)++;
    y = *(*data)++;
    l = *(*data)++;
    show_mouse(NULL);
    temp = create_bitmap(640, 336);
    blit(agi_screen, temp, 0, 0, 0, 0, 640, 336);
    show_mouse(screen);
    while (key[KEY_ENTER] || key[KEY_ESC]) { /* Wait */ }
    processString(logics[currentLog].data->messages[messNum - 1], tempString);
    printInBoxBig(tempString, x, y, l);
    while (!key[KEY_ENTER] && !key[KEY_ESC]) { /* Wait */ }
    while (key[KEY_ENTER] || key[KEY_ESC]) { clear_keybuf(); }
    show_mouse(NULL);
    blit(temp, agi_screen, 0, 0, 0, 0, 640, 336);
    show_mouse(screen);
    destroy_bitmap(temp);

    free(tempString);
}

void print_at_v(byte** data) // 4, 0x80         /* 2_440 (maybe laterz) */
{
    char* tempString = (char*)malloc(256);
    BITMAP* temp;
    int messNum, x, y, l;

    messNum = var[*(*data)++];
    x = *(*data)++;
    y = *(*data)++;
    l = *(*data)++;
    show_mouse(NULL);
    temp = create_bitmap(640, 336);
    blit(agi_screen, temp, 0, 0, 0, 0, 640, 336);
    show_mouse(screen);
    while (key[KEY_ENTER] || key[KEY_ESC]) { /* Wait */ }
    processString(logics[currentLog].data->messages[messNum - 1], tempString);
    printInBoxBig(tempString, x, y, l);
    while (!key[KEY_ENTER] && !key[KEY_ESC]) { /* Wait */ }
    while (key[KEY_ENTER] || key[KEY_ESC]) { clear_keybuf(); }
    show_mouse(NULL);
    blit(temp, agi_screen, 0, 0, 0, 0, 640, 336);
    show_mouse(screen);
    destroy_bitmap(temp);

    free(tempString);
}

void discard_view_v(byte** data) // 1, 0x80 
{
    discardView(var[*(*data)++]);
}

void clear_text_rect(byte** data) // 5, 0x00 
{
    int x1, y1, x2, y2, boxColour;

    x1 = *(*data)++;
    y1 = *(*data)++;
    x2 = *(*data)++;
    y2 = *(*data)++;
    boxColour = *(*data)++;
    if ((screenMode == AGI_GRAPHICS) && (boxColour > 0)) boxColour = 15;
    if (screenMode == AGI_TEXT) boxColour = 0;
    show_mouse(NULL);
    rectfill(agi_screen, x1 * 16, y1 * 16, (x2 * 16) + 15, (y2 * 16) + 15, boxColour);
    show_mouse(screen);
}

void set_upper_left(byte** data) // 2, 0x00    (x, y) ??
{
    *data += 2;
}

void waitKeyRelease()
{
    while (keypressed()) { /* Wait */ }
}

int menuEvent0() { waitKeyRelease(); events[0].activated = 1; return D_O_K; }
int menuEvent1() { waitKeyRelease(); events[1].activated = 1; return D_O_K; }
int menuEvent2() { waitKeyRelease(); events[2].activated = 1; return D_O_K; }
int menuEvent3() { waitKeyRelease(); events[3].activated = 1; return D_O_K; }
int menuEvent4() { waitKeyRelease(); events[4].activated = 1; return D_O_K; }
int menuEvent5() { waitKeyRelease(); events[5].activated = 1; return D_O_K; }
int menuEvent6() { waitKeyRelease(); events[6].activated = 1; return D_O_K; }
int menuEvent7() { waitKeyRelease(); events[7].activated = 1; return D_O_K; }
int menuEvent8() { waitKeyRelease(); events[8].activated = 1; return D_O_K; }
int menuEvent9() { waitKeyRelease(); events[9].activated = 1; return D_O_K; }
int menuEvent10() { waitKeyRelease(); events[10].activated = 1; return D_O_K; }
int menuEvent11() { waitKeyRelease(); events[11].activated = 1; return D_O_K; }
int menuEvent12() { waitKeyRelease(); events[12].activated = 1; return D_O_K; }
int menuEvent13() { waitKeyRelease(); events[13].activated = 1; return D_O_K; }
int menuEvent14() { waitKeyRelease(); events[14].activated = 1; return D_O_K; }
int menuEvent15() { waitKeyRelease(); events[15].activated = 1; return D_O_K; }
int menuEvent16() { waitKeyRelease(); events[16].activated = 1; return D_O_K; }
int menuEvent17() { waitKeyRelease(); events[17].activated = 1; return D_O_K; }
int menuEvent18() { waitKeyRelease(); events[18].activated = 1; return D_O_K; }
int menuEvent19() { waitKeyRelease(); events[19].activated = 1; return D_O_K; }
int menuEvent20() { waitKeyRelease(); events[20].activated = 1; return D_O_K; }
int menuEvent21() { waitKeyRelease(); events[21].activated = 1; return D_O_K; }
int menuEvent22() { waitKeyRelease(); events[22].activated = 1; return D_O_K; }
int menuEvent23() { waitKeyRelease(); events[23].activated = 1; return D_O_K; }
int menuEvent24() { waitKeyRelease(); events[24].activated = 1; return D_O_K; }
int menuEvent25() { waitKeyRelease(); events[25].activated = 1; return D_O_K; }
int menuEvent26() { waitKeyRelease(); events[26].activated = 1; return D_O_K; }
int menuEvent27() { waitKeyRelease(); events[27].activated = 1; return D_O_K; }
int menuEvent28() { waitKeyRelease(); events[28].activated = 1; return D_O_K; }
int menuEvent29() { waitKeyRelease(); events[29].activated = 1; return D_O_K; }
int menuEvent30() { waitKeyRelease(); events[30].activated = 1; return D_O_K; }
int menuEvent31() { waitKeyRelease(); events[31].activated = 1; return D_O_K; }
int menuEvent32() { waitKeyRelease(); events[32].activated = 1; return D_O_K; }
int menuEvent33() { waitKeyRelease(); events[33].activated = 1; return D_O_K; }
int menuEvent34() { waitKeyRelease(); events[34].activated = 1; return D_O_K; }
int menuEvent35() { waitKeyRelease(); events[35].activated = 1; return D_O_K; }
int menuEvent36() { waitKeyRelease(); events[36].activated = 1; return D_O_K; }
int menuEvent37() { waitKeyRelease(); events[37].activated = 1; return D_O_K; }
int menuEvent38() { waitKeyRelease(); events[38].activated = 1; return D_O_K; }
int menuEvent39() { waitKeyRelease(); events[39].activated = 1; return D_O_K; }
int menuEvent40() { waitKeyRelease(); events[40].activated = 1; return D_O_K; }
int menuEvent41() { waitKeyRelease(); events[41].activated = 1; return D_O_K; }
int menuEvent42() { waitKeyRelease(); events[42].activated = 1; return D_O_K; }
int menuEvent43() { waitKeyRelease(); events[43].activated = 1; return D_O_K; }
int menuEvent44() { waitKeyRelease(); events[44].activated = 1; return D_O_K; }
int menuEvent45() { waitKeyRelease(); events[45].activated = 1; return D_O_K; }
int menuEvent46() { waitKeyRelease(); events[46].activated = 1; return D_O_K; }
int menuEvent47() { waitKeyRelease(); events[47].activated = 1; return D_O_K; }
int menuEvent48() { waitKeyRelease(); events[48].activated = 1; return D_O_K; }
int menuEvent49() { waitKeyRelease(); events[49].activated = 1; return D_O_K; }

int (*(menuFunctions[50]))() = {
    menuEvent0, menuEvent1, menuEvent2, menuEvent3, menuEvent4,
    menuEvent5, menuEvent6, menuEvent7, menuEvent8, menuEvent9,
    menuEvent10, menuEvent11, menuEvent12, menuEvent13, menuEvent14,
    menuEvent15, menuEvent16, menuEvent17, menuEvent18, menuEvent19,
    menuEvent20, menuEvent21, menuEvent22, menuEvent23, menuEvent24,
    menuEvent25, menuEvent26, menuEvent27, menuEvent28, menuEvent29,
    menuEvent30, menuEvent31, menuEvent32, menuEvent33, menuEvent34,
    menuEvent35, menuEvent36, menuEvent37, menuEvent38, menuEvent39,
    menuEvent40, menuEvent41, menuEvent42, menuEvent43, menuEvent44,
    menuEvent45, menuEvent46, menuEvent47, menuEvent48, menuEvent49
};

/***************************************************************************
** freeMenuItems
**
** This function frees all dynamically allocated memory taken up by the
** menus.
***************************************************************************/
void freeMenuItems()
{
    int i, j;

    for (i = 0; i < numOfMenus; i++) {
        for (j = 0; the_menu[i].child[j].text != NULL; j++) {
            //free(the_menu[i].child[j].text);
        }

        /* Free the child menu array */
        //free(the_menu[i].child);
        the_menu[i].text = NULL;
        the_menu[i].proc = NULL;
        the_menu[i].child = NULL;
    }

    numOfMenus = 0;
}

void set_menu(byte** data) // 1, 0x00 
{
    int messNum, startOffset;
    char* messData;

    messNum = *(*data)++;
    messData = logics[currentLog].data->messages[messNum - 1];

    /* Find the real start of the message (Some menu headings have
    ** leading spaces which make the MEKA menu look weird). */
    for (startOffset = 0; messData[startOffset] == ' '; startOffset++) {}

    /* Create new menu and allocate space for MAX_MENU_SIZE items */
    the_menu[numOfMenus].text = strdup(messData + startOffset);
    the_menu[numOfMenus].proc = NULL;
    the_menu[numOfMenus].child =
        (struct MENU*)malloc(sizeof(struct MENU*) * MAX_MENU_SIZE);
    the_menu[numOfMenus].child[0].text = NULL;
    the_menu[numOfMenus].child[0].proc = NULL;
    the_menu[numOfMenus].child[0].child = NULL;

    numOfMenus++;

    /* Mark end of menu */
    the_menu[numOfMenus].text = NULL;
    the_menu[numOfMenus].proc = NULL;
    the_menu[numOfMenus].child = NULL;
}

void set_menu_item(byte** data) // 2, 0x00 
{
    int messNum, controllerNum, i;

    messNum = *(*data)++;
    controllerNum = *(*data)++;

    if (events[controllerNum].type == NO_EVENT) {
        events[controllerNum].type = MENU_EVENT;
    }
    events[controllerNum].activated = 0;

    i = 0; while (the_menu[numOfMenus - 1].child[i].text != NULL) i++;
    the_menu[numOfMenus - 1].child[i].text = strdup(logics[currentLog].data->messages[messNum - 1]);
    the_menu[numOfMenus - 1].child[i].proc = menuFunctions[controllerNum];
    the_menu[numOfMenus - 1].child[i].child = NULL;

    the_menu[numOfMenus - 1].child[i + 1].text = NULL;
    the_menu[numOfMenus - 1].child[i + 1].proc = NULL;
    the_menu[numOfMenus - 1].child[i + 1].child = NULL;
}

void submit_menu(byte** data) // 0, 0x00 
{

}

void enable_item(byte** data) // 1, 0x00 
{
    (*data)++;
}

void disable_item(byte** data) // 1, 0x00 
{
    (*data)++;
}

void menu_input(byte** data) // 0, 0x00 
{
    do_menu(the_menu, 10, 20);
}

void show_obj_v(byte** data) // 1, 0x01 
{
    int objectNum;

    objectNum = var[*(*data)++];
    /* Not supported yet */
}

void open_dialogue(byte** data) // 0, 0x00 
{

}

void close_dialogue(byte** data) // 0, 0x00 
{

}

void mul_n(byte** data) // 2, 0x80 
{
    var[*(*data)++] *= *(*data)++;
}

void mul_v(byte** data) // 2, 0xC0 
{
    var[*(*data)++] *= var[*(*data)++];
}

void div_n(byte** data) // 2, 0x80 
{
    var[*(*data)++] /= *(*data)++;
}

void div_v(byte** data) // 2, 0xC0 
{
    var[*(*data)++] /= var[*(*data)++];
}

void close_window(byte** data) // 0, 0x00 
{

}

#pragma code-name (pop)

byte getBankBasedOnCode(byte code)
{
    if (code <= HIGHEST_BANK1_FUNC)
    {
        return 1;
    }
    return RAM_BANK;
}

/***************************************************************************
** ifHandler
***************************************************************************/
void ifHandler(byte** data, byte codeBank)
{
    int ch;
    boolean stillProcessing = TRUE, testVal, notMode = FALSE, orMode = FALSE;
    byte b1, b2;
    short int disp, dummy;
    char debugString[80];
    byte previousBank = RAM_BANK;
    byte codeWindow[CODE_WINDOW_SIZE];
    byte* codeWindowAddress;
    byte** ppCodeWindowAddress;

    ppCodeWindowAddress = &codeWindowAddress;

    RAM_BANK = codeBank;
    while (stillProcessing) {
        ch = *(*data)++;

#ifdef DEBUG
        if (ch <= 18) {
            sprintf(debugString, "%s [%x]           ", testCommands[ch].commandName, ch);
            drawBigString(screen, debugString, 0, 400, 0, 7);
            if ((readkey() & 0xff) == 'q') closedown();
        }
#endif
        RAM_BANK = getBankBasedOnCode(ch);
        switch (ch) {
        case 0xff: /* Closing if bracket. Expression must be true. */
#ifdef DEBUG
            drawBigString(screen, "test is true             ", 0, 400, 0, 7);
            if ((readkey() & 0xff) == 'q') closedown();
#endif
            * data += 2;
            return;
        case 0xfd: /* Not mode toggle */
            notMode = (notMode ? FALSE : TRUE);
            break;
        case 0xfc:
            if (orMode) {
                /* If we have reached the closing OR bracket, then the
                ** test for the whole expression must be false. */
                stillProcessing = FALSE;
            }
            else {
                orMode = TRUE;
            }
            break;
        default:
            memcpy(&codeWindow[0], *data, CODE_WINDOW_SIZE);
            codeWindowAddress = &codeWindow[0];

            switch (ch) {
            case 0: testVal = FALSE; break; /* Should never happen */
            case 1: testVal = b1Equaln(ppCodeWindowAddress); break;
            case 2: testVal = b1Equalv(ppCodeWindowAddress); break;
            case 3: testVal = b1Lessn(ppCodeWindowAddress); break;
            case 4: testVal = b1Lessv(ppCodeWindowAddress); break;
            case 5: testVal = b1Greatern(ppCodeWindowAddress); break;
            case 6: testVal = b1Greaterv(ppCodeWindowAddress); break;
            case 7: testVal = b1Isset(ppCodeWindowAddress); break;
            case 8: testVal = b1Issetv(ppCodeWindowAddress); break;
            case 9: testVal = b1Has(ppCodeWindowAddress); break;
            case 10: testVal = b1Obj_in_room(ppCodeWindowAddress); break;
            case 11: testVal = b1Posn(ppCodeWindowAddress); break;
            case 12: testVal = b1Controller(ppCodeWindowAddress); break;
            case 13: testVal = b1Have_key(); break;
            case 14: testVal = said(ppCodeWindowAddress); break;
            case 15: testVal = b1Compare_strings(ppCodeWindowAddress); break;
            case 16: testVal = b1Obj_in_box(ppCodeWindowAddress); break;
            case 17: testVal = b1Center_posn(ppCodeWindowAddress); break;
            case 18: testVal = b1Right_posn(ppCodeWindowAddress); break;
            default:
                ////lprintf("catastrophe: Illegal test [%d], logic %d, posn %d.",
                    //ch, currentLog, logics[currentLog].currentPoint);
                testVal = FALSE;
                break; /* Should never happen */
            }
            RAM_BANK = previousBank;
#ifdef VERBOSE

            printf("Data was %p trying to add %p ", data, codeWindowAddress - &codeWindow[0]);
#endif // VERBOSE
            *data += (codeWindowAddress - &codeWindow[0]);
            
#ifdef VERBOSE
            printf("Data is %p %u \n", data, *data);
#endif
            if (notMode) testVal = (testVal ? FALSE : TRUE);
            notMode = 0;
            if (testVal) {
                if (orMode) {
                    /* Find the closing OR. It can't just search for 0xfc
                    ** because this could be a parameter for one of the test
                    ** commands rather than being the closing OR. We therefore
                    ** have to jump over each command as we find it. */
                    while (TRUE) {
                        ch = *(*data)++;
                        if (ch == 0xfc) break;
                        if (ch > 0xfc) continue;
                        if (ch == 0x0e) { /* said() has variable number of args */
                            ch = *(*data)++;
     
                            *data += (ch << 1);
                        }
                        else {
                            *data += testCommands[ch].numArgs;
                        }
                    }
                }
            }
            else {
                if (!orMode) stillProcessing = FALSE;
            }
            break;
        }

        RAM_BANK = previousBank;
    }

#ifdef DEBUG
    drawBigString(screen, "test is false            ", 0, 400, 0, 7);
    if ((readkey() & 0xff) == 'q') closedown();
#endif

    /* Test is false. */
    while (TRUE) {
        ch = *(*data)++;
        if (ch == 0xff) {
            b1 = *(*data)++;
            b2 = *(*data)++;
            disp = (b2 << 8) | b1;  /* Should be signed 16 bit */
            *data += disp;
            break;
        }
        if (ch >= 0xfc) continue;
        if (ch == 0x0e) {
            ch = *(*data)++;
            *data += (ch << 1);
        }
        else {
            *data += testCommands[ch].numArgs;
        }
    }
}

/***************************************************************************
** executeLogic
**
** Purpose: To execute the logic code for the logic with the given number.
***************************************************************************/
void executeLogic(int logNum)
{
    byte previousRamBank = RAM_BANK;
    boolean discardAfterward = FALSE, stillExecuting = TRUE;
    byte* code, * endPos, * startPos, b1, b2;
    byte codeAtTimeOfLastBankSwitch;
    LOGICEntry currentLogic;
    LOGICFile currentLogicFile;
    byte codeWindow[CODE_WINDOW_SIZE];
    byte* codeWindowAddress;
    byte** ppCodeWindowAddress;
    boolean lastCodeWasNonWindow = FALSE;
    
    //Temp
    int counter = 0;

    short int disp;
    char debugString[80];
    int i, dummy;

    currentLog = logNum;

    RAM_BANK = LOGIC_ENTRY_BANK;
    currentLogic = logics[logNum];
    
    RAM_BANK = LOGIC_FILE_BANK;
    currentLogicFile = *currentLogic.data;

#ifdef DEBUG
    sprintf(debugString, "LOGIC.%d:       ", currentLog);
    drawBigString(screen, debugString, 0, 384, 0, 7);
#endif
    /* Load logic file temporarily in order to execute it if the logic is
    ** not already in memory. */
    if (!currentLogic.loaded) {
        discardAfterward = TRUE;
        loadLogicFile(logNum);
    }
#ifdef DEBUG
    debugString[0] = 0;
    for (i = 0; i < 10; i++)
        sprintf(debugString, "%s %x", debugString, currentLogicFile->logicCode[i]);
    drawBigString(screen, debugString, 0, 416, 0, 7);
#endif
    /* Set up position to start executing code from. */
    //currentLogic.currentPoint = currentLogic.entryPoint;
    startPos = currentLogicFile.logicCode;
    code = startPos + currentLogic.entryPoint;
    endPos = startPos + currentLogicFile.codeSize;

#ifdef DEBUG
    drawBigString(screen, "Push a key to advance a step", 0, 400, 0, 7);
    if ((readkey() & 0xff) == 'q') closedown();
#endif

    RAM_BANK = currentLogicFile.codeBank;

    while ((code < endPos) && stillExecuting) {
        
        if (counter == 2)
        {
#ifdef VERBOSE
            printf("The code is now %u and the address is %p", *code, code);
            exit(0);
#endif // VERBOSE
        }

        memcpy(&codeWindow[0], code, CODE_WINDOW_SIZE);

        codeWindowAddress = &codeWindow[0] + 1;
        ppCodeWindowAddress = &codeWindowAddress;

        /* Emergency exit */
        if (key[KEY_F12]) {
            ////lprintf("info: Exiting MEKA due to F12, logic: %d, posn: %d",
                //logNum, currentLogic.currentPoint);
            exit(0);
        }

        currentLogic.currentPoint = (code - startPos);
#ifdef DEBUG
        debugString[0] = 0;
        for (i = 0; i < 10; i++)
            sprintf(debugString, "%s %x", debugString, code[i]);
        drawBigString(screen, debugString, 0, 416, 0, 7);

        if (*code < 0xfc) {
            sprintf(debugString, "(%d) %s [%x]           ", currentLogic.currentPoint, agiCommands[*code].commandName, *code);
            drawBigString(screen, debugString, 0, 400, 0, 7);
            if ((readkey() & 0xff) == 'q') closedown();
        }
#endif       
        printf("\n The code is %d, on bank %d address, %p", *code, RAM_BANK, code);
        codeAtTimeOfLastBankSwitch = *code;
        RAM_BANK = getBankBasedOnCode(codeAtTimeOfLastBankSwitch);
#ifdef VERBOSE
        printf("Bank is now %d to execute code %d", RAM_BANK, *code);
#endif // VERBOSE

        code++;
        switch (codeAtTimeOfLastBankSwitch) {
        case 0: /* return */
            stillExecuting = FALSE;
            break;
        case 1: b1Increment(ppCodeWindowAddress); break;
        case 2: b1Decrement(ppCodeWindowAddress); break;
        case 3: b1Assignn(ppCodeWindowAddress); break;
        case 4: b1Assignv(ppCodeWindowAddress); break;
        case 5: b1Addn(ppCodeWindowAddress); break;
        case 6: b1Addv(ppCodeWindowAddress); break;
        case 7: b1Subn(ppCodeWindowAddress); break;
        case 8: b1Subv(ppCodeWindowAddress); break;
        case 9: b1Lindirectv(ppCodeWindowAddress); break;
        case 10: b1Rindirect(ppCodeWindowAddress); break;
        case 11: b1Lindirectn(ppCodeWindowAddress); break;
        case 12: b1Set(ppCodeWindowAddress); break;
        case 13: b1Reset(ppCodeWindowAddress); break;
        case 14: b1Toggle(ppCodeWindowAddress); break;
        case 15: b1Set_v(ppCodeWindowAddress); break;
        case 16: b1Reset_v(ppCodeWindowAddress); break;
        case 17: b1Toggle_v(ppCodeWindowAddress); break;
        case 18:
            b1New_room(ppCodeWindowAddress);
            exitAllLogics = TRUE;
            stillExecuting = FALSE;
            break;
        case 19:
            b1New_room_v(ppCodeWindowAddress);
            exitAllLogics = TRUE;
            stillExecuting = FALSE;
            break;
        case 20: b1Load_logics(ppCodeWindowAddress); break;
        case 21: b1Load_logics_v(ppCodeWindowAddress); break;
        case 22:
            b1Call(ppCodeWindowAddress);
            /* The currentLog variable needs to be restored */
            currentLog = logNum;
            if (exitAllLogics) stillExecuting = FALSE;
#ifdef DEBUG
            sprintf(debugString, "LOGIC.%d:       ", currentLog);
            drawBigString(screen, debugString, 0, 384, 0, 7);
#endif
            break;
        case 23:
            b1Call_v(ppCodeWindowAddress);
            /* The currentLog variable needs to be restored */
            currentLog = logNum;
            if (exitAllLogics) stillExecuting = FALSE;
#ifdef DEBUG
            sprintf(debugString, "LOGIC.%d:       ", currentLog);
            drawBigString(screen, debugString, 0, 384, 0, 7);
#endif
            break;
        case 24: b1Load_pic(ppCodeWindowAddress); break;
        case 25: b1Draw_pic(ppCodeWindowAddress); break;
        case 26: b1Show_pic(ppCodeWindowAddress); break;
        case 27: b1Discard_pic(ppCodeWindowAddress); break;
        case 28: b1Overlay_pic(ppCodeWindowAddress); break;
        case 29: b1Show_pri_screen(ppCodeWindowAddress); break;
        case 30: b1Load_view(ppCodeWindowAddress); break;
        case 31: b1Load_view_v(ppCodeWindowAddress); break;
        case 32: b1Discard_view(ppCodeWindowAddress); break;
        case 33: b1Animate_obj(ppCodeWindowAddress); break;
        case 34: b1Unanimate_all(ppCodeWindowAddress); break;
        case 35: b1Draw(ppCodeWindowAddress); break;
        case 36: b1Erase(ppCodeWindowAddress); break;
        case 37: b1Position(ppCodeWindowAddress); break;
        case 38: b1Position_v(ppCodeWindowAddress); break;
        case 39: b1Get_posn(ppCodeWindowAddress); break;
        case 40: b1Reposition(ppCodeWindowAddress); break;
        case 41: set_view(ppCodeWindowAddress); break;
        case 42: set_view_v(ppCodeWindowAddress); break;
        case 43: set_loop(ppCodeWindowAddress); break;
        case 44: set_loop_v(ppCodeWindowAddress); break;
        case 45: fix_loop(ppCodeWindowAddress); break;
        case 46: release_loop(ppCodeWindowAddress); break;
        case 47: set_cel(ppCodeWindowAddress); break;
        case 48: set_cel_v(ppCodeWindowAddress); break;
        case 49: last_cel(ppCodeWindowAddress); break;
        case 50: current_cel(ppCodeWindowAddress); break;
        case 51: current_loop(ppCodeWindowAddress); break;
        case 52: current_view(ppCodeWindowAddress); break;
        case 53: number_of_loops(ppCodeWindowAddress); break;
        case 54: set_priority(ppCodeWindowAddress); break;
        case 55: set_priority_v(ppCodeWindowAddress); break;
        case 56: release_priority(ppCodeWindowAddress); break;
        case 57: get_priority(ppCodeWindowAddress); break;
        case 58: stop_update(ppCodeWindowAddress); break;
        case 59: start_update(ppCodeWindowAddress); break;
        case 60: force_update(ppCodeWindowAddress); break;
        case 61: ignore_horizon(ppCodeWindowAddress); break;
        case 62: observe_horizon(ppCodeWindowAddress); break;
        case 63: set_horizon(ppCodeWindowAddress); break;
        case 64: object_on_water(ppCodeWindowAddress); break;
        case 65: object_on_land(ppCodeWindowAddress); break;
        case 66: object_on_anything(ppCodeWindowAddress); break;
        case 67: ignore_objs(ppCodeWindowAddress); break;
        case 68: observe_objs(ppCodeWindowAddress); break;
        case 69: distance(ppCodeWindowAddress); break;
        case 70: stop_cycling(ppCodeWindowAddress); break;
        case 71: start_cycling(ppCodeWindowAddress); break;
        case 72: normal_cycle(ppCodeWindowAddress); break;
        case 73: end_of_loop(ppCodeWindowAddress); break;
        case 74: reverse_cycle(ppCodeWindowAddress); break;
        case 75: reverse_loop(ppCodeWindowAddress); break;
        case 76: cycle_time(ppCodeWindowAddress); break;
        case 77: stop_motion(ppCodeWindowAddress); break;
        case 78: start_motion(ppCodeWindowAddress); break;
        case 79: step_size(ppCodeWindowAddress); break;
        case 80: step_time(ppCodeWindowAddress); break;
        case 81: move_obj(ppCodeWindowAddress); break;
        case 82: move_obj_v(ppCodeWindowAddress); break;
        case 83: follow_ego(ppCodeWindowAddress); break;
        case 84: wander(ppCodeWindowAddress); break;
        case 85: normal_motion(ppCodeWindowAddress); break;
        case 86: set_dir(ppCodeWindowAddress); break;
        case 87: get_dir(ppCodeWindowAddress); break;
        case 88: ignore_blocks(ppCodeWindowAddress); break;
        case 89: observe_blocks(ppCodeWindowAddress); break;
        case 90: block(ppCodeWindowAddress); break;
        case 91: unblock(ppCodeWindowAddress); break;
        case 92: get(ppCodeWindowAddress); break;
        case 93: get_v(ppCodeWindowAddress); break;
        case 94: drop(ppCodeWindowAddress); break;
        case 95: put(ppCodeWindowAddress); break;
        case 96: put_v(ppCodeWindowAddress); break;
        case 97: get_room_v(ppCodeWindowAddress); break;
        case 98: load_sound(ppCodeWindowAddress); break;
        case 99: play_sound(ppCodeWindowAddress); break;
        case 100: stop_sound(ppCodeWindowAddress); break;
        case 101: print(ppCodeWindowAddress); break;
        case 102: print_v(ppCodeWindowAddress); break;
        case 103: display(ppCodeWindowAddress); break;
        case 104: display_v(ppCodeWindowAddress); break;
        case 105: clear_lines(ppCodeWindowAddress); break;
        case 106: text_screen(ppCodeWindowAddress); break;
        case 107: graphics(ppCodeWindowAddress); break;
        case 108: set_cursor_char(ppCodeWindowAddress); break;
        case 109: set_text_attribute(ppCodeWindowAddress); break;
        case 110: shake_screen(ppCodeWindowAddress); break;
        case 111: configure_screen(ppCodeWindowAddress); break;
        case 112: status_line_on(ppCodeWindowAddress); break;
        case 113: status_line_off(ppCodeWindowAddress); break;
        case 114: set_string(ppCodeWindowAddress); break;
        case 115: get_string(ppCodeWindowAddress); break;
        case 116: word_to_string(ppCodeWindowAddress); break;
        case 117: parse(ppCodeWindowAddress); break;
        case 118: get_num(ppCodeWindowAddress); break;
        case 119: prevent_input(ppCodeWindowAddress); break;
        case 120: accept_input(ppCodeWindowAddress); break;
        case 121: set_key(ppCodeWindowAddress); break;
        case 122: add_to_pic(ppCodeWindowAddress); break;
        case 123: add_to_pic_v(ppCodeWindowAddress); break;
        case 124: status(ppCodeWindowAddress); break;
        case 125: save_game(ppCodeWindowAddress); break;
        case 126: restore_game(ppCodeWindowAddress); break;
        case 127: init_disk(ppCodeWindowAddress); break;
        case 128: restart_game(ppCodeWindowAddress); break;
        case 129: show_obj(ppCodeWindowAddress); break;
        case 130: random_num(ppCodeWindowAddress); break;
        case 131: program_control(ppCodeWindowAddress); break;
        case 132: player_control(ppCodeWindowAddress); break;
        case 133: obj_status_v(ppCodeWindowAddress); break;
        case 134: quit(ppCodeWindowAddress); break;
        case 135: show_mem(ppCodeWindowAddress); break;
        case 136: pause(ppCodeWindowAddress); break;
        case 137: echo_line(ppCodeWindowAddress); break;
        case 138: cancel_line(ppCodeWindowAddress); break;
        case 139: init_joy(ppCodeWindowAddress); break;
        case 140: toggle_monitor(ppCodeWindowAddress); break;
        case 141: version(ppCodeWindowAddress); break;
        case 142: script_size(ppCodeWindowAddress); break;
        case 143: set_game_id(ppCodeWindowAddress); break;
        case 144: log(ppCodeWindowAddress); break;
        case 145: set_scan_start(ppCodeWindowAddress); break;
        case 146: reset_scan_start(ppCodeWindowAddress); break;
        case 147: reposition_to(ppCodeWindowAddress); break;
        case 148: reposition_to_v(ppCodeWindowAddress); break;
        case 149: trace_on(ppCodeWindowAddress); break;
        case 150: trace_info(ppCodeWindowAddress); break;
        case 151: print_at(ppCodeWindowAddress); break;
        case 152: print_at_v(ppCodeWindowAddress); break;
        case 153: discard_view_v(ppCodeWindowAddress); break;
        case 154: clear_text_rect(ppCodeWindowAddress); break;
        case 155: set_upper_left(ppCodeWindowAddress); break;
        case 156: set_menu(ppCodeWindowAddress); break;
        case 157: set_menu_item(ppCodeWindowAddress); break;
        case 158: submit_menu(ppCodeWindowAddress); break;
        case 159: enable_item(ppCodeWindowAddress); break;
        case 160: disable_item(ppCodeWindowAddress); break;
        case 161: menu_input(ppCodeWindowAddress); break;
        case 162: show_obj_v(ppCodeWindowAddress); break;
        case 163: open_dialogue(ppCodeWindowAddress); break;
        case 164: close_dialogue(ppCodeWindowAddress); break;
        case 165: mul_n(ppCodeWindowAddress); break;
        case 166: mul_v(ppCodeWindowAddress); break;
        case 167: div_n(ppCodeWindowAddress); break;
        case 168: div_v(ppCodeWindowAddress); break;
        case 169: close_window(ppCodeWindowAddress); break;
        case 170:  break;
        case 171:  break;
        case 172:  break;
        case 173:  break;
        case 174:  break;
        case 175:  break;
        case 176:  break;
        case 177:  break;
        case 178:  break;
        case 179:  break;
        case 180:  break;
        case 181:  break;

        case 0xfe: /* Unconditional branch: else, goto. */
#ifdef DEBUG
            sprintf(debugString, "(%d) else                           ", currentLogic.currentPoint);
            drawBigString(screen, debugString, 0, 400, 0, 7);
            if ((readkey() & 0xff) == 'q') closedown();
#endif
            lastCodeWasNonWindow = TRUE;
            b1 = *code++;
            b2 = *code++;
            disp = (b2 << 8) | b1;  /* Should be signed 16 bit */
            code += disp;
            break;

        case 0xff: /* Conditional branch: if */
#ifdef DEBUG
            sprintf(debugString, "(%d) if                             ", currentLogic.currentPoint);
            drawBigString(screen, debugString, 0, 400, 0, 7);
            if ((readkey() & 0xff) == 'q') closedown();
#endif
            lastCodeWasNonWindow = TRUE;
            ifHandler(&code, currentLogicFile.codeBank);
            break;

        default:    /* Error has occurred */
            ////lprintf("catastrophe: Illegal action [%d], logic %d, posn %d.",
                //*(code - 1), logNum, currentLogic.currentPoint);
            break;
        }
        RAM_BANK = currentLogicFile.codeBank;

        if (!lastCodeWasNonWindow)
        {
            code += (codeWindowAddress - &codeWindow[0] + 1);
        }
        lastCodeWasNonWindow = FALSE;

        counter++;
    }

    if (discardAfterward) discardLogicFile(logNum);

    RAM_BANK = previousRamBank;
}


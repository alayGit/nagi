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
#include "helpers.h"

#define HIGHEST_BANK1_FUNC 40
#define HIGHEST_BANK2_FUNC 100
#define HIGHEST_BANK3_FUNC 117
#define HIGHEST_BANK4_FUNC 181


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
void b4FreeMenuItems();

#pragma code-name (push, "BANKRAM01");
/****************************************************************************
** addLogLine
****************************************************************************/
void b1AddLogLine(char* message)
{
    FILE* fp;

    if ((fp = fopen("log.txt", "a")) == NULL) {
#ifdef VERBOSE
        fprintf(stderr, "Error opening log file.");
#endif // VERBOSE
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

void b2Set_view(byte** data) // 2, 0x00 
{
    int entryNum, viewNum;

    entryNum = *(*data)++;
    viewNum = *(*data)++;
    addViewToTable(entryNum, viewNum);
}

void b2Set_view_v(byte** data) // 2, 0x40 
{
    int entryNum, viewNum;

    entryNum = *(*data)++;
    viewNum = var[*(*data)++];
    addViewToTable(entryNum, viewNum);
}

void b2Set_loop(byte** data) // 2, 0x00 
{
    int entryNum, loopNum;

    entryNum = *(*data)++;
    loopNum = *(*data)++;
    setLoop(entryNum, loopNum);
    setCel(entryNum, 0);
}

void b2Set_loop_v(byte** data) // 2, 0x40 
{
    int entryNum, loopNum;

    entryNum = *(*data)++;
    loopNum = var[*(*data)++];
    setLoop(entryNum, loopNum);
    setCel(entryNum, 0);
}

void b2Fix_loop(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= FIXLOOP;
}

void b2Release_loop(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~FIXLOOP;
}

void b2Set_cel(byte** data) // 2, 0x00 
{
    int entryNum, celNum;

    entryNum = *(*data)++;
    celNum = *(*data)++;
    setCel(entryNum, celNum);
}

void b2Set_cel_v(byte** data) // 2, 0x40 
{
    int entryNum, celNum;

    entryNum = *(*data)++;
    celNum = var[*(*data)++];
    setCel(entryNum, celNum);
}

void b2Last_cel(byte** data) // 2, 0x40 
{
    int entryNum, varNum;

    entryNum = *(*data)++;
    varNum = *(*data)++;
    var[varNum] = viewtab[entryNum].numberOfCels - 1;
}

void b2Current_cel(byte** data) // 2, 0x40 
{
    int entryNum, varNum;

    entryNum = *(*data)++;
    varNum = *(*data)++;
    var[varNum] = viewtab[entryNum].currentCel;
}

void b2Current_loop(byte** data) // 2, 0x40 
{
    int entryNum, varNum;

    entryNum = *(*data)++;
    varNum = *(*data)++;
    var[varNum] = viewtab[entryNum].currentLoop;
}

void b2Current_view(byte** data) // 2, 0x40 
{
    int entryNum, varNum;

    entryNum = *(*data)++;
    varNum = *(*data)++;
    var[varNum] = viewtab[entryNum].currentView;
}

void b2Number_of_loops(byte** data) // 2, 0x40 
{
    int entryNum, varNum;

    entryNum = *(*data)++;
    varNum = *(*data)++;
    var[varNum] = viewtab[entryNum].numberOfLoops;
}

void b2Set_priority(byte** data) // 2, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].priority = *(*data)++;
    viewtab[entryNum].flags |= FIXEDPRIORITY;
}

void b2Set_priority_v(byte** data) // 2, 0x40 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].priority = var[*(*data)++];
    viewtab[entryNum].flags |= FIXEDPRIORITY;
}

void b2Release_priority(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~FIXEDPRIORITY;
}

void b2Get_priority(byte** data) // 2, 0x40 
{
    int entryNum, varNum;

    entryNum = *(*data)++;
    varNum = *(*data)++;
    var[varNum] = viewtab[entryNum].priority;
}

void b2Stop_update(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~UPDATE;
}

void b2Start_update(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= UPDATE;
}

void b2Force_update(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    /* Do immediate update here. Call update(entryNum) */
    updateObj(entryNum);
}

void b2Ignore_horizon(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= IGNOREHORIZON;
}

void b2Observe_horizon(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~IGNOREHORIZON;
}

void b2Set_horizon(byte** data) // 1, 0x00 
{
    horizon = *(*data)++;
}

void b2Object_on_water(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= ONWATER;
}

void b2Object_on_land(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= ONLAND;
}

void b2Object_on_anything(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~(ONWATER | ONLAND);
}

void b2Ignore_objs(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= IGNOREOBJECTS;
}

void b2Observe_objs(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~IGNOREOBJECTS;
}


void b2Distance(byte** data) // 3, 0x20 
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

void b2Stop_cycling(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~CYCLING;
}

void b2Start_cycling(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= CYCLING;
}

void b2Normal_cycle(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].cycleStatus = 0;
}

void b2End_of_loop(byte** data) // 2, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].param1 = *(*data)++;
    viewtab[entryNum].cycleStatus = 1;
    viewtab[entryNum].flags |= (UPDATE | CYCLING);
}

void b2Reverse_cycle(byte** data) // 1, 0x00
{
    int entryNum;

    entryNum = *(*data)++;
    /* Store the other parameters here */

    viewtab[entryNum].cycleStatus = 3;
}

void b2Reverse_loop(byte** data) // 2, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].param1 = *(*data)++;
    viewtab[entryNum].cycleStatus = 2;
    viewtab[entryNum].flags |= (UPDATE | CYCLING);
}

void b2Cycle_time(byte** data) // 2, 0x40 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].cycleTime = var[*(*data)++];
}

void b2Stop_motion(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~MOTION;
    viewtab[entryNum].direction = 0;
    viewtab[entryNum].motion = 0;
}

void b2Start_motion(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= MOTION;
    viewtab[entryNum].motion = 0;        /* Not sure about this */
}

void b2Step_size(byte** data) // 2, 0x40 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].stepSize = var[*(*data)++];
}

void b2Step_time(byte** data) // 2, 0x40 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].stepTime = var[*(*data)++];
}

void b2Move_obj(byte** data) // 5, 0x00 
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

void b2Move_obj_v(byte** data) // 5, 0x70 
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

void b2Follow_ego(byte** data) // 3, 0x00 
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

void b2Wander(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].motion = 1;
    viewtab[entryNum].flags |= MOTION;
}

void b2Normal_motion(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].motion = 0;
    viewtab[entryNum].flags |= MOTION;
}

void b2Set_dir(byte** data) // 2, 0x40 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].direction = var[*(*data)++];
}

void b2Get_dir(byte** data) // 2, 0x40 
{
    int entryNum;

    entryNum = *(*data)++;
    var[*(*data)++] = viewtab[entryNum].direction;
}

void b2Ignore_blocks(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags |= IGNOREBLOCKS;
}

void b2Observe_blocks(byte** data) // 1, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].flags &= ~IGNOREBLOCKS;
}

void b2Block(byte** data) // 4, 0x00 
{
    /* Is this used anywhere? - Not implemented at this stage */
    *data += 4;
}

void b2Unblock(byte** data) // 0, 0x00 
{

}

void b2Get(byte** data) // 1, 00 
{
    objects[*(*data)++].roomNum = 255;
}

void b2Get_v(byte** data) // 1, 0x80 
{
    objects[var[*(*data)++]].roomNum = 255;
}

void b2Drop(byte** data) // 1, 0x00 
{
    objects[*(*data)++].roomNum = 0;
}

void b2Put(byte** data) // 2, 0x00 
{
    int objNum, room;

    objNum = *(*data)++;
    room = *(*data)++;
    objects[objNum].roomNum = room;
}

void b2Put_v(byte** data) // 2, 0x40 
{
    int objNum, room;

    objNum = *(*data)++;
    room = var[*(*data)++];
    objects[objNum].roomNum = room;
}

void b2Get_room_v(byte** data) // 2, 0xC0 
{
    int objNum, room;

    objNum = var[*(*data)++];
    var[*(*data)++] = objects[objNum].roomNum;
}

void b2Load_sound(byte** data) // 1, 0x00 
{
    int soundNum;

    soundNum = *(*data)++;
    loadSoundFile(soundNum);
}

void b2Play_sound(byte** data) // 2, 00  sound() renamed to avoid clash
{
    int soundNum;

    soundNum = *(*data)++;
    soundEndFlag = *(*data)++;
    /* playSound(soundNum); */
    flag[soundEndFlag] = TRUE;
}

void b2Stop_sound(byte** data) // 0, 0x00 
{
    checkForEnd = FALSE;
    stop_midi();
}

#pragma code-name (pop)
#pragma code-name (push, "BANKRAM03")

int b3GetNum(char* inputString, int* i)
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

boolean b3CharIsIn(char testChar, char* testString)
{
    int i;

    for (i = 0; i < strlen(testString); i++) {
        if (testString[i] == testChar) return TRUE;
    }

    return FALSE;
}

void b3ProcessString(char* inputString, char* outputString)
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
                tempNum = b3GetNum(inputString, &i);
                if (inputString[i + 1] == '|') {
                    i += 2;
                    widthNum = b3GetNum(inputString, &i);
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
                tempNum = b3GetNum(inputString, &i);
                sprintf(outputString, "%s%s", outputString,
                    logics[currentLog].data->messages[tempNum - 1]);
                break;
            case 'g':
                tempNum = b3GetNum(inputString, &i);
                sprintf(outputString, "%s%s", outputString, logics[0].data->messages[tempNum - 1]);
                break;
            case 'w':
                tempNum = b3GetNum(inputString, &i);
                sprintf(outputString, "%s%s", outputString, wordText[tempNum]);
                break;
            case 's':
                tempNum = b3GetNum(inputString, &i);
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
    if (b3CharIsIn('%', outputString)) {
        strcpy(temp, outputString);
        b3ProcessString(temp, outputString);
    }

    free(numString);
    free(temp);
}

void b3Print(byte** data) // 1, 00 
{
    char* tempString = (char*)malloc(256);
    BITMAP* temp;

    show_mouse(NULL);
    temp = create_bitmap(640, 336);
    blit(agi_screen, temp, 0, 0, 0, 0, 640, 336);
    show_mouse(screen);
    while (key[KEY_ENTER] || key[KEY_ESC]) { /* Wait */ }
    b3ProcessString(logics[currentLog].data->messages[(*(*data)++) - 1], tempString);
    printInBoxBig(tempString, -1, -1, 30);
    while (!key[KEY_ENTER] && !key[KEY_ESC]) { /* Wait */ }
    while (key[KEY_ENTER] || key[KEY_ESC]) { clear_keybuf(); }
    show_mouse(NULL);
    blit(temp, agi_screen, 0, 0, 0, 0, 640, 336);
    show_mouse(screen);
    destroy_bitmap(temp);

    free(tempString);
}

void b3Print_v(byte** data) // 1, 0x80 
{
    char* tempString = (char*)malloc(256);
    BITMAP* temp;

    show_mouse(NULL);
    temp = create_bitmap(640, 336);
    blit(agi_screen, temp, 0, 0, 0, 0, 640, 336);
    while (key[KEY_ENTER] || key[KEY_ESC]) { /* Wait */ }
    b3ProcessString(logics[currentLog].data->messages[(var[*(*data)++]) - 1], tempString);
    //printf("Warning Print In Bigbox Not Implemented Implement This");
    //printInBoxBig2(tempString, -1, -1, 30);
    while (!key[KEY_ENTER] && !key[KEY_ESC]) { /* Wait */ }
    while (key[KEY_ENTER] || key[KEY_ESC]) { clear_keybuf(); }
    blit(temp, agi_screen, 0, 0, 0, 0, 640, 336);
    show_mouse(screen);
    destroy_bitmap(temp);

    free(tempString);
}

void b3Display(byte** data) // 3, 0x00 
{
    int row, col, messNum;
    char* tempString = malloc(256);

    col = *(*data)++;
    row = *(*data)++;
    messNum = *(*data)++;
    b3ProcessString(logics[currentLog].data->messages[messNum - 1], tempString);
    drawBigString(screen, tempString, row * 16, 20 + (col * 16), agi_fg, agi_bg);
    /*lprintf("info: display() %s, fg: %d bg: %d row: %d col: %d",
       tempString, agi_fg, agi_bg, row, col);*/

    free(tempString);
}

void b3Display_v(byte** data) // 3, 0xE0 
{
    int row, col, messNum;
    char* tempString = (char*)malloc(256);

    col = var[*(*data)++];
    row = var[*(*data)++];
    messNum = var[*(*data)++];
    //drawString(picture, logics[currentLog].data->messages[messNum-1],
    //   row*8, col*8, agi_fg, agi_bg);
    b3ProcessString(logics[currentLog].data->messages[messNum - 1], tempString);
    drawBigString(screen, tempString, row * 16, 20 + (col * 16), agi_fg, agi_bg);
    /*lprintf("info: display.v() %s, foreground: %d background: %d",
       tempString, agi_fg, agi_bg);*/

    free(tempString);
}

void b3Clear_lines(byte** data) // 3, 0x00 
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

void b3Text_screen(byte** data) // 0, 0x00 
{
    screenMode = AGI_TEXT;
    /* Do something else here */
    inputLineDisplayed = FALSE;
    statusLineDisplayed = FALSE;
    clear(screen);
}

void b3Graphics(byte** data) // 0, 0x00 
{
    screenMode = AGI_GRAPHICS;
    /* Do something else here */
    inputLineDisplayed = TRUE;
    statusLineDisplayed = TRUE;
    okToShowPic = TRUE;
    clear(screen);
}

void b3Set_cursor_char(byte** data) // 1, 0x00 
{
    char* temp = (char*)malloc(256);

    b3ProcessString(logics[currentLog].data->messages[(*(*data)++) - 1], temp);
    cursorChar = temp[0];

    free(temp);
}

void b3Set_text_attribute(byte** data) // 2, 0x00 
{
    agi_fg = (*(*data)++) + 1;
    agi_bg = (*(*data)++) + 1;
}

void b3Shake_screen(byte** data) // 1, 0x00 
{
    (*data)++;  /* Ignore this for now. */
}

void b3Configure_screen(byte** data) // 3, 0x00 
{
    min_print_line = *(*data)++;
    user_input_line = *(*data)++;
    status_line_num = *(*data)++;
}

void b3Status_line_on(byte** data) // 0, 0x00 
{
    statusLineDisplayed = TRUE;
}

void b3Status_line_off(byte** data) // 0, 0x00 
{
    statusLineDisplayed = FALSE;
}

void b3Set_string(byte** data) // 2, 0x00 
{
    int stringNum, messNum;

    stringNum = *(*data)++;
    messNum = *(*data)++;
    strcpy(string[stringNum], logics[currentLog].data->messages[messNum - 1]);
}

void b3Get_string(byte** data) // 5, 0x00 
{
    int strNum, messNum, row, col, l;

    strNum = *(*data)++;
    messNum = *(*data)++;
    col = *(*data)++;
    row = *(*data)++;
    l = *(*data)++;
    getString(logics[currentLog].data->messages[messNum - 1], string[strNum], row, col, l);
}

void b3Word_to_string(byte** data) // 2, 0x00 
{
    int stringNum, wordNum;

    stringNum = *(*data)++;
    wordNum = *(*data)++;
    strcpy(string[stringNum], wordText[wordNum]);
}

void b3Parse(byte** data) // 1, 0x00 
{
    int stringNum;

    stringNum = *(*data)++;
    lookupWords(string[stringNum]);
}

#pragma code-name (pop)
#pragma code-name (push, "BANKRAM04")

void b4Get_num(byte** data) // 2, 0x40 
{
    int messNum, varNum;
    char temp[80];

    messNum = *(*data)++;
    varNum = *(*data)++;
    getString(logics[currentLog].data->messages[messNum - 1], temp, 1, 23, 3);
    var[varNum] = atoi(temp);
}

void b4Prevent_input(byte** data) // 0, 0x00 
{
    inputLineDisplayed = FALSE;
    /* Do something else here */
}

void b4Accept_input(byte** data) // 0, 0x00 
{
    inputLineDisplayed = TRUE;
    /* Do something else here */
}

void b4Set_key(byte** data) // 3, 0x00 
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

void b4Add_to_pic(byte** data) // 7, 0x00 
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

void b4Add_to_pic_v(byte** data) // 7, 0xFE 
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

void b4Status(byte** data) // 0, 0x00 
{
    /* Inventory */
    // set text mode
    // if flag 13 is set then allow selection and store selection in var[25]
    var[25] = 255;
}

void b4Save_game(byte** data) // 0, 0x00 
{
    /* Not supported yet */
}

void b4Restore_game(byte** data) // 0, 0x00 
{
    /* Not supported yet */
}

void b4Restart_game(byte** data) // 0, 0x00 
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
    b4FreeMenuItems();
}

void b4Show_obj(byte** data) // 1, 0x00 
{
    int objectNum;

    objectNum = *(*data)++;
    /* Not supported yet */
}

void b4Random_num(byte** data) // 3, 0x20  random() renamed to avoid clash
{
    int startValue, endValue;

    startValue = *(*data)++;
    endValue = *(*data)++;
    var[*(*data)++] = (rand() % ((endValue - startValue) + 1)) + startValue;
}

void b4Program_control(byte** data) // 0, 0x00 
{
    controlMode = PROGRAM_CONTROL;
}

void b4Player_control(byte** data) // 0, 0x00 
{
    controlMode = PLAYER_CONTROL;
}

void b4Obj_status_v(byte** data) // 1, 0x80 
{
    int objectNum;

    objectNum = var[*(*data)++];
    /* Not supported yet */

    /* showView(viewtab[objectNum].currentView); */
    showObjectState(objectNum);
}

void b4Quit(byte** data) // 1, 0x00                     /* 0 args for AGI version 2_089 */
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

void b4Pause(byte** data) // 0, 0x00 
{
    while (key[KEY_ENTER]) { /* Wait */ }
    printInBoxBig("      Game paused.\nPress ENTER to continue.", -1, -1, 30);
    while (!key[KEY_ENTER]) { /* Wait */ }
    showPicture();
    okToShowPic = TRUE;
}

void b4Echo_line(byte** data) // 0, 0x00 
{

}

void b4Cancel_line(byte** data) // 0, 0x00 
{
    /*currentInputStr[0]=0;
    strPos=0;*/
}

void b4Init_joy(byte** data) // 0, 0x00 
{
    /* Not important at this stage */
}

void b4Version(byte** data) // 0, 0x00 
{
    while (key[KEY_ENTER] || key[KEY_ESC]) { /* Wait */ }
    printInBoxBig("MEKA AGI Interpreter\n    Version 1.0", -1, -1, 30);
    while (!key[KEY_ENTER] && !key[KEY_ESC]) { /* Wait */ }
    showPicture();
    okToShowPic = TRUE;
}

void b4Script_size(byte** data) // 1, 0x00 
{
    printf("The script size is %d", **data);
    (*data)++;  /* Ignore the script size. Not important for this interpreter */
}

void b4Set_game_id(byte** data) // 1, 0x00 
{
    (*data)++;  /* Ignore the game ID. Not important */
}

void b4Log(byte** data) // 1, 0x00 
{
    (*data)++;  /* Ignore log message. Not important */
}

void b4Set_scan_start(byte** data) // 0, 0x00 
{
    /* currentPoint is set in executeLogic() */
    logics[currentLog].entryPoint = logics[currentLog].currentPoint + 1;
    /* Does it return() at this point, or does it execute to the end?? */
}

void b4Reset_scan_start(byte** data) // 0, 0x00 
{
    logics[currentLog].entryPoint = 0;
}

void b4Reposition_to(byte** data) // 3, 0x00 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].xPos = *(*data)++;
    viewtab[entryNum].yPos = *(*data)++;
}

void b4Reposition_to_v(byte** data) // 3, 0x60 
{
    int entryNum;

    entryNum = *(*data)++;
    viewtab[entryNum].xPos = var[*(*data)++];
    viewtab[entryNum].yPos = var[*(*data)++];
}

void b4Trace_on(byte** data) // 0, 0x00 
{
    /* Ignore at this stage */
}

void b4Trace_info(byte** data) // 3, 0x00 
{
    *data += 3;  /* Ignore trace information at this stage. */
}

void b4Print_at(byte** data) // 4, 0x00           /* 3 args for AGI versions before */
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
    b3ProcessString(logics[currentLog].data->messages[messNum - 1], tempString);
    printInBoxBig(tempString, x, y, l);
    while (!key[KEY_ENTER] && !key[KEY_ESC]) { /* Wait */ }
    while (key[KEY_ENTER] || key[KEY_ESC]) { clear_keybuf(); }
    show_mouse(NULL);
    blit(temp, agi_screen, 0, 0, 0, 0, 640, 336);
    show_mouse(screen);
    destroy_bitmap(temp);

    free(tempString);
}

void b4Print_at_v(byte** data) // 4, 0x80         /* 2_440 (maybe laterz) */
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
    b3ProcessString(logics[currentLog].data->messages[messNum - 1], tempString);
    printInBoxBig(tempString, x, y, l);
    while (!key[KEY_ENTER] && !key[KEY_ESC]) { /* Wait */ }
    while (key[KEY_ENTER] || key[KEY_ESC]) { clear_keybuf(); }
    show_mouse(NULL);
    blit(temp, agi_screen, 0, 0, 0, 0, 640, 336);
    show_mouse(screen);
    destroy_bitmap(temp);

    free(tempString);
}

void b4Discard_view_v(byte** data) // 1, 0x80 
{
    discardView(var[*(*data)++]);
}

void b4Clear_text_rect(byte** data) // 5, 0x00 
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

void b4Set_upper_left(byte** data) // 2, 0x00    (x, y) ??
{
    *data += 2;
}

void b4WaitKeyRelease()
{
    while (keypressed()) { /* Wait */ }
}

int menuEvent0() { b4WaitKeyRelease(); events[0].activated = 1; return D_O_K; }
int menuEvent1() { b4WaitKeyRelease(); events[1].activated = 1; return D_O_K; }
int menuEvent2() { b4WaitKeyRelease(); events[2].activated = 1; return D_O_K; }
int menuEvent3() { b4WaitKeyRelease(); events[3].activated = 1; return D_O_K; }
int menuEvent4() { b4WaitKeyRelease(); events[4].activated = 1; return D_O_K; }
int menuEvent5() { b4WaitKeyRelease(); events[5].activated = 1; return D_O_K; }
int menuEvent6() { b4WaitKeyRelease(); events[6].activated = 1; return D_O_K; }
int menuEvent7() { b4WaitKeyRelease(); events[7].activated = 1; return D_O_K; }
int menuEvent8() { b4WaitKeyRelease(); events[8].activated = 1; return D_O_K; }
int menuEvent9() { b4WaitKeyRelease(); events[9].activated = 1; return D_O_K; }
int menuEvent10() { b4WaitKeyRelease(); events[10].activated = 1; return D_O_K; }
int menuEvent11() { b4WaitKeyRelease(); events[11].activated = 1; return D_O_K; }
int menuEvent12() { b4WaitKeyRelease(); events[12].activated = 1; return D_O_K; }
int menuEvent13() { b4WaitKeyRelease(); events[13].activated = 1; return D_O_K; }
int menuEvent14() { b4WaitKeyRelease(); events[14].activated = 1; return D_O_K; }
int menuEvent15() { b4WaitKeyRelease(); events[15].activated = 1; return D_O_K; }
int menuEvent16() { b4WaitKeyRelease(); events[16].activated = 1; return D_O_K; }
int menuEvent17() { b4WaitKeyRelease(); events[17].activated = 1; return D_O_K; }
int menuEvent18() { b4WaitKeyRelease(); events[18].activated = 1; return D_O_K; }
int menuEvent19() { b4WaitKeyRelease(); events[19].activated = 1; return D_O_K; }
int menuEvent20() { b4WaitKeyRelease(); events[20].activated = 1; return D_O_K; }
int menuEvent21() { b4WaitKeyRelease(); events[21].activated = 1; return D_O_K; }
int menuEvent22() { b4WaitKeyRelease(); events[22].activated = 1; return D_O_K; }
int menuEvent23() { b4WaitKeyRelease(); events[23].activated = 1; return D_O_K; }
int menuEvent24() { b4WaitKeyRelease(); events[24].activated = 1; return D_O_K; }
int menuEvent25() { b4WaitKeyRelease(); events[25].activated = 1; return D_O_K; }
int menuEvent26() { b4WaitKeyRelease(); events[26].activated = 1; return D_O_K; }
int menuEvent27() { b4WaitKeyRelease(); events[27].activated = 1; return D_O_K; }
int menuEvent28() { b4WaitKeyRelease(); events[28].activated = 1; return D_O_K; }
int menuEvent29() { b4WaitKeyRelease(); events[29].activated = 1; return D_O_K; }
int menuEvent30() { b4WaitKeyRelease(); events[30].activated = 1; return D_O_K; }
int menuEvent31() { b4WaitKeyRelease(); events[31].activated = 1; return D_O_K; }
int menuEvent32() { b4WaitKeyRelease(); events[32].activated = 1; return D_O_K; }
int menuEvent33() { b4WaitKeyRelease(); events[33].activated = 1; return D_O_K; }
int menuEvent34() { b4WaitKeyRelease(); events[34].activated = 1; return D_O_K; }
int menuEvent35() { b4WaitKeyRelease(); events[35].activated = 1; return D_O_K; }
int menuEvent36() { b4WaitKeyRelease(); events[36].activated = 1; return D_O_K; }
int menuEvent37() { b4WaitKeyRelease(); events[37].activated = 1; return D_O_K; }
int menuEvent38() { b4WaitKeyRelease(); events[38].activated = 1; return D_O_K; }
int menuEvent39() { b4WaitKeyRelease(); events[39].activated = 1; return D_O_K; }
int menuEvent40() { b4WaitKeyRelease(); events[40].activated = 1; return D_O_K; }
int menuEvent41() { b4WaitKeyRelease(); events[41].activated = 1; return D_O_K; }
int menuEvent42() { b4WaitKeyRelease(); events[42].activated = 1; return D_O_K; }
int menuEvent43() { b4WaitKeyRelease(); events[43].activated = 1; return D_O_K; }
int menuEvent44() { b4WaitKeyRelease(); events[44].activated = 1; return D_O_K; }
int menuEvent45() { b4WaitKeyRelease(); events[45].activated = 1; return D_O_K; }
int menuEvent46() { b4WaitKeyRelease(); events[46].activated = 1; return D_O_K; }
int menuEvent47() { b4WaitKeyRelease(); events[47].activated = 1; return D_O_K; }
int menuEvent48() { b4WaitKeyRelease(); events[48].activated = 1; return D_O_K; }
int menuEvent49() { b4WaitKeyRelease(); events[49].activated = 1; return D_O_K; }

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
void b4FreeMenuItems()
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

void b4Set_menu(byte** data) // 1, 0x00 
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

void b4Set_menu_item(byte** data) // 2, 0x00 
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

void b4Submit_menu(byte** data) // 0, 0x00 
{

}

void b4Enable_item(byte** data) // 1, 0x00 
{
    (*data)++;
}

void b4Disable_item(byte** data) // 1, 0x00 
{
    (*data)++;
}

void b4Menu_input(byte** data) // 0, 0x00 
{
    do_menu(the_menu, 10, 20);
}

void b4Show_obj_v(byte** data) // 1, 0x01 
{
    int objectNum;

    objectNum = var[*(*data)++];
    /* Not supported yet */
}

void b4Open_dialogue(byte** data) // 0, 0x00 
{

}

void b4Close_dialogue(byte** data) // 0, 0x00 
{

}

void b4Mul_n(byte** data) // 2, 0x80 
{
    var[*(*data)++] *= *(*data)++;
}

void b4Mul_v(byte** data) // 2, 0xC0 
{
    var[*(*data)++] *= var[*(*data)++];
}

void b4Div_n(byte** data) // 2, 0x80 
{
    var[*(*data)++] /= *(*data)++;
}

void b4Div_v(byte** data) // 2, 0xC0 
{
    var[*(*data)++] /= var[*(*data)++];
}

void b4Close_window(byte** data) // 0, 0x00 
{

}


#pragma code-name (pop)
#pragma code-name (push, "BANKRAM05")
boolean b5instructionHandler(byte code, int* currentLog, byte logNum, byte** ppCodeWindowAddress, byte bank)
{
    switch (code)
    {
    case 0: /* return */
        return FALSE;
        break;
    case 1: trampoline_1(&b1Increment,ppCodeWindowAddress, bank); break;
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
        return FALSE;
        break;
    case 19:
        b1New_room_v(ppCodeWindowAddress);
        exitAllLogics = TRUE;
        return FALSE;
        break;
    case 20: b1Load_logics(ppCodeWindowAddress); break;
    case 21: b1Load_logics_v(ppCodeWindowAddress); break;
    case 22:
        b1Call(ppCodeWindowAddress);
        /* The currentLog variable needs to be restored */
        *currentLog = logNum;
        if (exitAllLogics) return FALSE;
#ifdef DEBUG
        sprintf(debugString, "LOGIC.%d:       ", currentLog);
        drawBigString(screen, debugString, 0, 384, 0, 7);
#endif
        break;
    case 23:
        b1Call_v(ppCodeWindowAddress);
        /* The currentLog variable needs to be restored */
        *currentLog = logNum;
        if (exitAllLogics) return FALSE;
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
    case 41: b2Set_view(ppCodeWindowAddress); break;
    case 42: b2Set_view_v(ppCodeWindowAddress); break;
    case 43: b2Set_loop(ppCodeWindowAddress); break;
    case 44: b2Set_loop_v(ppCodeWindowAddress); break;
    case 45: b2Fix_loop(ppCodeWindowAddress); break;
    case 46: b2Release_loop(ppCodeWindowAddress); break;
    case 47: b2Set_cel(ppCodeWindowAddress); break;
    case 48: b2Set_cel_v(ppCodeWindowAddress); break;
    case 49: b2Last_cel(ppCodeWindowAddress); break;
    case 50: b2Current_cel(ppCodeWindowAddress); break;
    case 51: b2Current_loop(ppCodeWindowAddress); break;
    case 52: b2Current_view(ppCodeWindowAddress); break;
    case 53: b2Number_of_loops(ppCodeWindowAddress); break;
    case 54: b2Set_priority(ppCodeWindowAddress); break;
    case 55: b2Set_priority_v(ppCodeWindowAddress); break;
    case 56: b2Release_priority(ppCodeWindowAddress); break;
    case 57: b2Get_priority(ppCodeWindowAddress); break;
    case 58: b2Stop_update(ppCodeWindowAddress); break;
    case 59: b2Start_update(ppCodeWindowAddress); break;
    case 60: b2Force_update(ppCodeWindowAddress); break;
    case 61: b2Ignore_horizon(ppCodeWindowAddress); break;
    case 62: b2Observe_horizon(ppCodeWindowAddress); break;
    case 63: b2Set_horizon(ppCodeWindowAddress); break;
    case 64: b2Object_on_water(ppCodeWindowAddress); break;
    case 65: b2Object_on_land(ppCodeWindowAddress); break;
    case 66: b2Object_on_anything(ppCodeWindowAddress); break;
    case 67: b2Ignore_objs(ppCodeWindowAddress); break;
    case 68: b2Observe_objs(ppCodeWindowAddress); break;
    case 69: b2Distance(ppCodeWindowAddress); break;
    case 70: b2Stop_cycling(ppCodeWindowAddress); break;
    case 71: b2Start_cycling(ppCodeWindowAddress); break;
    case 72: b2Normal_cycle(ppCodeWindowAddress); break;
    case 73: b2End_of_loop(ppCodeWindowAddress); break;
    case 74: b2Reverse_cycle(ppCodeWindowAddress); break;
    case 75: b2Reverse_loop(ppCodeWindowAddress); break;
    case 76: b2Cycle_time(ppCodeWindowAddress); break;
    case 77: b2Stop_motion(ppCodeWindowAddress); break;
    case 78: b2Start_motion(ppCodeWindowAddress); break;
    case 79: b2Step_size(ppCodeWindowAddress); break;
    case 80: b2Step_time(ppCodeWindowAddress); break;
    case 81: b2Move_obj(ppCodeWindowAddress); break;
    case 82: b2Move_obj_v(ppCodeWindowAddress); break;
    case 83: b2Follow_ego(ppCodeWindowAddress); break;
    case 84: b2Wander(ppCodeWindowAddress); break;
    case 85: b2Normal_motion(ppCodeWindowAddress); break;
    case 86: b2Set_dir(ppCodeWindowAddress); break;
    case 87: b2Get_dir(ppCodeWindowAddress); break;
    case 88: b2Ignore_blocks(ppCodeWindowAddress); break;
    case 89: b2Observe_blocks(ppCodeWindowAddress); break;
    case 90: b2Block(ppCodeWindowAddress); break;
    case 91: b2Unblock(ppCodeWindowAddress); break;
    case 92: b2Get(ppCodeWindowAddress); break;
    case 93: b2Get_v(ppCodeWindowAddress); break;
    case 94: b2Drop(ppCodeWindowAddress); break;
    case 95: b2Put(ppCodeWindowAddress); break;
    case 96: b2Put_v(ppCodeWindowAddress); break;
    case 97: b2Get_room_v(ppCodeWindowAddress); break;
    case 98: b2Load_sound(ppCodeWindowAddress); break;
    case 99: b2Play_sound(ppCodeWindowAddress); break;
    case 100: b2Stop_sound(ppCodeWindowAddress); break;
    case 101: b3Print(ppCodeWindowAddress); break;
    case 102: b3Print_v(ppCodeWindowAddress); break;
    case 103: b3Display(ppCodeWindowAddress); break;
    case 104: b3Display_v(ppCodeWindowAddress); break;
    case 105: b3Clear_lines(ppCodeWindowAddress); break;
    case 106: b3Text_screen(ppCodeWindowAddress); break;
    case 107: b3Graphics(ppCodeWindowAddress); break;
    case 108: b3Set_cursor_char(ppCodeWindowAddress); break;
    case 109: b3Set_text_attribute(ppCodeWindowAddress); break;
    case 110: b3Shake_screen(ppCodeWindowAddress); break;
    case 111: b3Configure_screen(ppCodeWindowAddress); break;
    case 112: b3Status_line_on(ppCodeWindowAddress); break;
    case 113: b3Status_line_off(ppCodeWindowAddress); break;
    case 114: b3Set_string(ppCodeWindowAddress); break;
    case 115: b3Get_string(ppCodeWindowAddress); break;
    case 116: b3Word_to_string(ppCodeWindowAddress); break;
    case 117: b3Parse(ppCodeWindowAddress); break;
    case 118: b4Get_num(ppCodeWindowAddress); break;
    case 119: b4Prevent_input(ppCodeWindowAddress); break;
    case 120: b4Accept_input(ppCodeWindowAddress); break;
    case 121: b4Set_key(ppCodeWindowAddress); break;
    case 122: b4Add_to_pic(ppCodeWindowAddress); break;
    case 123: b4Add_to_pic_v(ppCodeWindowAddress); break;
    case 124: b4Status(ppCodeWindowAddress); break;
    case 125: b4Save_game(ppCodeWindowAddress); break;
    case 126: b4Restore_game(ppCodeWindowAddress); break;
    case 127: break;
    case 128: b4Restart_game(ppCodeWindowAddress); break;
    case 129: b4Show_obj(ppCodeWindowAddress); break;
    case 130: b4Random_num(ppCodeWindowAddress); break;
    case 131: b4Program_control(ppCodeWindowAddress); break;
    case 132: b4Player_control(ppCodeWindowAddress); break;
    case 133: b4Obj_status_v(ppCodeWindowAddress); break;
    case 134: b4Quit(ppCodeWindowAddress); break;
    case 135: break;
    case 136: b4Pause(ppCodeWindowAddress); break;
    case 137: b4Echo_line(ppCodeWindowAddress); break;
    case 138: b4Cancel_line(ppCodeWindowAddress); break;
    case 139: b4Init_joy(ppCodeWindowAddress); break;
    case 140: break;
    case 141: b4Version(ppCodeWindowAddress); break;
    case 142: trampoline_1(&b4Script_size, ppCodeWindowAddress, bank); break;
    case 143: b4Set_game_id(ppCodeWindowAddress); break;
    case 144: b4Log(ppCodeWindowAddress); break;
    case 145: b4Set_scan_start(ppCodeWindowAddress); break;
    case 146: b4Reset_scan_start(ppCodeWindowAddress); break;
    case 147: b4Reposition_to(ppCodeWindowAddress); break;
    case 148: b4Reposition_to_v(ppCodeWindowAddress); break;
    case 149: b4Trace_on(ppCodeWindowAddress); break;
    case 150: b4Trace_info(ppCodeWindowAddress); break;
    case 151: b4Print_at(ppCodeWindowAddress); break;
    case 152: b4Print_at_v(ppCodeWindowAddress); break;
    case 153: b4Discard_view_v(ppCodeWindowAddress); break;
    case 154: b4Clear_text_rect(ppCodeWindowAddress); break;
    case 155: b4Set_upper_left(ppCodeWindowAddress); break;
    case 156: b4Set_menu(ppCodeWindowAddress); break;
    case 157: b4Set_menu_item(ppCodeWindowAddress); break;
    case 158: b4Submit_menu(ppCodeWindowAddress); break;
    case 159: b4Enable_item(ppCodeWindowAddress); break;
    case 160: b4Disable_item(ppCodeWindowAddress); break;
    case 161: b4Menu_input(ppCodeWindowAddress); break;
    case 162: b4Show_obj_v(ppCodeWindowAddress); break;
    case 163: b4Open_dialogue(ppCodeWindowAddress); break;
    case 164: b4Close_dialogue(ppCodeWindowAddress); break;
    case 165: b4Mul_n(ppCodeWindowAddress); break;
    case 166: b4Mul_v(ppCodeWindowAddress); break;
    case 167: b4Div_n(ppCodeWindowAddress); break;
    case 168: b4Div_v(ppCodeWindowAddress); break;
    case 169: b4Close_window(ppCodeWindowAddress); break;
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
    }
    exit(0);
}

#pragma code-name (pop)

byte getBankBasedOnCode(byte code)
{
    if (code <= HIGHEST_BANK1_FUNC)
    {
        return 1;
    }
    else if (code <= HIGHEST_BANK2_FUNC)
    {
        return 2;
    }
    else if (code <= HIGHEST_BANK3_FUNC)
    {
        return 3;
    }
    else if (code <= HIGHEST_BANK4_FUNC)
    {
        return 4;
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
            * data += (codeWindowAddress - &codeWindow[0]);

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
    byte instructionCodeBank;
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

        if (logNum != 0)
        {
#ifdef VERBOSE
            printf("The code is now %u and the address is %p and the bank is %d \n", *code, code, RAM_BANK);
#endif // VERBOSE
            exit(0);
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
#ifdef VERBOSE
        printf("\n The code is %d, on bank %d address, %p \n", *code, RAM_BANK, code);
#endif // VERBOSE
        codeAtTimeOfLastBankSwitch = *code;
       instructionCodeBank = getBankBasedOnCode(codeAtTimeOfLastBankSwitch);

#ifdef VERBOSE
        printf("Bank is now %d to execute code %d \n", RAM_BANK, codeAtTimeOfLastBankSwitch);
#endif // VERBOSE 


 /*       if (counter == 4)
        {
            exit(0);
        }*/

        if (*code < 0xfe)
        {
            code++;
            RAM_BANK = INSTRUCTION_HANDLER_BANK;
            b5instructionHandler(codeAtTimeOfLastBankSwitch, &currentLog, logNum, ppCodeWindowAddress, instructionCodeBank);
            RAM_BANK = currentLogicFile.codeBank;
        }
        else {
            switch (codeAtTimeOfLastBankSwitch) {
            case 0xfe: /* Unconditional branch: else, goto. */
                code++;
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
                code++;
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
        }

        RAM_BANK = currentLogicFile.codeBank;

        if (!lastCodeWasNonWindow)
        {
#ifdef VERBOSE
            printf("Now jumping (%p - %p - 1) = %p \n", codeWindowAddress, &codeWindow[0], (codeWindowAddress - &codeWindow[0] - 1));
#endif // VERBOSE
            code += (codeWindowAddress - &codeWindow[0]) - 1;
        }



        lastCodeWasNonWindow = FALSE;

        counter++;
    }

    if (discardAfterward) discardLogicFile(logNum);

    RAM_BANK = previousRamBank;
}


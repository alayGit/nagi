#include <stdio.h>
/**************************************************************************
** GENERAL.H
**
** Contains general definitions used by most C files.
**************************************************************************/

#ifndef _GENERAL_H_
#define _GENERAL_H_

#ifndef TRUE
#define  TRUE    1
#endif
#ifndef FALSE
#define  FALSE   0
#endif

#define SEQUENTIAL_LFN 2

typedef unsigned char byte;
typedef unsigned short int word;
typedef char boolean;

typedef struct { 
	int w;
	int h;
	int v_w;
	int v_h;
	int color_depth;
} BITMAP;

#endif  /* _GENERAL_H_ */

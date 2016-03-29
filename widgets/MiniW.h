#ifndef MINIW_H
#define MINIW_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <locale.h>

#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/Xos.h>
#include <X11/CoreP.h>
#include <X11/Xft/Xft.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/XawInit.h>

#include "mls.h"
#ifndef TF
#  define TF() TRACE(2,"")
#endif

/********************************************
 *                                          *
 *     the Mini Widget example Code         * 
 *                                          *
 ********************************************/

typedef struct {		// widget data
/* resources */
  char *label;			// my label
  int   w,h;			// my size
/* private */
  XftColor	color;
  XftDraw	*draw;
  XftFont	*font;
  XGlyphInfo	extents;
} MiniPart;

typedef struct {		// widget data
  CorePart core;		// parent
  MiniPart mini;		// self
} MiniWidget_t, *MiniWidget;

typedef struct {		// widget class methods
  XtPointer extension;		// unused, dummy
} MiniClassPart;

typedef struct  {
  CoreClassPart core_class;	// core class members
  MiniClassPart mini_class;	// our class members/unused: see above
} MiniWidgetClass_t;

extern WidgetClass  miniWidgetClass;



#endif

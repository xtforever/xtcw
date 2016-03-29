#ifndef Select_WIDGET_H
#define Select_WIDGET_H

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
#include <vexmo/WheelP.h>

/********************************************
 *                                          *
 *     the Select Widget example Code         * 
 *                                          *
 ********************************************/

#define XtNlabelFont   "labelFont"
#define XtNvalueFont   "valueFont"
#define XtNgraphColor  "graphColor"
#define XtNrulerColor  "rulerColor"
#define XtNtextColor   "textColor"
#define XtCFace        "Face"
#define XtNselectMax   "selectMax"
#define XtNselectMin   "selectMin"
#define XtNselectVal   "selectVal"

#define XtNchapterStr	"chapterStr"
#define XtNvarnameStr	"varnameStr"
#define XtNlabelStr	"labelStr"
#define XtNunitStr	"unitStr"
#define XtNlocked	"locked"

#define XtNvalueChange  "valueChange"

typedef struct draw_area {
  int x,y,w,h,px,py;
} draw_area_t;


typedef struct {		// widget data
/* resources */

  char *label;			// my label
  XftFont	*label_font;
  XftFont	*value_font;
  XftColor	text_col;
  Pixel		graph_col, ruler_col;
  int		select_min, select_max, select_val;
  char *varnameStr; char *chapterStr; 
  char *labelStr; char *unitStr;
  XtCallbackList value_change_cb_list;
    Boolean	locked;

/* private */
  GC		graph_gc, bg_gc, ruler_gc;
  XftDraw	*draw;
  XGlyphInfo	extents;
  int		select_height;
  int		select_val_cur;
  int		select_key, display_value, value;

  draw_area_t	val_area;

} SelectPart;

typedef struct {		// widget data
  CorePart core;		// parent
  WheelPart wheel;		// wheel 
  SelectPart select;		// self
} SelectWidget_t, *SelectWidget;

typedef struct {		// widget class methods
  XtPointer extension;		// unused, dummy
} SelectClassPart;

typedef struct  {
  CoreClassPart core_class;	// core class members
  WheelClassPart wheel_class;
  SelectClassPart select_class;	// our class members/unused: see above
} SelectWidgetClass_t;


extern WidgetClass  selectWidgetClass;

#endif

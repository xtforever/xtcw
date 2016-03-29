#ifndef Slider_WIDGET_H
#define Slider_WIDGET_H

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
#include "WheelW.h"

/********************************************
 *                                          *
 *     the Slider Widget example Code         * 
 *                                          *
 ********************************************/

#define XtNlabelFont   "labelFont"
#define XtNtickFont    "tickFont"
#define XtNgraphColor  "graphColor"
#define XtNrulerColor  "rulerColor"
#define XtNtextColor   "textColor"
#define XtCFace        "Face"
#define XtNsliderMax   "sliderMax"
#define XtNsliderMin   "sliderMin"
#define XtNsliderVal   "sliderVal"
#define XtNshowScale   "showScale"


typedef struct {		// widget data
/* resources */
  char *label;			// my label
  XftFont	*label_font, *tick_font;
  XftColor	text_col;
  Pixel		graph_col, ruler_col;
  int		slider_min, slider_max, slider_val,
		show_scale;

/* private */
  GC		graph_gc, bg_gc, ruler_gc;
  XftDraw	*draw;
  XGlyphInfo	extents;
  int		slider_height;
  int		slider_val_cur;
} SliderPart;

typedef struct {		// widget data
  CorePart core;		// parent
  WheelPart wheel;		// wheel 
  SliderPart slider;		// self
} SliderWidget_t, *SliderWidget;

typedef struct {		// widget class methods
  XtPointer extension;		// unused, dummy
} SliderClassPart;

typedef struct  {
  CoreClassPart core_class;	// core class members
  WheelClassPart wheel_class;
  SliderClassPart slider_class;	// our class members/unused: see above
} SliderWidgetClass_t;


extern WidgetClass  sliderWidgetClass;

#endif

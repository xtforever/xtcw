#ifndef CTRL_H
#define CTRL_H

#include <X11/IntrinsicI.h>
#include <X11/Xft/Xft.h>
#include <X11/CoreP.h>
#include "xftdef.h"
#include "XWBUILD/WheelP.h"

/* define unique representation types not found in <X11/StringDefs.h> */
#define XtRCtrlResource "CtrlResource"


#define XtNisflip "isflip"
#define XtCIsflip "Isflip"

#define XtNlabelStr "labelStr"
#define XtNunitStr  "unitStr"
#define XtNtextStr  "textStr"

#define XtNlabelFont "labelFont"
#define XtNunitFont  "unitFont"
#define XtNtextFont  "textFont"

#define XtNvspace "vspace"
#define XtNhspace "hspace"
#define XtCSpacing "Spacing"

#ifndef	XtNgravity
#define	XtNgravity	"gravity"
#define	XtCGravity	"Gravity"
#endif


#define XtNlabelGravity "labelGravity"
#define XtNunitGravity  "unitGravity"
#define XtNtextGravity  "textGravity"


#ifndef XtNfocusCB 
#define XtNfocusCB "focusCB"
#endif
#define XtNupdateCB "updateDB"



enum CtrlMsg {
  MSG_INC, MSG_DEC, MSG_FIRE, MSG_UPDATE
};


/* declare specific FlipWidget class and instance datatypes 
typedef struct _FlipClassRec *FlipWidgetClass;
typedef struct _FlipRec *FlipWidget;
*/

/* declare the class constant */


#define MAX_CLABEL_STR 100

enum CLABEL {
  LABEL =0,
  UNIT  =1,
  TEXT  =2,
  MAX_CLABEL = 3
};

typedef struct clabel_st {
  XftFont *font;
  char str[MAX_CLABEL_STR];
  XGlyphInfo extents;
  int x,y,width,height,position;
  int gravity;
  int str_x, str_y;
} clabel_t;

typedef struct CtrlNotify_st {
  wheel_state state;
  enum CtrlMsg   cmd;
  int value, max_value, min_value;
  char str[MAX_CLABEL][MAX_CLABEL_STR];
} CtrlNotify_t;



typedef struct {
    /* resources */
  Pixel pixel[6];	// two colors each state
  XftColor col[3];      // FG colors for each state
  GC gc[3];
  char *fontnames[MAX_CLABEL];
  Dimension vspace,hspace, width,height;
  char *str[MAX_CLABEL];
  Boolean isflip;

  
    /* private */
  Boolean recursive_set_values;
  XftDraw *draw;
  CtrlNotify_t   msg;
  clabel_t       clabel[MAX_CLABEL];
  XtCallbackList notify_cb_list;
  
  char *varnameStr; char *capterStr;
  int val_min, val_max, val_cur, val_key; 

} CtrlPart;

typedef struct ctrl_widget_data_st {
    CorePart	core;
    CtrlPart	ctrl;
} CtrlWidget_t, *CtrlWidget;

typedef struct {
  XtPointer extension;
} CtrlClassPart;

struct ctrl_widget_function_st {
    CoreClassPart	core_class;
    CtrlClassPart	ctrl_class;
};

extern WidgetClass  ctrlWidgetClass;
int wheel_input(Widget w, char *key);

#endif 

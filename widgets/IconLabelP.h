/********************************************************************\
**                             _________________________________    **
**   A n t h o n y            |________    __    __    _________|   **
**                                     |  |o_|  |o_|  |             **
**           T h y s s e n           __|   __    __   |__           **
**                                __|   __|  |  |  |__   |__        **
**  `` Dragon Computing! ''    __|   __|     |  |     |__   |__     **
**                            |_____|        |__|        |_____|    **
**                                                                  **
\********************************************************************/
/*
** IconLabelP.h - IconLabel Widget   Display a Pixmap with or without a label
**
** Author: Anthony Thyssen
**         Griffith University
**         anthony@cit.gu.edu.au
**
** Date:   March 30, 1996
**
** This Widget was created due to problems with using the original X
** Consortium Label Widget for displaying Bitmap and Pixmap in my
** XbmBrowser program.  I designed this widget to basically display a
** pixmap and optionally a title or filename centered underneath this
** image.
** 
** FEATURES
**    1/  Pixmap displayed correctly (a flag indicates to treat it as a bitmap)
**    2/  Shaped Windows using the bitmap mask given
**    3/  Optional Label centered either above or below the image
**    4/  Numerous inter-dimension controls for good user control
** 
*/

#ifndef _IconLabelP_h
#define _IconLabelP_h

#include "IconLabel.h"

// #include <X11/Xaw/XawP.h>
#include <X11/Xaw/LabelP.h>

#ifdef _THREED_
#  include <X11/Xaw/ThreeDP.h>
#endif

#include <X11/Xft/Xft.h>

/*---------------------------------------------------------------------------*/
/*---------------------------- Class Definitions ----------------------------*/

/* Just use a empty class part for this widget */
typedef struct {int foo;} IconLabelClassPart;

/* Full class record for widget */
typedef struct _IconLabelClassRec {
    CoreClassPart       core_class;
    SimpleClassPart     simple_class;
#ifdef _THREED_
    ThreeDClassPart     threeD_class;
#endif

    IconLabelClassPart  icon_label_class;
} IconLabelClassRec;

/* The class record for this widget */
extern IconLabelClassRec iconLabelClassRec;


/*---------------------------------------------------------------------------*/
/*--------------------------- Instance Definitions --------------------------*/

typedef struct {
/* --- public resources --- */
/* miscellanous */
  Pixel       foreground;   /* foreground for label and bitmaps */
  XFontStruct *font;        /* font to display labels and bitmaps */
  Boolean     shape;        /* is the widget to use shaped windows? */
  Boolean     resize;       /* resize allowed on this widget? */
/* pixmap stuff */
  String	pixmapName;
  Pixmap      pixmap;       /* pixmap or bitmap to be displayed */
  Pixmap      mask;         /* bitmap mask for window shape (without label) */
  Boolean     is_bitmap;    /* is the pixmap really a bitmap? */
/* label stuff */  
  String      label;        /* the text label to use (NULL if off) */
  Boolean     label_top;    /* label at top (or bottom by default) */
/* internal space */
  Dimension   internal_height; /* space inside the widget */
  Dimension   internal_width;
  Dimension   label_gap;       /* extra space between label and pixmap */
/* Extra Information Pointer */
  XtPointer   info_ptr;

/* --- private variables --- */
  GC          normal_GC;    /* Drawing GCs (sensitive or insensitive ) */
  GC          gray_GC;
  Pixmap      stipple;
  XRectangle  pm_rect;     /* rectangles for position and shape */
  XRectangle  lb_rect;     /*   of the pixmap/label parts */
  int         pm_depth;    /* depth of the pixmap */
  int         label_len;   /* length of the label */
  Boolean     shape_on;    /* is shape currently on? */
} IconLabelPart;


/* Full Instance record for widget */
typedef struct _IconLabelRec {
    CorePart       core;
    SimplePart     simple;
#ifdef _THREED_
    ThreeDPart  threeD;
#endif
    IconLabelPart  icon_label;
} IconLabelRec;

#endif /* _IconLabelP_h */


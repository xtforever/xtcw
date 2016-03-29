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
** IconLabel.h - IconLabel Widget   Display a Pixmap with or without a label
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

#ifndef _IconLabel_h
#define _IconLabel_h

#include <X11/Xaw/Simple.h>

/* Resources   *  -- new to this widget (subclass of Core,Simple) 

   Name                Class              RepType         Default Value
   ----                -----              -------         -------------
   background          Background         Pixel           XtDefaultBackground
   border              BorderColor        Pixel           XtDefaultForeground
   borderWidth         BorderWidth        Dimension       1
   cursor              Cursor             Cursor          None
   cursorName          Cursor             String          NULL
   destroyCallback     Callback           Pointer         NULL
 * infoPtr             InfoPtr            XtPointer       NULL
 * font                Font               XFontStruct*    XtDefaultFont
 * foreground          Foreground         Pixel           XtDefaultForeground
   height              Height             Dimension       0
   insensitiveBorder   Insensitive        Pixmap          Gray
 * internalHeight      Height             Dimension       1
 * internalWidth       Width              Dimension       2
 * isBitmap            IsBitmap           Boolean         False
 * label               Label              String          NULL
 * labelGap            Height             Dimension       1
 * labelTop            LabelTop           Boolean         False
   mappedWhenManaged   MappedWhenManaged  Boolean         True
 * mask                Mask               Bitmap          None
 * pixmap              Pixmap             Pixmap          None
   pointerColor        Foreground         Pixel           XtDefaultForeground
   pointerColorBackground Background      Pixel           XtDefaultBackground
 * resize              Resize             Boolean         True
   sensitive           Sensitive          Boolean         True
 * shape               Shape              Boolean         True
   width               Width              Dimension       0
   x                   Position           Position        0
   y                   Position           Position        0


Resource Explaination

   font              the font to draw the text label in.
   
   foreground        the colors to use for the window background and to 
   background        use to display bitmaps and the text label with.

   pixmap            The pixmap/bitmap to be displayed in the widget.
   mask              The mask should be the same size (not checked) and is
                     currently only used to shape the widget.

   isBitmap          The Pixmap given is really a bitmap and is to be
                     drawn as such. Setting this on basically makes this
                     widget act just like the Xaw Label widget and like
                     that widget could draw pixmaps incorrectly on some`
                     monocrome displays.
   
   label             String to display as a label to the pixmap if NULL
                     no label will be displayed

   labelTop          Put the label above the pixmap?
                     It is placed under the pixmap (centered) by default.

   internalWidth     Sets the amount of space around the text label
   internalHeight    and the pixmap and between the two items. This space
                     is allways provided around the label while the
                     pixmap is normally centered in whatever area is left.

   labelGap          Extra space between the pixmap and the label on top of
                     the usual internalHeight space provided.

   shape             Use the Shape extension and mask on the pixmap. If the
                     label is turned on a extra box (padded by internalWidth
                     and Height) is added for it.

   resize            if true only the user modifing the widget can resize
                     the window.

   infoPtr           A pointer to a user structure containing information.
                     This allows a program to lookup the information a
                     particular widget represents, when selected, due to
                     some action by the user.
                     See the GetInfoPtr() and SetInfoPtr() functions below.

*/

/* specific to this widget */
#define XtNmask     "mask"
#define XtCMask     "Mask"
#define XtNisBitmap "isBitmap"
#define XtCIsBitmap "IsBitmap"
#define XtNlabelTop "labelTop"
#define XtCLabelTop "LabelTop"
#define XtNshape    "shape"
#define XtCShape    "Shape"
#define XtNlabelGap "labelGap"
#define XtNinfoPtr  "infoPtr"
#define XtCInfoPtr  "InfoPtr"
#define XtNpixmapName "pixmapName"


/* in case XtStringsDef.h is not included */
#ifndef _XtStringDefs_h_
#define XtNforeground "foreground"
#define XtNlabel "label"
#define XtNfont "font"
#define XtNinternalWidth "internalWidth"
#define XtNinternalHeight "internalHeight"
#define XtNresize "resize"
#define XtCResize "Resize"
#define XtNpixmap "pixmap"
#define XtCPixmap "Pixmap"
#endif

/* Class record constants */
extern WidgetClass iconLabelWidgetClass;

typedef struct _IconLabelClassRec *IconLabelWidgetClass;
typedef struct _IconLabelRec      *IconLabelWidget;


/* Public Functions */

/*      Function Name: SetInfoPtr
 *      Description: Set the pointer to extra information (for fast lookup)
 *      Arguments: IconLabelWidget, and the pointer to store
 *      Returns: none.
 */
extern void SetInfoPtr(
#if NeedFunctionPrototypes
    Widget     /* ilw */,
    XtPointer  /* info_ptr */
#endif
);

/*      Function Name: GetInfoPtr
 *      Description: Set the pointer to extra information (for fast lookup)
 *      Arguments: IconLabelWidget with the information to be retrieved
 *      Returns: the current pointer to information
 */
extern XPointer GetInfoPtr(
#if NeedFunctionPrototypes
    Widget     /* ilw */
#endif
);

#endif /* _IconLabel_h */


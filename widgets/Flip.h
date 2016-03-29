/*

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/

#ifndef _Flip_h
#define _Flip_h

#include <X11/Intrinsic.h>
#include "xftdef.h"
#include "XWBUILD/Wheel.h"

/****************************************************************
 *
 * Flip widget
 *
 ****************************************************************/

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 destroyCallback     Callback		Pointer		NULL
 height		     Height		Dimension	0
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 sensitive	     Sensitive		Boolean		True
 width		     Width		Dimension	0
 x		     Position		Position	0
 y		     Position		Position	0

*/

/* define any special resource names here that are not in <X11/StringDefs.h> */



#define XtNentryList "armedBg"


#ifndef XtNfocusCB 
#define XtNfocusCB "focusCB"
#endif

#define XtNupdateCB "updateDB"

enum FlipState {
  normal = 0,
  selected = 1,
  armed = 2
};

typedef struct FlipNotify_st {
  enum FlipState state;
  int crsr;
  int mlist;
  int mlabel;
} FlipList_t;

/* declare specific FlipWidget class and instance datatypes */
typedef struct _FlipClassRec *FlipWidgetClass;
typedef struct _FlipRec *FlipWidget;

/* declare the class constant */
extern WidgetClass flipWidgetClass;

int FlipWheelInput(Widget w, char *key);
void FlipSetList(Widget w, int mlist);
FlipList_t *FlipGetList(Widget w);
#endif /* _Flip_h */

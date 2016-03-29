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

#ifndef _FlipP_h
#define _FlipP_h

#include "Flip.h"

/* include superclass private header file */
#include <X11/CoreP.h>

/* define unique representation types not found in <X11/StringDefs.h> */
#define XtRFlipResource "FlipResource"

typedef struct {
  XtPointer extension;
} FlipClassPart;

typedef struct _FlipClassRec {
    CoreClassPart	core_class;
    FlipClassPart	flip_class;
} FlipClassRec;

extern FlipClassRec flipClassRec;


typedef struct {
    /* resources */
   Pixel pixel[6];	// two colors each state
  char *xftfontname;
  char *entryList;

    /* private */
  int value, old_value;
  int value_list;
  char *label;
  GC gc[3];
  XftFont *font;
  XftDraw *draw;
  XftColor xftCol[3];
  enum FlipState state;
  XGlyphInfo extents;
  XtCallbackList notify_cb_list, focus_cb_list, update_cb_list;
  FlipList_t list;
} FlipPart;

typedef struct _FlipRec {
    CorePart	core;
    FlipPart	flip;
} FlipRec;

#endif /* _FlipP_h */

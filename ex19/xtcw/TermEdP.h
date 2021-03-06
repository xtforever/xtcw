/* Generated by wbuild
 * (generator version 3.3)
 */
#ifndef XTCW_TERMEDP_H
#define XTCW_TERMEDP_H
#include <X11/CoreP.h>
#include <xtcw/TermEd.h>
_XFUNCPROTOBEGIN

typedef struct {
/* methods */
/* class variables */
int dummy;
} TermEdClassPart;

typedef struct _TermEdClassRec {
CoreClassPart core_class;
TermEdClassPart termEd_class;
} TermEdClassRec;

typedef struct {
/* resources */
int  gWidth;
int  gHeight;
int  size;
int  selTime;
String  text;
XftColor  fg;
XftColor  bg;
XftColor  cbg;
XftColor  cfg;
String  host;
String  port;
/* private state */
XtIntervalId  timerid;
Boolean  blink;
XftFont * xftFont;
XftDraw * draw;
XftColor * col[4];
uint32_t * scr;
int  grid_pix_width;
int  grid_pix_height;
Bool  selection_visible;
XRectangle  rsel;
XRectangle  rsel_old;
GC  gc_copy;
uint8_t * cmd;
uint  cmd_len;
int  progm;
struct  fork2_info * child;
XtInputId  cid;
XtInputId  eid;
Bool  INTERACTIVE;
int  server_id;
} TermEdPart;

typedef struct _TermEdRec {
CorePart core;
TermEdPart termEd;
} TermEdRec;

externalref TermEdClassRec termEdClassRec;

_XFUNCPROTOEND
#endif /* XTCW_TERMEDP_H */

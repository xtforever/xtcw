/* Generated by wbuild
 * (generator version 3.3)
 */
#ifndef XTCW_WVIEWQVP_H
#define XTCW_WVIEWQVP_H
#include <xtcw/WlabelP.h>
#include <xtcw/WviewQV.h>
_XFUNCPROTOBEGIN

typedef struct {
/* methods */
/* class variables */
int dummy;
} WviewQVClassPart;

typedef struct _WviewQVClassRec {
CoreClassPart core_class;
WheelClassPart wheel_class;
WlabelClassPart wlabel_class;
WviewQVClassPart wviewQV_class;
} WviewQVClassRec;

typedef struct {
/* resources */
QVar  var;
QVar  enable;
/* private state */
int  drawn;
Bool  change_val;
} WviewQVPart;

typedef struct _WviewQVRec {
CorePart core;
WheelPart wheel;
WlabelPart wlabel;
WviewQVPart wviewQV;
} WviewQVRec;

externalref WviewQVClassRec wviewQVClassRec;

_XFUNCPROTOEND
#endif /* XTCW_WVIEWQVP_H */

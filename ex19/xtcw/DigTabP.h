/* Generated by wbuild
 * (generator version 3.3)
 */
#ifndef XTCW_DIGTABP_H
#define XTCW_DIGTABP_H
#include <../widgets/GridboxP.h>
#include <xtcw/DigTab.h>
_XFUNCPROTOBEGIN
#define CFG_OUTPUT 2 


#define CFG_STATUS 0x10 


typedef struct {
/* Constraint resources */
/* Private constraint variables */
int dummy;
} DigTabConstraintPart;

typedef struct _DigTabConstraintRec {
GridboxConstraintPart gridbox;
DigTabConstraintPart digTab;
} DigTabConstraintRec;


typedef struct {
/* methods */
/* class variables */
int dummy;
} DigTabClassPart;

typedef struct _DigTabClassRec {
CoreClassPart core_class;
CompositeClassPart composite_class;
ConstraintClassPart constraint_class;
GridboxClassPart gridbox_class;
DigTabClassPart digTab_class;
} DigTabClassRec;

typedef struct {
/* resources */
String  pin_config;
String  pin_status;
XtCallbackList  callback;
/* private state */
int  sbuf;
int  num_pins;
int  iwids;
} DigTabPart;

typedef struct _DigTabRec {
CorePart core;
CompositePart composite;
ConstraintPart constraint;
GridboxPart gridbox;
DigTabPart digTab;
} DigTabRec;

externalref DigTabClassRec digTabClassRec;

_XFUNCPROTOEND
#endif /* XTCW_DIGTABP_H */

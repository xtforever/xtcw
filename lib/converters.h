#ifndef CONVERTERS_H
#define CONVERTERS_H

#include <X11/IntrinsicP.h>

#ifndef  XtRDistance
#define  XtRDistance "Distance"
#endif

#ifndef  XtRQVar
#define  XtRQVar "QVar"
#endif

#ifndef  XtRArrayChar
#define  XtRArrayChar "ArrayChar"
#endif

#ifndef  XtRArrayInt
#define  XtRArrayInt "ArrayInt"
#endif

#ifndef XtRAnesthetic
#define XtRAnesthetic "Anesthetic"
#endif

typedef String Anesthetic;
typedef int Distance;
typedef int ArrayChar;
typedef int ArrayInt;
typedef int QVar;

void converters_init(void);

#include "long.h"
#include "icon.h"
#include "XCC.h"
#include "choosecol.h"
#include "strarray.h"
#include "StrToPmap.h"
#include "Pen.h"
#include "str2color.h"

#define done_bert(type, value) \
    do {\
	if (to->addr != NULL) {\
	    if (to->size < sizeof(type)) {\
	        to->size = sizeof(type);\
	        return False;\
	    }\
	    *(type*)(to->addr) = (value);\
        } else {\
	    static type static_val;\
	    static_val = (value);\
	    to->addr = (XtPointer)&static_val;\
        }\
        to->size = sizeof(type);\
        return True;\
    } while (0)

#define done_bob(type, value) \
   {                                    \
       if (toVal->addr != NULL) {       \
          if (toVal->size < sizeof(type)) {\
             toVal->size = sizeof(type);\
             return False;              \
          }                             \
          *(type*)(toVal->addr) = (value);\
       }                                \
       else {                           \
          static type static_val;       \
          static_val = (value);         \
          toVal->addr = (XPointer)&static_val;\
       }                                \
       toVal->size = sizeof(type);      \
   }


#endif

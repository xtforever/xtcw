#ifndef WCREG2_H
#define WCREG2_H

#include <X11/Intrinsic.h>

#define RCB(W, S) wcreg_callback( W, S,  #S );
#define RCP(W, S) wcreg_class( W, S ## WidgetClass, #S )
#define RAC(W, S) WcRegisterAction(XtWidgetToApplicationContext(W), #S, S) 

void wcreg_callback( Widget w, XtCallbackProc fn, char *s );
void wcreg_class( Widget w, void *class, char *s );
void wcreg_action( Widget w, String name, XtActionProc proc );


#endif

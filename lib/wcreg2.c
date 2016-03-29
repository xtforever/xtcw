#include "wcreg2.h"
#include "WcCreate.h"

void wcreg_callback( Widget w, XtCallbackProc fn, char *s )
{
    XtAppContext app;
    app = XtWidgetToApplicationContext(w);
    WcRegisterCallback( app, s, fn, NULL );
}

void wcreg_class( Widget w, void *class, char *s )
{
    XtAppContext app;
    app = XtWidgetToApplicationContext(w);
    WcRegisterClassPtr( app, s, class );
}

void wcreg_action( Widget w, String name, XtActionProc proc )
{
  WcRegisterAction(XtWidgetToApplicationContext(w), name, proc);

}

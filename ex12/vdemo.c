#include "mls.h"
#include "ini_read2.h"

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xmu/Editres.h>
#include <X11/Vendor.h>
#include <X11/Xaw/XawInit.h>

#include <WcCreate.h>
#include <Xp.h>
#include "wcreg2.h"


/* the new widgets */
#include "vexmo/Wdemo.h"

#define APP_NAME "vdemo"

Widget TopLevel;

char *fallback_resources[] = {
    APP_NAME "allowShellResize: True",
    "*WclResFiles:" APP_NAME ".ad\n",
  NULL };

/* All Wcl applications should provide at least these Wcl options:
*/
static XrmOptionDescRec options[] = {
  { "-TraceLevel",	"*traceLevel",	XrmoptionSepArg, NULL },
  { "-ListenPort",      "*listenPort",  XrmoptionSepArg, NULL },
  WCL_XRM_OPTIONS
};

typedef struct BasicSetting {
    int traceLevel;
    char *listenPort;
} BasicSetting;

#define FLD(n)  XtOffsetOf(BasicSetting,n)
static XtResource basicSettingRes[] = {

  { "traceLevel", "TraceLevel", XtRInt, sizeof(int),
   FLD(traceLevel), XtRImmediate, 0
  },
  { "listenPort", "ListenPort", XtRString, sizeof(String),
   FLD(listenPort), XtRString, "10000"
  }

};
#undef FLD

struct BasicSetting SETTINGS;


void slider_cb( Widget w, void *u, void *c )
{
    int pos = (int)c;
    printf( "pos: %d\n", pos );
}

void test_cb( Widget w, void *u, void *c )
{
  printf("cb\n");
}

void quit_gui( Widget w, void *u, void *c )
{
    XtAppSetExitFlag( XtWidgetToApplicationContext(w) );
}

static void wm_quit ( Widget w, XEvent *event, String *params,
		   Cardinal *num_params )
{
    XtAppSetExitFlag( XtWidgetToApplicationContext(w) );
}

/******************************************************************************
**  Private Functions
******************************************************************************/

/*ARGSUSED*/
static void RegisterApplication ( Widget top )
{
    /* -- Register widget classes and constructors */
  RCP( top, wdemo );

    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
  RCB( top, quit_gui );
  RCB( top, slider_cb );
}


static void syntax(void)
{
  puts( syntax_wcl );
  puts( "-TraceLevel <num>\n"
	"-ListenPort <num>\n" );
}



/******************************************************************************
*   MAIN function
******************************************************************************/
int main ( int argc, char **argv )
{
    XtAppContext app;
    signal(SIGPIPE, SIG_IGN); /* ignore broken pipe on write */
    m_init();

    XawInitializeWidgetSet();
    /*  -- Intialize Toolkit creating the application shell
     */
    Widget appShell = XtOpenApplication (&app, APP_NAME,
	     options, XtNumber(options),  /* resources: can be set from argv */
	     &argc, argv,
	     fallback_resources,
	     sessionShellWidgetClass,
	     NULL, 0
	   );

    /* enable Editres support */
    XtAddEventHandler(appShell, (EventMask) 0, True, _XEditResCheckMessages, NULL);

    XtAddCallback( appShell, XtNdieCallback, quit_gui, NULL );

    /* not parsed options are removed by XtOpenApplication
       the only entry left should be the program name
    */
    if (argc != 1) { m_destruct(); syntax(); exit(1); }

    TopLevel = appShell;





    /*  -- Register all application specific callbacks and widget classes
     */
    RegisterApplication ( appShell );

    /*  -- Register all Athena and Public widget classes, CBs, ACTs
     */
    XpRegisterAll ( app );

    /*  -- Create widget tree below toplevel shell using Xrm database
     */
    WcWidgetCreation ( appShell );

    /* get application resources and widget ptr */
   XtGetApplicationResources(	appShell, (XtPointer)&SETTINGS,
				basicSettingRes,
				XtNumber(basicSettingRes),
				(ArgList)0, 0 );
    /* init widgets */
   trace_level = SETTINGS.traceLevel;

    /* init application functions
       All widgets are created, but not visible.
       functions can now communicate with widgets
    */

    /*  -- Realize the widget tree and enter the main application loop
     */
    XtRealizeWidget ( appShell );

    /* if the user closes the window
       the Window Manager will call our quit() function
    */
    static XtActionsRec actions[] = {
        {"quit",	wm_quit}
    };
    XtAppAddActions
	(app, actions, XtNumber(actions));
    XtOverrideTranslations
	(TopLevel, XtParseTranslationTable ("<Message>WM_PROTOCOLS: quit()"));

    /* http://www.lemoda.net/c/xlib-wmclose/ */
    static Atom wm_delete_window;
    wm_delete_window = XInternAtom(XtDisplay(TopLevel), "WM_DELETE_WINDOW",
				   False);
    (void) XSetWMProtocols (XtDisplay(TopLevel), XtWindow(TopLevel),
                            &wm_delete_window, 1);

    XtAppMainLoop ( app ); /* use XtAppSetExitFlag */

    XtDestroyWidget(appShell);

    m_destruct();
    return EXIT_SUCCESS;
}

/* most important define,
   check APP_NAME.ad !
*/
#define APP_NAME "video"




#include "mls.h"
#include "ini_read2.h"
#include "pipeshell.h"
#include "mrb.h"
#include "micro_vars.h"


#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>              /* Obtain O_* constant definitions */

#include "parser_util.h"

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xmu/Editres.h>
#include <X11/Vendor.h>
#include <X11/Xaw/XawInit.h>

#include <WcCreate.h>
#include <Xp.h>

#include "wcreg2.h"
#include <xtcw/register_wb.h>

#include "xtcw/Wcap.h"

Widget TopLevel;
int trace_main;
#define TRACE_MAIN 1

char *fallback_resources[] = {
    APP_NAME ".allowShellResize: False",
    "*WclResFiles:" APP_NAME ".ad\n",
    "*traceLevel: 2",
    NULL };

/* All Wcl applications should provide at least these Wcl options:
*/
static XrmOptionDescRec options[] = {
  { "-TraceLevel",	"*traceLevel",	XrmoptionSepArg, NULL },

  WCL_XRM_OPTIONS
};


typedef struct BasicSetting {
    int vset;

    int traceLevel;

    Widget widget_startstop;
    Widget widget_frame;

    XtAppContext ctx;

} BasicSetting;



#define FLD(n)  XtOffsetOf(BasicSetting,n)
static XtResource basicSettingRes[] = {
    { NULL, NULL, XtRWidget, sizeof(Widget),
      FLD(widget_startstop), XtRString, "*startstop"
    },
    { NULL, NULL, XtRWidget, sizeof(Widget),
      FLD(widget_frame), XtRString, "*frame"
    },


    { "traceLevel", "TraceLevel", XtRInt, sizeof(int),
      FLD(traceLevel), XtRImmediate, 0
    }
};
#undef FLD

struct BasicSetting CONFIG;

void startstop_cb( Widget w, void *u, void *c )
{
    static int startstop = 1;
    startstop = ! startstop;
    XtVaSetValues( CONFIG.widget_frame, "grab", startstop, NULL );
    printf("grab\n");
}

void quit_gui( Widget w, void *u, void *c )
{
    TRACE(1,"QUIT");
    XtAppSetExitFlag( XtWidgetToApplicationContext(w) );
}

static void wm_quit ( Widget w, XEvent *event, String *params,
		   Cardinal *num_params )
{
    TRACE(1,"WM_QUIT");
    XtAppSetExitFlag( XtWidgetToApplicationContext(w) );
}


/* --------------------------------------------------------------------------------------------------------------------

                        IMPLEMENTATION


                        Must Provide
                        - RegisterApplication
                        - InitializeApplication

  -------------------------------------------------------------------------------------------------------------------- */

static void RegisterApplication ( Widget top )
{
    /* -- Register widget classes and constructors */
    RCP( top, wcap );

    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
    RCB( top, quit_gui );
    RCB( top, startstop_cb );

}

/*  init application functions and structures and widgets
    All widgets are created, but not visible.
    functions can now communicate with widgets
*/
static void InitializeApplication( Widget top )
{
    trace_level = CONFIG.traceLevel;
    CONFIG.vset = v_init();

    XtAppContext app = XtWidgetToApplicationContext(top);
    (void) app;
}

/******************************************************************************
**  Private Functions
******************************************************************************/

static void syntax(void)
{
  puts( syntax_wcl );
  puts( "-TraceLevel <num>\n"
	"-ListenPort <num>\n" );
}


/* if the window manager closes the window, tell the
   window manager to send a message to xt. xt will
   call the wm_quit callback.
*/
void grab_window_quit(Widget top)
{
    XtAppContext app = XtWidgetToApplicationContext(top);
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
    wm_delete_window = XInternAtom(XtDisplay(top), "WM_DELETE_WINDOW",
				   False);
    (void) XSetWMProtocols (XtDisplay(top), XtWindow(top),
                            &wm_delete_window, 1);
}



/******************************************************************************
*   MAIN function
******************************************************************************/
int main ( int argc, char **argv )
{
    trace_main = TRACE_MAIN;

    XtAppContext app;
    signal(SIGPIPE, SIG_IGN); /* ignore broken pipe on write */
    m_init();
    trace_level = trace_main;

    XtSetLanguageProc (NULL, NULL, NULL);
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
    XtGetApplicationResources(	appShell, (XtPointer)&CONFIG,
				basicSettingRes,
				XtNumber(basicSettingRes),
				(ArgList)0, 0 );


    InitializeApplication(appShell);

    /*  -- Realize the widget tree and enter the main application loop
     */
    XtRealizeWidget ( appShell );
    grab_window_quit( appShell );


    XtAppMainLoop ( app ); /* use XtAppSetExitFlag */
    XtDestroyWidget(appShell);

    v_free(CONFIG.vset);
    m_destruct();

    return EXIT_SUCCESS;
}

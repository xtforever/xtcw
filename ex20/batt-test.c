/* most important define,
   check APPNAME.ad !
*/
#define APP_NAME "batt-test"

/*
 */


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

#include "xtcw/Wbatt.h"



Widget TopLevel;
int DB;
int trace_main;
int q_filter;

#define TRACE_MAIN 2


char *fallback_resources[] = {
    APP_NAME ".allowShellResize: False",
    "*WclResFiles:" APP_NAME ".ad\n",
    "*traceLevel: 2",
    NULL };

/* All Wcl applications should provide at least these Wcl options:
*/
static XrmOptionDescRec options[] = {
  { "-TraceLevel",	"*traceLevel",	XrmoptionSepArg, NULL },
  { "-ListenPort",      "*listenPort",  XrmoptionSepArg, NULL },
  WCL_XRM_OPTIONS
};


typedef struct BasicSetting {
    int vset;

    int filelist;
    int traceLevel;
    char *listenPort;
    Widget widget_filelist;
    Widget widget_entry;
    Widget widget_filter;

} BasicSetting;



#define FLD(n)  XtOffsetOf(BasicSetting,n)
static XtResource basicSettingRes[] = {

    { NULL, NULL, XtRWidget, sizeof(Widget),
      FLD(widget_filelist), XtRString, "*fileList"
    },
    { NULL, NULL, XtRWidget, sizeof(Widget),
      FLD(widget_entry), XtRString, "*entry"
    },
    { NULL, NULL, XtRWidget, sizeof(Widget),
      FLD(widget_filter), XtRString, "*filter"
    },

  { "listenPort", "ListenPort", XtRString, sizeof(String),
   FLD(listenPort), XtRString, "10000"
  },

  { "traceLevel", "TraceLevel", XtRInt, sizeof(int),
   FLD(traceLevel), XtRImmediate, 0
  }



};
#undef FLD

struct BasicSetting SETTINGS;

void test_cb( Widget w, void *u, void *c )
{
  printf("cb\n");
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
static XrmQuark q_en, q_rst;
static void switch_enable( Widget w, void *u, void *c );
static void inc_rst( Widget w, void *u, void *c );
static void quarks_init(void);

static void RegisterApplication ( Widget top )
{
    /* -- Register widget classes and constructors */
    RCP( top, wbatt );

    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
    RCB( top, quit_gui );
    RCB( top, switch_enable );
    RCB( top, inc_rst );
}

/*  init application functions and structures and widgets
    All widgets are created, but not visible.
    functions can now communicate with widgets
*/
static void InitializeApplication( Widget top )
{
    trace_level = SETTINGS.traceLevel;
    SETTINGS.filelist = m_create(100, sizeof(char*));
    SETTINGS.vset = v_init();
    quarks_init();
}


static void quarks_init(void)
{
    q_en  = XrmStringToQuark( "en" );
    q_rst = XrmStringToQuark( "rst" );
}

static void switch_enable( Widget w, void *u, void *c )
{

    int en = *mv_var(q_en);
    TRACE(1,"en: %d", en);
    mv_write( q_en, !en );
}

static void inc_rst( Widget w, void *u, void *c )
{
    TRACE(1,"");
    int rst = *mv_var(q_rst);
    mv_write( q_rst, rst + 1);
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
    mv_init();


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
    XtGetApplicationResources(	appShell, (XtPointer)&SETTINGS,
				basicSettingRes,
				XtNumber(basicSettingRes),
				(ArgList)0, 0 );

    InitializeApplication(appShell);





    /*  -- Realize the widget tree and enter the main application loop
     */
    XtRealizeWidget ( appShell );
    grab_window_quit( appShell );
    //    pin_start_test();

    XtAppMainLoop ( app ); /* use XtAppSetExitFlag */
    XtDestroyWidget(appShell);
    m_destruct();

    return EXIT_SUCCESS;
}

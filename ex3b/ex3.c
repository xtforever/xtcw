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

#include <WcCreate.h>
#include <Xp.h>
#include "wcreg2.h"
#include <vexmo/register_wb.h>

#define MAX_MSG_SIZE 10000
#define APP_NAME "ex3"
#define APP_INI_FILE APP_NAME ".ini"
Widget TopLevel, TopMgr, widget_data;
int AutoCreateChildren = 1;

int com_data = 0, wlist = 0;

char *fallback_resources[] = {
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

void test_cb( Widget w, void *u, void *c )
{
  printf("cb\n");
}

void quit_gui( Widget w, void *u, void *c )
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
    /*
    RCP( "Wsensor", wsensorWidgetClass );
    RCP( "Wtext", wtextWidgetClass );
    */
    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
  RCB( top, quit_gui );
  RCB( top, test_cb );

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

    /*  -- Intialize Toolkit creating the application shell
     */
    Widget appShell = XtOpenApplication (&app, APP_NAME,
	     options, XtNumber(options),  /* resources: can be set from argv */
	     &argc, argv,
	     fallback_resources,
	     applicationShellWidgetClass,
	     NULL, 0
	   );

    /* enable Editres support */
    XtAddEventHandler(appShell, (EventMask) 0, True, _XEditResCheckMessages, NULL);

    /* not parsed options are removed by XtOpenApplication
       the only entry left should be the program name
    */
    if (argc != 1) { m_destruct(); syntax(); exit(1); }

    TopLevel = appShell;
    XtGetApplicationResources(	appShell, (XtPointer)&SETTINGS,
				basicSettingRes,
				XtNumber(basicSettingRes),
				(ArgList)0, 0 );

    rc_init( APP_INI_FILE );
    trace_level = SETTINGS.traceLevel;

    /*  -- Register all application specific callbacks and widget classes
     */
    RegisterApplication ( appShell );

    /*  -- Register all Athena and Public widget classes, CBs, ACTs
     */
    XpRegisterAll ( app );

    /*  -- Create widget tree below toplevel shell using Xrm database
     */
    WcWidgetCreation ( appShell );

    /* init global widget ptr */
    // widget_data = WcFullNameToWidget( appShell, "*ls" );
    // assert( widget_data );

    /* init application functions
       All widgets are created, but not visible.
       functions can now communicate with widgets
    */
    // slog_init( appShell, SETTINGS.listenPort, data_received );


    /*  -- Realize the widget tree and enter the main application loop
     */
    XtRealizeWidget ( appShell );



    XtAppMainLoop ( app );
    m_destruct();
    return EXIT_SUCCESS;
}

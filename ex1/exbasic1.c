#include "mls.h"
#include "ini_read2.h"


#include <signal.h>
#include <stdlib.h>
#include <math.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>


#include <WcCreate.h>
#include <Xp.h>
#include "wcreg2.h"

#define APP_NAME "exbasic1"
#define APP_INI_FILE APP_NAME ".ini"
Widget TopLevel, TopMgr;
int AutoCreateChildren = 1;



char *fallback_resources[] = {
  "*WclResFiles:" APP_NAME ".ad\n",
  NULL };


/* All Wcl applications should provide at least these Wcl options:
*/
static XrmOptionDescRec options[] = {
    WCL_XRM_OPTIONS
};


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
}


/******************************************************************************
*   MAIN function
******************************************************************************/
int main ( int argc, char **argv )
{
    XtAppContext app;
    signal(SIGPIPE, SIG_IGN); /* broken pipe on write */

    trace_level=0; /* disable debug info */
    m_init();
    rc_init( APP_INI_FILE );

    /*  -- Intialize Toolkit creating the application shell
     */
    Widget appShell = XtOpenApplication (&app, APP_NAME,
	     options, XtNumber(options),  /* resources: can be set from argv */
	     &argc, argv,
	     fallback_resources,
	     applicationShellWidgetClass,
	     NULL, 0
	   );
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


    /*  -- Realize the widget tree and enter the main application loop
     */
    XtRealizeWidget ( appShell );


    XtAppMainLoop ( app );
    m_destruct();
    return EXIT_SUCCESS;
}

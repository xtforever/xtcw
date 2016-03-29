#include <stdlib.h>
#include <math.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <WcCreate.h>
#include <Xp.h>
#include "mls.h"
#include <X11/Shell.h>
#include "WiconFlip.h"
#include "vexmo/Wsensor.h"
#include "vexmo/Wtext.h"
#include "vexmo/Wcheck.h"
#include "Tachometer.h"


/* All Wcl applications should provide at least these Wcl options:
*/
static XrmOptionDescRec options[] = {
    WCL_XRM_OPTIONS
};


/******************************************************************************
**  Private Functions
******************************************************************************/

/*ARGSUSED*/
static void RegisterApplication ( XtAppContext app )
{
    /* -- Useful shorthand for registering things with the Wcl library */
#define RCP( name, class  ) WcRegisterClassPtr   ( app, name, class );
#define RCO( name, constr ) WcRegisterConstructor( app, name, constr );
#define RAC( name, func   ) WcRegisterAction     ( app, name, func );
#define RCB( name, func   ) WcRegisterCallback   ( app, name, func, NULL );

    /* -- Register widget classes and constructors */
    /*
    RCP( "Wsensor", wsensorWidgetClass );
    RCP( "Wtext", wtextWidgetClass );
    */
    RCP( "Tachometer", tachometerWidgetClass );
    RCP( "Wcheck", wcheckWidgetClass ); 
    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
}


/******************************************************************************
*   MAIN function
******************************************************************************/
int main ( int argc, char **argv )
{
    XtAppContext app;

    trace_level=2;
    m_init();

    /*  -- Intialize Toolkit creating the application shell
     */
    Widget appShell = XtOpenApplication (&app, "vexmo-test",
	     options, XtNumber(options),  /* resources: can be set from argv */
	     &argc, argv,
	     NULL,
	     applicationShellWidgetClass,
	     NULL, 0
	   );


    /*  -- Register all application specific callbacks and widget classes
     */
    RegisterApplication ( app );

    /*  -- Register all Athena and Public widget classes, CBs, ACTs
     */
    XpRegisterAll ( app );

    /*  -- Create widget tree below toplevel shell using Xrm database
     */
    WcWidgetCreation ( appShell );

    /*  -- Realize the widget tree and enter the main application loop
     */
    XtRealizeWidget ( appShell );

    /* realize application objects */

    XtAppMainLoop ( app );

    m_destruct();
    return EXIT_SUCCESS;
}

/* rework : au1064@gmail.com 03.05.2013 */
/* rework : au1064@gmail.com 25.03.2014 */


/*
* SCCS_data: @(#) Xp.c	1.3 94/09/22 14:27:53
*
*     This module contains registration routine for all Athena
*     widget constructors and classes.
*
* Module_interface_summary:
*
*     void XpRegisterAthena  ( XtAppContext app )
*     void XpRegisterAll     ( XtAppContext app )
*     void AriRegisterAthena ( XtAppContext app )
*
*******************************************************************************
*/

#include <X11/Xatom.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include "WcCreateP.h"
#include "Xp.h"



#ifdef DEBUG
#include "XpAthenaP.h"
#else
#include "XpAthena.h"
#endif

/* --  Register all Athena widgets, converters, callbacks, actions.
*******************************************************************************
*/
#define RCP( name, class ) WcRegisterClassPtr  ( app, name, class );
#define RCO( name, func )  WcRegisterConstructor(app, name, func  )
#include "register_wb.h"

void AriRegisterAthena ( app )
    XtAppContext app;
{
    XpRegisterAthena(app);
}

void XpRegisterAll ( app )
    XtAppContext app;
{
    XpRegisterAthena(app);
}

void XpRegisterAthena ( app )
    XtAppContext app;
{
    ONCE_PER_XtAppContext( app );

/* -- Force Athena to load XmuNewCvtStringToWidget so
 *    WcCvtStringToWidget stays in effect when loaded by WcInitialize()
 */
    XtInitializeWidgetClass( toggleWidgetClass );
    XtInitializeWidgetClass( formWidgetClass );

    /* -- register all Xp widget classes */
    RCP("XpTable",			xpTableWidgetClass	);
    RCP("xpTableWidgetClass",		xpTableWidgetClass	);

    /* layout manager */
    RCP("Gridbox",			gridboxWidgetClass	);

    /* -- register all Athena widget classes */
    /* Simple Widgets (Chapt 3) */
    RCP("Command",			commandWidgetClass	);
    RCP("commandWidgetClass",		commandWidgetClass	);
    RCP("Core",				coreWidgetClass		);
    RCP("coreWidgetClass",		coreWidgetClass		);
    RCP("Grip",				gripWidgetClass		);
    RCP("gripWidgetClass",		gripWidgetClass		);
    RCP("Label",			labelWidgetClass	);
    RCP("labelWidgetClass",		labelWidgetClass	);
    RCP("List",				listWidgetClass		);
    RCP("listWidgetClass",		listWidgetClass		);
    RCP("Object",			objectClass		);
    RCP("objectClass",			objectClass		);

    RCP("Panner",			pannerWidgetClass	);
    RCP("pannerWidgetClass",		pannerWidgetClass	);
    RCP("Porthole",			portholeWidgetClass	);
    RCP("portholeWidgetClass",		portholeWidgetClass	);

    RCP("Rect",				rectObjClass		);
    RCP("rectObjClass",			rectObjClass		);

    RCP("Repeater",			repeaterWidgetClass	);
    RCP("repeaterWidgetClass",		repeaterWidgetClass	);

    RCP("Scrollbar",			scrollbarWidgetClass	);
    RCP("scrollbarWidgetClass",		scrollbarWidgetClass	);
    RCP("Simple",			simpleWidgetClass	);
    RCP("simpleWidgetClass",		simpleWidgetClass	);
    RCP("StripChart",			stripChartWidgetClass	);
    RCP("stripChartWidgetClass",	stripChartWidgetClass	);
    RCP("Toggle",			toggleWidgetClass	);
    RCP("toggleWidgetClass",		toggleWidgetClass	);

    /* Menus (Chapt 4) */
    RCP("SimpleMenu",			simpleMenuWidgetClass	);
    RCP("simpleMenuWidgetClass",	simpleMenuWidgetClass	);
    RCO("XawCreateSimpleMenu",		XpCreateSimpleMenu	);
    RCP("SmeBSB",			smeBSBObjectClass	);
    RCP("smeBSBObjectClass",		smeBSBObjectClass	);
    RCP("SmeLine",			smeLineObjectClass	);
    RCP("smeLineObjectClass",		smeLineObjectClass	);
    RCP("Sme",				smeObjectClass		);
    RCP("smeObjectClass",		smeObjectClass		);
    RCP("MenuButton",			menuButtonWidgetClass	);
    RCP("menuButtonWidgetClass",	menuButtonWidgetClass	);

    /* Text Widgets (Chapt 5) */
    RCP("AsciiText",			asciiTextWidgetClass	); /* NB name */
    RCP("asciiTextWidgetClass",		asciiTextWidgetClass	);
    RCP("AsciiSrc",			asciiSrcObjectClass	);
    RCP("asciiSrcObjectClass",		asciiSrcObjectClass	);
    RCP("AsciiSink",			asciiSinkObjectClass	);
    RCP("asciiSinkObjectClass",		asciiSinkObjectClass	);
    RCP("TextSink",			textSinkObjectClass	);
    RCP("textSinkObjectClass",		textSinkObjectClass	);
    RCP("TextSrc",			textSrcObjectClass	);
    RCP("textSrcObjectClass",		textSrcObjectClass	);
    RCP("Text",				textWidgetClass		);
    RCP("textWidgetClass",		textWidgetClass		);

    /* Composite and Constraint Widgets (Chapt 6) */
    RCP("ApplicationShell",		applicationShellWidgetClass	);
    RCP("applicationShellWidgetClass",	applicationShellWidgetClass	);
    RCP("Box",				boxWidgetClass		);
    RCP("boxWidgetClass",		boxWidgetClass		);
    RCP("Composite",			compositeWidgetClass	);
    RCP("compositeWidgetClass",		compositeWidgetClass	);
    RCP("Constraint",			constraintWidgetClass	);
    RCP("constraintWidgetClass",	constraintWidgetClass	);
    RCP("Dialog",			dialogWidgetClass	);
    RCP("dialogWidgetClass",		dialogWidgetClass	);
    RCP("Form",				formWidgetClass		);
    RCP("formWidgetClass",		formWidgetClass		);
    RCP("OverrideShell",		overrideShellWidgetClass	);
    RCP("overrideShellWidgetClass",	overrideShellWidgetClass	);
    RCP("Paned",			panedWidgetClass	);
    RCP("panedWidgetClass",		panedWidgetClass	);
    RCP("Shell",			shellWidgetClass	);
    RCP("shellWidgetClass",		shellWidgetClass	);
    RCP("TopLevelShell",		topLevelShellWidgetClass	);
    RCP("topLevelShellWidgetClass",	topLevelShellWidgetClass	);
    RCP("TransientShell",		transientShellWidgetClass	);
    RCP("transientShellWidgetClass",	transientShellWidgetClass	);

    RCP("Tree",				treeWidgetClass		);
    RCP("treeWidgetClass",		treeWidgetClass		);

    RCP("Viewport",			viewportWidgetClass	);
    RCP("viewportWidgetClass",		viewportWidgetClass	);
    RCP("WmShell",			wmShellWidgetClass	);
    RCP("wmShellWidgetClass",		wmShellWidgetClass	);



    VexmoRegister(app);
}

/*
    -- Create SimpleMenu as pop-up child
*******************************************************************************
*/

Widget XpCreateSimpleMenu( pw, name, args, nargs )
    Widget      pw;     /* children's parent                            */
    String      name;   /* widget name to create                        */
    Arg        *args;   /* args for widget                              */
    Cardinal    nargs;  /* args count                                   */
{
  return XtCreatePopupShell(name, simpleMenuWidgetClass, pw, args, nargs);
}

/* backward compatibility:
*/
Widget WcCreateSimpleMenu( pw, name, args, nargs )
    Widget      pw;     /* children's parent                            */
    String      name;   /* widget name to create                        */
    Arg        *args;   /* args for widget                              */
    Cardinal    nargs;  /* args count                                   */
{
  return XtCreatePopupShell(name, simpleMenuWidgetClass, pw, args, nargs);
}

/*
    -- Append to end of XawText widget
*******************************************************************************
*/

int XpTextAppend( w, msg )
    Widget w;
    char*  msg;
{
    XawTextBlock textBlock;
    XawTextPosition textPos;

    textPos = XawTextGetInsertionPoint( w );
    textBlock.firstPos = 0;			/* start at ptr */
    textBlock.length = strlen( msg );
    textBlock.ptr = msg;
    textBlock.format = FMT8BIT;
    return XawTextReplace( w, textPos, textPos, &textBlock );
}

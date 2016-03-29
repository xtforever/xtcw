/*
* SCCS_data: %Z% %M%	%I% %E% %U%
*
* Widget Creation Library - WcCreate.c
*
* This module contains the functions used to create and manage a widget tree
* using the Xrm database.  Also, library global data is allocated here.
*
*******************************************************************************
*/

#include <X11/IntrinsicP.h>

#ifdef sun
#include <X11/ObjectP.h>	/* why don't they just use X from mit!?! */
#include <X11/RectObjP.h>
#endif

#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include "MapAgP.h"	/* for MapAg_STATIC */
#include "WcCreateP.h"

/*
*******************************************************************************
* Wcl Global Data Allocations
*******************************************************************************
*/

const char *syntax_wcl =
"Most X programs attempt to use the same names for command line options and arguments. \n"
"All applications written with the X Toolkit Intrinsics automatically accept the following options:\n"
"\n"
"-display display\n"
"\tThis option specifies the name of the X server to use.\n"
"\n"
"-geometry geometry\n"
"\tThis option specifies the initial size and location of the window.\n"
"\n"
"-bg color, -background color\n"
"\tEither option specifies the color to use for the window background.\n"
"\n"
"-bd color, -bordercolor color\n"
"\tEither option specifies the color to use for the window border.\n"
"\n"
"-bw number, -borderwidth number\n"
"\tEither option specifies the width in pixels of the window border. \n"
"\n"
"-fg color, -foreground color\n"
"\tEither option specifies the color to use for text or graphics. \n"
"\n"
"-fn font, -font font\n"
"\tEither option specifies the font to use for displaying text. \n"
"\n"
"-iconic\n"
"\tThis option indicates that the user would prefer that the application's windows initially \n"
"        not be visible as if the windows had be immediately iconified by the user. \n"
"        Window managers may choose not to honor the application's request. \n"
"\n"
"-name\n"
"\tThis option specifies the name under which resources for the application should be found. \n"
"        This option is useful in shell aliases to distinguish between invocations of an application, \n"
"        without resorting to creating links to alter the executable file name.\n"
"\n"
"-rv, -reverse\n"
"\tEither option indicates that the program should simulate reverse video if possible, \n"
"        often by swapping the foreground and background colors. \n"
"        Not all programs honor this or implement it correctly. It is usually only used on monochrome displays. \n"
"\n"
"+rv\n"
"\tThis option indicates that the program should not simulate reverse video. \n"
"        This is used to override any defaults since reverse video doesn't always work properly. \n"
"\n"
"-selectionTimeout\n"
"\tThis option specifies the timeout in milliseconds within which two communicating \n"
"        applications must respond to one another for a selection request. \n"
"\n"
"-synchronous\n"
"\tThis option indicates that requests to the X server should be sent synchronously, \n"
"        instead of asynchronously. Since Xlib normally buffers requests to the server, \n"
"        errors do not necessarily get reported immediately after they occur. \n"
"        This option turns off the buffering so that the application can be debugged. \n"
"        It should never be used with a working program. \n"
"\n"
"-title string\n"
"\tThis option specifies the title to be used for this window. This information is sometimes \n"
"        used by a window manager to provide some sort of header identifying the window. \n"
"\n"
"-xnllanguage language[_territory][.codeset]\n"
"\tThis option specifies the language, territory, and codeset for use in \n"
"        resolving resource and other filenames. \n"
"\n"
"-xrm resourcestring\n"
"\tThis option specifies a resource name and value to override any defaults. \n"
"        It is also very useful for setting resources that don't have explicit command line arguments.\n"
"\n"
"\n"
"*wclInitResFile <filenames>\n"
"*WclResFiles <filenames>\n"
"\tList of files to be loaded into resource db.\n"
"\n"
"*wclTraceResFiles\n"
"        Trace loading of files by Wcl to stderr.\n"
"\n"
"*wcTrace\n"
"        Enable debug output to stderr.\n"
"\n"
"-ResFile <files>\n"
"-rf <files>\n"
"\tList of files to be loaded into resource db. ONLY command line\n"
"\n"
"-Trace\n"
"-tr\n"
"-trtd\n"
"-trtx\n"
"-trrf \n"
"        Trace loading of files by Wcl to stderr.\n"
"\n"
"-Warnings\n"
"        Show warnings on stderr\n"
"\n"
"WclResFiles\t\t\tList of files to be loaded into resource db.\n"
"WclInitResFile                  ONLY set by command line.\n"
"WclTraceResFiles\t\tTrace loading of files by Wcl to stderr.\n"
"WclErrorDatabaseFile\t\tFile with overrides for Wcl warning messages.\n"
"WclWidgetResourceFiles          Files with widget resources not reported by\n"
"\t\t\t\tXtGetConstraintResourceList() nor by\n"
"\t\t\t\tXtGetResourceList(). ** not yet implemented **\n"
"WclTemplateFiles\t\tList of files with templates.\n"
"WclTraceTemplateDef\t\tTrace definition of templates to stderr.\n"
"WclVerboseWarnings\t\tSome warnings are supressed normally, but this\n"
"\t\t\t\tresource causes Wcl to act like ANSI C and\n"
"\t\t\t\tgive somewhat informative but usually annoying\n"
"\t\t\t\tmessages.\n"
"WclDynamicLibs                  List of full pathnames of libraries which\n"
"\t\t\t\tcan be named using -lName shorthand in\n"
"\t\t\t\tWcDynamicAction(), WcDynamicCallback(), and in\n"
"\t\t\t\tthe new (Wcl 2.02) callback and method syntax.\n"
"WclSlowTightNames\t\tDefault: False.  Causes WcFullNameToWidget()\n"
"\t\t\t\tto use the same algorithm as used by the Athena\n"
"\t\t\t\tstring-to-widget converter for widget names\n"
"\t\t\t\tstarting with `*' - specifically, the reference\n"
"\t\t\t\twidget is moved up in the widget tree, parent\n"
"\t\t\t\tby parent, with a call to XtNameToWidget()\n"
"\t\t\t\tat each parent, until the top of the tree is\n"
"\t\t\t\treached.  This is slower than the default\n"
"\t\t\t\talgorithm which tries the reference widget,\n"
"\t\t\t\tthen the root of the widget tree containing\n"
"\t\t\t\tthe reference widget.\n"
"WclAppClassName                 If non-null, then this name will be used as\n"
"\t\t\t\tthe class name for all widgets created using\n"
"\t\t\t\tWcCreateRoot().\n"
  "";




/*  -- Wcl rec
*******************************************************************************
    This contains parameters which are specific to an applications use of the
    Widget Creation Library.  It is somewhat analogous to the XtAppContext.
*/
static WclRec wclRec;
       Wcl    wcl = &wclRec;

/*  -- Agents: dyn-lib, callbacks, closures, classes, constructors
*******************************************************************************
    In each case, we must allocate and initialize the storage for the agent
    object, as well as the pointer to the record.
*/
static MapAgRec dlAgentRec  = MapAg_STATIC;
       MapAg    dlAgent     = &dlAgentRec;

static MapAgRec cbAgentRec  = MapAg_STATIC;
       MapAg    cbAgent     = &cbAgentRec;

static MapAgRec cdAgentRec  = MapAg_STATIC;
       MapAg    cdAgent     = &cdAgentRec;

static MapAgRec clAgentRec  = MapAg_STATIC;
       MapAg    clAgent     = &clAgentRec;

static MapAgRec conAgentRec = MapAg_STATIC;
       MapAg    conAgent    = &conAgentRec;

static MapAgRec rfAgentRec = MapAg_STATIC;
       MapAg    rfAgent    = &rfAgentRec;

static MapAgRec temAgentRec = MapAg_STATIC;
       MapAg    temAgent    = &temAgentRec;


/*
*******************************************************************************
* Private Macros 
*******************************************************************************
*/

/* Creation Types
*/
#define WcROOT   0
#define WcCREATE 1
#define WcPOPUP  2

/*
*******************************************************************************
* Xrm Database Resource Definitions and Access Functions
*******************************************************************************
*/

/*  -- Wcl Library Control Resources
************************************
    For run-time efficientcy, the resources are broken up into three
    groups.  We want to load all of the resource files before getting
    any other wcl resources from Xrm.
*/

#ifdef XtOffsetOf
#define FLD(n)  XtOffsetOf(WclRec,n)
#else
#define FLD(n)  XtOffset(WclRecPtr,n)
#endif

static XtResource wclInitResFileResources[] = {
 { WcNwclTraceResFiles, WcCWclTraceResFiles, XtRBoolean, sizeof(Boolean),
   FLD(traceResFiles), XtRImmediate, 0
 },
 { WcNwclInitResFile, WcCWclInitResFile, XtRString, sizeof(String),
   FLD(initResFile), XtRString, ""
 },
};

static XtResource wclResFilesResources[] = {
 { WcNwclTraceResFiles, WcCWclTraceResFiles, XtRBoolean, sizeof(Boolean),
   FLD(traceResFiles), XtRImmediate, 0
 },
 { WcNwclResFiles, WcCWclResFiles, XtRString, sizeof(String),
   FLD(resFiles), XtRString, ""
 },
};

static XtResource wclResources[] = {
 { WcNwclErrorDatabaseFile, WcCWclErrorDatabaseFile, XtRString, sizeof(String),
   FLD(errorDatabaseFile), XtRString, (caddr_t)WCL_ERRORDB
 },
 { WcNwclWidgetResourceFiles, WcCWclWidgetResourceFiles,
   XtRString, sizeof(String),
   FLD(widgetResourceFiles), XtRString, ""
 },
 { WcNwclTemplateFiles, WcCWclTemplateFiles, XtRString, sizeof(String),
   FLD(templateFiles), XtRString, ""
 },
 { WcNwclTraceTemplateDef, WcCWclTraceTemplateDef, XtRBoolean, sizeof(Boolean),
   FLD(traceTemplateDef), XtRImmediate, 0
 },
 { WcNwclVerboseWarnings, WcCWclVerboseWarnings, XtRBoolean, sizeof(Boolean),
   FLD(verboseWarnings), XtRImmediate, 0
 },
 { WcNwclDynamicLibs, WcCWclDynamicLibs, WcRDynamicLibs, sizeof(String),
   FLD(dynamicLibs), XtRString, ""
 },
 { WcNwclSlowTightNames, WcCWclSlowTightNames, XtRBoolean, sizeof(Boolean),
   FLD(slowTightNames), XtRImmediate, 0
 },
 { WcNwclAppClassName, WcCWclAppClassName, XtRString, sizeof(String),
   FLD(appClassName), XtRString, ""
 },
};

#undef FLD

/*  -- Creation resources and macros to make them easy to read.
*/

#ifdef XtOffsetOf
#define FLD(n)	XtOffsetOf(ResourceRec,n)
#else
#define FLD(n)	XtOffset(ResourceRecPtr,n)
#endif

#define ToStr	XtRString,	sizeof(String)
#define ToQRec	WcRQuarkRec,	sizeof(QuarkRec)
#define ToQuark	WcRQuark,	sizeof(XrmQuark)
#define ToBool	XtRBoolean,	sizeof(Boolean)
#define ToCB	XtRCallback,	sizeof(XtCallbackList)

static QuarkRec nullQuarkRec = { NULL, NULLQUARK };

#define DEF_NULL  XtRImmediate,	(caddr_t) NULL
#define DEF_NOQR  WcRQuarkRec,	(caddr_t) &nullQuarkRec
#define DEF_NOQ   WcRQuark,	(caddr_t) NULLQUARK
#define DEF_YES   XtRImmediate,	(caddr_t) TRUE
#define DEF_NO    XtRImmediate,	(caddr_t) FALSE

/* -- Resources fetched before a widget is created
*/
static XtResource wcPreRes[] = {
 { WcNwcPreCreateDumpResources,
			WcCWcDumpResources, ToBool, FLD(preCreateDump),	DEF_NO
 },
 { WcNwcTemplate,	WcCWcTemplate,      ToQuark,FLD(template),	DEF_NOQ
 },
 { WcNwcTraceTemplate,	WcCWcTraceTemplate, ToBool, FLD(traceTemplate),	DEF_NO
 },
 { WcNwcPostTemplateDumpResources,
			WcCWcDumpResources, ToBool, FLD(postTemplateDump),
									DEF_NO
 },
 { WcNwcCreate,		WcCWcCreate,	    ToQRec ,FLD(create),	DEF_NOQR
 },
 /**************************** THESE ARE OBSOLETE ****************************/
 { WcNwcClassName,	WcCWcClassName,	    ToQRec ,FLD(className),	DEF_NOQR
 },
 { WcNwcClass,		WcCWcClass,	    ToQRec ,FLD(class),		DEF_NOQR
 },
 { WcNwcConstructor,	WcCWcConstructor,   ToQRec ,FLD(constructor),	DEF_NOQR
 },
};

/* -- Resources fetched after a widget is created
*/
static XtResource wcPostRes[] = {
 { WcNwcTrace,		WcCWcTrace,	    ToBool, FLD(trace),		DEF_NO
 },
 { WcNwcPostCreateDumpResources,
			WcCWcDumpResources, ToBool, FLD(postCreateDump),
									DEF_NO
 },
 { WcNwcCallback,	WcCWcCallback,	    ToCB,   FLD(callback),	DEF_NULL
 },
 { WcNwcPopups,		WcCWcPopups,	    ToStr,  FLD(popups),	DEF_NULL
 },
 { WcNwcChildren,	WcCWcChildren,	    ToStr,  FLD(children),	DEF_NULL
 },
 { WcNwcManaged,	WcCWcManaged,	    ToBool, FLD(managed),	DEF_YES
 },
 { WcNwcAfterPopups,	WcCWcAfterPopups,   ToCB,   FLD(afterPopups),	DEF_NULL
 },
 { WcNwcAfterChildren,	WcCWcAfterChildren, ToCB,   FLD(afterChildren),	DEF_NULL
 },
 { WcNwcAfterManageChildren,
			WcCWcAfterManageChildren,
					    ToCB,   FLD(afterManageChildren),
									DEF_NULL
 }
};

/* Forward Declarations of Static Functions
*/

static Widget WcxCreate			_(( ResourceRec*, Widget, char*, int ));

/*
*******************************************************************************
* Get Wc* Resources from Xrm resource database
*******************************************************************************
    WcxGetPreRes()  only gets resources required BEFORE a widget is created.
    WcxGetPostRes() only gets resources required AFTER  a widget is created.
    This reduces resource conversions.
*/

static void WcxGetPreRes( rp, pw, nm )
    ResourceRec* rp;
    Widget	 pw;
    char*	 nm;
{
    XtGetSubresources(	pw, (XtPointer)rp, nm, nm,
			wcPreRes, XtNumber(wcPreRes), NULL, 0 );

    if ( rp->preCreateDump )
	WcPreCreateDumpResources( pw, nm, stderr );

    /*
     * Apply template, if any.
     */
    if ( NULLQUARK != rp->template )
    {
	if ( WcApplyTemplate( rp->template, pw, nm, rp->traceTemplate ) )
	{
	    XtGetSubresources(	pw, (XtPointer)rp, nm, nm,
				wcPreRes, XtNumber(wcPreRes),0,0);
	    if ( rp->postTemplateDump )
		WcPreCreateDumpResources( pw, nm, stderr );
	}
	else
	{
	    char* parentName = WcWidgetToFullName( pw );

	    WcWARN3( pw, "WcCreate", "templateNotFound",
		"Wcl Warning: %s.%s cannot find template %s",
		parentName, nm, XrmQuarkToString(rp->template) );
	    XtFree( parentName );
	}
    }
}

static void WcxGetPostRes( rp, w )
    ResourceRec* rp;
    Widget       w;
{
    XtGetApplicationResources(	w, (XtPointer)rp,
				wcPostRes, XtNumber(wcPostRes), NULL, 0 );
}

/*
*******************************************************************************
* Create Widgets from Quarks obtained from XrmDatabase
*******************************************************************************
*/

/*  -- Create Widget using WcCreate (or WcClass, WcClassName, or WcConstructor)
*******************************************************************************
    We got a quark from the Xrm database which indicates the class or the
    constructor of the new widget.  Get the WidgetClass pointer or the
    Constructor function address from the Wcl Mapping Agent using this quark.
*/

static Widget WcxCreateFromQuark( rp, pw, name, creationType )
    ResourceRec* rp;
    Widget       pw;
    char*        name;
    int		 creationType;
{
    XtAppContext	app = XtWidgetToApplicationContext( pw );
    WcWidgetConstructor	Constructor;
    WidgetClass		class;
    char*		stringInResDb;


    if ( NULLQUARK == rp->create.quark && NULLQUARK == rp->constructor.quark 
      && NULLQUARK == rp->class.quark  && NULLQUARK == rp->className.quark )
    {
	char* parentName = WcWidgetToFullName( pw );

	WcWARN2( pw, "WcCreate", "missingCreate", 
"Wcl Warning: %s.%s.wcCreate resource missing - need class or constructor",
		parentName, name );

	XtFree( parentName );
	return NULL;
    }

    /* Look for constructor specification
    */
    if	( ( (NULLQUARK != rp->create.quark) && (NULL != 
	   (Constructor = WcMapConstructorFind( app, rp->create.quark )) ))
	||( (NULLQUARK != rp->constructor.quark) && (NULL !=
	   (Constructor = WcMapConstructorFind( app, rp->constructor.quark)) ))
	)
    {
	/* Found widget constructor registered with this name.
	** No specification of what happens when constructors fail.
	*/

	Widget child = Constructor( pw, name, NULL, 0 );

	if (NULL == child)
	{
	    char* constNameFromResDb;
	    if (NULLQUARK != rp->create.quark)
		constNameFromResDb = rp->create.string;
	    else
		constNameFromResDb = rp->constructor.string;
	    WcWARN3( pw, "WcCreate", "constructorFailed",
		"Wcl Warning: %s( %s, %s, NULL, 0) - Failed!",
		constNameFromResDb, XtName(pw), name );
	}

	return child;
    }

    /* Look for class or class name specification
    */
    if	( ( (NULLQUARK != rp->create.quark) && (NULL != 
	   (class = WcMapClassFind( app, rp->create.quark )) ))
	||( (NULLQUARK != rp->class.quark) && (NULL !=
	   (class = WcMapClassFind( app, rp->class.quark )) ))
	||( rp->className.quark && (NULL !=
	   (class = WcMapClassFind( app, rp->className.quark )) ))
	)
    {
	/* Found widget class registered with this name.
	** According to the spec, Xt exits if it cannot create a widget.
	*/
    
	if (WcPOPUP == creationType)
	    return XtCreatePopupShell( name, class, pw, NULL, 0 );
	else
	    return XtCreateWidget    ( name, class, pw, NULL, 0 );
    }

    /* Not found in quark-to-create map.
    */

    if (NULLQUARK != rp->create.quark)
	stringInResDb = rp->create.string;
    else if (NULLQUARK != rp->constructor.quark)
	stringInResDb = rp->constructor.string;
    else if (NULLQUARK != rp->class.quark)
	stringInResDb = rp->class.string;
    else 
	stringInResDb = rp->className.string;

    WcWARN1( pw, "WcCreate", "notRegistered", 
	"Wcl Warning: Could not convert %s to Class or Constructor.",
	stringInResDb );

    return (Widget)NULL;
}


/*  -- Create Named Popup Children from Xrm Database
*******************************************************************************
    This function creates widget's popup children specified by names list,
    by calling WcCreateDatabasePopup() for each of the names provided.

    Then invoke all the callbacks on the callback list.
*/

void WcxCreateNamedPopups ( pw, names, afterPopups )
    Widget		pw;		/* parent widget		  */
    char*		names;		/* comma or blank separated names */
    XtCallbackRec*	afterPopups;
{
    char        cleanName[MAX_XRMSTRING];
    char*       next;

    if  ( WcNull(names) ) return;

    next = WcCleanName( names, cleanName );

    while ( cleanName[0] )
    {
	ResourceRec res;

        WcxCreate ( &res, pw, cleanName, WcPOPUP );

        next = WcSkipWhitespace_Comma( next );
        next = WcCleanName( next, cleanName );
    }

    if ( afterPopups )
	for ( ; afterPopups->callback ; afterPopups++ )
	    afterPopups->callback( pw, afterPopups->closure, NULL );

    return;
}


/*  -- Create And Manage Named Children from Xrm Database
*******************************************************************************
    This function creates widget children as specified by names list,
    by calling WcxCreate() for each of the names provided.

    The callback list, if non-null, is executed.

    All the children are then managed, unless WcManaged resource is FALSE.

    Note that widgets created by WcxCreate() may or may not be children of 
    'pw' due to the use of constructors.  Only children of 'pw' may be managed 
    via a call to XtManageChildren().  Other widgets must be managed 
    individually.  Usually, these other widgets are created by the 
    XmCreateScrolled*() or XmCreate*Dialog confusion functions.
*/

void WcxCreateNamedChildren ( pw, names, afterChildren, afterManageChildren )
    Widget		pw;
    char*		names;		/* comma or blank separated names */
    XtCallbackRec*	afterChildren;	/* invoked BEFORE managing kids	  */
    XtCallbackRec*	afterManageChildren;
{
    int		children = 0;
    Widget	widget_children[MAX_CHILDREN];

    int		other = 0;
    Widget	widget_other[MAX_CHILDREN];

    char	cleanName[MAX_XRMSTRING];
    char*	next;
    int		i;

    if  ( WcNull(names) ) return;

    if ( !XtIsSubclass( pw, compositeWidgetClass ) )
    {
	char* parent = WcWidgetToFullName( pw );
	WcWARN1( pw, "WcCreate", "cantHaveChildren", 
"Wcl Warning: WcCreate(%s) - Not a Composite, can't have children.", parent);
	XtFree( parent );
	return;
    }

    next = WcCleanName( names, cleanName );

    while ( cleanName[0] )
    {
	ResourceRec res;		/* child's creation resources	*/

	Widget child = WcxCreate( &res, pw, cleanName, WcCREATE);

#if defined(XtSpecificationRelease) 
	if ( child && XtIsRectObj(child) )
#else
	if ( child )
#endif
	{
	    if ( res.managed && (XtParent( child ) == pw ) )
		widget_children[children++] = child;
	    else if ( res.managed )
		widget_other[other++] = child;
	}
	next = WcSkipWhitespace_Comma( next );
	next = WcCleanName( next, cleanName );
    }

    /* Invoke all the callbacks before managing the children
    */
    if ( afterChildren )
	for ( ; afterChildren->callback ; afterChildren++ )
	    afterChildren->callback( pw, afterChildren->closure, NULL );

    if ( children ) 
	XtManageChildren( widget_children, children );

    for ( i = 0 ; i < other ; i++)
	XtManageChild( widget_other[i] );

    /* Invoke all the callbacks after managing the children
    */
    if ( afterManageChildren )
	for ( ; afterManageChildren->callback ; afterManageChildren++ )
	    afterManageChildren->callback( pw, afterManageChildren->closure, 
									NULL );
}

/*  -- Print creation trace message
*******************************************************************************
*/

static void WcxPrintCreationTrace( rp, child, creationType )
    ResourceRec* rp;
    Widget       child;
    int		 creationType;
{
    char*	trace;
    char*	childName  = WcWidgetToFullName( child );	/* MUST FREE */
    char*	childClass = WcWidgetClassName( child );	/* DONT FREE */

    switch ( creationType )
    {
    case WcCREATE: if ( rp->managed )
			trace = "  managed:"  ;
		   else
			trace = "unmanaged:"  ;
		   break;
    case WcPOPUP:	trace = "   pop-up:"  ;   break;
    case WcROOT:	trace = " New Root:"  ;   break;
    }

    WcPrint( "Wcl ", trace, childName, " of class ", childClass, NULL );

    XtFree( childName  );
}

/*  -- After a widget is created
*******************************************************************************
    Returns the number of children sucessfully created.
*/

void WcxPostCreate( rp, newWidget, creationType )
    ResourceRec* rp;
    Widget	 newWidget;
    int		 creationType;
{
    XtCallbackRec*	cb;

    WcxGetPostRes( rp, newWidget );

    if (rp->trace)
	WcxPrintCreationTrace( rp, newWidget, creationType );

    if (rp->postCreateDump)
	WcPostCreateDumpResources( newWidget, stderr );

    /* Invoke all callbacks named in WcCallback resource
    */
    if (rp->callback)
	for ( cb = rp->callback; cb->callback; cb++ )
            cb->callback( newWidget, cb->closure, NULL );

    if ( WcNonNull(rp->popups) )
	WcxCreateNamedPopups  ( newWidget, rp->popups,   rp->afterPopups );

    if ( WcNonNull(rp->children) )
	WcxCreateNamedChildren( newWidget, rp->children,
				rp->afterChildren, rp->afterManageChildren );

    return;
}

/*  -- Create child widget
*******************************************************************************
*/

static Widget WcxCreate( rp, pw, name, creationType )
    ResourceRec* rp;
    Widget       pw;
    char*        name;
    int		 creationType;
{
    Widget child;

    WcxGetPreRes( rp, pw, name );			/* WcxPreCreate */

    child = WcxCreateFromQuark( rp, pw, name, creationType );

    if (child)
	WcxPostCreate( rp, child, creationType );

    return child;
}

/*
*******************************************************************************
* Public Functions
*******************************************************************************
*/

/*  -- Initialize the Widget Creation Library
*******************************************************************************
    This is the only time Wcl library resources are fetched.  They
    are *not* re-fetched when resource files are read, although perhaps
    they will be in the future.
*/

void WcInitialize( root )
    Widget root;
{
    XtAppContext app = XtWidgetToApplicationContext( root );

    ONCE_PER_XtAppContext( app );

    WcRootWidget( root );

    WcAddConverters( app );

    /* Fetch only wclInitResFile from Xrm - set only on command line
    */
    XtGetApplicationResources(	root, (XtPointer)wcl,
				wclInitResFileResources,
				XtNumber(wclInitResFileResources),
				(ArgList)0, 0 );

    if ( WcNonNull( wcl->initResFile ) )
    {
	/* A resource file has been specified on the command line - load it.
	*/
	char cleanName[MAX_XRMSTRING];
	WcCleanName( wcl->initResFile, cleanName );
	WcLoadResourceFile( root, cleanName );
    }

    /* Load all resource files we find specified in wclResFiles resource(s)
    */
    XtGetApplicationResources(	root, (XtPointer)wcl,
				wclResFilesResources,
				XtNumber(wclResFilesResources),
				(ArgList)0, 0 );

    /* Load all resource files we can
    */
    while ( WcMoreResourceFilesToLoad( root, wcl ) )
    {
	XtGetApplicationResources( root, (XtPointer)wcl,
				   wclResFilesResources,
				   XtNumber(wclResFilesResources),
				   (ArgList)0, 0 );
    }

    /* All files are loaded, now fetch the other wcl library-wide resources.
    */
    XtGetApplicationResources(  root, (XtPointer)wcl,
				wclResources, XtNumber(wclResources),
				(ArgList)0, 0 );

    WcWarningInitialize( app, wcl );

    WcWidgetResourcesInitialize( app, wcl );

    WcTemplateInitialize( app, wcl );

    WcRegisterIntrinsic ( app );

    WcRegisterWcCallbacks ( app );

    WcRegisterWcActions ( app );
}

/*  -- Create Widget Tree from Xrm Database
*******************************************************************************
    This routine creates widget children as defined in Xrm database.  It checks
    the widget resource WcChildren and WcPopups which are lists of names of
    children to create. Each child must then be further defined in the databse.

    This function is frequently called from main() after the application shell 
    is created via XtInitialize().
*/

int WcWidgetCreation ( root )
    Widget       root;
{
    int		children;
    ResourceRec	res;

    WcInitialize( root );

    WcxPostCreate( &res, root,  WcROOT );

    if (XtIsComposite(root))
	children = ((CompositeWidget)root)->composite.num_children;
    else
	children = 0;

    if ( 0 == (root->core.num_popups + children) )
    {
	char* fullName = WcWidgetToFullName( root );
        WcWARN2( root, "WcWidgetCreation", "noChildren", 
"Wcl Error: WcWidgetCreation(%s) - Failed \n\
    Problem: No children could be created from Xrm database.\n\
    Possible: Resource file not found \n\
		perhaps XENVIRONMENT is not set, \n\
		or XAPPLRESDIR not terminated with '/' \n\
    Possible: top level widget in resource file not named '%s'",
       		fullName, XtName(root) );
	XtFree( fullName );
	return 1;
    }
    return 0;
}

/*  -- Create And Manage Named Children from Xrm Database
*******************************************************************************
*/

void WcCreateNamedChildren ( pw, names )
    Widget	pw;		/* parent widget			*/
    char*	names;		/* (list of) widget names to create	*/
{
    if ( NULL != pw && WcNonNull(names) )
	WcxCreateNamedChildren ( pw, names, 
				(XtCallbackRec*)NULL, (XtCallbackRec*)NULL );
}

/*  -- Create Named Child from Xrm Database
*******************************************************************************
    Note that the child is NOT managed, and that none of the parents
    creation time callbacks are invoked.  This function is invoked via
    the application, so the application can do whatever it wants...
*/
Widget WcCreateChild( pw, name )
    Widget	pw;
    char*	name;
{
    ResourceRec res;		/* child's creation resources	*/

    if ( !XtIsSubclass( pw, compositeWidgetClass ) )
    {
	char* parent = WcWidgetToFullName( pw );
	WcWARN1( pw, "WcCreate", "cantHaveChildren", 
"Wcl Warning: WcCreate(%s) - Not a Composite, can't have children.", parent);
	XtFree( parent );
	return (Widget)0;
    }

    if ( NULL != pw && WcNonNull(name) )
	return WcxCreate( &res, pw, name, WcCREATE);

    return (Widget)0;
}

/*  -- Create Named Child using template from Xrm Database
*******************************************************************************
    Note that the child is NOT managed, and that none of the parents
    creation time callbacks are invoked.  This function is invoked via
    the application, so the application can do whatever it wants...
*/
Widget WcCreateChildFromTemplate( pw, name, template )
    Widget	pw;
    char*	name;
    char*	template;
{
    ResourceRec res;		/* child's creation resources	*/

    if ( !XtIsSubclass( pw, compositeWidgetClass ) )
    {
	char* parent = WcWidgetToFullName( pw );
	WcWARN1( pw, "WcCreate", "cantHaveChildren", 
"Wcl Warning: WcCreate(%s) - Not a Composite, can't have children.", parent);
	XtFree( parent );
	return (Widget)0;
    }

    if ( NULL != pw && WcNonNull(name) )
    {
	/* See if template tracing is enabled for this child
	*/
	XtGetSubresources( pw, (XtPointer)(&res), name, name,
			   wcPreRes, XtNumber(wcPreRes), NULL, 0 );

	if ( WcApplyTemplate( XrmStringToQuark( template ), pw, name,
			      res.traceTemplate ) )
	{
	    return WcxCreate( &res, pw, name, WcCREATE);
	}
	else
	{
	    char* parentName = WcWidgetToFullName( pw );

	    WcWARN3( pw, "WcCreate", "templateNotFound",
		"Wcl Warning: %s.%s cannot find template %s",
		parentName, name, template );
	    XtFree( parentName );
	}
    }
    return (Widget)0;
}


/*  -- Create Named Popup Children from Xrm Database
*******************************************************************************
*/

void WcCreateNamedPopups ( pw, names )
    Widget      pw;         /* children's parent                            */
    char*       names;      /* (list of) names of popup widgets to create   */
{
    if ( NULL != pw && WcNonNull(names) )
	WcxCreateNamedPopups ( pw, names, (XtCallbackRec*)NULL );
}

Widget WcCreatePopup( pw, name )
    Widget	pw;
    char*	name;
{
    ResourceRec res;		/* child's creation resources   */

    if ( NULL != pw && WcNonNull(name) )
	return WcxCreate( &res, pw, name, WcPOPUP);
    else
	return (Widget)0;
}

Widget WcCreatePopupFromTemplate( pw, name, template )
    Widget	pw;
    char*	name;
    char*	template;
{
    ResourceRec res;		/* child's creation resources   */

    if ( NULL != pw && WcNonNull(name) )
    {
	/* See if template tracing is enabled for this child
	*/
	XtGetSubresources( pw, (XtPointer)(&res), name, name,
			   wcPreRes, XtNumber(wcPreRes), NULL, 0 );

	if ( WcApplyTemplate( XrmStringToQuark( template ), pw, name,
			      res.traceTemplate ) )
	{
	    return WcxCreate( &res, pw, name, WcPOPUP);
	}
	else
	{
	    char* parentName = WcWidgetToFullName( pw );

	    WcWARN3( pw, "WcCreate", "templateNotFound",
		"Wcl Warning: %s.%s cannot find template %s",
		parentName, name, template );
	    XtFree( parentName );
	}
    }
    return (Widget)0;
}

/*  -- Create New Widget Tree from the Resource Database
*******************************************************************************
    WcCreateRoot() can be invoked from a callback and action too.
*/

Widget WcCreateRoot( dpy, shellName )
    Display* dpy;
    char*    shellName;
{
    ResourceRec	res;
    Widget	newShell;
    char*	appClass;

    if ( WcNull(shellName) )
	return (Widget)0;

    if ( WcNonNull( wcl->appClassName ) )
	appClass = wcl->appClassName;
    else
	appClass = WcAppNameToAppClass( shellName );	/* must be XtFree'd */

    newShell = XtAppCreateShell(
	shellName, appClass,
	topLevelShellWidgetClass, 
	dpy, 
	NULL, 0 );

    if ( appClass != wcl->appClassName )
	XtFree( appClass );

    /* -- Remember this shell as a root widget for WcFullNameToWidget
    */
    WcRootWidget(newShell);

    /* -- create widget tree under this new application shell 
    */
    WcxPostCreate( &res, newShell,  WcROOT );

    /*  -- Realize the widget tree
    */
    XtRealizeWidget ( newShell );

    return newShell;
}

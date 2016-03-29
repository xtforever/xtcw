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


#include "socket_log.h"

#define MAX_MSG_SIZE 10000
#define APP_NAME "exbasic2"
#define APP_INI_FILE APP_NAME ".ini"
Widget TopLevel, TopMgr, widget_data;
int AutoCreateChildren = 1;

int com_data = 0, wlist = 0;


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

/** dieses CB wird von Wlist5 aufgerufen um
    einzelne zeilen zu lesen.
*/ 
void wlist_get_data(Widget w, void *u, void *c)
{
}


void LoadDataCB( Widget w, void *u, void *c )
{
  int i;
  char *s;

  if( com_data == 0 ) {
    com_data = m_create( 100, sizeof(char*));
    XtVaSetValues( widget_data, "tableStrs", com_data, NULL );
  }
  m_free_strings(com_data,1);
  asprintf( &s, "12/24.11:59\tsam\terror while loading shared libraries: libvexmo.so.1.0.0: cannot open shared object file: No such file or directory" );
  m_put( com_data, &s );
  for(i=0;i<10000;i++)
    {
      asprintf( &s, "%d\t%d\t%d\t%d", i, (i+1)*3, (i+1)*7, ((i+1)*127) );
      m_put(com_data, &s );
    }

  wlist4_update_all(widget_data);
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
  RCB( top, LoadDataCB ); 
  RCB( top, test_cb ); 

}


void data_received( int sfd, char *buf, int n )
{
  if( n > MAX_MSG_SIZE ) n=MAX_MSG_SIZE;
  slog_sendto_all( buf, n );
}


static void syntax(void)
{
  printf(
	 "Most X programs attempt to use the same names for command line options and arguments. All applications written with the X Toolkit Intrinsics automatically accept the following options:\n"

"-display display\n"
"\tThis option specifies the name of the X server to use.\n" 
"-geometry geometry\n"
"\tThis option specifies the initial size and location of the window.\n"
"-bg color, -background color\n"
"\tEither option specifies the color to use for the window background.\n"
"-bd color, -bordercolor color\n"
"\tEither option specifies the color to use for the window border.\n" 
"-bw number, -borderwidth number\n"
"\tEither option specifies the width in pixels of the window border. \n"
"-fg color, -foreground color\n"
"\tEither option specifies the color to use for text or graphics. \n"
"-fn font, -font font\n"
"\tEither option specifies the font to use for displaying text. \n"
"-iconic\n"

"\tThis option indicates that the user would prefer that the application's windows initially not be visible as if the windows had be immediately iconified by the user. Window managers may choose not to honor the application's request. \n"
"-name\n"

"\tThis option specifies the name under which resources for the application should be found. This option is useful in shell aliases to distinguish between invocations of an application, without resorting to creating links to alter the executable file name.\n" 
"-rv, -reverse\n"
"\tEither option indicates that the program should simulate reverse video if possible, often by swapping the foreground and background colors. Not all programs honor this or implement it correctly. It is usually only used on monochrome displays. \n"
"+rv\n"
"\tThis option indicates that the program should not simulate reverse video. This is used to override any defaults since reverse video doesn't always work properly. \n"
"-selectionTimeout\n"
"\tThis option specifies the timeout in milliseconds within which two communicating applications must respond to one another for a selection request. \n"
"-synchronous\n"
"\tThis option indicates that requests to the X server should be sent synchronously, instead of asynchronously. Since Xlib normally buffers requests to the server, errors do not necessarily get reported immediately after they occur. This option turns off the buffering so that the application can be debugged. It should never be used with a working program. \n"
"-title string\n"
"\tThis option specifies the title to be used for this window. This information is sometimes used by a window manager to provide some sort of header identifying the window. \n"
"-xnllanguage language[_territory][.codeset]\n"
"\tThis option specifies the language, territory, and codeset for use in resolving resource and other filenames. \n"
"-xrm resourcestring\n"
"\tThis option specifies a resource name and value to override any defaults. It is also very useful for setting resources that don't have explicit command line arguments. \n"
	 "-TraceLevel <num>\t\n"
	 "-ListenPort <num>\t\n"
	 );
}


/******************************************************************************
*   MAIN function
******************************************************************************/
int main ( int argc, char **argv )
{
    XtAppContext app;
    signal(SIGPIPE, SIG_IGN); /* broken pipe on write */

    m_init();

    /*  -- Intialize Toolkit creating the application shell
     */
    Widget appShell = XtOpenApplication (&app, APP_NAME,
	     options, XtNumber(options),  /* resources: can be set from argv */
	     &argc, argv,
	     NULL,
	     applicationShellWidgetClass,
	     NULL, 0
	   );
    XtAddEventHandler(appShell, (EventMask) 0, True, _XEditResCheckMessages, NULL);

    /* not parsed options are removed by XtOpenApplication */
    if (argc != 1) { m_destruct(); syntax(); exit(1); }
    TopLevel = appShell;

    XtGetApplicationResources(	appShell, (XtPointer)&SETTINGS,
				basicSettingRes,
				XtNumber(basicSettingRes),
				(ArgList)0, 0 );
    printf( "%d %s\n", SETTINGS.traceLevel, SETTINGS.listenPort );
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
    widget_data = WcFullNameToWidget( appShell, "*ls" );
    assert( widget_data );

    /* init application functions */
    slog_init( appShell, SETTINGS.listenPort, data_received );


    /*  -- Realize the widget tree and enter the main application loop
     */
    XtRealizeWidget ( appShell );




    XtAppMainLoop ( app );
    m_destruct();
    return EXIT_SUCCESS;
}

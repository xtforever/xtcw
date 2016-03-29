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

/* test the new widgets */
#include "Woptc.h"
#include "Wcmd2.h"


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
  { "-list",            "*listMax",   XrmoptionSepArg, NULL },
  { "-trace",           "*trace",     XrmoptionSepArg, NULL },
  WCL_XRM_OPTIONS
};

typedef struct BasicSetting {
    int trace;
    int list_max;
    int trace_level;
    char *listenPort;
    Widget widget_ls; /* listBox widget */
    Widget widget_t1; /* Scrollbar widget */
} BasicSetting;

#define FLD(n)  XtOffsetOf(BasicSetting,n)
static XtResource basicSettingRes[] = {
  { "trace", "Trace", XtRInt, sizeof(int),
    FLD(trace), XtRImmediate, (XtPointer)100,
  },
  { "listMax", "ListMax", XtRInt, sizeof(int),
    FLD(list_max), XtRImmediate, (XtPointer)100,
  },
  { "traceLevel", "TraceLevel", XtRInt, sizeof(int),
    FLD(trace_level), XtRImmediate, (XtPointer)0,
  },
  { "listenPort", "ListenPort", XtRString, sizeof(String),
   FLD(listenPort), XtRString, "10000"
  },
  { NULL, NULL, XtRWidget, sizeof(Widget),
   FLD(widget_ls), XtRString, "*ls"
  },
  { NULL, NULL, XtRWidget, sizeof(Widget),
   FLD(widget_t1), XtRString, "*t1"
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

void h1_toggle( Widget w, void *u, void *c )
{
    int toggle = (int) c;
    printf( "toggle %s\n", toggle ? "ON" : "OFF" );
    if( toggle )
        XtUnmanageChild( SETTINGS.widget_ls );
    else
        XtManageChild( SETTINGS.widget_ls );
}

void scroll_cb( Widget w, void *u, void *c )
{
    float pos = *(float*)c;
    printf("scroll pos: %f\n", pos );

    int h,lmax, lh;
    XtVaGetValues( SETTINGS.widget_ls,
                   "list_height", &h,
                   "line_max", &lmax,
                   "line_height", &lh,
                   NULL );

    printf("H %d, LM %d, LH %d\n", h, lmax, lh );
    float frac = h * 1.0 / (lmax*lh);
    int s = s_printf(0,0, "%f", frac );

    XtVaSetValues( SETTINGS.widget_t1, XtVaTypedArg, "frac", XtRString,
                   m_buf(s), m_len(s), NULL );

    int line_zero = lmax * 1.0 * pos + 0.5;
    wlist4_gotoxy( SETTINGS.widget_ls, 0, line_zero * lh );
    m_free(s);
}


/******************************************************************************
**  Private Functions
******************************************************************************/
void fill_widget_with_data()
{
    int i;
    char *b;
    int s = m_create(SETTINGS.list_max,sizeof(char*));
    for(i=0;i<SETTINGS.list_max;i++) {
        asprintf(&b,"%d\t%d\tHello %d", i+1,i+1,i+1 );
        m_put(s, &b );
    }
    XtVaSetValues( SETTINGS.widget_ls, "tableStrs", s, NULL );

}

/*ARGSUSED*/
static void RegisterApplication ( Widget top )
{
    /* -- Register widget classes and constructors */
  RCP( top, woptc );
  RCP( top, wcmd2 );

    /* -- Register application specific actions */

    /* -- Register application specific callbacks */
  RCB( top, quit_gui );
  RCB( top, test_cb );
  RCB( top, scroll_cb );
  RCB( top, h1_toggle );
}


static void syntax(void)
{
  puts( syntax_wcl );
  puts( "-TraceLevel <num>\n"
	 "-ListenPort <num>\n"
        "-list <num> Number of Lines inside List\n"
          "-trace <num> Trace Level\n");
}


/******************************************************************************
*   MAIN function
******************************************************************************/
int main ( int argc, char **argv )
{
    XtAppContext app;
    signal(SIGPIPE, SIG_IGN); /* ignore broken pipe on write */
    m_init(); /* memory management initalisation */

    /*  -- Intialize Toolkit creating the application shell
     */
    Widget appShell = XtOpenApplication (&app, APP_NAME,
	     options, XtNumber(options),  /* resources: can be set from argv */
	     &argc, argv,
	     fallback_resources,
	     applicationShellWidgetClass,
	     NULL, 0
	   );

    /* SETTINGS.xxx wird erst nach
       getapplicationresources
       mit daten gefüllt, obwohl die cmd line options hier
       übergeben werden: Überraschung!
    */

    /* make TopLevel Window global accessible */
    TopLevel = appShell;

    /* enable Editres support */
    XtAddEventHandler(appShell, (EventMask) 0, True, _XEditResCheckMessages, NULL);

    /* not parsed options are removed by XtOpenApplication
       the only entry left should be the program name
    */
    if (argc != 1) { m_destruct(); syntax(); exit(1); }

    /* load additional resource file */
    rc_init( APP_INI_FILE );

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


    /* enable debugging output */
    trace_level = SETTINGS.trace_level;


   /* init widgets */
   fill_widget_with_data();

   /*  -- Realize the widget tree and enter the main application loop
    */
   XtRealizeWidget ( appShell );


   XtAppMainLoop ( app ); /* use XtAppSetExitFlag */

   XtDestroyWidget(appShell);
   rc_free();
   m_destruct();
   return EXIT_SUCCESS;
}

/* most important define */
#define APP_NAME "ex15"

/*
 */


#include "mls.h"
#include "ini_read2.h"
#include "pipeshell.h"
#include "mrb.h"

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

  -------------------------------------------------------------------------------------------------------------------- */

int my_pin;
int filter_str;


void update_filelist(void) {
    int s = SETTINGS.filelist;
    XtVaSetValues( SETTINGS.widget_filelist, "tableStrs", s, NULL );
};

void append_filelist(char *s)
{
    s=strdup(s);
    m_put(SETTINGS.filelist, &s );
}

void reset_filelist()
{
    m_free_strings(SETTINGS.filelist, 1 );
}
void childcb(int pin)
{
    enum child_proc_state stat = child_status(pin);
    if(  stat == CHILD_RUNNING )
        {
            char *s = child_data(pin);
            append_filelist(s);
            TRACE(1,"data: %s", s );
            return;
        }

    if( stat == CHILD_EXIT_SUCCESS ) {
        TRACE(1,"EXIT SUCCESS");
        update_filelist();
    }
    else
        TRACE(1,"EXIT FAILURE %d", child_exit_status(pin) );
}

#define S_APPEND (-1)

void start_search(Widget w, void *a, void *b)
{
    reset_filelist();
    int cmd_m = v_lookup(SETTINGS.vset, "filter" );
    char *old = STR(cmd_m, 0);
    *(char**)mls(cmd_m, 0) = "./datamaker";
    my_pin = child_startv(TopLevel, cmd_m, childcb );
    *(char**)mls(cmd_m, 0) = old;
}

void update_filter(void)
{
    int str = 0;
    int p = 1;
    char *s;

    s=v_get(SETTINGS.vset, "filter", 1);
    str = s_app(0, s , NULL );

    while(1) {
        s=v_get(SETTINGS.vset, "filter", ++p);
        if( is_empty(s) ) break;
        s_app(str, ",", s, NULL );
    }

    XtVaSetValues(SETTINGS.widget_filter, "label", m_buf(str), NULL );
    m_free(str);
}

void reset_filter( Widget w, void *u, void *c )
{
    v_clr(SETTINGS.vset, "filter");
    update_filter();
    printf("cb\n");
}

void append_filter(char *s)
{
    if( is_empty(s) ) return;
    v_clr( SETTINGS.vset, "filter" );
    v_vaset(SETTINGS.vset, "filter", s, NULL );
}

void remove_last_filter( Widget w, void *u, void *c )
{
    int q = v_lookup(SETTINGS.vset, "filter" );
    int l = m_len(q);
    if( l > 1 ) {
        l--;
        free( STR(q, l) );
        m_setlen(q, l);
    }
    update_filter();
}



void user_entry( Widget w, void *u, void *c )
{
    char *s;
    XtVaGetValues(SETTINGS.widget_entry, "label", &s, NULL );
    printf("cb %s\n", s);
    append_filter(s);
    start_search(w,u,c);
    update_filter();
}




/******************************************************************************
**  PIN Test Functions
******************************************************************************/



void pin_stop_test(Widget w, void *a, void *b)
{
    child_close(my_pin); my_pin=-1;
}



/******************************************************************************
**  Private Functions
******************************************************************************/


/*ARGSUSED*/
static void RegisterApplication ( Widget top )
{
    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
  RCB( top, quit_gui );
  RCB( top, user_entry );
  RCB( top, reset_filter );
  RCB( top, remove_last_filter );
}


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

    SETTINGS.filelist = m_create(100, sizeof(char*));
    SETTINGS.vset = v_init();

    /* init widgets */
    trace_level = SETTINGS.traceLevel;
    trace_child = trace_level;
    /*  init application functions
        All widgets are created, but not visible.
        functions can now communicate with widgets
    */

    /*  -- Realize the widget tree and enter the main application loop
     */
    XtRealizeWidget ( appShell );
    grab_window_quit( appShell );
    //    pin_start_test();

    XtAppMainLoop ( app ); /* use XtAppSetExitFlag */
    XtDestroyWidget(appShell);
    child_destruct();
    m_destruct();

    return EXIT_SUCCESS;
}

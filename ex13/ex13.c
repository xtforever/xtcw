/* most important define */
#define APP_NAME "ex13"

/*
 */


#include "mls.h"
#include "ini_read2.h"
#include "mrb.h"
#include "pipeshell.h"

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

#define COLOR_NORMAL 0
#define COLOR_PLAY 1
#define COLOR_PAUSE 2

typedef struct BasicSetting {
    int traceLevel;
    char *listenPort;
    Widget widget_label;
} BasicSetting;



#define FLD(n)  XtOffsetOf(BasicSetting,n)
static XtResource basicSettingRes[] = {


  { NULL, NULL, XtRWidget, sizeof(Widget),
    FLD(widget_label), XtRString, "*p1top"
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


void show_title(Widget w)
{
    char *name;
    XtVaGetValues(w, "fileName", &name, NULL );
    if( name )
        XtVaSetValues(SETTINGS.widget_label, "label", name, NULL );
}

void click_left(Widget w)
{
    int num;
    XtVaGetValues(w, "value", &num, NULL );
    if( num > 0 ) {
        num--;
        XtVaSetValues(w, "value", num, NULL );
    }
}

void click_right(Widget w)
{
    int num;
    XtVaGetValues(w, "value", &num, NULL );
    num++;
    XtVaSetValues(w, "value", num, NULL );
}


void pic_cb( Widget self, void *u, void *c )
{
    XEvent *event = c;
    int x,y;
    x= event->xbutton.x;
    y= event->xbutton.y;
    printf( "%d,%d\n", event->xbutton.x,event->xbutton.y );
    Dimension w,h;
    XtVaGetValues( self, "width", &w, "height", &h, NULL );
    printf( "%d,%d\n", w,h );

    /* check left/right */
    /* 40% 20% 40% */
    int left = (w*40)/100;
    int right = w - left;
    if( x < left ) click_left(self);
    else if( x > right ) click_right(self);
    else return;
    show_title(self);

}




/******************************************************************************
**  PIN Test Functions
******************************************************************************/

int my_pin;

void childcb(int pin)
{
    enum child_proc_state stat = child_status(pin);
    if(  stat == CHILD_RUNNING )
        {
            TRACE(1,"data: %s", child_data(pin) );
            return;
        }

    if( stat == CHILD_EXIT_SUCCESS )
        TRACE(1,"EXIT SUCCESS");
    else
        TRACE(1,"EXIT FAILURE %d", child_exit_status(pin) );
}

void pin_start_test(Widget w, void *a, void *b)
{
    // my_pin = pin_start(TopLevel, "./datamaker", mycb );

    my_pin = child_start(TopLevel, "./datamaker", childcb );
}

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
  RCB( top, pic_cb );
  RCB( top, pin_stop_test );
  RCB( top, pin_start_test );
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

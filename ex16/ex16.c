/* most important define */
#define APP_NAME "ex16"

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


typedef struct BasicSetting {
    XtAppContext app;
    int vset;

    int traceLevel;
    char *listenPort;

    Widget widget_ent[2];
    char *p[2];
    int update_ms;
    int thermaldiv[2];
    int temp[2];



} BasicSetting;



#define FLD(n)  XtOffsetOf(BasicSetting,n)
static XtResource basicSettingRes[] = {

    { NULL, NULL, XtRWidget, sizeof(Widget),
      FLD(widget_ent[0]), XtRString, "*ent1"
    },
    { NULL, NULL, XtRWidget, sizeof(Widget),
      FLD(widget_ent[1]), XtRString, "*ent2"
    },

    { "thermal1", "Thermal1", XtRString, sizeof(String),
      FLD(p[0]), XtRString, ""
    },
    { "thermal2", "Thermal2", XtRString, sizeof(String),
      FLD(p[1]), XtRString, ""
    },
    { "thermal1div", "Thermal1div", XtRInt, sizeof(int),
      FLD(thermaldiv[0]), XtRImmediate, 0
    },
    { "thermal2div", "Thermal2div", XtRInt, sizeof(int),
      FLD(thermaldiv[1]), XtRImmediate, 0
    },

    { "update_ms", "Update_ms", XtRInt, sizeof(int),
      FLD(update_ms), XtRImmediate, 0
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
    TRACE(1,"WM_QUIT"); quit_gui(w,NULL,NULL);
}


/* --------------------------------------------------------------------------------------------------------------------

                        IMPLEMENTATION

  -------------------------------------------------------------------------------------------------------------------- */



/******************************************************************************
**  Application Actions
**  Actions will be called by Translations
**  Translations bind events to actions
******************************************************************************/

static int down_x, down_y;
static void btndown(Widget w, XEvent* e, String* s, Cardinal* n)
{
    int x,y,x0=e->xbutton.x, y0= e->xbutton.y;
    Display *disp = XtDisplay(TopLevel);
    Window child_return;
    XTranslateCoordinates (disp, XtWindow(w), XtWindow(TopLevel),
                           x0, y0, & x, & y, & child_return);

    TRACE(1,"DOWN: %dx%d", x,y );
    down_x = x;
    down_y = y;

}

static void btnmove(Widget w, XEvent* e, String* s, Cardinal* n)
{
    int x0=e->xbutton.x, y0= e->xbutton.y;
    TRACE(1,"%dx%d",x0,y0  );
    Display *disp = XtDisplay(TopLevel);
    Window  win   = XtWindow(TopLevel);
    int screen    = DefaultScreen(disp);
    Window root   = RootWindow(disp,screen);

    int x,y;
    Window child_return;
    XTranslateCoordinates (disp, XtWindow(w), root,
                           x0, y0, & x, & y, & child_return);

    TRACE(1, "%dx%d", x,y );
    XMoveWindow(disp,win,x - down_x,y-down_y );
}

static void load_temp_single(int i)
{
    char str[100];
    SETTINGS.temp[i] = 0;
    char *path = SETTINGS.p[i];
    FILE *fp = fopen(path,"r");
    if(!fp) return;
    fread( str, sizeof str, 1, fp );
    SETTINGS.temp[i] = atoi(str);
    fclose(fp);
}static int down_x, down_y;
static void btndown(Widget w, XEvent* e, String* s, Cardinal* n)
{
    int x,y,x0=e->xbutton.x, y0= e->xbutton.y;
    Display *disp = XtDisplay(TopLevel);
    Window child_return;
    XTranslateCoordinates (disp, XtWindow(w), XtWindow(TopLevel),
                           x0, y0, & x, & y, & child_return);

    TRACE(1,"DOWN: %dx%d", x,y );
    down_x = x;
    down_y = y;

}

static void btnmove(Widget w, XEvent* e, String* s, Cardinal* n)
{
    int x0=e->xbutton.x, y0= e->xbutton.y;
    TRACE(1,"%dx%d",x0,y0  );
    Display *disp = XtDisplay(TopLevel);
    Window  win   = XtWindow(TopLevel);
    int screen    = DefaultScreen(disp);
    Window root   = RootWindow(disp,screen);

    int x,y;
    Window child_return;
    XTranslateCoordinates (disp, XtWindow(w), root,
                           x0, y0, & x, & y, & child_return);

    TRACE(1, "%dx%d", x,y );
    XMoveWindow(disp,win,x - down_x,y-down_y );
}


static void disp_temp_single(int i)
{
    char str[100];
    float f = SETTINGS.temp[i] * 1.0 / SETTINGS.thermaldiv[i];
    sprintf(str,"%4.2f", f );
    XtVaSetValues( SETTINGS.widget_ent[i], "label", str, NULL );
}

static void load_temp()
{
    int i;
    for(i=0;i<2;i++) {
        load_temp_single(i);
        disp_temp_single(i);
    }
}

static void update_cb(void *user_data, XtIntervalId *id)
{
    TRACE(1,"");
    XtAppAddTimeOut(SETTINGS.app, SETTINGS.update_ms,
                    update_cb, 0 );
    load_temp();

}

/******************************************************************************
**  Private Functions
******************************************************************************/


/*ARGSUSED*/
static void RegisterApplication ( Widget top )
{

    /* -- Register application specific callbacks */


    /* -- Register application specific actions */
  RAC( top, btnmove );
  RAC( top, btndown );
}


static void syntax(void)
{
  puts( "-TraceLevel <num>\n"
	"-ListenPort <num>\n" );
  puts( syntax_wcl );
}

/** append to the list of client properties a property that
    tells the window manager to keep this window above others
*/
void make_stay_above(Widget top)
{
#define _NET_WM_STATE_REMOVE        0    // remove/unset property
#define _NET_WM_STATE_ADD           1    // add/set property
#define _NET_WM_STATE_TOGGLE        2    // toggle property

    Display *display = XtDisplay(top);
    int screen = DefaultScreen(display);
    Window root = RootWindow(display,screen);

    Atom wmStateAbove = XInternAtom( display, "_NET_WM_STATE_ABOVE", 1 );
    if( wmStateAbove != None ) {
        printf( "_NET_WM_STATE_ABOVE has atom of %ld\n", (long)wmStateAbove );
    } else {
        printf( "ERROR: cannot find atom for _NET_WM_STATE_ABOVE !\n" );
    }

    Atom wmNetWmState = XInternAtom( display, "_NET_WM_STATE", 1 );
    if( wmNetWmState != None ) {
        printf( "_NET_WM_STATE has atom of %ld\n", (long)wmNetWmState );
    } else {
        printf( "ERROR: cannot find atom for _NET_WM_STATE !\n" );
    }
    // set window always on top hint
    if( wmStateAbove == None ) return;

    XClientMessageEvent xclient;
    memset( &xclient, 0, sizeof (xclient) );
    //
    //window  = the respective client window
    //message_type = _NET_WM_STATE
    //format = 32
    //data.l[0] = the action, as listed below
    //data.l[1] = first property to alter
    //data.l[2] = second property to alter
    //data.l[3] = source indication (0-unk,1-normal app,2-pager)
    //other data.l[] elements = 0
    //
    xclient.type = ClientMessage;
    xclient.window = XtWindow(top); // GDK_WINDOW_XID(window);
    xclient.message_type = wmNetWmState; //gdk_x11_get_xatom_by_name_for_display( display, "_NET_WM_STATE" );
    xclient.format = 32;
    xclient.data.l[0] = _NET_WM_STATE_ADD; // add ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
    xclient.data.l[1] = wmStateAbove; //gdk_x11_atom_to_xatom_for_display (display, state1);
    xclient.data.l[2] = 0; //gdk_x11_atom_to_xatom_for_display (display, state2);
    xclient.data.l[3] = 0;
    xclient.data.l[4] = 0;
    //gdk_wmspec_change_state( FALSE, window,
    //  gdk_atom_intern_static_string ("_NET_WM_STATE_BELOW"),
    //  GDK_NONE );
    XSendEvent( display,
                //mywin - wrong, not app window, send to root window!
                root, // !! DefaultRootWindow( display ) !!!
                False,
                SubstructureRedirectMask | SubstructureNotifyMask,
                (XEvent *)&xclient );
}

/** tell window manager to not decorated this shell window
 */
void make_borderless_window(Widget top )
{
    Window window = XtWindow(top);
    Display *display = XtDisplay(top);

    struct MwmHints {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
        long input_mode;
        unsigned long status;
    };

    enum {
        MWM_HINTS_FUNCTIONS = (1L << 0),
        MWM_HINTS_DECORATIONS =  (1L << 1),

        MWM_FUNC_ALL = (1L << 0),
        MWM_FUNC_RESIZE = (1L << 1),
        MWM_FUNC_MOVE = (1L << 2),
        MWM_FUNC_MINIMIZE = (1L << 3),
        MWM_FUNC_MAXIMIZE = (1L << 4),
        MWM_FUNC_CLOSE = (1L << 5)
    };
    struct MwmHints hints;
    Atom mwmHintsProperty = XInternAtom(display, "_MOTIF_WM_HINTS", 0);

    hints.flags = MWM_HINTS_DECORATIONS;
    hints.decorations = 0;
    XChangeProperty(display, window, mwmHintsProperty, mwmHintsProperty, 32,
                    PropModeReplace, (unsigned char *)&hints, 5);
}

/* if the window manager closes the window, tell the
   window manager to send a message to xt. xt will
   call the wm_quit callback.
*/
void grab_window_quit(Widget top)
{
    XtOverrideTranslations
	(TopLevel, XtParseTranslationTable ("<Message>WM_PROTOCOLS: wm_quit()"));

    /* http://www.lemoda.net/c/xlib-wmclose/ */
    static Atom wm_delete_window;
    wm_delete_window = XInternAtom(XtDisplay(top), "WM_DELETE_WINDOW",
				   False);
    (void) XSetWMProtocols (XtDisplay(top), XtWindow(top),
                            &wm_delete_window, 1);
}

Widget init_application(int *argc, char **argv )
{
    m_init();
    XtAppContext app;
    XtSetLanguageProc (NULL, NULL, NULL);
    XawInitializeWidgetSet();
    /*  -- Intialize Toolkit creating the application shell
     */
    Widget appShell = XtOpenApplication (&app, APP_NAME,
	     options, XtNumber(options),  /* resources: can be set from argv */
	     argc, argv,
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
    if (*argc != 1) { m_destruct(); syntax(); exit(1); }
    TopLevel = appShell;
    RAC( appShell, wm_quit );
    RCB( appShell, quit_gui );

    /*  -- Register all Athena and Public widget classes, CBs, ACTs
     */
    XpRegisterAll ( app );
    return appShell;

}



/******************************************************************************
*   MAIN function
******************************************************************************/
int main ( int argc, char **argv )
{
    trace_main = TRACE_MAIN;
    signal(SIGPIPE, SIG_IGN); /* ignore broken pipe on write */
    Widget appShell = init_application(&argc, argv );

    /*  -- Register all application specific callbacks and widget classes
     */
    RegisterApplication ( appShell );

    /*  -- Create widget tree below toplevel shell using Xrm database
           register callbacks,actions, widget classe before
     */
    WcWidgetCreation ( appShell );

    /* get application resources and widget ptr */
    XtGetApplicationResources(	appShell, (XtPointer)&SETTINGS,
				basicSettingRes,
				XtNumber(basicSettingRes),
				(ArgList)0, 0 );
    SETTINGS.app = XtWidgetToApplicationContext(appShell);
    SETTINGS.vset = v_init();

    /* init widgets */
    trace_level = SETTINGS.traceLevel;
    TRACE(1,"Thermal1 %s\nThermal2 %s", SETTINGS.p[0], SETTINGS.p[1] );

    /*  init application functions
        All widgets are created, but not visible.
        functions can now communicate with widgets
    */
    update_cb(0,0); /* start timer */

    /*  -- Realize the widget tree and enter the main application loop
     */
    XtRealizeWidget ( appShell );
    grab_window_quit( appShell );
    make_borderless_window(appShell);
    make_stay_above(appShell);
    XtAppMainLoop(XtWidgetToApplicationContext(appShell)); /* use XtAppSetExitFlag */
    XtDestroyWidget(appShell);
    v_free( SETTINGS.vset );
    m_destruct();
    return EXIT_SUCCESS;
}

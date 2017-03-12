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
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>


#include <WcCreate.h>
#include <Xp.h>
#include "wcreg2.h"
#include <xtcw/register_wb.h>
#include <xborderless.h>


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
    char *cache_dir;

} BasicSetting;

/* application resource struct syntax

   Widget ent = <String> "*ent1"

   int thermal1div = 0;

   Type <ResourceName> VariableNameC = <Type> Value
*/

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

    { "cache_dir", "Cache_dir", XtRString, sizeof(String),
      FLD(cache_dir), XtRString, ""
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

int file_read(int buf, char *fn)
{
    if(buf<=0) buf=m_create(10,1); else m_clear(buf);
    FILE *fp = fopen(fn,"r");
    if(fp) {
        m_fscan(buf,'\n',fp);
        fclose(fp);
    }
    return buf;
}



/******************************************************************************
**  Application Actions
**  Actions will be called by Translations
**  Translations bind events to actions
******************************************************************************/

static void load_temp_single(int i)
{
    long val;
    int tempStr;
    tempStr = file_read(0,SETTINGS.p[i] );
    SETTINGS.temp[i] = mstr_to_long(tempStr,0,&val) ? 0 : val;
    m_free(tempStr);
}

static void disp_temp_single(int i)
{
    char str[100];
    float f = SETTINGS.temp[i] * 1.0 / SETTINGS.thermaldiv[i];
    snprintf(str,sizeof str, "%4.2f", f );
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

FILE *open_cachfile(int write)
{
    const char *s;
    if ((s = getenv("HOME")) == NULL) {
        s = getpwuid(getuid())->pw_dir;
    }
    if( s==NULL ) s="/tmp";

    int fn = s_printf( 0,0, "%s/%s", s, SETTINGS.cache_dir );

    FILE *fp = fopen( m_buf(fn), write ? "w" : "r" );
    m_free(fn);
    return fp;
}

void save_window_position(int x, int y)
{
    FILE *fp = open_cachfile(1);
    if( !fp ) return;
    fprintf( fp, "%d %d\n", x,y );
    fclose(fp);
}

int load_window_position(int *x, int *y)
{
    FILE *fp = open_cachfile(0);
    if( !fp ) return 1;
    fscanf( fp, "%d %d", x,y );
    fclose(fp);
    return 0;
}

/******************************************************************************
**  Private Functions
******************************************************************************/


static void handle_sigterm(int signum)
{
    fprintf(stderr,"terminate\n");
    XtAppSetExitFlag( XtWidgetToApplicationContext(TopLevel) );
}

static void catch_sigterm(void)
{
    //in the source file
    static struct sigaction myaction;
    myaction.sa_handler = handle_sigterm;
    myaction.sa_flags = 0;

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);
    sigaction(SIGTERM, &myaction, NULL);
    sigaction(SIGINT, &myaction, NULL);
    myaction.sa_mask = mask;
}

/*ARGSUSED*/
static void RegisterApplication ( Widget top )
{

    /* -- Register application specific callbacks */


    /* -- Register application specific actions */

}


static void syntax(void)
{
  puts( "-TraceLevel <num>\n"
	"-ListenPort <num>\n" );
  puts( syntax_wcl );
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

    Widget appShell = init_application(&argc, argv );
    catch_sigterm();
    trace_main = TRACE_MAIN;
    signal(SIGPIPE, SIG_IGN); /* ignore broken pipe on write */

    /*  -- Register all application specific callbacks and widget classes
     */
    RegisterApplication ( appShell );
    add_winmove_translations(appShell);

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

    int x0,y0;
    if( load_window_position(&x0,&y0) == 0 )
        XMoveWindow(XtDisplay(appShell),XtWindow(appShell), x0, y0 );

    XtAppMainLoop(XtWidgetToApplicationContext(appShell)); /* use XtAppSetExitFlag */

    Dimension x,y;
    XtVaGetValues( appShell, "x", &x, "y", &y, NULL );
    save_window_position(x,y);

    XtDestroyWidget(appShell);
    v_free( SETTINGS.vset );
    m_destruct();
    return EXIT_SUCCESS;
}

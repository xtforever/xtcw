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

#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

#include "xtcw/Calib.h"
#include "xfullscreen.h"
#include "xborderless.h"
#include "solver.h"
#include "xinput-util.h"

int x_width, x_height;
Widget TopLevel;

char* fallback_resources[] = {
    "*calib.foreground: White",
    "*calib.highlight:  Red",
    "*calib.lineWidth: 10",
    "*calib.background: Lightblue",
    NULL
};



/* --------------------------------------------------------------------------------------------------------------------

                        IMPLEMENTATION

  -------------------------------------------------------------------------------------------------------------------- */

/*************************************/
/* resources and command line parser */
/*************************************/
typedef struct app_conf {
    char *deviceName;
    long deviceId;
    char *matrix;
    char *screenSize;
} app_conf;
static app_conf CONF;

static XrmOptionDescRec options[] = {
    { "-deviceName",	"*deviceName",	XrmoptionSepArg, NULL },
    { "-matrix",	"*CTM",	XrmoptionSepArg, NULL },
    { "-screenSize",    "*screenSize", XrmoptionSepArg, NULL },
};

#define FLD(n)  XtOffsetOf(app_conf,n)
static XtResource app_res[] = {
  { "deviceName", "DeviceName", XtRString, sizeof(String),
    FLD(deviceName), XtRString, ""
  },
  { "matrix", "CTM", XtRString, sizeof(String),
    FLD(matrix), XtRString, ""
  },
  { "screenSize", "ScreenSize", XtRString, sizeof(String),
    FLD(screenSize), XtRString, ""
  },
};
#undef FLD

static void get_config(Widget top)
{
    /* get application resources */
    XtGetApplicationResources(	top, (XtPointer)&CONF,
				app_res,
				XtNumber(app_res),
				(ArgList)0, 0 );
}
/*  -------------------------------------------------------------------------------------------------------------------- */

/* workaround compile bug on RPi float stack alignment not 8-byte */
void pf(double x)
{
    char s[10];
    char *e=gcvt(x,6,s); (void)e; printf("%s ", s);
}

/* compute CTM from sample points and apply it to xinput device id */
void touch_cal(Display *dpy, long id, int width, int height, struct pt *points, int cnt )
{
    int i;
    Matrix ctm;
    compute_ctm(points,cnt,width,height,ctm.m);

    /* output CTM */
    for(i=0;i<9;i++)
        printf("%f ", ctm.m[i] );
    printf("\n");

    /* apply CTM */
    xi_apply_matrix(dpy, id, &ctm);
}


/* Calib widget callback
   use 4 points to calculate CTM
   then apply CTM and exit application
*/
static void process_cal_data( Widget w, void *u, void *p )
{
    #ifdef DEBUG
    puts("cal data");
    int i;
    cal_point *c = p;
    for(i=0;i<4;i++) {
        printf("%d %d %d %d\n", c->x0, c->y0, c->x1, c->y1 );
        c++;
    }
    printf("Width=%d Height=%d\n", x_width, x_height );
    #endif

    touch_cal( XtDisplay(w), CONF.deviceId, x_width, x_height, p, 4 );
    XtAppSetExitFlag( XtWidgetToApplicationContext(w) );
}

static void parse_screen_size(char *s, int *width, int *height )
{
    if( s && *s ) {
        int e = sscanf(s,"%dx%d", width, height );
        if( e!=2) {
            fprintf(stderr, "invalid screen size: '%s'\n", s );
            exit(EXIT_FAILURE);
        }
    }
}

/*  -------------------------------------------------------------------------------------------------------------------- */
/*                               END IMPLEMENTATION                                                                      */
/*  -------------------------------------------------------------------------------------------------------------------- */

/******************************************************************************
**  Private Functions
******************************************************************************/
void quit_gui( Widget w, void *u, void *c )
{
    XtAppSetExitFlag( XtWidgetToApplicationContext(w) );
}

static void wm_quit ( Widget w, XEvent *event, String *params,
		   Cardinal *num_params )
{
    XtAppSetExitFlag( XtWidgetToApplicationContext(w) );
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
    /* xorg/app/oclock/oclock.c */
    static XtActionsRec actions[] = {
        {"quit",	wm_quit}
    };
    XtAppAddActions
	(app, actions, XtNumber(actions));
    XtOverrideTranslations
	(TopLevel, XtParseTranslationTable ("<Message>WM_PROTOCOLS: quit()"));
    Atom wm_delete_window;
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
    Display *dpy;
    XtAppContext app;
    Widget appShell,w;
    XtSetLanguageProc (NULL, NULL, NULL);
    XawInitializeWidgetSet();
    /*  -- Intialize Toolkit creating the application shell
     */
    appShell = XtOpenApplication (&app, argv[0],
                                        options, XtNumber(options),
                                         &argc, argv,
                                         fallback_resources,
                                         sessionShellWidgetClass,
                                         NULL, 0
                                         );
    TopLevel = appShell;
    dpy = XtDisplay(appShell);

    /* LOGIC:
       if deviceName is not given: show devices and ids and exit
       if deviceName is given and calibration matrix is supplied:
       apply calibration and exit.
       if deviceName is given and no matrix suplied start
       calibration process, calculate matrix, output matrix
       to stdout and apply matrix.
    */
    get_config(appShell);
    long int id = xi_get_deviceid(dpy,CONF.deviceName);
    if( id <0 ) {
        xi_list_devices(dpy);
        exit(EXIT_SUCCESS);
    }
    CONF.deviceId = id;

    if( CONF.matrix && *CONF.matrix ) {
        xi_apply_matrix_string(dpy,id,CONF.matrix);
        exit(EXIT_SUCCESS);
    }

    xi_apply_unity(dpy, id );

    w = XtVaCreateManagedWidget( "calib", calibWidgetClass, TopLevel,
                             NULL );
    XtAddCallback(w, XtNcallback, process_cal_data, NULL );

    /* enable Editres support */
    XtAddEventHandler(appShell, (EventMask) 0, True, _XEditResCheckMessages, NULL);
    /* grab window close event */
    XtAddCallback( appShell, XtNdieCallback, quit_gui, NULL );

    /*  -- Realize the widget tree and enter the main application loop
     */
    XtRealizeWidget ( appShell );
    grab_window_quit( appShell );

    if( CONF.screenSize && *CONF.screenSize ) {
        parse_screen_size( CONF.screenSize, &x_width, &x_height );
    } else  {
        make_borderless_window(appShell);

        if( xfullscreen(appShell, &x_width, &x_height) ) {
            Dimension xw,xh;
            XtVaGetValues(appShell, "height", &xh, "width", &xw, NULL );
            x_width = xw; x_height = xh;
        }

    }

    XtAppMainLoop ( app ); /* use XtAppSetExitFlag */
    XtDestroyWidget(appShell);
    return EXIT_SUCCESS;
}

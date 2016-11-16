/* most important define,
   check APPNAME.ad !
*/
#define APP_NAME "xcal"

/*
 */

#include "mls.h"

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

#include "xtcw/Calib.h"
#include "fullscreen.h"
#include "xborderless.h"




Widget TopLevel;

char *fallback_resources[] = {
    APP_NAME ".allowShellResize: False",
    "*WclResFiles:" APP_NAME ".ad\n",
    "*traceLevel: 2",
    NULL };

/* All Wcl applications should provide at least these Wcl options:
*/
static XrmOptionDescRec options[] = {
  { "-TraceLevel",	"*traceLevel",	XrmoptionSepArg, NULL },
  WCL_XRM_OPTIONS
};

typedef struct BasicSetting {
    int traceLevel;

} BasicSetting;



#define FLD(n)  XtOffsetOf(BasicSetting,n)
static XtResource basicSettingRes[] = {

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

void pr_mx(void *B, int rows, int cols)
{
    double *b = B;
    int i,j;

    for( i=0;i<rows;i++) {
        for(j=0;j<cols;j++)
            printf("%c%5.2f", j==cols-1 ? '|' : ' ', *b++);
        printf("\n");
    }

    for(i=0;i<cols;i++)
        printf("------");
    printf("\n\n");
}

/* gauss solver:
   B   : matrix [ eq ] [eq+1]
   res : vector [ eq ]
*/
void gauss1(void *b, void *res, int eq )
{
#define MX(a,y,x) (((double*)a)[ (x) + (y) * (n+1)])
    int i,n,k,j;
    double t,temp;
    double *B = b;
    double *a = res;

    // n=-0.5 + sqrt(0.25+m_len(b));
    n=eq;
    pr_mx( B,n,n+1);

    for (i=0;i<n;i++)
        for (k=i+1;k<n;k++)
            if ( MX(B,i,i) < MX( B,k,i) )
                for (j=0;j<=n;j++)
                {
                    temp=MX(B,i,j);
                    MX(B,i,j) = MX(B,k,j);
                    MX(B,k,j) = temp;
                }

    pr_mx(B,3,4);

    /* gauss elimination */
    for (i=0;i<n-1;i++) {

        for (k=i+1;k<n;k++)
            {
                t=MX(B,k,i)/MX(B,i,i);
                printf("%f\n", t );

                /* make the elements below the pivot elements
                   equal to zero or elimnate the variables
                */
                for (j=0;j<=n;j++)
                    MX(B,k,j)=MX(B,k,j) - t * MX(B,i,j);

                pr_mx(B,3,4);
            }


    }


    for (i=n-1;i>=0;i--)        // back-substitution
        {
            a[i]=MX(B,i,n); /* make the variable to be calculated equal
                             to the rhs of the last equation */
            for (j=0;j<n;j++)
                if (j!=i) /* then subtract all the lhs values except
                             the coefficient of the
                             variable whose value is being calculated */
                    a[i]=a[i]-MX(B,i,j)*a[j];
            a[i]=a[i]/MX(B,i,i); /* now finally divide the rhs by the coefficient
                                  of the variable to be calculated */
        }

    pr_mx(a,1,3);
    #undef MX
}



void print_matrix(char *name,
                  void *B, int rows, int cols)
{
    double *b = B;
    int i,j;

    printf("------------- %s ----------------------------\n", name );

    for( i=0;i<rows;i++) {
        for(j=0;j<cols;j++)
            printf("%c%5.2f", ' ', *b++);
        printf("\n");
    }

    for(i=0;i<cols;i++)
        printf("------");
    printf("\n\n");
}


struct pt {
    int xp,yp, /* disp */
        xt,yt; /* touch */

};


void touch_cal(struct pt *points, int cnt, int width, int height)
{
    struct pt *p;
    /* normieren */
    double xy[ 4 * cnt ];
    int i; int t;
    for( i=0; i<cnt; i++) {
        p=points+i;
        t=i*4;
        xy[0+t] = p->xt * 1.0 / width;
        xy[1+t] = p->yt * 1.0 / height;
        xy[2+t] = p->xp * 1.0 / width;
        xy[3+t] = p->yp * 1.0 / height;
    }

    print_matrix( "XY", xy, cnt, 4 );

    double ata[9], atx[3], aty[3];
    memset( ata, 0, sizeof ata );
    memset( atx, 0, sizeof atx );
    memset( aty, 0, sizeof aty );

    for( i=0; i<cnt; i++ ) {
        double *xy1 = xy + 4 * i;
        ata[0] += xy1[0] * xy1[0]; /* sigma xt^2 */
        ata[1] += xy1[0] * xy1[1];  /* sigma xt*yt */
        ata[2] += xy1[0];          /* sigma xt */
        ata[4] += xy1[1] * xy1[1]; /* sigma yt^2 */
        ata[5] += xy1[1];          /* sigma yt */

        atx[0] += xy1[0]*xy1[2]; /* sigma xt*xp */
        atx[1] += xy1[1]*xy1[2]; /* sigma yt*xp */
        atx[2] += xy1[2];        /* sigma xp */

        aty[0] += xy1[0]*xy1[3]; /* sigma xt*yp */
        aty[1] += xy1[1]*xy1[3]; /* sigma yt*yp */
        aty[2] += xy1[3];        /* sigma yp */
    }

    ata[3] = ata[1];
    ata[6] = ata[2];
    ata[7] = ata[5];
    ata[8] = cnt;


    print_matrix( "ata", ata, 3,3 );
    print_matrix( "atx", atx, 3,1 );
    print_matrix( "aty", aty, 3,1 );

    // abc
    double bgauss1[ 4 * 3 ], bgauss2[ 4 * 3 ];
    for(i=0;i<3;i++) {
        double *b = bgauss1 + i*4;
        double *a = ata     + i*3;
        double *x = atx + i;
        b[0] = a[0]; b[1] = a[1]; b[2] = a[2]; b[3] = x[0];
    }
    // def
    for(i=0;i<3;i++) {
        double *b = bgauss2 + i*4;
        double *a = ata     + i*3;
        double *x = aty + i;
        b[0] = a[0]; b[1] = a[1]; b[2] = a[2]; b[3] = x[0];
    }

    print_matrix( "ata\\atx", bgauss1, 3,4 );
    print_matrix( "ata\\aty", bgauss2, 3,4 );

    double abc[3], def[3];

    gauss1(bgauss1, abc, 3 );
    gauss1(bgauss2, def, 3 );

    print_matrix( "abc", abc, 3,1 );
    print_matrix( "def", def, 3,1 );

    for(i=0;i<3;i++)
        printf("%f ", abc[i] );
    for(i=0;i<3;i++)
        printf("%f ", def[i] );
    puts("");
}

void process_cal_data( Widget w, void *u, void *p )
{
    int i;
    puts("cal data");
    cal_point *c = p;
    for(i=0;i<4;i++) {
        printf("%d %d %d %d\n", c->x0, c->y0, c->x1, c->y1 );
        c++;
    }

    touch_cal( p,4, 1280,1024 );


    XtAppSetExitFlag( XtWidgetToApplicationContext(w) );
}




/******************************************************************************
**  Private Functions
******************************************************************************/


/*ARGSUSED*/
static void RegisterApplication ( Widget top )
{
    /* -- Register application specific actions */
    /* -- Register application specific callbacks */

    RCB( top, process_cal_data );
    RCB( top, quit_gui );
    /* user widgets */
    RCP( top, calib  );

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

    /*  init application functions
        All widgets are created, but not visible.
        functions can now communicate with widgets
    */



    /*  -- Realize the widget tree and enter the main application loop
     */
    XtRealizeWidget ( appShell );
    grab_window_quit( appShell );


    make_borderless_window(appShell);
    fullscreen(appShell);

    XtAppMainLoop ( app ); /* use XtAppSetExitFlag */
    XtDestroyWidget(appShell);

    m_destruct();

    return EXIT_SUCCESS;
}

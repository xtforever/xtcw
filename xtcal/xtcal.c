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

#ifdef DEBUG
#define pr_mx(a,b,c) dpr_mx(a,b,c)
#define print_matrix(a,b,c,d) dprint_matrix(a,b,c,d)
#else
#define pr_mx(a,b,c) do {} while(0)
#define print_matrix(a,b,c,d) do {} while(0)
#endif


typedef struct Matrix {
    float m[9];
} Matrix;

int x_width, x_height;

Widget TopLevel;

static void matrix_print(void *f);
static int apply_matrix(Display *dpy, int deviceid, Matrix *m);
static void matrix_set(Matrix *m, int row, int col, float val);
static void matrix_set_unity(Matrix *m);

void quit_gui( Widget w, void *u, void *c )
{
    XtAppSetExitFlag( XtWidgetToApplicationContext(w) );
}

static void wm_quit ( Widget w, XEvent *event, String *params,
		   Cardinal *num_params )
{
    XtAppSetExitFlag( XtWidgetToApplicationContext(w) );
}

/* --------------------------------------------------------------------------------------------------------------------

                        IMPLEMENTATION

  -------------------------------------------------------------------------------------------------------------------- */


static XrmOptionDescRec options[] = {
  { "-deviceName",	"*deviceName",	XrmoptionSepArg, NULL }
};
typedef struct app_conf {
    char *deviceName;
    long deviceId;
} app_conf;
static app_conf CONF;

#define FLD(n)  XtOffsetOf(app_conf,n)
static XtResource app_res[] = {
  { "deviceName", "DeviceName", XtRString, sizeof(String),
    FLD(deviceName), XtRString, ""
  }
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

void dpr_mx(void *B, int rows, int cols)
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



void dprint_matrix(char *name,
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

/* workaround compile bug on RPi float stack alignment not 8-byte */
void pf(double x)
{
    char s[10];
    char *e=gcvt(x,6,s); (void)e; printf("%s ", s);
}


void touch_cal(struct pt *points, int cnt, int width, int height)
{
    double abc[3], def[3];
    struct pt *p;

    memset(abc,0,sizeof(abc));
    memset(def,0,sizeof(abc));

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

    gauss1(bgauss1, abc, 3 );
    gauss1(bgauss2, def, 3 );

    print_matrix( "abc", abc, 3,1 );
    print_matrix( "def", def, 3,1 );


    for(i=0;i<3;i++)
        pf( abc[i] );
    for(i=0;i<3;i++)
        pf( def[i] );
    printf( "0 0 1\n");

    Matrix mc;
    matrix_set_unity(&mc);
    for(i=0;i<3;i++) {
        matrix_set(&mc,0,i, abc[i] );
        matrix_set(&mc,1,i, def[i] );
    }
    apply_matrix(XtDisplay(TopLevel), CONF.deviceId, &mc);
}

void process_cal_data( Widget w, void *u, void *p )
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

    touch_cal( p,4, x_width, x_height );
    XtAppSetExitFlag( XtWidgetToApplicationContext(w) );
}




/******************************************************************************
**  Private Functions
******************************************************************************/

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


char* fallback_resources[] = {
    "*calib.foreground: White",
    "*calib.highlight:  Red",
    "*calib.lineWidth: 10",
    "*calib.background: Lightblue",
    NULL
};


static void matrix_set(Matrix *m, int row, int col, float val)
{
    unsigned int i = row*3+col;
    assert( i < 9);
    m->m[i] = val;
}

static void matrix_set_unity(Matrix *m)
{
    memset(m, 0, sizeof(m->m));
    matrix_set(m, 0, 0, 1);
    matrix_set(m, 1, 1, 1);
    matrix_set(m, 2, 2, 1);
}

void xi_show(Display *d,  XDeviceInfo    *info )
{
    printf("%s : %ld\n", info->name, info->id );
}

void xi_list_devices(Widget top)
{
    XDeviceInfo         *info;
    int                 loop;
    int                 num_devices;

    Display *display = XtDisplay(top);
    info = XListInputDevices(display, &num_devices);
    for(loop=0; loop<num_devices; loop++) {
        xi_show(display, info+loop);
    }
}

long xi_find_device_id(Widget top, char *device_name)
{
    XDeviceInfo         *info;
    int                 loop;
    int                 num_devices;

    Display *display = XtDisplay(top);
    if( ! (device_name && *device_name)) return 0;
    info = XListInputDevices(display, &num_devices);
    for(loop=0; loop<num_devices; loop++) {
        if( strcmp(device_name, info[loop].name)==0 )
            return info[loop].id;
    }
    return 0;
}

static void matrix_print(void *f)
{
    Matrix *m = f;
    printf("[ %3.3f %3.3f %3.3f ]\n", m->m[0], m->m[1], m->m[2]);
    printf("[ %3.3f %3.3f %3.3f ]\n", m->m[3], m->m[4], m->m[5]);
    printf("[ %3.3f %3.3f %3.3f ]\n", m->m[6], m->m[7], m->m[8]);
}

static int
apply_matrix(Display *dpy, int deviceid, Matrix *m)
{
    Atom prop_float, prop_matrix;

    union {
        unsigned char *c;
        float *f;
    } data;
    int format_return;
    Atom type_return;
    unsigned long nitems;
    unsigned long bytes_after;

    int rc;

    prop_float = XInternAtom(dpy, "FLOAT", False);
    prop_matrix = XInternAtom(dpy, "Coordinate Transformation Matrix", False);

    if (!prop_float)
    {
        fprintf(stderr, "Float atom not found. This server is too old.\n");
        return EXIT_FAILURE;
    }
    if (!prop_matrix)
    {
        fprintf(stderr, "Coordinate transformation matrix not found. This "
                "server is too old\n");
        return EXIT_FAILURE;
    }

    rc = XIGetProperty(dpy, deviceid, prop_matrix, 0, 9, False, prop_float,
                       &type_return, &format_return, &nitems, &bytes_after,
                       &data.c);
    if (rc != Success || prop_float != type_return || format_return != 32 ||
        nitems != 9 || bytes_after != 0)
    {
        fprintf(stderr, "Failed to retrieve current property values\n");
        return EXIT_FAILURE;
    }

    matrix_print(data.f);

    memcpy(data.f, m->m, sizeof(m->m));
    matrix_print(data.f);
    XIChangeProperty(dpy, deviceid, prop_matrix, prop_float,
                     format_return, PropModeReplace, data.c, nitems);

    XFree(data.c);
    XFlush(dpy);
    return EXIT_SUCCESS;
}

void xi_set_unity(Widget top, long id)
{
    Matrix M;
    matrix_set_unity(&M);
    apply_matrix(XtDisplay(top), id, &M );
}



/******************************************************************************
*   MAIN function
******************************************************************************/
int main ( int argc, char **argv )
{
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
    get_config(appShell);
    long int id = xi_find_device_id(appShell,CONF.deviceName);
    if( id <0 ) {
        xi_list_devices(appShell);
        exit(EXIT_SUCCESS);
    }
    CONF.deviceId = id;
    printf("id=%ld\n", id );
    xi_set_unity(appShell, id );

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

    make_borderless_window(appShell);

    if( xfullscreen(appShell, &x_width, &x_height) )
        {
            Dimension xw,xh;
            XtVaGetValues(appShell, "height", &xh, "width", &xw, NULL );
            x_width = xw; x_height = xh;
        }


    XtAppMainLoop ( app ); /* use XtAppSetExitFlag */
    XtDestroyWidget(appShell);
    return EXIT_SUCCESS;
}

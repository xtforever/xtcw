#include <stdlib.h>
#include <math.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <WcCreate.h>
#include <Xp.h>
#include "mls.h"
#include <X11/Shell.h>
#include "WiconFlip.h"
#include "vexmo/Wsensor.h"
#include "vexmo/Wtext.h"
#include "vexmo/Wcheck.h"
#include "Tachometer.h"
#include "ini_read2.h"
#include "m_string.h"
#include "backend.h"
#include "Woptc.h"
#include "Wsensor.h"
#include "Wbutton.h"
#include "Gridbox.h"
#include <signal.h>


#define APP_NAME "multi2"
Widget TopLevel, TopMgr;
int AutoCreateChildren = 1;

/* 8x8 grid */
uint16_t grid[8];
void digital_toggle( Widget w, void *u, void *c );


/* All Wcl applications should provide at least these Wcl options:
*/
static XrmOptionDescRec options[] = {
    WCL_XRM_OPTIONS
};



enum PIN_TYPE {
    DIGITAL_IN,
    DIGITAL_OUT,
    ANALOG_IN,
    TWI_IN
};

struct pin_map {
    char *name;
    Widget w;
    char *resource;
    int pin;
    int type;
    int var; /* var[0]=pin_name */
};

/* kleine krücke */
int CUR_TYPE;

struct xumm_device {
    int state;
    int map;
    int vset;
    backend_device_t be;
} XUMM;


static char *pin_name(int pin, int type)
{
    static char buf[] = "A00";
    char *p;

    switch( type ) 
	{
	case DIGITAL_IN:
	case DIGITAL_OUT:
	    buf[0] = 'D';
	    break;
	case ANALOG_IN:
	    buf[0] = 'A';
	    break;
	case TWI_IN:
	    buf[0] = 'T';
	    break;
	}
    p=buf+1;
    if( pin < 0 ) pin = -pin;
    if( pin > 9 ) { *p++ = '0' + pin/10; pin %= 10; }
    *p++ = '0'+pin;
    *p=0;
    return buf;
}

Widget widget_find( Widget w, char *name)
{
  char clean[1024];
  WcCleanName( name, clean+1 );
  clean[0]='*';
  printf("Clean Name: %s\n", clean );
  Widget ret = WcFullNameToWidget(w,clean);
  if(!ret) WARN("Widget '%s' not found!", name );
  return ret;
}

/* find a free place inside TopMgr and create a child widget
   if type equals analog create a Wsensor widget
   if type equals digital create a Woptc widget
   and set readonly flag if its a digital_in widget
*/
Widget create_child( struct pin_map *pm )
{    
    static int last_x = 0, last_y = 0;

    Widget w;
    Arg wargs[10];
    int n;

    char *label;
    Dimension gx = last_x,gy =last_y;
    Boolean readOnly;

    printf("x=%d,y=%d", gx, gy );

    label = pm->name;
    if( pm->type == DIGITAL_IN || pm->type == ANALOG_IN ) readOnly=1;
    else readOnly = 0;
    
    n=0;
    XtSetArg(wargs[n], XtNgridx, gy); n++;
    XtSetArg(wargs[n], XtNgridy, gx); n++;

    if( pm->type == DIGITAL_IN || pm->type == DIGITAL_OUT ) {
	XtSetArg(wargs[n], XtNreadOnly, readOnly); n++;
	w = XtCreateManagedWidget(label,woptcWidgetClass, TopMgr, wargs, n );
	XtAddCallback( w, XtNcallback, digital_toggle, NULL ); 
    }
    else {
	XtSetArg(wargs[n], XtNlabelStr, label); n++;
	XtSetArg(wargs[n], XtNalarmLo, -1 ); n++;
	XtSetArg(wargs[n], XtNalarmHi, -1 ); n++;
	XtSetArg(wargs[n], XtNvalue, 9999 ); n++;
	w = XtCreateManagedWidget(label,wsensorWidgetClass, TopMgr, wargs, n );
    }

    if( last_y == 1 ) { last_y=0; last_x++; } else last_y=1;
    return w;
}

void add_mapping( char *name, int pin, int type )
{
    struct pin_map p;
    p.pin  = pin;
    p.type = type;
    p.var  = v_lookup( XUMM.vset, pin_name(pin,type) );
    p.name = strdup(name);
    p.resource = "value";
    p.w = widget_find( TopLevel, p.name );

    if( !p.w && AutoCreateChildren )
	p.w = create_child(&p);

    m_put( XUMM.map, &p );
}

void write_cmd( char *buf )
{
    char nl[1] = { 10 };
    backend_write(& XUMM.be, buf, strlen(buf) );
    backend_write( & XUMM.be, nl, 1 );
}

void digital_write( int pin, int status )
{
    char *buf;
    asprintf(&buf, "out %d %d", pin, status );
    write_cmd( buf );
    free(buf);
}

int get_mapping_pin( char *name )
{
    int key = m_lookup_str( XUMM.map, name, 1 );
    if( key<0 ) WARN("pin %s not found", name);
    else {
	struct pin_map *p = mls(XUMM.map,key);
	key = p->pin;
    }
    return key;
}

void quit_gui( Widget w, void *u, void *c )
{
    XtAppSetExitFlag( XtWidgetToApplicationContext(w) );
}

void digital_toggle( Widget w, void *u, void *c )
{
    struct pin_map *d;
    int status = (int) c;
    int p;
    m_foreach( XUMM.map, p, d ) {
	if( d->w == w ) {
	    int pin = d->pin;
	    if( pin >= 0 ) digital_write( pin, status );
	    break;
	}
    }
 }

/******************************************************************************
**  Private Functions
******************************************************************************/

/*ARGSUSED*/
static void RegisterApplication ( XtAppContext app )
{
    /* -- Useful shorthand for registering things with the Wcl library */
#define RCP( name, class  ) WcRegisterClassPtr   ( app, name, class );
#define RCO( name, constr ) WcRegisterConstructor( app, name, constr );
#define RAC( name, func   ) WcRegisterAction     ( app, name, func );
#define RCB( name, func   ) WcRegisterCallback   ( app, name, func, NULL );

    /* -- Register widget classes and constructors */
    /*
    RCP( "Wsensor", wsensorWidgetClass );
    RCP( "Wtext", wtextWidgetClass );
    */
    RCP( "Tachometer", tachometerWidgetClass );
    RCP( "Wcheck", wcheckWidgetClass ); 
    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
    RCB( "digital_toggle", digital_toggle ); 
    RCB( "quit_gui", quit_gui ); 
}

int create_cmd(int c, char *cmd, int type )
{
    if( c ) m_clear(c); 
    c=s_app(c,cmd,0);
    struct pin_map *d;
    int p;
    m_foreach( XUMM.map, p, d ) 
	{
	    if( d->type == type )
		s_printf( c, -1, " %d", d->pin ); 
	}
    return c;
}



/* mapping: 0=TEMP */
void parse_expr(char *s)
{
    TRACE(2, "parse: %s", s );
    int exp = m_split( 0, s, '=', 1 );
    if( m_len(exp) != 2 ) goto leave; 
    
    int pin = atoi( STR(exp,0) );
    char *name = STR(exp,1 );
    
    add_mapping( name, pin, CUR_TYPE );

 leave:
    m_free_strings(exp,0);
}


/* mapping: 0=TEMP, 1=LICHT */
void parse_mapping( char *s )
{
    int i;
    char **d;
    int exp= m_split(0,s, ',', 1 );
    m_foreach( exp,i,d )
	parse_expr(*d);
    m_free_strings(exp,0);
}


void dump_mapping()
{
    struct pin_map *d;
    int p;
    m_foreach( XUMM.map, p, d ) {
	printf("Name: %s, Nr: %d, PinName: %s\n", 
	       d->name,
	       d->pin,
	       STR(d->var,0) );
    }

}

void xumm_update_gui(void)
{
    int p; int *key;


    m_foreach(XUMM.vset,p,key)
	{
	    TRACE(1, "%s=%s\n", v_kget(*key,0), v_kget(*key,1) ); 
	}


   struct pin_map *d;

    m_foreach( XUMM.map, p, d ) {
	TRACE(1,"Name: %s, Nr: %d, PinName: %s, Value:%s\n", 
	       d->name,
	       d->pin,
	       STR(d->var,0),
	       v_kget(d->var,1) );

	if( d->w ) {
	    int x=atoi(v_kget(d->var,1));
	    XtVaSetValues( d->w, XtNvalue, x, NULL );
	}
    }
}

void server_init()
{
    XUMM.state++;
    int cmd;
    cmd = create_cmd(0,"cain",ANALOG_IN);
    write_cmd( m_buf(cmd) );     /* cain 0 1 2 3 */

    create_cmd(cmd, "cdin", DIGITAL_IN );
    write_cmd( m_buf(cmd) );     /* cdin 0 1 2 3 */

    create_cmd(cmd, "ctwi", TWI_IN );
    write_cmd( m_buf(cmd) );     /* ctwi 0 1 2 3 */
    
    /* write_cmd( "save" ); */
    m_free(cmd);
}

void server_read_measurements(void)
{
    write_cmd( "din" );
    write_cmd( "ain" );
    write_cmd( "gtwi" );

}


void cmd_var(char *s)
{
    int p=4;
    parse_variable_assignment_int(XUMM.vset,XUMM.be.line_buffer,&p);    
    xumm_update_gui();
}

parse_cmd_t cmd_tab[] = { 
    { "DIN", cmd_var },
    { "AIN", cmd_var },
    { "OUT", cmd_var },
    { "TWI", cmd_var },

    { NULL, NULL }
};

void xumm_input( enum callback_type type, void *u, void *c)
{
    backend_device_t *be = & XUMM.be;

    if( type == INPUT_CB ) {
	TRACE(2,"data");
	backend_dispatch( cmd_tab, m_buf(be->line_buffer));
	return;
    }

    if( type == TIMER_CB ) {
	if( XUMM.be.state == BE_READY ) 
	    { 
		server_init();
		XUMM.be.state++;
		return;
	    }
	
	server_read_measurements();
    }
}


void xumm_init( Widget top )
{
    Display *dpy = XtDisplay(top);

    XUMM.state = 0;
    XUMM.map = m_create(10,sizeof(struct pin_map));
    XUMM.vset = v_create();


    char *s   = XGetDefault(dpy, APP_NAME, "SERVER" ); 
    if(!s || ! index(s, ':' )) ERR(APP_NAME " resource 'SERVER: servername:portname' expected");
    int srv = m_split(0,s, ':', 1 );
    
   
    XtAppContext app = XtWidgetToApplicationContext(top);
    backend_device_t *be = & XUMM.be;
    be->state = BE_INIT;
    be->name = "XUMM";
    be->server_ip = STR(srv,0);
    be->server_port = STR(srv,1);
    be->device_num = 1;
    be->line_buffer = m_create(100,1);
    be->start = backend_connect;
    be->function = xumm_input;
    be->app = app;
    be->input_timer_interval = 1000;

    be_register( be );



    s = XGetDefault(dpy, "xumm", "ANALOG_IN" ); 
    if( s ) { CUR_TYPE = ANALOG_IN; parse_mapping(s); }

    s = XGetDefault(dpy, "xumm", "DIGITAL_IN" ); 
    if( s ) { CUR_TYPE = DIGITAL_IN; parse_mapping(s); }

    s = XGetDefault(dpy, "xumm", "DIGITAL_OUT" ); 
    if( s ) { CUR_TYPE = DIGITAL_OUT; parse_mapping(s); }

    s = XGetDefault(dpy, "xumm", "TWI_IN" ); 
    if( s ) { CUR_TYPE = TWI_IN; parse_mapping(s); }
    

    /* dump_mapping(); */

}


/******************************************************************************
*   MAIN function
******************************************************************************/
int main ( int argc, char **argv )
{
    XtAppContext app;
    signal(SIGPIPE, SIG_IGN); /* broken pipe on write */

    trace_level=1;
    m_init();
    rc_init( "multimeter.ini" );
    converters_xft_init();
    
    /*  -- Intialize Toolkit creating the application shell
     */
    Widget appShell = XtOpenApplication (&app, APP_NAME,
	     options, XtNumber(options),  /* resources: can be set from argv */
	     &argc, argv,
	     NULL,
	     applicationShellWidgetClass,
	     NULL, 0
	   );
    TopLevel = appShell;


    /*  -- Register all application specific callbacks and widget classes
     */
    RegisterApplication ( app );

    /*  -- Register all Athena and Public widget classes, CBs, ACTs
     */
    XpRegisterAll ( app );

    /*  -- Create widget tree below toplevel shell using Xrm database
     */
    WcWidgetCreation ( appShell );

    Widget TopMgr = XtVaCreateManagedWidget( "gb", gridboxWidgetClass, appShell, NULL );

    /* realize application objects */
    // TopMgr = widget_find( appShell, "gb" );
    int i;
    char box_name[50];
#define M_WIDTH 5
#define M_HEIGHT 2
    for(i=0;i< M_WIDTH * M_HEIGHT;i++) {
	sprintf( box_name, "box%d", i );
	XtVaCreateManagedWidget( box_name, wsensorWidgetClass, TopMgr,
				 XtNgridx, i % M_WIDTH,
				 XtNgridy, i / M_WIDTH,
				 XtNweightx, 1, XtNweighty, 1, XtNfill, FillBoth,
			     NULL );
    }

    char *s = "Kaushan Script-32:style=Regular";
    int   l = strlen(s)+1;
    XtVaCreateManagedWidget( "exit", wbuttonWidgetClass, TopMgr,
			     XtNgridWidth,  M_WIDTH,
			     XtNgridx, 0,
			     XtNgridy, M_HEIGHT,
			     XtNweightx, 1, XtNweighty, 0, XtNfill, FillWidth,
			     NULL );
   


    be_init(XtWidgetToApplicationContext(appShell));
    xumm_init(appShell);

    /*  -- Realize the widget tree and enter the main application loop
     */
    XtRealizeWidget ( appShell );





    XtAppMainLoop ( app );

    

    m_destruct();
    return EXIT_SUCCESS;
}

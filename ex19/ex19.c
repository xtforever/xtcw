/* most important define,
   check APPNAME.ad !


PWM Widget
-----------------
PWM     FREQ
293     10000 Hz
-----------------
*/
#define APP_NAME "ex19"

/*
 */


#include "mls.h"
#include "ini_read2.h"
#include "pipeshell.h"
#include "mrb.h"
#include "micro_vars.h"


#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include "communication.h"
#include "parser_util.h"


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

#include "xtcw/WviewQV.h"
#include "xtcw/Wlab.h"
#include "xtcw/Wsel.h"
#include "xtcw/DigTab.h"
#include "xtcw/KaroEd.h"
#include "xtcw/TermEd.h"
#include "xtcw/Radio2.h"
#include "xtcw/Qvlabel.h"


Widget TopLevel;
int DB;
int trace_main;
int q_filter;

#define TRACE_MAIN 1


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
    Widget widget_digtab;

    /* arduino proxy */
    char *host_name;
    char *host_service;

    XtAppContext ctx;

} BasicSetting;



#define FLD(n)  XtOffsetOf(BasicSetting,n)
static XtResource basicSettingRes[] = {
    { NULL, NULL, XtRWidget, sizeof(Widget),
      FLD(widget_digtab), XtRString, "*filter"
    },

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

    { "host_name", "Host_name", XtRString, sizeof(String),
      FLD(host_name), XtRString, "localhost"
    },
    { "host_service", "Host_service", XtRString, sizeof(String),
      FLD(host_service), XtRString, "10000"
    },

    { "traceLevel", "TraceLevel", XtRInt, sizeof(int),
      FLD(traceLevel), XtRImmediate, 0
    }
};
#undef FLD

struct BasicSetting CONFIG;

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


                        Must Provide
                        - RegisterApplication
                        - InitializeApplication

  -------------------------------------------------------------------------------------------------------------------- */



static XrmQuark q_en, q_rst, q_cfg, q_pin;
static void switch_enable( Widget w, void *u, void *c );
static void inc_rst( Widget w, void *u, void *c );
static void quarks_init(void);
static void mcu_input_cb( XtPointer p, int *n, XtInputId *id );

static int mcu_open(uint mcu);

enum MCU_STATE {
    DISABLED,
    INIT,
    OPEN
};


#define COMM_TIMER_MS 2000
static int MCU_COMM = 0;
struct mcu_comm {
    enum MCU_STATE state;
    struct mrb *qin;
    int line;
    char *host;
    char *service;
    int fd;
    XtInputId inp_id;
    XtAppContext app;
};

static struct mcu_comm *mcu_ptr(uint ctx)
{
    if(MCU_COMM<1 || ctx == 0  || ctx > m_len(MCU_COMM))
        {
            ERR("mcu ctx undefined %d", ctx); exit(1);
        }
    return mls(MCU_COMM, ctx-1 );
}

/* try to connect to "host:service"
   after connection wait for command
   if no commands are received send "x\n" and
   wait until command is received.
*/
static uint mcu_create(Widget top, char *host, char* service)
{
    if(! MCU_COMM ) MCU_COMM = m_create(10,sizeof(struct mcu_comm));
    struct mcu_comm *mcu = m_add(MCU_COMM);
    mcu->host = host;
    mcu->service = service;
    mcu->app = XtWidgetToApplicationContext(top);
    mcu->state = 0;
    mcu->qin = mrb_create(1024);
    mcu->line = m_create(100,1);
    mcu_open( m_len(MCU_COMM) );
    return m_len(MCU_COMM);
}

static int mcu_open(uint mcuid)
{
    struct mcu_comm *mcu = mcu_ptr(mcuid);
    mcu->fd  = sock_connect_service(mcu->host,mcu->service);
    if( mcu->fd < 0 ) return -1;
    mcu->inp_id = XtAppAddInput( mcu->app,mcu->fd,
                                 (XtPointer)  (XtInputReadMask),
                                 mcu_input_cb,
                                 (XtPointer) (intptr_t) mcuid  );
    mcu->state = OPEN;
    return 0;
}

static void mcu_close(uint mcuid)
{
    struct mcu_comm *mcu = mcu_ptr(mcuid);

    if( mcu->fd >= 0 )  { close(mcu->fd); mcu->fd = -1; }
    if( mcu->inp_id )   { XtRemoveInput(mcu->inp_id); mcu->inp_id = 0; }
    mcu->state = INIT;
}

static void mcu_send(uint mcuid, char *s)
{
    struct mcu_comm *mcu = mcu_ptr(mcuid);
    if( mcu->state == OPEN ) dprintf( mcu->fd, "%s", s );
}


static void mcu_destroy(uint mcuid)
{
    struct mcu_comm *mcu = mcu_ptr(mcuid);
    mcu_close(mcuid);
    m_free(mcu->line); mcu->line=0;
    mrb_destroy( mcu->qin ); mcu->qin = 0;
    mcu->state = DISABLED;
}


struct mv2_signal {
  void *d;
  void (*fn) (void*);
};

struct micro_vars2 {
  int vars;
  MapAg map;
};

struct mv2_variable {
    char *name;
    int type;
    int signal;
    char locked;
    union { int vint; float vfloat; void *vptr; } val;
    int size;
};

static struct micro_vars2 MV2;

void mv2_init(void)
{
    if( MV2.vars ) return;
    MV2.map       = MapAg_New();
    MV2.vars      = m_create(100,sizeof(struct mv2_variable));
    m_add(MV2.vars); /* first element is reserved */
}

void mv2_destroy(void)
{
    MapAg_Free( MV2.map );
    struct mv2_variable *var; int p;
    m_foreach( MV2.vars, p, var ) {
        free( var->name );
        if( var->signal ) m_free(var->signal);
        if( var->size ) free( var->val.vptr );
    };
    m_free( MV2.vars ); MV2.vars=0;
}

int mv2_var_lookup( intptr_t q )
{
    mv2_init();
    struct mv2_variable *var;

    int p = (intptr_t) MapAg_Find( MV2.map,  (void*)q,0,0 );
    if( !p ) {
        p = m_new(MV2.vars,1);
        MapAg_Define(MV2.map, q,0,0, (intptr_t)p  );
        var = mls(MV2.vars, p);
        memset(var,0,sizeof(*var));
    }
    return p;
}

enum MV2_TYPES {
    MV2_DATA = 0,
    MV2_STR  = 0,
    MV2_INT,
    MV2_MSTR,
    MV2_FLOAT
};

static uint size_of_type(enum MV2_TYPES t, uint size)
{
    switch( t ) {
    default: WARN("wrong type arg"); return 0;
    case MV2_DATA: return size;
    case MV2_INT: return sizeof(int);
    case MV2_MSTR: return sizeof(int);
    case MV2_FLOAT: return sizeof(float);
    };
}

void mv2_write( char *varname, int type, void *d, uint size )
{
    int q = XrmStringToQuark(varname);
    int p = mv2_var_lookup(q);
    struct mv2_variable *var = mls(MV2.vars,p);
    if( !var->name ) {
        var->name = strdup(varname);
        var->type = type;
        var->size = size_of_type(type,size);
        var->val.vptr = malloc(size);
        if( !var->val.vptr ) ERR("malloc");
    } else if( var->type != type )
        ERR("type mismatch: var-%d != %d", var->type, type );

    if( var->type == MV2_DATA ) {
        var->val.vptr = realloc( var->val.vptr, size );
        memcpy( var->val.vptr, d, size );
        var->size = size;
    } else {
        memcpy( &var->val.vint, d, size );
    }
    if(! var->signal ) return;
    if( var->locked ) return;
    struct mv2_signal *sig;
    var->locked = 1;   /* um rekursion zu verhindern wird
                          das signal blockiert */
    m_foreach( var->signal, p, sig ) {
        if( sig->fn ) sig->fn( sig->d );
    }
    var->locked = 0;
}

void mv2_parse_input(int buf)
{
    TRACE(2, "Input: %s", (char*)m_buf(buf) );
    if( CHAR(buf,0) != 'X' ) return;

    int varname, mstr, start_parse;
    mstr = m_create(10,1);
    varname = m_create(10,1);
    start_parse = 2;
    int *p = &start_parse;
    while(1) {
        if( parser_id( buf,p,varname ) ) goto parse_end;
        if( parser_skip( buf,p, '=' )) goto parse_end;;
        if( parser_id( buf, p, mstr )) goto parse_end;
        mv2_write( m_buf(varname), MV2_STR, m_buf(mstr), m_len(mstr) );
    }

 parse_end:
    m_free(varname);
    m_free(mstr);
}

void mv2_onwrite( int q, void (*fn) (void*), void *d, int remove )
{
  struct mv2_signal *sig;
  int p = mv2_var_lookup(q);
  struct mv2_variable* ent = mls(MV2.vars, p);
  if(!ent->signal) ent->signal = m_create( 2, sizeof(struct mv_signal));


  /* finde signal und falls remove==TRUE entfernen */
  m_foreach( ent->signal, p, sig )
      {
          if( sig->fn == fn && sig->d == d ) {
              if( remove ) m_del(ent->signal,p);
              return;
          }
      }
  if( remove ) return;

  /* signal noch nicht vorhanden, remove==FALSE, jetzt hinzufügen */
  sig = mls( ent->signal, m_new(ent->signal,1));
  sig->d = d;
  sig->fn = fn;
}

/** @brief read from fd and fill available space in queue
 * @returns -1 on error, 0 ok
 */
static int mrb_read_max(int fd, struct mrb *q)
{
    int free_space;
    char *buf  = mrb_maxsize(q, &free_space);
    if( free_space <= 0 ) return 0;

    int nread = read( fd, buf, free_space );
    if( nread > 0 )  {
        mrb_alloc( q, nread );
        return 0;
    }

    if( (errno == EINTR) || (errno == EAGAIN)
        || (errno == EWOULDBLOCK) )
        {
            return 0; /* kein fehler */
        }

    WARN("READ ERROR on mcu socket");
    return -1;
}

/* read all from socket,
   get all lines from queue
   parse all lines
   close the connection if somethink went wrong
*/
static void mcu_input_cb( XtPointer p, int *n, XtInputId *id )
{
    uint mcuid = (uintptr_t) p;
    struct mcu_comm *mcu = mcu_ptr(mcuid);
    if( mrb_read_max(mcu->fd, mcu->qin ) ) {
        mcu_close(mcuid);
        return;
    }

    while( mrb_get_line( mcu->qin, mcu->line ) )
        {
            mv2_parse_input(mcu->line);
            int p1=2; mv_parse(mcu->line,&p1,"");
            m_clear(mcu->line);
        }
}

static void mcu_communication( XtPointer data, XtIntervalId *id )
{
    XtAppContext app = data;
    struct mcu_comm *mcu;
    int i;
    /* verbindung aufbauen */
    m_foreach( MCU_COMM, i, mcu ) {
        if( mcu->state != INIT ) {
            mcu_send(i+1,"wr get\n");
            continue;
        }
        TRACE(2,"check com %s:%s",
              mcu->host, mcu->service );
        mcu_open( i+1 );
    }




    XtAppAddTimeOut(app,COMM_TIMER_MS, mcu_communication, app );
}


void input_pin_status_changed( void *d )
{
    int p = mv2_var_lookup( q_pin );
    struct mv2_variable *var = mls( MV2.vars, p );
    if( var->type != MV2_STR ) ERR("type mismatch");
    TRACE(2, "pin=%s", (char*)var->val.vptr );
    XtVaSetValues( CONFIG.widget_digtab, "pin_status",
                   var->val.vptr, NULL );
}

void port_cfg_changed( void *d )
{
    int p = mv2_var_lookup( q_cfg );
    struct mv2_variable *var = mls( MV2.vars, p );
    if( var->type != MV2_STR ) ERR("type mismatch");
    TRACE(2, "cfg=%s", (char*)var->val.vptr );
    XtVaSetValues( CONFIG.widget_digtab, "pin_config",
                   var->val.vptr, NULL );
}

static void print_config( Widget w, void *u, void *c )
{
    char *cfg,*stat;
    XtVaGetValues( w, "pin_status", &stat,
                   "pin_config", &cfg,
                   NULL );
    printf("%s %s\n", cfg, stat );
    char *buf;
    asprintf(&buf,"wr cfg %s\n", cfg );
    mcu_send(1, buf );
}

static void RegisterApplication ( Widget top )
{
    /* -- Register widget classes and constructors */
    RCP( top, wviewQV );
    RCP( top, wlab );
    RCP( top, wsel );
    RCP( top, digTab );
    RCP( top, karoEd );
    RCP( top, termEd );
    RCP( top, radio2 );
    RCP( top, qvlabel );

    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
    RCB( top, quit_gui );
    RCB( top, switch_enable );
    RCB( top, inc_rst );
    RCB( top, print_config );
}

/*  init application functions and structures and widgets
    All widgets are created, but not visible.
    functions can now communicate with widgets
*/
static void InitializeApplication( Widget top )
{
    trace_level = CONFIG.traceLevel;
    CONFIG.filelist = m_create(100, sizeof(char*));
    CONFIG.vset = v_init();
    quarks_init();
    // XtAppContext app = XtWidgetToApplicationContext(top);
    // XtAppAddTimeOut(app, COMM_TIMER_MS, mcu_communication, app );
    // mcu_create( TopLevel, CONFIG.host_name, CONFIG.host_service );

    mv2_onwrite( q_pin, input_pin_status_changed, 0, 0 );
    mv2_onwrite( q_cfg, port_cfg_changed, 0, 0 );

}

static void quarks_init(void)
{
    q_en  = XrmStringToQuark( "en" );
    q_rst = XrmStringToQuark( "rst" );
    q_pin = XrmStringToQuark( "pin" );
    q_cfg = XrmStringToQuark( "cfg" );
}



static void switch_enable( Widget w, void *u, void *c )
{

    int en = *mv_var(q_en);
    TRACE(1,"en: %d", en);
    mv_write( q_en, !en );
}

static void inc_rst( Widget w, void *u, void *c )
{
    TRACE(1,"");
    int rst = *mv_var(q_rst);
    mv_write( q_rst, rst + 1);
}


/******************************************************************************
**  Private Functions
******************************************************************************/

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

    int jörg = 0;

    
    XtAppContext app;
    signal(SIGPIPE, SIG_IGN); /* ignore broken pipe on write */
    m_init();
    mv_init();
    trace_level = trace_main;

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
    XtGetApplicationResources(	appShell, (XtPointer)&CONFIG,
				basicSettingRes,
				XtNumber(basicSettingRes),
				(ArgList)0, 0 );


    InitializeApplication(appShell);

    /*  -- Realize the widget tree and enter the main application loop
     */
    XtRealizeWidget ( appShell );
    grab_window_quit( appShell );
    //    pin_start_test();
    // start communication prozess */

    XtAppMainLoop ( app ); /* use XtAppSetExitFlag */
    XtDestroyWidget(appShell);
    m_destruct();

    return EXIT_SUCCESS;
}

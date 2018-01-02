#include "srvconnection.h"

#include "communication.h"
#include "mls.h"
#include "mrb.h"

#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>              /* Obtain O_* constant definitions */


enum MCU_STATE {
    DISABLED,
    INIT,
    OPEN
};

int SRVC = 2;
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
    srvc_cb callback;
    Widget w;
    void *user;
};

static int mcu_open(uint mcuid);
static void mcu_close(uint mcuid);
static struct mcu_comm *mcu_ptr(uint ctx);
static uint mcu_create(Widget top, char *host, char* service, srvc_cb cb, void *user);





static void dummy_cb(Widget w, void *u, void *d)
{
    int line = (intptr_t) d;
    if( line < 1 ) return;
    
    
    TRACE(SRVC, "%s", (char*)m_buf(line));
}


static void mcu_input_cb( XtPointer p, int *n, XtInputId *id )
{
    uint mcuid = (uintptr_t) p;
    struct mcu_comm *mcu = mcu_ptr(mcuid);

    TRACE(1,"input");

    if( mrb_sock_read(mcu->qin, mcu->fd  ) ) {
        mcu_close(mcuid);
        return;
    }

    
    
    while( mrb_get_line(mcu->qin, mcu->line) ) {
    
	if( mcu->callback )
	    mcu->callback(mcu->w, mcu->user, (void*) (intptr_t) mcu->line );
	else
	    dummy_cb(mcu->w, mcu->user, (void*) (intptr_t)mcu->line );
	m_clear(mcu->line);
    }
}



static void srv_conn_cb( XtPointer data, XtIntervalId *id )
{
    XtAppContext app = data;
    struct mcu_comm *mcu;
    int i;
    /* verbindung aufbauen */

    m_foreach( MCU_COMM, i, mcu ) {
        if( mcu->state != INIT ) {
            continue;
        }
        TRACE(2,"check com %s:%s",
              mcu->host, mcu->service );
        mcu_open( i+1 );
    }
    XtAppAddTimeOut(app, COMM_TIMER_MS, srv_conn_cb, app );    
}


int srvc_connect(Widget w, char *host, char *port, srvc_cb cb, void *user)
{
    int id;
    TRACE(SRVC, "connection to %s:% sok", host, port);
    id = mcu_create(w,host,port, cb, user );
    XtAppContext app = XtWidgetToApplicationContext(w);
    srv_conn_cb(app,NULL);
    return id;
}





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
static uint mcu_create(Widget top, char *host, char* service, srvc_cb cb, void *user)
{
    if(! MCU_COMM ) MCU_COMM = m_create(10,sizeof(struct mcu_comm));
    struct mcu_comm *mcu = m_add(MCU_COMM);
    mcu->host = host;
    mcu->service = service;
    mcu->app = XtWidgetToApplicationContext(top);
    mcu->state = 0;
    mcu->qin = mrb_create(1024);
    mcu->line = m_create(100,1);
    mcu->callback = cb;
    mcu->user = user;
    mcu->w = top;
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



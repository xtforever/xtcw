#include "slopnet.h"
#include "command-parser.h"
#include "mls.h"
#include "slop4.h"


static int cmd_put(int msg,void *ctx)
{
    int sln = (intptr_t) ctx;
    TRACE(1,"");
    return 0;
}

static int cmd_get(int msg,void *ctx)
{
    int sln = (intptr_t) ctx;
    TRACE(1,"");
    return 0;
}

static int cmd_exit(int msg,void *ctx)
{
    int sln = (intptr_t) ctx;
    TRACE(1,"");
    return 1;
}

static int cmd_circle(int msg,void *ctx)
{
    int sln = (intptr_t) ctx;
    TRACE(1,"server told us to draw a circle");
    return 0;
}


/**
 * this function is called if some data has arrived
 * sln_client_select -> sln_input_cb -> slop_put which calls this function 
 */
static void process_new_packet(int error, int msg, int sln, void *ctx)
{
    if( error ) return;
    m_putc(msg,0);
    TRACE(1,"Msg: %s\n", (char*)mls(msg,0));

    int err;
    cp_func_t fn = cp_lookup(msg);
    if(fn) {
	err= fn( (void*)(intptr_t)msg, (void*)(intptr_t)sln );
	if( err ) sln_set_exit_flag(1);
    }
}

/* returns socket fd */
int xtlua_init( Widget w, char *host, char *port )
{
    int sln = sln_init();
    
    /* init commands */
    cp_init();
    cp_add( "PUT:", cmd_put );
    cp_add( "EXIT", cmd_exit );
    cp_add( "CIRCLE:", cmd_circle );    

    int fd = sln_connect(sln, host, port, process_new_packet, w );
    if(fd < 0 ) ERR("server not found");

    return fd;
}



#include "slop-server.h"
#include "slopnet.h"
#include "command-parser.h"
#include "mls.h"
#include "luaexec.h"

#define SOCKET_PORT "1234"

int cmd_put(int msg,void *ctx)
{
    TRACE(1,"%s", (char*) mls(msg,0) );

    m_putc(msg,0);
    luaexecute( mls(msg,4), ctx );

    int sln = (intptr_t) ctx;
    sln_printf(sln,"EXIT");
    return 0;
}


int cmd_get(int msg, void *ctx)
{
    TRACE(1,"");
    return 0;
}    


void execute_msg(int msg, void *ctx)
{
    cp_func_t fn = cp_lookup(msg);
    if(fn) fn( (void*)(intptr_t)msg, ctx);
}

       


/**
 * this function is called if some data has arrived 
 */
void process_new_packet(int error, int msg, int sln, void *ctx)
{
    if( error ) return;

    m_putc(msg,0);
    TRACE(1,"Msg: %s\n", (char*)mls(msg,0));
    execute_msg(msg,(void*)(intptr_t)sln);
}



void server_start()
{

    sln_server_loop( SOCKET_PORT, process_new_packet,0);
    TRACE(1,"server started.Port: %s", SOCKET_PORT );
}



int main(int argc, char **argv)
{
    m_init();
    trace_level=1;

/* init commands */
    cp_init();
    cp_add( "PUT:", cmd_put );
    cp_add( "GET", cmd_get );

    server_start();
    
    m_destruct();
    return EXIT_SUCCESS;
}

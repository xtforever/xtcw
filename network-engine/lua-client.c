/**
 * read a lua file and send it with slop protocol to the given 
 * port at localhost.
 * wait for any data from the server and execute it.
 * if server sends "EXIT" the client will exit.
 * 
 *
 *
 * the server can be simulated by:
 * socat - tcp-l:1234
 * just type EXIT to exit the client
 *
 */


#include "slop-server.h"
#include "slopnet.h"
#include "command-parser.h"
#include "mls.h"
#include "slop4.h"




int cmd_put(int msg,void *ctx)
{
    TRACE(1,"");
    return 0;
}

int cmd_get(int msg,void *ctx)
{
    TRACE(1,"");
    return 0;
}

int cmd_exit(int msg,void *ctx)
{
    TRACE(1,"");
    return 1;
}

int cmd_circle(int msg,void *ctx)
{
    TRACE(1,"server told us to draw a circle");
    return 0;
}



/**
 * this function is called if some data has arrived
 * sln_client_select -> sln_input_cb -> slop_put which calls this function 
 */
void process_new_packet(int error, int msg, int sln, void *ctx)
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



int main(int argc, char **argv)
{
    FILE *fp;
    m_init();
    trace_level=1;

    /* init commands */
    if( argc < 3 || (fp=fopen(argv[2],"r"))==NULL ) 
	ERR("%s <port> <filename.lua>", argv[0] );

    /* init commands */
    cp_init();
    cp_add( "PUT:", cmd_put );
    cp_add( "EXIT", cmd_exit );
    cp_add( "CIRCLE:", cmd_circle );


    
    /* read file */
    int i = 0;
    int buf = m_create(1000,1); 
    int ch; int crc=0;
    char *s = "PUT:";
    /* write program header to slop buffer */
    while(s[i]) crc = slop_encode(buf,crc, s[i++]);
    /* write program to slop buffer, -1 will finalize the packet */
    do {
	ch=fgetc(fp);
	crc = slop_encode(buf,crc, ch);
    } while( ch >= 0);
    fclose(fp);

    /* create a slop context for decoding incomming data */
    int sln = sln_init();
    /* create a client socket annd write slop buffer to it */
    int fd = sln_connect(sln, "localhost", argv[1], process_new_packet, 0 );
    if(fd < 0 ) ERR("server not found");
    if( sock_write(fd,buf) ) ERR("could not write to socket");
    m_free(buf);

    /* main loop, calls sln_input_cb() if data arrives on socket fd */
    /* wait for response from server, parse slop, call process_new_packet */
    sln_client_select(sln);

    /* free all allocated slopnet contexts */
    sln_destruct();

    /* free list of registered commands */
    cp_destruct();

    /* free all remaining unfreed memory */
    m_destruct();
    
    return EXIT_SUCCESS;
}

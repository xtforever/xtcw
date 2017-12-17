
/*
 * standalone compile:
 *
 * gcc -DTEST_MAIN slop-server.c -I../lib ../lib/mls.c slopnet.c ../lib/mrb.c slop4.c ../lib/table256-crc16.c ../lib/communication.c
 *
 * 
 */

#include "slop-server.h"
#include "slopnet.h"
#include "mls.h"
#include "communication.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>









#if TEST_MAIN 


/**
 * this function is called if some data has arrived 
 */
void process_new_packet(int error, int msg, int sln, void *ctx)
{
    m_putc(msg,0);
    printf("Msg: %s\n", (char*)mls(msg,1));

    sln_printf(sln,"Hello %s\n", (char*)mls(msg,1) );
}



int main(int argc, char **argv)
{
    m_init();
    trace_level=1;

    sln_server_loop( "1234", process_new_packet, 0 ); 

    m_destruct();
    return EXIT_SUCCESS;
}
#endif

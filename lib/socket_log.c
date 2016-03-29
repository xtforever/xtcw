#include "config.h"

#include "socket_log.h"
#include "communication.h"
#include "mls.h"
#include "util.h"
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>

#define MAX_CLIENT_SOCKETS 100

struct socket_log_state {
    XtAppContext app;
    int client_socket[MAX_CLIENT_SOCKETS*2];
    int buffer;
    int n_connections;
} SLOG;

/** daten vom client annehmen und anzeigen */
static void slog_client( XtPointer p, int *n, XtInputId *id );

/** client hinzufügen, falls kein platz wird die verbindung unterbrochen */
static void append_client_socket(int sfd)
{
    int i;
    TRACE(trace_slog,"client sfd=%d", sfd );
    for(i=1;i<MAX_CLIENT_SOCKETS;i++) {
        if( SLOG.client_socket[i] == 0 ) {
            TRACE(trace_slog,"append client %d", i );
            SLOG.client_socket[i] = sfd;
            SLOG.client_socket[i+MAX_CLIENT_SOCKETS] =
                XtAppAddInput( SLOG.app, sfd,
                               (XtPointer) XtInputReadMask,
                               slog_client,(XtPointer) i );
            SLOG.n_connections++;
            return;
        }
    }
    close(sfd);
    system_error( "could not append client socket" );
}

/** client entfernen und verbindung beenden (close) */
static void remove_client(int sfd)
{
    int i;
    TRACE(trace_slog,"");
    for(i=1;i<MAX_CLIENT_SOCKETS;i++) {
        if( SLOG.client_socket[i] == sfd ) {
            SLOG.client_socket[i] = 0;
            XtRemoveInput(SLOG.client_socket[i+MAX_CLIENT_SOCKETS]);
            TRACE(trace_slog,"client %d removed, socket closed", i);
            close(sfd);
            SLOG.n_connections--;
            return;
        }
    }
}

/** vom Xt Main Loop aufgerufen, da sich am socket was getan hat */
static void slog_client( XtPointer p, int *sfd, XtInputId *id )
{
    char buffer[1024];
    int n = read( *sfd, buffer, sizeof(buffer)-1 );
    TRACE(trace_slog, "sfd=%d  num=%d  entry=%d\n", *sfd, n, (int)p );
    if( n<=0 ) { /* exit on error */
        remove_client(*sfd);
        return;
    }

    buffer[n] = 0;
    TRACE(trace_slog,"logger client: %s", buffer );
    /* dispatch commmand |buffer| here! */
}


/** if something happens on the master socket (listen)
    the Xt main-loop will call this function.
*/
static void slog_ready( XtPointer p, int *n, XtInputId *id )
{
    int sfd;
    while(1) { /* there can be more than one pending connection requests */
        sfd = sock_accept_incomming_connection(*n);
        if( sfd > 0 )
            append_client_socket( sfd );
        else break;
    }
}

/** called by main init */
void slog_init(Widget top)
{
    char *port;
    int sfd;
    memset(&SLOG,0,sizeof SLOG);
    port = rc_key_str( "logger", "port" );
    if( is_empty(port) ) port="10000";
    sfd = sock_listen_on_port(port);
    if( sfd < 0 ) return;

    SLOG.client_socket[0] = sfd;
    SLOG.buffer = m_create( 1000,1 );
    /* socket ist im modus "listen" sobald eine verbindung
       aufgebaut wird, wird der callback aufgerufen und
       die verbindung kann mit "accept" angenommen werden
    */
    SLOG.app = XtWidgetToApplicationContext(top);
    SLOG.client_socket[MAX_CLIENT_SOCKETS] = XtAppAddInput( SLOG.app,
                                           sfd,
                                           (XtPointer) XtInputReadMask,
                                           slog_ready, 0 );
}

/** nachricht an alle verbundenen sockets senden */
static void sendto_all(int msg)
{
    int sd,i;
    int p = m_len(msg);
    /* keine null am ende und kein leerer string */
    if( p > 1 ) p--; else return;
    for(i=1;i<MAX_CLIENT_SOCKETS;i++) {
        sd = SLOG.client_socket[i];
        if( sd > 0 ) {
            if( send(sd , m_buf(msg), m_len(msg), MSG_NOSIGNAL ) < 0 )
                remove_client(sd);
        }
    }
}

void slog_mputs( int buf )
{
    if(buf < 1 ) return;
    sendto_all(buf);

}


/** exported api: write something to the slog socket */
void slog_write_va( const char *format, va_list ap )
{
    int l;
    if( SLOG.n_connections <=0 ) return;
    vas_printf( SLOG.buffer,0, (char*)format, ap );
    l = m_len(SLOG.buffer);
    if( l < 3 ) return;
    slog_mputs(SLOG.buffer);
}

/** exported api: write something to the slog socket */
void slog_write(const char *format, ...)
{
  va_list ap;
  va_start(ap,format);
  slog_write_va( format, ap );
  va_end(ap);
}

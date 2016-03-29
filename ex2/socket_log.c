#include "config.h"
#include <sys/time.h>
#include "socket_log.h"
#include "communication.h"
#include "mls.h"
#include "util.h"
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>

int trace_slog = 0;
struct socket_log_state SLOG;

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
                               slog_client,(XtPointer) (intptr_t)i );
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
    TRACE(trace_slog, "sfd=%d  num=%d  entry=%d\n", *sfd, n, (intptr_t)p );
    if( n<=0 ) { /* exit on error */
        remove_client(*sfd);
        return;
    }

    buffer[n] = 0;
    TRACE(trace_slog,"logger client: %s", buffer );
    
    if( SLOG.cb ) SLOG.cb( *sfd, buffer, n );
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
int slog_init(Widget top, char *port, slog_cb_t cb )
{
    int sfd;
    memset(&SLOG,0,sizeof SLOG);
    if( is_empty(port) ) return -1;
    sfd = sock_listen_on_port(port);
    if( sfd < 0 ) return -1;
    SLOG.cb = cb;
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
    return 0;
}

/** nachricht an alle verbundenen sockets senden */
void slog_sendto_all(char *msg, int len)
{
    int sd,i;
    for(i=1;i<MAX_CLIENT_SOCKETS;i++) {
        sd = SLOG.client_socket[i];
        if( sd > 0 ) 
        {			
	  if( send(sd , msg, len, MSG_NOSIGNAL ) < 0 )
	    remove_client(sd);
        }
    }
}

void slog_mputs( int buf )
{
    if(buf < 1 ) return;
    int len = m_len(buf);
    if( len < 2 ) return;
    while( len > 0 && ( CHAR(buf,len-1)==0 )) len--; /* keine 0 senden */
    if( len > 0 ) slog_sendto_all(m_buf(buf), len);
}


/** exported api: write something to the slog socket */
void slog_write_va( const char *format, va_list ap )
{
    if( SLOG.n_connections <=0 ) return;

    struct timeval TimeVal;
    double DTime;
    gettimeofday(&TimeVal, NULL);
    DTime = ((double)TimeVal.tv_sec)+((double)TimeVal.tv_usec)/1000000.0;
    s_printf(SLOG.buffer, 0, "%f ",DTime);
    vas_printf( SLOG.buffer,-1, (char*)format, ap );
    if( s_lastchar(SLOG.buffer) != 10 ) s_app(SLOG.buffer, "\n" );
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

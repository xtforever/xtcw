#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

int trace_m123;

#include "if_mpg123.h"
#include "communication.h"


static struct M123 {
    XtAppContext app;
    char *server, *port;
    int fd, state;
    struct mrb *qin, *qout;
    XtInputId inp_id;
    void (*func)(int old,int new);
    int line;
    int pos;
    int frame_cnt;
} M123;

static void m123_input_cb( XtPointer p, int *n, XtInputId *id );
static void read_max(int fd, struct mrb *q);
static void m123_open(void);
/*
   static void init_communication( XtPointer data, XtIntervalId *id );
   XtAppAddTimeOut(app,COMM_TIMER_MS, init_communication, app );
   XtRemoveInput( be->inp_id ); be->inp_id = 0; }
   be->inp_id = XtAppAddInput( be->app, be->fd,
                                (XtPointer)  (XtInputReadMask),
                                be->input_data_cb, be  );
   XtInputId inp_id;
   XtInputCallbackProc input_data_cb;
   XtAppContext app;
*/


/* starten und handle im xserver registrieren,
   falls nicht gestartet werden kann einen timer
   für nächsten versuch registrieren.
*/
int m123_init(Widget top, char *server, char* port, void (*func)(int old, int new))
{
    M123.func = func;
    M123.server = server;
    M123.port = port;
    // XtAppContext
    M123.app = XtWidgetToApplicationContext(top);
    M123.state = 0;
    M123.qin = mrb_create(1024);
    M123.line = m_create(100,1);
    m123_open();
    return 0;
}

/* returns 1 - if files exists */
static int fexists(char *path)
{
            struct stat sb;
            return !stat(path, &sb);
}


static void m123_open(void)
{
    M123.fd  = sock_connect_service(M123.server,M123.port);
    if( M123.fd < 0 ) ERR("mpg123 server not found");
    M123.inp_id = XtAppAddInput( M123.app,M123.fd,
                                 (XtPointer)  (XtInputReadMask),
                                 m123_input_cb, 0  );
    M123.state = M123_STOP;
}

static void m123_close(void)
{
    if( M123.fd >= 0 ) { close(M123.fd); M123.fd = -1; }
    if( M123.inp_id ) { XtRemoveInput(M123.inp_id); M123.inp_id = 0; }
    M123.state = M123_DISCONNECT;
}

void m123_destroy(void)
{
    m123_close();
    m_free(M123.line);
    mrb_destroy( M123.qin ); M123.qin = 0;
}


void m123_pause(void)
{
    dprintf( M123.fd, "PAUSE\n" );
}

/* datei einladen und abspielen */
int m123_load(char *fn)
{
    TRACE( trace_m123, fn );
    if(! fexists(fn) ) return -1;
    dprintf( M123.fd, "LOAD %s\n", fn );

    /* sobald der song läuft wird M123_PLAY gesetzt.
       damit auch ein status update stattfindet,
       jetzt erst einen anderen status als M123_PLAY setzen
    */
    if( M123.state == M123_PLAY )
        M123.state = M123_STOP;
    return 0;
}

int m123_status(void) { return M123.state; }

int m123_playpos(void) { return M123.pos; }

void m123_set_playpos(int relative ) {
    if( M123.state != M123_PLAY || M123.frame_cnt == 0 ) return;
    int frame_no = M123.frame_cnt * relative / 10000;
    TRACE( trace_m123, "pos: %d frame: %d", relative, frame_no  );
    dprintf( M123.fd, "JUMP %d\n", frame_no );
}

static int get_line( int line, struct mrb *q )
{
    // TRACE(trace_m123, "used %d", mrb_bytesused(q) );
    int ch, found = 0;
    while( (ch=mrb_get(q)) != -1 )
        if( ch == 10 ) {
            m_putc(line,0);
            found = 1;
            break;
        } else m_putc(line,ch);

    //    TRACE(trace_m123, "used %d", mrb_bytesused(q) );
    return found;
}

static void play_status(int line)
{
    int ch = CHAR(line,3);
    int s = M123.state;
    TRACE(trace_m123,"status: %c", ch  );
    switch(ch) {
    case '1':
        TRACE(trace_m123,"pause" );
        M123.state = M123_PAUSE;
        break;
    case '2':
        TRACE(trace_m123,"play/unpause" );
        M123.state = M123_PLAY;
        break;
    default:
        M123.state = M123_STOP;
        TRACE(trace_m123,"stop" );
        break;
    }

    if( s != M123.state ) M123.func(s, M123.state);
}

/* Format:
   F n1 n2
*/
static void parse_position(int line)
{
    if( m_len(line) < 8 ) return;
    int n1,n2;
    if( sscanf( mls(line,3), "%d %d", &n1, &n2 ) == 2 ) {
        M123.frame_cnt = n1+n2;
        M123.pos = (n1*10000) / (n1+n2);
    }
}


static void play_pos(int line)
{
    //    TRACE(trace_m123,"");
    int s = M123.state;
    M123.state = M123_PLAY;
    parse_position(line);
    M123.func(s, M123.state );
}

static void parse_input(int line)
{
    if( CHAR(line,0) != '@' ) return;

    switch( CHAR(line,1) )
        {
        case 'P': play_status( line ); break;
        case 'F': play_pos(line); break;
        default:
            TRACE(trace_m123, "%s", m_buf(line) );
            break;
        }
}

static void m123_input_cb( XtPointer p, int *n, XtInputId *id )
{
    // TRACE(trace_m123,"");
    Widget top = p;
    read_max(M123.fd, M123.qin );
    while( get_line( M123.line, M123.qin ) )
        {
            parse_input(M123.line);
            m_clear(M123.line);
        }
}

static void read_max(int fd, struct mrb *q)
{
    int free_space;
    char *buf  = mrb_maxsize(q, &free_space);
    //    TRACE(trace_m123, "free space: %d", free_space );
    if( free_space == 0 ) return;

    int nread = read( fd, buf, free_space );
    if( nread <= 0 )  {
        if( (errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK) )
	  {
	      return; /* kein fehler */
	  }
        ERR("READ ERR or file not found"); /* todo: close socket/reopen socket */
        m123_close();
        m123_open();
    }
    mrb_alloc( q, nread );
}

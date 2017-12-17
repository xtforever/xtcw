#include "subshell.h"


#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <errno.h>
#include <stdarg.h>

#include "mls.h"
#include "mrb.h"

int trace_child = 2;

static void xclose( int *fd )
{
    if( *fd < 0 ) return;
    close(*fd);
    *fd = -1;
}




/** create a child process and connect a 3 pipes
    to child 

    create some pipes:
    
    0 R                READ from child STDOUT
    1 W     closed
    2 R     closed
    3 W                WRITE to child  STDIN
    4 R                READ from child STDERR
    5 W     closed            

    0: read <--- pipe < stdout
    1: write --> pipe > stdin
    2: read <--- pipe < stderr

*/
static void fork2_exec(struct fork2_info *child, char *filename, char **args )
{
    memset( child->fd, 0xff, sizeof( child->fd ));
    if( pipe2( child->fd, O_NONBLOCK ) ) ERR("pipe2");
    if( pipe2( child->fd+2, O_NONBLOCK ) ) ERR("pipe2");
    if( pipe2( child->fd+4, O_NONBLOCK ) ) ERR("pipe2");

    pid_t cpid = fork();
    if (cpid == -1) ERR("fork");
    if (cpid == 0) {            /* Child reads from pipe */
        dup2(child->fd[1],1);   /* make STDOUT==1 same as WRITE-TO==1 end
                                   of pipe-A  */

	dup2(child->fd[2],0);   /* make STDIN==0 same as READ-FROM==0 end
                                   of pipe-B */

	/* stdin = read-end of pipe a */
	dup2(child->fd[5], 2);  /* stderr = write-end of pipe c */

        xclose(child->fd+0);    /* Close unused read end pipe-A */
        xclose(child->fd+3);    /* Close unused write end pipe-B */
        xclose(child->fd+4);    /* Close unused read end pipe-c */

        if( trace_child >= trace_level ) {
            char **sp;
            fprintf(stderr, "exec: %s ", filename );
            for(sp = args; *sp; sp++ )
                fprintf(stderr, "<%s> ", *sp );
            fprintf(stderr, "\n" );
        }

        execvp( filename, args );

        perror( "can not exec child process" );
        _exit(EXIT_FAILURE);    /* never reached */
    }
    xclose( child->fd+1 );      /* close unused WRITE end of pipe-A */
    xclose( child->fd+2 );      /* close unused READ  end of pipe-B */
    xclose( child->fd+5 );      /* close unused WRITE end of pipe-C */
    child->stat = CHILD_RUNNING;
    child->pid = cpid;
}


struct fork2_info *fork2_open(char *filename, ...)
{
    char *name;
    va_list ap;
    struct fork2_info *child = calloc(1,sizeof (struct fork2_info));
    child->pipe_buf[0] = mrb_create(MRB_BUFSIZE);
    child->pipe_buf[1] = mrb_create(MRB_BUFSIZE);

    int args = m_create(10,sizeof(char*));
    name = strdup(filename);
    m_put(args,&name);

    va_start(ap,filename);
    while( (name = va_arg( ap, char* )) != NULL )
    {
        name = strdup(name);
	m_put(args, &name);
    }
    name = NULL;
    m_put(args,&name);
    va_end(ap);

    fork2_exec(child,filename, m_buf( args));
    m_free_strings(args,0);
    return child;
}

/* non blocking read from child pipe
   returns
   >=0: bytes read
   -1: read error
*/
static int mrb_read(int fd, struct mrb *mrb)
{
    int free_space;
    char *buf  = mrb_maxsize(mrb, &free_space);
    TRACE(trace_child,"chunksize: %d", free_space );
    if( free_space == 0 ) return 0;
    int nread = read( fd, buf, free_space );
    TRACE(trace_child, "read: %d %s", nread, nread < 0 ? "read error" : "OK" );
    if( nread < 0 )  {
        return -1;
    }
    mrb_alloc( mrb, nread );
    return nread;
}


/** kill child and free all resources */
void fork2_close( struct fork2_info *child )
{
    int status,err;
    if( child->stat == CHILD_RUNNING ) {
        kill( child->pid, SIGKILL );
        TRACE(trace_child, "sending kill" );
        err=waitpid(child->pid,&status,0);
        TRACE(trace_child, "waitpid = %d", err );
        child->stat = 0;
    }

    free(child->pipe_buf[0]);
    free(child->pipe_buf[1]);
    xclose( child->fd+CHILD_STDOUT_RD );
    xclose( child->fd+CHILD_STDERR_RD );
    xclose( child->fd+CHILD_STDIN_WR );
    free(child);
}


/** Ein newline ab |*pos| suchen.
    returns: 1 - zeile bis pos ausgeben, 0 - warten auf weitere daten
*/
static int find_newline(struct mrb *c, int *pos)
{
    int ch;
    for(;;) {
        ch = mrb_peek(c, pos );
        if( ch == 10 ) return 1;
        if( ch < 0 ) {
            /* ist puffer voll aber kein newline vorhanden? overflow error */
            if( *pos == mrb_bufsize(c) ) return 1;
            return 0;
        }
    }
}

static void normalize_str(int m)
{
    if( m_len(m) > 0 && CHAR(m, m_len(m)-1 ) == 10 )
        CHAR(m, m_len(m)-1 ) = 0;
    else m_putc(m,0);
}


/* nächste zeile aus dem eingabe-puffer holen
   m - marray of char, wird immer gelöscht
   returns 0: keine weiteren zeilen im puffer
*/
static int  mrb_getline(struct mrb *c, int m, int *pos)
{

    char *buf;
    int chunk;
    if(! find_newline( c, pos ) ) return 0;
    TRACE(trace_child, "buffer read bytes: %d", *pos );
    m_clear(m);
    while( *pos > 0 ) {
        chunk = *pos;
        buf = mrb_read_chunk(c, &chunk );
        TRACE(trace_child,"copy %d bytes to linebuffer", chunk );
        if( chunk <= 0 ) ERR("mrb_read_chunk error");
        m_write(m,m_len(m), buf, chunk );
        (*pos) -= chunk;
    }
    normalize_str(m);
    *pos = 0;
    return m_len(m);
}

int fork2_read(struct fork2_info *child, int pipe )
{
    int fd = CHILD_STDOUT_RD;
    if( pipe ) {
	pipe = 1;
	fd = CHILD_STDERR_RD;
    }
    
    if( mrb_read( child->fd[fd], child->pipe_buf[pipe] ) <=0 )
        {
            TRACE(trace_child, "error reading from child" );
            return -1;
        }
    return 0; /* OK */
}



int fork2_getchar(struct fork2_info *child, int pipe )
{
    if( pipe ) pipe = 1;
    return mrb_get(child->pipe_buf[pipe]);
}

/* nächste zeile aus dem eingabe-puffer holen
   m - marray of char, wird immer gelöscht
   returns 1: keine weiteren zeilen im puffer, sonst 0
*/
int fork2_getline( struct fork2_info *child, int pipe, int lnbuf )
{
    if( pipe ) pipe = 1;
    return mrb_getline(child->pipe_buf[pipe], lnbuf, child->scan_pos+pipe) == 0;
}

int fork2_write( struct fork2_info *child, char *s )
{
    dprintf(child->fd[CHILD_STDIN_WR],"%s", s);
    return 0;
}

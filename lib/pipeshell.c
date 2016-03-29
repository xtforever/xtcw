/** open 2 pipes to a subshell
    - manage multiple subshells
    - multi-line input buffer
    - detect kill signals
    - stores exit code
    - no polling
    - use select() interface
    - use callback interface
    - preemptive child close support
    - avoids zombie process
    - non blocking status check

    Most simple interface
    ------------------------


    my_pin = child_start(TopLevel, "./datamaker", childcb );

    - - -

    void childcb(int pin)
    {
      enum child_proc_state stat = child_status(pin);
      if(  stat == CHILD_RUNNING )
        {
            TRACE(1,"data: %s", child_data(pin) );
            return;
        }

      if( stat == CHILD_EXIT_SUCCESS )
        TRACE(1,"EXIT SUCCESS");
      else
        TRACE(1,"EXIT FAILURE %d", child_exit_status(pin) );
    }

*/


#include "pipeshell.h"



#include "mls.h"
#include "mrb.h"

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>              /* Obtain O_* constant definitions */

struct child_stat {
    char *filename;
    pid_t pid;
    int fd[4];
    int exit_value;
    int flags;
    struct mrb *qin;
    enum child_proc_state stat;
    int error;
    int line;
    XtInputId id;
    void (*cb)(int);
    int args;
};

int trace_child;
int CLIST = 0;

/******************************************************************************
**  Utilities
******************************************************************************/

static void xclose( int *fd )
{
    if( *fd < 0 ) return;
    close(*fd);
    *fd = -1;
}

static void* m_add2(int list, int *pos)
{
    *pos = m_new(list,1);
    return mls(list,*pos);
}

static int mrb_get_line( struct mrb *q, int line  )
{
    int ch, found = 0;
    while( (ch=mrb_get(q)) != -1 )
        if( ch == 10 ) {
            m_putc(line,0);
            found = 1;
            break;
        } else m_putc(line,ch);

    return found;
}

/** einen möglichst großen datenblock in den puffer einlesen
 * @returns 0 wenn alles ok ist, ansonsten errno
 */
static int mrb_read_max(struct mrb *q, int fd)
{
    int free_space;
    char *buf  = mrb_maxsize(q, &free_space);
    if( free_space == 0 ) return 0;
    int nread = read( fd, buf, free_space );
    if( nread <= 0 ) return errno;
    mrb_alloc( q, nread );
    return 0;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* GETTERS */
enum child_proc_state child_status(int hc)
{
    struct child_stat *child = mls(CLIST, hc);
    return child->stat;
}
int child_read_fd(int hc)
{
    struct child_stat *child = mls(CLIST, hc);
    return child->fd[0];
}
int child_exit_status(int hc)
{
    struct child_stat *child = mls(CLIST, hc);
    return child->exit_value;
}
char* child_data(int hc)
{
    struct child_stat *child = mls(CLIST, hc);
    return m_buf(child->line);
}
/** returns index of child argument list */
int child_args(int hc)
{
    struct child_stat *child = mls(CLIST, hc);
    return child->args;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/** run child process "name", returns handle for process
    returns: <=0 : error, no process forked
    >0 : child process handle
*/
static struct child_stat *child_new(int *pos)
{
    struct child_stat *child;
    *pos=m_len(CLIST);
    while( (*pos)-- ) {
        child = mls(CLIST,*pos);
        if( child->stat == CHILD_NOT_INIT ) return child;
    }
    return m_add2( CLIST, pos );
}



/** init child process structure
 *  @returns child handle
 */
static int child_init( char *name, int flags )
{
    int hc;
    struct child_stat *child;
    if( !CLIST ) CLIST = m_create(10,sizeof(struct child_stat));
    if(is_empty(name)) return -1;

    child = child_new( &hc );
    child->filename = strdup(name);
    child->args = m_create(10,sizeof(char*));
    v_kset( child->args, name, 0 );
    child->flags = flags;
    child->qin = mrb_create(MRB_BUFSIZE);
    child->line = m_create(100,1);
    memset( child->fd, 0xff, sizeof( child->fd ));
    return hc;
}


static void decode_waitpid_status(int status)
{
    if(WIFEXITED(status))
        printf("Child exited, status=%d\n", WEXITSTATUS(status));
    if(WIFSIGNALED(status)) {
        printf("Child exit on signal=%d\n", WTERMSIG(status));
        if(WCOREDUMP(status))
            printf("Child core dump\n");
    }
    if(WIFSTOPPED(status))
        printf("Child stopped\n");
    if(WIFCONTINUED(status))
        printf("Child cont.\n");
}


static void child_get_exit_value(struct child_stat *child)
{
    int status;
    int err = waitpid(child->pid,&status,0);
    if( (err == -1) ) {
        TRACE(trace_child,"error in waitpid %d\n", err );
        child->exit_value = -1;
        child->stat = CHILD_EXIT_FAILURE;
        child->error = CHILD_ERR_PROCESS_NOT_FOUND;
        return;
    }

    if(WIFEXITED(status)) {
        child->exit_value = WEXITSTATUS(status);
    }
    else if(WIFSIGNALED(status)) {
        TRACE(trace_child,"child exit on signal %d\n", WTERMSIG(status));
        child->exit_value = 128 + WTERMSIG(status);
    }
    else child->exit_value = -1;

    TRACE(trace_child,"Child exited, status=%d\n", child->exit_value);
    child->stat = child->exit_value ?
        CHILD_EXIT_FAILURE : CHILD_EXIT_SUCCESS;
    child->error = CHILD_ERR_NO_ERROR;
}

/** kill child process and close file handles */
static int child_close_handle( int hc )
{
    struct child_stat *child = mls(CLIST, hc);

    if( child->stat == CHILD_RUNNING ) {
        kill( child->pid, SIGKILL );
        child_get_exit_value(child);
    }

    xclose( child->fd+0 );
    xclose( child->fd+1 );
    return  child->error;
}

/** kill every subprocess */
void child_destruct(void)
{
     if( !CLIST ) return;
    int len = m_len(CLIST);
    while( len-- ) child_close(len);
    m_free(CLIST); CLIST = 0;
}
static void m_drop(int m)
{
    m_setlen(m, m_len(m)-1 );
}

/** create a child process and connect a pipe
    to child stdout
*/
static void child_exec(int hc)
{
    char *null_string = NULL;
    struct child_stat *child = mls(CLIST, hc);
    if( pipe2( child->fd, O_NONBLOCK ) ) ERR("pipe2");
    if( pipe2( child->fd+2, O_NONBLOCK ) ) ERR("pipe2");
    pid_t cpid = fork();
    if (cpid == -1) ERR("fork");
    if (cpid == 0) {            /* Child reads from pipe */
        dup2(child->fd[1],1);   /* make STDOUT==1 same as WRITE-TO==1 end
                                   of pipe-A  */
        dup2(child->fd[2],0);   /* make STDIN==0 sama es READ-FROM==0 end
                                   of pipe-B */
        xclose(child->fd+0);    /* Close unused read end pipe-A */
        xclose(child->fd+3);    /* Close unused write end pipe-B */

        m_put(child->args,&null_string);
        //        execlp( child->filename, child->filename, NULL );
        execvp( child->filename, m_buf(child->args) );
        m_drop(child->args);

        perror( "can not exec child process" );
        _exit(EXIT_FAILURE);    /* never reached */
    }
    xclose( child->fd+1 );      /* close unused WRITE end of pipe-A */
    xclose( child->fd+2 );      /* close unused READ  end of pipe-B */
    child->stat = CHILD_RUNNING;
    child->pid = cpid;
}


/** polling of child status
    @returns 0 - no signal
*/
static int child_waitpid(int hc)
{
    struct child_stat *child = mls(CLIST, hc);
    int status;

    if( child->stat != CHILD_RUNNING ) return child->stat;

    int err = waitpid(child->pid,&status,WNOHANG );
    if( err == 0 ) return 0;
    if( err == -1 ) {
        TRACE(trace_child,"error in waitpid\n");
        child->stat = CHILD_EXIT_FAILURE;
        child->error = CHILD_ERR_PROCESS_NOT_FOUND;
        return -3;
    }
    if (WIFEXITED(status)) {
        child->exit_value = WEXITSTATUS(status);
        TRACE(trace_child,"Child exited, status=%d\n", WEXITSTATUS(status));
        child->stat =
            WEXITSTATUS(status) ?
            CHILD_EXIT_FAILURE : CHILD_EXIT_SUCCESS;
        return WEXITSTATUS(status) ? 2 : 1;
    }
    return 0;
}

/** fehlerprüfung und daten einlesen
 */
static int child_read2(int hc)
{
    int err;
    struct child_stat *child = mls(CLIST, hc);
    TRACE(1,"");

    /* einlesen ist non-blocking */
    err = mrb_read_max(child->qin, child_read_fd(hc) );
    if( ! err ) return 0;

    /* irgendetwas geht fürchterlich schief
       was ist mit dem subprocess? */
    if( child_waitpid(hc) != 0 ) return 1;

    /* subprocess läuft, wie sieht es mit errno aus? */
    TRACE(1,"err %s", strerror(err) );
    if( (err == EINTR) || (err == EWOULDBLOCK) ) return 0;

    /* errno sagt nichts gutes, besser die verbindung beenden */
    return 1;
}

void child_close(int hc)
{
    struct child_stat *child = mls(CLIST, hc);
    if( child->stat == CHILD_NOT_INIT ) return;
    child_close_handle(hc);      /* get exit status */
    XtRemoveInput(child->id);    /* remove listener */
    if(child->cb) child->cb(hc); /* tell app. */
    m_free(child->line);         /* free mem */
    mrb_destroy(child->qin);
    free( child->filename );
    child->stat = CHILD_NOT_INIT;
    m_free_strings(child->args,0);
}

static void child_input_cb( XtPointer p, int *n, XtInputId *id )
{
    int hc = (int) p;
    struct child_stat *child = mls(CLIST, hc);
    TRACE(trace_child,"");
    if( child_read2(hc) )  {
        child_close(hc);
        return;
    }

    while( mrb_get_line(child->qin,child->line)==1) {
        if( child->cb ) child->cb(hc);
        m_clear(child->line);
    };
}

int child_start(Widget top, char *cmd, void (*cb)(int) )
{
    int hc = child_init(cmd, 0);
    struct child_stat *child = mls(CLIST, hc);
    child->cb = cb;
    child_exec(hc);
    child->id = XtAppAddInput(XtWidgetToApplicationContext(top),
                              child_read_fd(hc),
                              (XtPointer)  (XtInputReadMask),
                              child_input_cb,(XtPointer) hc );
    return hc;
}

int child_startv(Widget top, int cmd_m, void (*cb)(int) )
{
    int l = m_len(cmd_m);
    int i;
    int hc = child_init(STR(cmd_m,0), 0);
    struct child_stat *child = mls(CLIST, hc);
    child->cb = cb;
    for(i=1;i<l;i++) v_kset(child->args, STR(cmd_m, i), -1 );
    child_exec(hc);
    child->id = XtAppAddInput(XtWidgetToApplicationContext(top),
                              child_read_fd(hc),
                              (XtPointer)  (XtInputReadMask),
                              child_input_cb,(XtPointer) hc );
    return hc;
}

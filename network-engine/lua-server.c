#include "slop-server.h"
#include "slopnet.h"
#include "command-parser.h"
#include "mls.h"
#include "luaexec.h"
#include "communication.h"
#include "mcucom.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SOCKET_PORT "1234"
#define MCU_SOCKET_PORT "1235"


int CHILD_PID;

int waiting_for_child(void)
{
    int status;
    int err = waitpid(-1,&status, 0 ); /* WNOHANG */
    if( err == 0 ) return 0;
    if( err == -1 ) {
        printf("error in waitpid\n");
        return 3;
    }
    if (WIFEXITED(status)) {
        printf("Child exited, status=%d\n", WEXITSTATUS(status));
        if( WEXITSTATUS(status) ) return 2;
        return 1;
    }
    return 0;
}

void new_client(int fd, int mcu1)
{
    int pid;
    if( (pid=fork()) < 0) {
	ERR("fork");
    } else if( pid == 0 ) {
	luaexec_main(fd, mcu1);
    }
    close(fd);
    TRACE(1,"SERVER STOPED UNTIL CHILD HAS EXITED");
    waiting_for_child();     /* wait for child to exit */
    TRACE(1,"PARENT CONTINUE");
}


int SERVER_SELECT_EXIT = 0;

static void server_wait(int listener, int mcu1)
{
    fd_set master;
    fd_set tmp_rd_fds;
    int fdmax;     /* maximum file descriptor number plus one */
    int newfd;     /* newly accept()ed socket descriptor */
    int ret;
    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&tmp_rd_fds);

    /* add the listener to the master set */
    FD_SET(listener, &master);

    /* keep track of the biggest file descriptor */
    fdmax = listener +1; /* so far, it's this one*/
     
    for(;  !  SERVER_SELECT_EXIT ; )
    {
	tmp_rd_fds = master;
	ret = select(fdmax, &tmp_rd_fds, NULL, NULL, NULL);
	if( ret == -1) {
	    if( errno == EINTR || errno == EAGAIN ) continue; 
	    ERR("select()");
	}

	if( ret == 0 ) continue;
	TRACE(1,"select returns: %d", ret );
	newfd = sock_accept_incomming_connection(listener);
	if( newfd < 0 ) {
	    WARN("accept error");
	} else {
	    TRACE(1,"accept new conn");
	    new_client(newfd, mcu1);
	}
    }
}

int main(int argc, char **argv)
{
    char * port = SOCKET_PORT; 
    m_init();
    trace_level=1;

    int sfd = sock_listen_on_port(port);
    if( sfd < 0 ) ERR("could not bind to %s", port );
    TRACE(1,"server started.Port: %s", SOCKET_PORT );
    sock_make_socket_blocking(sfd);

    int mcu1 = mcu_init("localhost", "1235" );
    int mcufd = mcu_connect(mcu1);
    if( mcufd >= 0 ) 
	sock_make_socket_blocking(mcufd);
    else WARN("mcu not found");
    
    server_wait(sfd,mcu1);


    
    close(sfd);
    m_destruct();
    return EXIT_SUCCESS;
}

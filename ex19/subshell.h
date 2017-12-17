#ifndef SUBSHELL_H
#define SUBSHELL_H

#include <sys/wait.h>
#include "mrb.h"

enum fork2_proc_state {
    CHILD_NOT_INIT,
    CHILD_RUNNING,
    CHILD_EXIT_SUCCESS,
    CHILD_EXIT_FAILURE
};

enum fork2_pipes {
    CHILD_STDOUT_RD=0,
    CHILD_STDERR_RD=4,
    CHILD_STDIN_WR=3
};


struct fork2_info {
    int fd[6];
    pid_t pid;
    int exit_value;
    int flags;
    struct mrb *pipe_buf[2];
    enum fork2_proc_state stat;
    int error;
    int scan_pos[2];
};

extern int trace_child;

struct fork2_info *fork2_open(char *filename,...);
void fork2_close( struct fork2_info *child );
int fork2_getline( struct fork2_info *child, int pipe, int lnbuf );
int fork2_read(struct fork2_info *child, int pipe );
int fork2_getchar(struct fork2_info *child, int pipe );
int fork2_write(struct fork2_info *child, char *s );


#endif

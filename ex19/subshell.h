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

struct fork2_info {
    int fd[6];
    pid_t pid;
    int exit_value;
    int flags;
    struct mrb *buf;
    enum fork2_proc_state stat;
    int error;
    int scan_pos;
};

extern int trace_child;

struct fork2_info *fork2_open(char *filename,...);
void fork2_close( struct fork2_info *child );
int fork2_getline( struct fork2_info *child, int lnbuf );
int fork2_read(struct fork2_info *child );


#endif

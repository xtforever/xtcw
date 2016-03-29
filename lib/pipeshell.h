#ifndef PIPESHELL_H
#define PIPESHELL_H

#include <X11/Intrinsic.h>


enum child_proc_state {
    CHILD_NOT_INIT,
    CHILD_RUNNING,
    CHILD_EXIT_SUCCESS,
    CHILD_EXIT_FAILURE
};

enum child_err {
    CHILD_ERR_NO_ERROR,
    CHILD_ERR_PROCESS_NOT_FOUND,
    CHILD_ERR_PIPE_READ
};


int child_start(Widget top, char *cmd, void (*cb)(int) );
int child_startv(Widget top, int cmd_m, void (*cb)(int) );
enum child_proc_state child_status(int hc);
int child_exit_status(int hc);
char* child_data(int hc);
void child_close( int hc );
void child_destruct(void);
int child_args(int hc);

extern int trace_child;

#endif

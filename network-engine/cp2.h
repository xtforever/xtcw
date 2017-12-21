#ifndef COMMAND_PARSER2_H
#define COMMAND_PARSER2_H

#include "mls.h"


typedef int (*cp2_func_t) (void *ctx, void *arg);

struct cp2_data_st {
    char *name;
    cp2_func_t func;
};

struct cp2_context {
    void *ctx;
    int cmd_m;
};

int  cp2_init(void* ctx);
void cp2_free(int n);
void cp2_destruct(void);
int  cp2_add(int n, char *name, void *p );
cp2_func_t cp2_lookup(int n, int str_m);

#endif

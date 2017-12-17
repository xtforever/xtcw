#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include "mls.h"


typedef int (*cp_func_t) (void *arg, void *ctx);

struct cp_data_st {
    char *name;
    cp_func_t func;
};

void cp_init(void);
void cp_destruct(void);
void cp_add( char *name, void *p );
cp_func_t cp_lookup(int mstr);

#endif

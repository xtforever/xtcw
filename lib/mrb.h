#ifndef MRB
#define MRB

#ifdef USE_CONFIG_H
#include "config.h"
#endif

#ifndef MRB_BUFSIZE
#define MRB_BUFSIZE 1024
#endif

#include <stdio.h>
#include <unistd.h>
#include "mls.h"

struct mrb {
    int fd;
    int rd,wr;
    int size;
    unsigned char buf[0];
};

int mrb_bytesused(struct mrb *b);
int mrb_peek(struct mrb *b, int *n);
int mrb_get(struct mrb *b);
int mrb_bufsize(struct mrb *b);
char *mrb_read_chunk(struct mrb *b, int *n);
struct mrb* mrb_create(int size);
void *mrb_maxsize(struct mrb *b, int *bytes );
void *mrb_alloc(struct mrb *b, int bytes );
void mrb_free( struct mrb *b, int bytes );
void mrb_destroy(struct mrb *m);

#endif

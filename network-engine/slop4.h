#ifndef SLOP4_H
#define SLOP4_H

#include "mls.h"

typedef void (*slop_callback_t) ( int error,int msg, void *ctx);

struct slop_state {
    int  state;

    int msg;
    int crc;
    int cs;
    int error;


    slop_callback_t cb;
    void *ctx;
};


void slop_init( struct slop_state *slop );
void slop_free( struct slop_state *slop );

void slop_put( struct slop_state *slop, int ch );
void slop_encode_msg(int m,
		     const char *p, int len, int with_crc );


int slop_encode(int buf, int crc, int ch);
int slop_encode_str( int buf, int crc, char *s );
#endif

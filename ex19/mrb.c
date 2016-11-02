#include "mrb.h"

/* RINGBUFFER
   rd=-1      : Buffer empty, wr=0
   rd == wr   : Buffer full
*/

inline static char buf_empty(struct mrb *b) { return (b->rd < 0); }
inline static char buf_full(struct mrb *b) { return (b->rd == b->wr); }
inline static void read_inc(struct mrb *b, int n)
{
    b->rd = (b->rd + n) % b->size;
    if( b->rd == b->wr ) {
        b->rd = -1; b->wr=0;
    }
}
inline static void write_inc(struct mrb *b, int n)
{
    b->wr = (b->wr + n) % b->size;
}
inline static int  write_size_max(struct mrb *b)
{
    return ( b->rd < b->wr ) ? (b->size - b->wr) : (b->rd - b->wr);
}
inline static int  read_size_max(struct mrb *b)
{
    return ( b->rd >= b->wr ) ? (b->size - b->rd) : (b->wr - b->rd);
}


int mrb_bufsize(struct mrb *b)
{
    return b->size;
}

/** zeichen ab pos *n zurückgeben und *n um 1 erhöhen
    ist *n zu größ oder negativ wird *n auf die anzahl der
    zeichen im puffer gesetzt und -1 zurückgegeben
    returns: zeichen ab pos *n, oder -1
*/
int mrb_peek(struct mrb *b, int *n)
{
    int used;
    if( !n ) return -1;
    used = mrb_bytesused(b);
    if( *n < 0 || *n >= used ) { *n = used; return -1; }

    int p = (b->rd + *n) % b->size;
    *n = (*n) +1;
    return b->buf[p];
}

int mrb_get(struct mrb *b)
{
    if( buf_empty(b) ) return -1;
    int ch = b->buf[b->rd];
    read_inc(b,1);
    return ch;
}

/** get ptr to *n consecutive allocated bytes
*/
char *mrb_read_chunk(struct mrb *b, int *n)
{
    int size;
    if( !n || !(*n)) return NULL;
    if( buf_empty(b) ) { *n = 0; return NULL; }

    size = read_size_max(b);
    if( *n > size ) *n = size;

    char *buf = (char*) b->buf + b->rd;
    read_inc(b,*n);
    return buf;
}

struct mrb* mrb_create(int size)
{
    struct mrb *m = calloc( size + (sizeof(struct mrb)),1  );
    if( !m ) ERR("out of memory");
    m->rd = -1;
    m->size = size;
    return m;
}

/** benutzte bytes im speicher */
int mrb_bytesused(struct mrb *b)
{
    if( buf_empty(b) ) return 0;
    if( b->rd < b->wr ) return (b->wr - b->rd);
    return (b->size - b->rd) + b->wr;
}

/** größter zusammenhängender freier speicherbereich ermitteln */
void *mrb_maxsize(struct mrb *b, int *bytes )
{
    *bytes = write_size_max(b);
    return b->buf + b->wr;
}

/** zusammenhängende bytes im puffer belegen */
void *mrb_alloc(struct mrb *b, int bytes )
{
    char *buf = (char *)b->buf + b->wr;
    if( bytes == 0 ) return buf;
    if( b->rd < 0 ) b->rd = 0;
    b->wr += bytes;
    if( b->wr > b->size ) ERR("buffer overflow");
    if( b->wr == b->size ) b->wr = 0;
    return buf;
};

/** anzahl (bytes) speicherplätze freigeben */
void mrb_free( struct mrb *b, int bytes )
{
    int n = mrb_bytesused(b) - bytes;
    if( n < 0 ) ERR("free more than alloced");

    if( n == 0 ) {
        b->rd = -1;
        b->wr = 0;
        return;
    }

    read_inc(b,bytes);
}

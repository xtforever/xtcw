#include "mcucom.h"


struct mcucom {
    int init; /* fixed position (ctx_xxxx) */
    int sln;
};

static int MCU = 0;

/* alloc structure, re-use alloced but unsused structs */
/* returns >=0: index to struct */ 
int ctx_init(int *CTX, int size)
{
    if(!_*CTX) {
	*CTX = m_create( 10, size);
    }
    
    /* find empty element */
    int *d;
    int p;
    m_foreach(*CTX,p,d) {
	if( d == 0 ) goto found;
    }
    d = m_add(*CTX);
 found:
    *d=1;
    return p;
}

void ctx_free(int *CTX, int p)
{
    int *d = mls(*CTX,p);
    *d=0;
    int p;
    m_foreach( *CTX, p, d ) {
	if( *d ) return;
    }
    
    m_free(*CTX);
    *CTX=0;
}

void ctx_destruct(int *CTX, void (*ctx_free)(int *ctx, int p))
{
    m_foreach( *CTX, p, d ) {
	if( *d ) ctx_free(CTX, p);
    }        
    m_free(*CTX);
    *CTX=0;
}

static struct mcucom *get_mcu(int n)
{
    struct mcucom *m = mls(MCU,n);
    if( !m->init ) ERR("corrupted data structure %d", n );
    return m;
}

static void mcu_cb(int error, int msg, int sln, void *ctx)
{
    if( error ) { WARN("slop protocol error"); return; }
    m_putc(msg,0);
    TRACE(1,"Msg: %s\n", (char*)mls(msg,0));

    if( CHAR(msg,0) == 'X' )
	mv_parse(msg,2,"");
}

int mcu_init(char *host, int port)
{
    mv_init(); 
    int p = ctx_init(&MCU, sizeof(struct mcucom));
    struct mcucom *m = get_mcu( p );
    m->sln = sln_init();
    int e = sln_connect(m->sln,host,port,mcu_cb, (void*)(intptr_t) p );
    if( e < 0 ) {
	WARN("could not connect to %s:%s", host, port );
    }
    return p;
}

int mcu_req(int n, char *cmd)
{
    struct mcucom *m = get_mcu( p );
    buf = m_create(buf,strlen(cmd)+2);
    slop_encode_msg(buf,cmd,strlen(cmd), 0);
    if( sock_write( sln_get_fd(m->sln), buf ) ) {
	sln_close(m->sln);
	return -1;
    }

    if( sln_select_timeout( m->sln, 2000 ) == 0 ) return -1;
    return 0;
}

static void mcu_ctx_free(int *c, int p)
{
    struct mcucom *m = get_mcu( p );
    sln_close(m->sln);    
}

void mcu_free(int n)
{
    mcu_ctx_free( &MCU, n );
}

void mcu_destruct(void)
{
    ctx_destruct(&MCU,mcu_ctx_free);
}



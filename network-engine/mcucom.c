#include "mcucom.h"
#include "mls.h"
#include "micro_vars.h"
#include "slopnet.h"
#include "slop4.h"

struct mcucom {
    int init; /* fixed position (ctx_xxxx) */
    int sln;
    char *host, *port;
    int msg;
    int err;
};

static int MCU = 0;

/* alloc structure, re-use alloced but unsused structs */
/* returns >=0: index to struct */ 
int ctx_init(int *CTX, int size)
{
    if(! *CTX) {
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

void ctx_free(int *CTX, int p, void (*free_cb)(int *ctx, int p))
{
    free_cb(CTX,p);         /* user data free */
    int *d = mls(*CTX,p);   
    *d=0;                   /* internal data free */

    /* clear CTX if no struct is left */
    m_foreach( *CTX, p, d ) {
	if( *d ) return;
    }
    m_free(*CTX);
    *CTX=0;
}

void ctx_destruct(int *CTX, void (*free_cb)(int *ctx, int p))
{
    int p;
    int *d;
    m_foreach( *CTX, p, d ) {
	if( *d )free_cb(CTX, p);
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
    int p;
    if( error ) { WARN("slop protocol error"); return; }
    m_putc(msg,0);
    TRACE(1,"Msg: %s\n", (char*)mls(msg,0));

    p=2;
    if( CHAR(msg,0) == 'X' )
	mv_parse(msg,&p,"");

    p = (intptr_t)ctx;
    struct mcucom *m = get_mcu( p );
    m_write( m->msg, 0, m_buf(msg), m_len(msg) );
    
}

int mcu_init(char *host, char * port)
{
    mv_init(); 
    int p = ctx_init(&MCU, sizeof(struct mcucom));
    struct mcucom *m = get_mcu( p );
    m->sln = sln_init();
    m->host = strdup(host);
    m->port = strdup(port);
    m->msg = m_create(100,1);
    m->err = 0;
    return p;
}

int mcu_error(int p)
{
    struct mcucom *m = get_mcu( p );
    return m->err;
}

int mcu_get_sln(int p)
{
    struct mcucom *m = get_mcu( p );
    return m->sln;
}
int mcu_get_msg(int p)
{
    struct mcucom *m = get_mcu( p );
    return m->msg;
}

/* returns fd for new connection, -1 on error */
int mcu_connect(int p)
{
    struct mcucom *m = get_mcu( p );

    if( m->sln >= 0 ) sln_free(m->sln);
    m->sln = sln_init();
    int e = sln_connect(m->sln,m->host,m->port,mcu_cb, (void*)(intptr_t) p );
    m->err = e < 0;
    if( m->err ) {
	WARN("could not connect to %s:%s", m->host, m->port );
    }    
    return e;
}


int mcu_req(int n, const char *cmd)
{
    int e;
    struct mcucom *m = get_mcu( n );
    int buf = m_create(strlen(cmd)+2, 1);
    slop_encode_msg(buf,cmd,strlen(cmd), 0);
    if( sock_write( sln_get_fd(m->sln), buf ) ) {
	TRACE(1,"sock write error");
	sln_free(m->sln);
	m->sln = -1;
	return m->err = 1;
    }
    e = sln_select_timeout( m->sln, 5000 );
    return m->err = (e != 1);
}

static void mcu_ctx_free(int *c, int p)
{
    struct mcucom *m = get_mcu( p );
    sln_free(m->sln); m->sln=-1;    
    free(m->host); free(m->port);
    m_free(m->msg); 
}

void mcu_free(int n)
{
    ctx_free( &MCU, n, mcu_ctx_free);
}

void mcu_destruct(void)
{
    ctx_destruct(&MCU,mcu_ctx_free);
}



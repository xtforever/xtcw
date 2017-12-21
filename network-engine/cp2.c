/* commmand parser
   1. fetch command name
   2. find function ptr
   3. execute function
*/
#include "cp2.h"
#include "mls.h"

static int CP_DATA;


/* returns >=0: index to cp2 struct */ 
int cp2_init(void* ctx)
{
    if(!CP_DATA) {
	CP_DATA = m_create( 10, sizeof(struct cp2_context));
    }
    
    struct cp2_context *cp2;
    int p;
    m_foreach(CP_DATA,p,cp2) {
	if( cp2->cmd_m <= 0 ) goto found;
    }
    cp2 = m_add(CP_DATA);
 found:
    cp2->cmd_m = m_create(10, sizeof(struct cp2_data_st));
    cp2->ctx = ctx;
    return p;
}


static struct cp2_context *cp2_get(int n)
{
    return mls(CP_DATA,n);
}
 

void cp2_destruct(void)
{
    struct cp2_context *d;
    int p;
    m_foreach(CP_DATA,p,d) {
	m_free_strings(d->cmd_m, 0);
    }
    
    m_free(CP_DATA);
    CP_DATA=0;
}


void cp2_free(int n)
{
    struct cp2_context *cp = cp2_get(n);
    m_free_strings(cp->cmd_m, 0);
    cp->cmd_m = 0;

    /* if there is no context left, clean memory */
    int p;
    m_foreach(CP_DATA,p,cp) {
	if( cp->cmd_m > 0 ) return;
    }
    cp2_destruct();
}


/* returns number of added command */
int cp2_add(int n, char *name, void *func)
{
    
    struct cp2_context *cp = cp2_get(n);
    
    int p = m_lookup_str(cp->cmd_m, name, 0 );
    struct cp2_data_st *d = mls(cp->cmd_m, p);
    d->func = func;
    return p;
}



static int cmp( char *s, int m )
{
    char ch;
    int len = strlen(s);
    if( len > m_len(m) ) return 1;
    
    while( len-- ) {
	ch = CHAR(m,len);
	if( ch != s[len] ) return 1;
    }
    return 0;
}

static int lookup(int m, int key)
{
    char **s; int p;
    m_foreach( m, p, s ) {
	if( cmp(*s,key) == 0 ) return p;
    }
    return -1;
}

/* suche nach str_m in der befehls-liste */
cp2_func_t cp2_lookup(int n, int str_m)
{
    struct cp2_context *cp = cp2_get(n);

    
    int p = lookup( cp->cmd_m, str_m );
    if( p < 0) return 0;
    struct cp2_data_st *d = mls(cp->cmd_m, p);
    return d->func;
}


/* commmand parser
   1. fetch command name
   2. find function ptr
   3. execute function
*/
#include "command-parser.h"
#include "mls.h"

static int CP_DATA;

static int cp_error( void *client)
{
    TRACE(1,"unknown command" );
    return 1;
}


void cp_init(void)
{
    CP_DATA = m_create( 10, sizeof(struct cp_data_st));
}

void cp_destroy(void)
{
    m_free_strings(CP_DATA, 0);
}

void cp_add( char *name, void *func)
{
    int p = m_lookup_str(CP_DATA, name, 0 );
    struct cp_data_st *cp = mls(CP_DATA, p);
    cp->func = func;
}



static int cmp( char *s, int m )
{
    int p;
    char *d;
    m_foreach( m, p, d ) {
        if( *s==0 ) return 0; /* match */
        if( *s != *d ) break; /* no match */
        s++;
    }
    return 1; /* no match */
}

 static int lookup(int m, int key)
 {
     char **s; int p;
     m_foreach( m, p, s ) {
         if( cmp(*s,key) == 0 ) return p;
     }
     return -1;
 }

cp_func_t cp_lookup(int m)
{
    int p = lookup(CP_DATA, m);
    if( p < 0) return cp_error;
    struct cp_data_st *cp = mls(CP_DATA, p);
    return cp->func;
}

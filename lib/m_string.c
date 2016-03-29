#include "m_string.h"

/*
28.05.2014 jh erstellung
 */

/* returns: length of string */  
int s_strlen(int m)
{
    int p = m_len(m);
    return p && CHAR(m,p-1)==0 ? p-1 : p;
}

/* returns: new string handle */
int s_new(void)
{
    int m= m_create(10,1);
    m_putc(m,0);
    return m;
}


void s_free(int m)
{
    m_free(m);
}


static int s_app1( int m, char *s )
{
    int p = s_strlen(m);
    m_write( m, p, s, strlen(s)+1 );
    return m;
}

static int vas_app(int m, va_list ap)
{
    char *name;
    while( (name = va_arg( ap, char* )) != NULL )
    {
	s_app1( m, name );
    }
    return m;
}

/** anhängen der char* strings an |m| */
int s_app(int m, ...)
{
    va_list ap;
    if(!m) m=s_new();
    va_start(ap,m);
    vas_app(m,ap);
    va_end(ap);
    return m;
}


/** suche das zeichen |c| in |m| beginnend bei der position |p| 
    returns: -1 falls ch nicht gefunden wurde, sonst die position
    des zeichens ch im array m 
*/
int s_index(int m, int p, char ch)
{
    while( p < m_len(m) ) 
	if( ch == CHAR(m,p) ) return p; else p++;
    return -1;
}

static int match(int m, int *res, char *s, int *p )
{
    int t;
    char ch;
    if( *res >= m_len(m) ) return 1;
    ch= s[*p];
    if( !ch ) return 1;
    if( ch == '*' ) goto test_expr;
    if( ch == CHAR(m,*res) ) { (*res)++; (*p)++; return 0; }
    return 1;

 test_expr:
    (*p)++;
    ch = s[*p]; 
    if( ch == 0 ) { (*res) = m_len(m); return 0; } /* match rest */
    t = *res;
    /* suche ch ab |t| */
    while( t < m_len(m) && CHAR(m,t) !=  0 ) 
	{
	    if( CHAR(m, t) == ch ) { (*res) = t+1; (*p)++; return 0; } /* found prefix */
	    t++;
	}
    return 1; /* not found */
}

/* loesche das prefix |pref| aus |m| */
int s_prefix( int m, char *pref )
{
    if( !m ) m=s_new();
    int res = 0;
    int pos=0;
    while( match(m, &res, pref, &pos )==0 )
	if( ! pref[pos] ) {
	    m_remove( m, 0, res );
	    break;
	}
    return m;
}

/* kopieren des strings */
int s_dub(int m)
{
    int x=m_create( m_len(m),1 );
    m_write(x, 0, m_buf(m), m_len(m) );
    return x;
}

/* integer to str 
   place string at p into array m
   if p<0 append str to m
   if m == 0 create new str
*/
int s_itos(int m, int p, int val)
{
    int v;
    int len = 1;			// 0 
    if( val < 0 ) len++;		// -1
    v=val; while( v/=10 ) len++;	// -10... 
    if( m == 0 ) { 
	m=m_create(len,1); p=0; 
    }
    if( p < 0 || p >= m_len(m) ) { 
	p = m_len(m); 
	if( p > 0 ) p--; 
    }
    m_setlen( m, p + len +1 );
    snprintf( mls(m,p),len+1, "%d", val ); 
    return m;
}

/* string printf
   place string at p into array m
   if p<0 append str to m
   if m == 0 create new str
*/
int s_printf(int m, int p, char *format, ...)
{
    int len;
    va_list ap;

    va_start(ap,format);
    len=vsnprintf(0,0, format, ap );
    len++;
    if( m == 0 ) { 
	m=m_create(len,1); p=0; 
    }
    if( p < 0 || p > m_len(m) ) { 
	p = m_len(m); 
	if( p > 0 ) p--; // overwrite terminating zero 
    }
    m_setlen( m, p + len );
    vsnprintf( mls(m,p),len, format, ap );
    va_end(ap);
    return m;
}

int s_cp(int m, char *s)
{
    if( !m ) m = s_new(); else m_clear(m);
    m_write( m, 0, s, strlen(s)+1 );
    return m;
}

int s_regex(int res, int s, int regex )
{
    
}
 

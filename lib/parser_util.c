#include "parser_util.h"

int parser_remove_whitespace(int m, char *src)
{
  int i,len = is_empty(src) ? 1 : strlen(src) +1;
  if( !m ) m=m_create(len,1); else m_clear(m);
  
  i=0; while( i< len-1 && isspace(src[i])) i++;
  while( i < len-1 && ! isspace( src[i] )) 
    m_putc( m, src[i++] ); 
  m_putc(m,0); 
  return m;
}

int parser_next_char( int buf, int *p )
{
  int c;
  if( *p >= m_len(buf) ) return -1;
  c = CHAR(buf,*p);
  if( c == 0 ) return -1;
  (*p)++;
  return c;
}

int parser_skip_whitespace(int buf, int *p)
{
  int c;
  while( *p < m_len(buf) ) {
    c = CHAR(buf,*p);
    if( c == 0 ) return -1;
    if( ! isspace(c) ) return 0;
    (*p)++;
  }
  return -1;
}

int parser_int(int buf, int *p, int *ret )
{
  int sign = 1;
  int c;

    *ret = 0;
    if( parser_skip_whitespace(buf,p) ) return -1;
    
    c = CHAR(buf,*p);
    if( c == '-' ) { sign=-1; (*p)++; c = CHAR(buf,*p); }
    else if( ! isdigit(c) ) return -1;
    do {
	(*ret) = (*ret)*10 + c - '0';
	(*p)++; 
	if( (*p) >= m_len(buf) ) break;
	c = CHAR(buf,*p); 
    } while( isdigit(c) );
    (*ret) *= sign;
    return 0;
}

/** suche und überspringe skip_char, überspringe führende leerzeichen
 */
int parser_skip(int buf, int *p, int skip_char )
{
  int c;
  if( parser_skip_whitespace(buf,p) ) return -1;
  c = CHAR(buf,*p);
  if( c == skip_char ) { (*p)++; return 0; }
  return -1;
}

/* kein gleichzeichen und kein space erlaubt */
int parser_id(int buf, int *p, int ret_id )
{
  int c;
  if( ret_id<=0 ) ret_id = m_create(10,1); else m_clear(ret_id);
  if( parser_skip_whitespace(buf,p) ) return -1;
  do {
    c = CHAR(buf,*p);
    if( ! isgraph(c) || c == '=' ) break;
    m_putc( ret_id, c );
    (*p)++;
  } while(1);
  m_putc(ret_id,0);
  return ( m_len(ret_id) <= 1 );
}


int parser_strdup(int buf, int *p, char **s )
{
    int e = m_len(buf);
    if( (*p) >= e ) return -1;
    if( e == 1 && CHAR(buf,0) == 0 ) return -1; /* empty string */ 
    if( CHAR(buf,e-1) != 0 ) m_putc(buf,0);
    *s=strdup( mls(buf,*p) );
    return 0;
}

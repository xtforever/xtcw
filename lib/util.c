#include <ctype.h>
#include "util.h"
#include "ini_read2.h"
#include "mls.h"



#include <time.h>

ulong get_time_ms(void)
{
    struct timespec monotime;
    clock_gettime(CLOCK_MONOTONIC, &monotime);
    return monotime.tv_sec * 1000 + monotime.tv_nsec / 1000000;     
}

/* wandelt den string s0 in eine ganzzahl mit vorzzeichen
   führende leerzeichen werden ignoriert
*/
int getInt( char **s0, int *p )
{
    char *s = *s0;
    int sign, number=0;
    while( isspace( *s )) s++;
    if( *s == '-' ) { sign=-1; s++; } else sign=1;
    if( *s < '0' || *s > '9' ) { *s0=s; return -1; } 
    while( *s >='0' && *s <= '9' )
	number = (number * 10) + *(s++) - '0';
    number *= sign;
    *s0=s;
    *p = number;
    return 0;
}

int rc_get_int(char*chap, char *key, int *valp)
{
    int val;
    char *s = rc_key_value(chap,key);
    if( getInt(&s,&val) ) return -1;
    if( valp ) *valp = val;
    return 0;
}

int rc_key_int(char*chap, char *key)
{
    int val;
    if( rc_get_int(chap,key,&val) == 0 ) return val;
    return -1;
}

char *rc_key_str(char*chap, char *key)
{
   return rc_key_value(chap,key);
}

int skip_ws(int line, int *p)
{
  while( *p < m_len(line) )
    {
      if(! isspace( CHAR(line,*p)) ) return 0;
      (*p)++;
    }
  return -1;
}

/** suche den beginn des nächsten wortes 
 */
int next_id( int line, int *p)
{
  if( skip_ws(line,p) ) return -1;
  while( *p < m_len(line) )
    {
      if( isspace( CHAR(line,*p)) ) return skip_ws(line,p);
      (*p)++;
    }
  return -1;
}

/** kopiert ein wort von line nach buf
    führende leerzeichen werden übersprungen 
    trenn-zeichen sind: ",", "=", ":"
    returns: das trenn-zeichen oder -1 falls kein wort gelesen wurde
*/
int cut_id(int line, int *p, int buf )
{
  int ch = -1;
  m_clear(buf);
  if( skip_ws(line,p) < 0 ) return -1;
  while( *p < m_len(line) )
    {
	ch = CHAR(line,*p); (*p)++;
	if(! isspace(ch) && ch != ',' && ch!='=' 
	   && ch != ':' ) m_putc( buf,ch); else break;
    }
  m_putc(buf,0);
  if( m_len(buf) < 2 ) return -1;
  return ch;
}


/** speichert eine zuweisung ID=num1,num2,..,num-x in einem varset
 */
int parse_variable_assignment_int(int vset, int line, int *p )
{
    int key, ch;
    int varname = m_create(20,1);

    while(1) {

	if( cut_id( line, p, varname ) < 0 ) break; /* no more vars */
	key = v_lookup(vset, MSTR(varname) ); 
        if( key < 0 ) break;
	v_kclr(key);
	do {
	    if( (ch=cut_id( line, p, varname )) < 0 ) goto leave_err;
	    v_kset(key, MSTR(varname), -1 ); 
	} while( ch == ',' );

    }
    m_free(varname);    
    return 0;

 leave_err:
    m_free(varname);
    return -1;
}

void copy_variable( int vset, char *dst, char *src )
{
    int key_target = v_lookup(vset, dst );
    int key_source = v_lookup(vset, src );

    int i = 0;
    char **strp,*s;
    while( m_next( key_source, &i, &strp ) ) {
	s = *strp;
	v_kset( key_target, s, i );
    }
}



/** vergleiche die ersten n zeichen des arrays m und des strings s 
 */
int m_isequal( int m, const char *s, int n)
{
    if( m > 0 && m_len(m) >= n && s && *s ) {
	return ( strncmp( s, (char*)mls(m,0), n ) == 0 );
    }
    return 0;
}

int find_str(char **list, int n, char *key)
{
    while(n-- > 0) 
	if( strcasecmp( list[n], key ) == 0 ) break;
    return n;
}


int strlist_line_width( char *p )
{
    char *s=p;
    while( *s && *s != '\n' ) s++;
    return s-p;
}

int isword(char a)
{
    return ( a!=0 && !isspace(a));
}

int strlist_cut_word( char *p, int *a, int *b )
{
    char *s=p;
    while( isspace(*s) ) s++;
    if( ! *s ) return -1;
    *a=s-p;
    while( isword(*s) ) s++;
    *b=s-p;
    return 0;
}


/* text - segmented form */
void tseg_init(tseg_text_t *ts)
{
    ts->text   = m_create(50,1);
    ts->m_seg  = m_create(5,sizeof(text_segment_t));
    ts->m_attr = m_create(5,sizeof(text_attribute_t));
}
void tseg_free(tseg_text_t *ts)
{
    m_free(ts->text);
    m_free(ts->m_seg);
    m_free(ts->m_attr);
}

static int tseg_map_attr( int ma, char ch )
{
    int i;
    text_attribute_t *d;
    m_foreach( ma, i, d ) {
	if( d->name == ch ) return i; 
    }
    return -1; /* unknown */
}

static int tseg_get_format(int ma, char *s, int *a, int *b)
{
    int code,x;
    
    code = s[*a]; 
    switch( code ) {
    case 'c': x = T_CENTER; break;
    default: return 0;
    }
    
    (*a)++; 
    return x;
}
    
    

static int tseg_get_attr(int ma, char *s, int *a, int *b)
{
    int code,x;
    
    if( *a > *b ) return -1;
    code = s[*a]; 
    (*a)++; if( *a > *b ) return -1;
    x = tseg_map_attr(  ma, code );
    if( *a > *b ) return x;
    code = s[*a]; 
    if( code == ':' ) (*a)++;
    return x;
}

void tseg_set_text(tseg_text_t *ts, char *string )
{
    m_clear( ts->text );
    m_write( ts->text, 0, string, strlen(string)+1 );
    int ms = ts->m_seg;
    int ma = ts->m_attr;
    text_segment_t seg;
    char *s = string;
    int a,b,x,attr = 0;
    while( isspace(*s) ) s++;
    m_clear( ms );
    while(1) {
	memset(&seg,0,sizeof seg);
	if( strlist_cut_word(s,&a,&b)) break;
	if( b-a <= 0 ) break; 
	if( s[b] == '\n' ) seg.format = T_HARD_BREAK;
	
	if( s[a] == '@' ) /* attribute prefix found */
	    {
		a++;
		if( s[a] != '@' ) {
		    if( (x = tseg_get_format(ma,s,&a,&b))) 
			seg.format |= x;
		
		    if( (x=tseg_get_attr(ma, s, &a, &b )) >= 0 ) 
			attr = x;
		}
	    }
	seg.start = (s-string)+a;
	seg.end   = (s-string)+b-1;
	seg.attribute = attr; 
	s+=b;
	m_put( ms, &seg );
    }
}

FILE *xopen_file( char *fn, char *mode )
{
    int buf = s_printf( 0,0, "%s/%s",  rc_key_value("files","datadir"), fn );

    FILE *fp= fopen( m_buf(buf), mode );
    if( !fp ) WARN("cannot access File %s", fn); 
    m_free(buf);
    return fp;
}

FILE *xopen_write(char *fn)
{
    
    return xopen_file( fn,"w" );
}

FILE *xopen_read(char *fn)
{
    return xopen_file(fn, "r" );
}


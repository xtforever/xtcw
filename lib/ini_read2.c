/*
  beispiel fuer:

  getline
  m_fscan2
  
  und ini file reader 


 */


#include "ini_read2.h"
#include "mls.h"
#include "ini_parse.tab.h"
#include "ini_lex.lex.h"
#include <limits.h>
#include <stdlib.h>

int yyparse(void);

void vl_dump(int opts);

struct RC_DB {
  int db;
  int cur_chap;
  int cur_var;
} RC;

typedef struct rc_ent_st {
  char *name;
  int chap;
} rc_ent_t;

void rc_init( char *filename )
{
  FILE *fp;
  if( RC.db > 0 ) {
    WARN("Warning RC init called twice");
    return;
  }
  memset( &RC,0,sizeof RC);
  RC.db = m_create(5,sizeof(rc_ent_t));
  ASSERT( (fp=fopen( filename, "r" )));
  yyin=fp;
  yyparse();
  fclose(fp);
}

void rc_add_chapter( char *s )
{
  rc_ent_t ent;
  ent.name = strdup(s);
  RC.cur_chap = ent.chap = v_init();
  m_put( RC.db, &ent );
}
void rc_var_add( char *s )
{
  RC.cur_var = v_lookup( RC.cur_chap, s ); 
}
void rc_var_set( char *s1, char *s2 )
{
  char *buf;
  asprintf( &buf, "%s&%s", s1,s2 );
  m_put( RC.cur_var, &buf );
}
void rc_var_define(char *name, char *s1)
{
  v_set( RC.cur_chap, name, s1, -1 );
}

void rc_free(void)
{
  int m = RC.db;
  int p;
  rc_ent_t *d;

  m_foreach( m,p,d ) {
    v_free( d->chap );
    free( d->name );
  };
  m_free( m );
}

void rc_dump(void)
{
  int m = RC.db;
  int p;
  rc_ent_t *d;

  m_foreach( m,p,d ) {
    puts( d->name );
    vl_dump( d->chap );
  }
}

/**
   @returns: -1 end of file
   0 leere zeile gelesen
   1..n anzahl gelesener zeichen
**/
int m_getline( int buf, FILE *fp )
{
  if( m_len(buf) ) m_clear(buf);
  int l = m_fscan2( buf, 10, fp );
  return ( l == 10 ) ? m_len(buf)-1 : -1;
}


void vl_dump_single(int m)
{
  int p; char **d;
  int ch = ' ';
  m_foreach( m,p,d ) {
    printf( "%c[%s]", ch, *d );
    if( ch==32 ) ch='='; else if( ch=='=' ) ch=',';
  }
  putchar(10);
}
  
void vl_dump(int opts)
{
  int p,*d;
  
  m_foreach( opts, p,d ) {
    vl_dump_single( *d );

  }

}




/**
   @returns 
    0 - next chapter found
   -1 - eof
**/
/*
int ini_read( FILE *fp, int line, int opts )
{
  int ln=0;
  int sl=m_create(10, sizeof(char*));  // split line buffer
  ssize_t read;
  while ((read = m_getline(line, fp)) != -1) {
    ln++;
    if(! read || CHAR(line,0)=='#' ) continue; // ignoriere leerzeilen, comments
    if( CHAR(line,0) == '[' ) 

    char *s = mls(line,0);
    char *p = strchr( s, '=' );
    
    if( !p ) {
      multi_line_opts( fp, opts, s ); 
      // fprintf(stderr, "Error in Line: %d\n", ln );
      continue;
    }
    
    *p=0; // trennt name von argument liste
    
    char *first_char;
    do  {
      p++; first_char = p;
      p = strchr( first_char, ',' );
      if( p ) *p=0;
      v_set( opts, s, first_char, -1 ); 
    } while( p );
  
  }
  m_free_strings(sl,0);
  m_free(line);
  return opts;
}


*/

 
/** returns the first string assigned to key in chapter chap */ 
char *rc_key_value(char *chap, char *key )
{
    int k = rc_find_key(chap,key);
    if( k <= 0 ) return ""; 
    return STR(k,1);
}

int rc_find_key( char *chap, char *key )
{
  int p = m_lookup_str( RC.db, chap, 1 );
  if( p < 0 ) {
    TRACE(2,"chap %s not found in rc.db", chap );
    return -1;
  }
  rc_ent_t *ent = (rc_ent_t*) mls( RC.db, p );
  
  p= v_find_key( ent->chap, key );
  if( p < 0 ) {
    TRACE(2,"key %s in chap %s not found", key, chap );
    return -2;
  }
  return INT(ent->chap,p);
}

int rc_get_key( char *name, char *key )
{
  int p;
  
  p=rc_find_key(name,key);
  if( p<0) {
    p=rc_find_key("common",key);
  }
  return p;
}

/* PAIR: (name & value) */
/* suche nach (val) */
int rc_find_nearest_pair( int key, int val, int *pos, int *bufp, int *value )
{
    int d;
    int diff = INT_MAX;
    int tmp_pos=0;
    *pos=0;

    while( !rc_get_pair(key,*pos,bufp,value) )
	{
	    d = abs( *value - val );
	    if( d == 0 ) return 0;
	    if( d < diff ) { diff=d; tmp_pos = *pos; }
	    (*pos)++;
	}
    
    *pos = tmp_pos;
    rc_get_pair(key,*pos,bufp,value);
    return 1;
}

int rc_get_pair( int key, int p, int *bufp, int *value )
{
  int val, buf;
  char *s, *sep; 
  p++;
  if( p >= m_len(key) ) return -1;
  ASSERT(bufp);
  if( *bufp <=0 ) *bufp = m_create(10,1);
  buf=*bufp;
  s = STR(key,p);
  m_clear(buf);
  sep=strchr(s,'&');
  if( sep ) {
    m_write( buf, 0, s, sep - s ); m_putc(buf,0);
    val = atoi( sep+1 );
  }
  else {
    m_write( buf, 0, s, strlen(s)+1 ); 
    val = atoi( s );
  }
  
  TRACE(1, "%s = %d\n",(char*) mls(buf,0), val );
  if( value ) *value = val; 
  return 0;
}

void rc_print_key( char *name, char *key )
{
  int p,n,buf,val;
  
  p=rc_get_key(name,key);
  if( p<0) {
    ERR("key %s not found", key);
  }
  n = 0; buf=0;
  
  for(n=0; !rc_get_pair(p,n,&buf,&val); n++ )
    {    
      printf( "%s:%d ",(char*) mls(buf,0), val );
    }
  printf("\n");
  m_free(buf);
}




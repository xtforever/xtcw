#include "micro_vars.h"
#include "mls.h"
#include "parser_util.h"

static struct micro_vars MV;


void mv_init(void)
{
  static int first_time = 1;
  if( first_time ) {
    MV.map       = MapAg_New();
    MV.data      = m_create(100,sizeof(struct mv_data));
    m_new(MV.data,1); /* first element is reserved */
    first_time=0;
  }
}

int mv_var_lookup( int q )
{
  struct mv_data *ent;

  int p = (int) MapAg_Find( MV.map,  q,0,0 );
  if( !p ) {
    p = m_new(MV.data,1);
    MapAg_Define(MV.map, q,0,0, p  );
    ent = mls(MV.data, p);
    ent->signal = 0;
    ent->data = -1;
    ent->type =0;
  }
  return p;
}


int *mv_var( int q )
{
  int p = mv_var_lookup(q);
  return & ((struct mv_data*)mls(MV.data, p))->data;
}

int *mv_svar( char *s )
{
  return mv_var( XrmStringToQuark(s));
}

int mv_zerovar( char *s )
{
  int q = XrmStringToQuark(s);
  *mv_var(q)=0;
  return q;
}


void mv_onwrite( int q, void (*fn) (void*), void *d, int remove )
{
  struct mv_signal *sig;
  int p = mv_var_lookup(q);
  struct mv_data* ent = mls(MV.data, p);
  if(!ent->signal) ent->signal = m_create( 2, sizeof(struct mv_signal));

  /* finde signal und falls remove==TRUE entfernen */
  m_foreach( ent->signal, p, sig )
    {
        if( sig->fn == fn && sig->d == d ) {
            if( remove ) m_del(ent->signal,p);
            return;
        }
    }
  if( remove ) return;

  /* signal noch nicht vorhanden, remove==FALSE, jetzt hinzufügen */
  sig = mls( ent->signal, m_new(ent->signal,1));
  sig->d = d;
  sig->fn = fn;
}

/** write to var q and call signal handler
 **/
void mv_write( int q, int data )
{
  struct mv_signal *sig;

  int p = mv_var_lookup(q);
  struct mv_data* ent = mls(MV.data, p);
  ent->data = data;
  if(! ent->signal ) return;

  /* um rekursion zu verhindern wird signal blockiert */
  if( ent->locked ) return;
  ent->locked = 1;
  m_foreach( ent->signal, p, sig ) {
    if( sig->fn ) sig->fn( sig->d );
  }
  ent->locked = 0;
}

void mv_parse( int buf, int *p, char *group )
{
  int q_var, varname, id, d, len, start_parse;

  id = m_create(10,1);
  varname = s_printf( 0,0, group );
  len = s_strlen(varname);

  while(1) {
    start_parse = *p;
    if( parser_id( buf,p, id) ) goto parse_end;
    if( parser_skip( buf,p, '=' )) goto parse_end;;
    if( parser_int( buf, p, &d )) goto parse_end;
    m_write( varname, len, m_buf(id), m_len(id) );

    q_var = XrmStringToQuark( m_buf(varname) );
    mv_write( q_var, d );
  }

 parse_end:
  *p=start_parse;
  m_free(varname);
  m_free(id);
}


struct mv_variable {
  char *name;
  char *type;
  union { int d_int; float d_float; int *p_int; };
};

int mv_get_int( int Q )
{
  return 0;
}

int mv_set_int( int Q )
{
  return 0;
}

char *mv_get_str( int Q, int p)
{
  return 0;
}

int mv_set_str(int Q, int p, char *s )
{
  return 0;
}

int mv_read_rsrc( Widget w, char *res, int Q )
{
  struct mv_data *ent;
  int p = mv_var_lookup( Q );
  ent = mls(MV.data, p);
  if( strcmp( ent->type, XtRInt ) == 0 )
      XtVaGetValues(w, XtVaTypedArg, res, ent->type,
                    & ent->data, sizeof (int), NULL );
  else
      XtVaGetValues(w, XtVaTypedArg, res, ent->type,
                    m_buf( ent->data ), m_len(ent->data), NULL );
  return 0;
}

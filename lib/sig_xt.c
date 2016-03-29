#include "sig_xt.h"
#include "common_sigxt.h"

/***********************************************************

   Signal Handler for libXt

	signale werden ohne prüfung an alle registrierten
	handler weitergereicht.
	sobald ein handler ein signal exclusiv behalten
	will, gibt er einen wert ungleich null zurück

	wird ein signal registriert kann ein pointer "class"
	angegeben werden, der dem handler übergeben wird.
        
        wird ein signal gesetzt, wird ein pointer "data"
        an den handler weitergereicht.

 ***********************************************************/


// return 1: do not pass signal to other receivers


void sig_init(void)
{
  static int sig_init = 0;
  if( sig_init ) return;
  sig_init = 1;
  SIG.widgets = m_create(10,sizeof(int));
}

void sig_destruct(void)
{
  int p;
  int *recv_list;
  m_foreach( SIG.widgets, p, recv_list )
    {
      m_free(*recv_list);
    }
  m_free(SIG.widgets);
  SIG.widgets=0;
}

void sig_register( sig_receive_t *fn, Widget w, int type, void *class )
{
  int p,
    wids; // Widget[]
  struct SIG_XT_RECV recv, *d;

  if( type >= VIS_SIGNAL_MAX || type < 1 ) 
    ERR("unknown signal %d", type );

  if( type >= m_len(SIG.widgets) ) // resize signal array 
    m_setlen(SIG.widgets,type+1);

  wids = INT(SIG.widgets,type);
  if( wids == 0 ) // undefined signal, create new entry
    {
      wids = m_create(1,sizeof(struct SIG_XT_RECV));
      INT(SIG.widgets,type) = wids;
    }

  m_foreach( wids,p,d ) // check if widget is registered
    {
      if( d->wid == w ) {
	WARN("receiver widget %x for type %x allready registered",
	     (int) w, type );
	return;
      }
    }

  recv.wid = w; recv.fn = fn; recv.class = class;
  m_put( wids, &recv );
}

void sig_send( Widget from, int type, void *data )
{
  struct SIG_XT_RECV *d;
  int wids; // Widget[]
  int p;

  if( (type >= m_len( SIG.widgets)) || (wids=INT(SIG.widgets,type))<1 ) {
    WARN("no handler for signal %x registered", type );
    return;
  }

  TRACE(2,"signal: %x", type );

  m_foreach( wids,p,d )
    {
      if( d->wid != from  ) {
	if( d->fn( d->wid, from, type, d->class, data ) ) break;
      }
    }
}

#ifdef SIG_TEST
int sig_test_receive( Widget w, Widget from, int type, void *class, void *data )
{
  TRACE(2,"from:%x to:%x %s %s",(int)from, (int)w, (char*)class, (char*)data );
  return 0;
}

void sig_test(void)
{
  sig_init();

  sig_register( sig_test_receive, (void*)42, 1, "class1" );
  sig_register( sig_test_receive, (void*)42, 2, "class2" );
  sig_register( sig_test_receive, (void*)42, 3, "class3" );

  sig_send( (void*)1, 1, "user1" );
  sig_send( (void*)2, 2, "user2" );
  sig_send( (void*)3, 3, "user3" );

  sig_send((void*)4, 2, "sig2 from user4" );

  sig_destruct();
}
#endif

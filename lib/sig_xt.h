#ifndef SIG_XT
#define SIG_XT

/***********************************************************

   Signal Handler for libXt

   a widget registers its signal handlers with

	  sig_register( handler_proc, widget, type )



   a signal is emitted via

	  sig_send( sender_widget, type, data )



  signal_handler:
          if the widget receives a signal and do not wish 
          to pass the signal to others, it returns 1



 ***********************************************************/

#include <X11/Intrinsic.h>
#include "mls.h"

// return 1: do not pass signal to other receivers
typedef int sig_receive_t (Widget w, Widget from, int type, void *class, void *data );

struct SIG_XT {
  int widgets; // SIG_XT_RECV[][] 
} SIG;

struct SIG_XT_RECV {
  Widget wid;
  sig_receive_t *fn;
  void *class;
};

void sig_init(void);
void sig_destruct(void);
void sig_register( sig_receive_t *fn, Widget w, int type, void *class );
void sig_send( Widget from, int type, void *data );

#endif

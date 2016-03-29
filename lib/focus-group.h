#ifndef FOCUS_GROUP_H
#define FOCUS_GROUP_H

#include <X11/Intrinsic.h>
#include "mls.h"
#include "Wheel.h"
void focus_keyboard( Widget w, XEvent*ev, String *s, Cardinal* n);

void focus_add( Widget w );
void focus_add2( Widget w, char *group );
void focus_init(Widget top);
void focus_cmd(char *s);
void focus_destroy(void);
void focus_put( Widget w );
void focus_set();
void focus_set_cur_widget(Widget w);
void focus_push(Widget w);
void focus_highlight_cur_wid(void);
Widget focus_get_widget(void);
void focus_push_action(Widget prev, Widget w);
void focus_pop(void);
void focus_next(void);
void focus_prev(void);
void focus_exec(char *key);
void focus_key_event(Widget w, void* client, XEvent *ev, Boolean *cont);
void focus_numkeys_input_cb(Widget w, void *data, void *class );
void focus_push_group( char *group );

/* Inc,Dec,Fire - führende leerzeichen werden entfernt */
void focus_cb( Widget w, void *u, void *c );
#endif

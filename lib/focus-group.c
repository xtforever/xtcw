#include "focus-group.h"

char trace_focus = 0;

#define SWAP(a,b) do { int x = a; a=b; b=x; } while(0)

#ifndef TF
#define TF() TRACE(trace_focus,"")
#endif
/*.
 First Group      | second group
 w0, w1, w2,    0 |  w3 w4 w5 w6
*/

typedef struct FOW_ST
{
  int wid;
  int names;
  int old_wid, old_group;
  int cur_wid, cur_group;
  Widget input_widget;
  int keypad_input;
} fow_t;

#define FOCUS_TIMEOUT_SEC 6
static int focus_timeout = 0;
static int focus_timeout_widget = -1;
static fow_t FOW;
static XtTranslations focus_translations;
void focus_override_translations(Widget w);


void focus_call_action(int p, char *name)
{
  Widget *w;
  if( p >= 0 && p < m_len( FOW.wid) ) {
      w =(Widget*) mls( FOW.wid, FOW.cur_wid );
      XtCallActionProc( *w, name, 0,0,0 );
  } else WARN("needed widget position: %d", p );
}

void focus_call_reset(int p)
{
  TF();
  focus_call_action(p,"reset");
}
void focus_call_highlight(int p)
{
  TF();
  focus_call_action(p,"highlight");
}
void focus_highlight_cur_wid(void)
{
  TF();
  focus_call_highlight(FOW.cur_wid);
}
void focus_call_notify(int p)
{
  TF();
  focus_call_action(p,"notify");
}


/* 1. das widget an der position *pos* in der  focus liste wird zum
   aktuellen wird
   2. highlight wird aufgerufen
   3. falls es ein keypad input möchte wird es als keypad input widget gespeichert
*/
static void set_current_widget( int pos, int do_highlight )
{
  Widget w;
  FOW.cur_wid = pos;
  w =*(Widget*) mls( FOW.wid, FOW.cur_wid );
  if( do_highlight ) XtCallActionProc( w, "highlight", 0,0,0 );

  // check if widget wants keypad input support
  if( XtIsSubclass( w, wheelWidgetClass ) ) {
    if( wheel_exec_command(w, WHEEL_KEYPAD, 0)) {
      FOW.input_widget = w;
      FOW.keypad_input = 1;
    }
  }
}

/** ein sich im edit-mode (armed) befindliches widget
    auf "selected" zurücksetzen
*/
static void disarm_widget(void)
{
  int state;

  /* falls kein aktuelles widget gesetzt ist,
     oder der timer nicht fuer das
     aktuelle widget gestartet wurde -> ret
  */
  if( FOW.cur_wid < 0 ||   focus_timeout_widget != FOW.cur_wid )
    return;

  Widget w =*(Widget*) mls( FOW.wid, FOW.cur_wid );
  TRACE(trace_focus, "Test if Widget %s is armed", XtName(w));

  XtVaGetValues( w, "state", &state, NULL );
  if( state == STATE_ARMED ) {
    TRACE(trace_focus, "Widget %s is armed", XtName(w));
    XtCallActionProc( w, "reset", 0,0,0 );
    XtCallActionProc( w, "highlight", 0,0,0 );
  }
}

static void focus_start_timer(void)
{
  TRACE(trace_focus, "");
  focus_timeout = FOCUS_TIMEOUT_SEC;
  focus_timeout_widget = FOW.cur_wid;
  TRACE(trace_focus, "Cur Widget# %d",FOW.cur_wid );
}

static void focus_stop_timer(void)
{
  focus_timeout = 0;
  focus_timeout_widget = -1;
}


static void focus_timer(void *user_data, XtIntervalId *id)
{

  if( focus_timeout > 0 ) {
    focus_timeout--;
    TRACE(trace_focus, "timer: %d", focus_timeout );
    if( focus_timeout == 0 ) disarm_widget();
  }
  XtAppAddTimeOut(user_data, 1000, focus_timer, user_data );
}


void focus_init(Widget top)
{
  FOW.names = m_create( 10, sizeof(char*));
  FOW.wid = m_create( 50, sizeof(Widget));
  FOW.cur_wid = -1;
  FOW.cur_group = -1;
  FOW.old_group = -1;
  FOW.old_wid = -1;
  FOW.input_widget = 0;
  FOW.keypad_input = 0;
  focus_timeout = 0;
  focus_timer( XtWidgetToApplicationContext(top), 0);
}

void focus_destroy(void)
{
  m_free(FOW.wid);
  m_free_strings(FOW.names,0);
  FOW.wid=0;
}

void focus_put( Widget w )
{
  m_put( FOW.wid, &w );
}

#define WID(m,p) (*(Widget*) mls(m,p))

void focus_add2( Widget w, char *group )
{
  Widget *g;
  int i;
  void *null = 0;
  int group_nr;
  focus_override_translations(w);
  group_nr = m_lookup_str(FOW.names, group, 1 );
  if( group_nr < 0 ) // does not exists, append a group
    {
      m_lookup_str(FOW.names, group, 0 );	/* add group */
      /* add group terminator to previous g. */
      if( m_len(FOW.wid) ) m_put(FOW.wid,&null);
      m_put(FOW.wid,&w);
      return;
    }

  /* find end of group group_nr and insert widget before */
  m_foreach( FOW.wid, i, g )
    {
      if( *g == 0 ) {
	if( group_nr ) group_nr--; else goto insert_before_end_of_widget_group;
      }
    }
  m_put(FOW.wid,&w);
  return;

 insert_before_end_of_widget_group:
  i--; // goto end of group
  m_ins(  FOW.wid, i, 1 );
  m_write( FOW.wid, i, &w, 1 );
}


/* set, notify, make active, activate, ... */
void focus_set()
{
  focus_call_notify( FOW.cur_wid );
}

// set focus to widget w
void focus_set_cur_widget(Widget w)
{
  Widget *g;
  int group = 0;
  int i;

  m_foreach( FOW.wid, i, g )
    {
      if( *g == 0 ) {
	group++;
	continue;
      }
      if( *g == w ) goto found;
    }
  return; // nothing found, leave focus unchanged

 found:
  FOW.cur_group = group;
  set_current_widget( i, 0 );
}





/* focus new widget, detect focus group change
   if new focus group is selected, save last active widget
   used for mouse/touch initiated focus actions

 */
void focus_push(Widget w)
{
  Widget *g;
  int group = 0;
  int i;
  TF();

  m_foreach( FOW.wid, i, g )
    {
      if( *g == 0 ) {
	group++;
	continue;
      }
      if( *g == w ) goto found;
    }
  return; // nothing found, leave focus unchanged

 found:
  if( group != FOW.cur_group ) {
    TRACE(trace_focus,"Focus Group Change" );
    // save last active widget, if its a group change
    FOW.old_group = FOW.cur_group;
    FOW.old_wid = FOW.cur_wid;
    // set current group
    FOW.cur_group = group;
  }

  if( FOW.cur_wid != i && FOW.cur_wid >=0 )
    {
      focus_call_reset( FOW.cur_wid );
    }

  set_current_widget( i, 1 );
}


void focus_push_group( char *group )
{
  int i, group_cnt;
  Widget *g;
  int group_nr = m_lookup_str(FOW.names, group, 1 );
  if( group_nr < 0 ) // does not exists, append a group
    {
      printf( "Cannot switch to focus group: %s\n", group );
      return;
    }

  group_cnt = 0;
  for(i=0;i<m_len(FOW.wid);i++) {
    g = (Widget *) mls(FOW.wid,i); if( !g ) ERR("corrupted");
    if( *g && group_cnt == group_nr )  {
      focus_push(*g);
      return;
    }
    if( ! *g ) group_cnt++;
  }

  WARN("can find group, list corrupted");
}


Widget focus_get_widget(void)
{
  int n;
  n = FOW.cur_wid >= 0 ? FOW.cur_wid : 0;
  if( n >= m_len(FOW.wid )) return NULL;
  return *(Widget*) mls( FOW.wid, n );
}

void focus_push_action(Widget prev, Widget w)
{
  focus_set_cur_widget(prev);
  focus_push(w);
}

/*
  Set focus to Widget in last active group
  Exchange OLD <--> CUR
  used for "return" buttons
 */
void focus_pop(void)
{



  if( FOW.old_wid < 0 || FOW.old_wid == FOW.cur_wid ) return;

  // deactivare old widget
  if( FOW.old_wid >= 0 ) focus_call_reset( FOW.old_wid );

  SWAP( FOW.old_group, FOW.cur_group );
  SWAP( FOW.old_wid, FOW.cur_wid  );

  // activate new widget
  set_current_widget(FOW.cur_wid, 1 );
}

/*
void focus_next(void)
{
  Widget *w;
  int n = m_len( FOW.wid) ;
  int p = FOW.cur_wid;

  if( n == 0 ) return; // no focus list
  if( p < 0 ) p=-1;

  p++; // next widget
  if( p < n ) {
    w =(Widget*) mls( FOW.wid, p );
    if( *w != 0 ) goto set_focus;
  }
  p--; // p points to end list

  // find start of focus list
  while( p > 0 )
    {
      w =(Widget*) mls( FOW.wid, p-1 );
      if( *w == 0 ) break;
      p--;
    }

  // set focus to widget number p
 set_focus:
  if( FOW.cur_wid == p || p < 0 || p >= n ) return;

  // activate new widget
  // deactivate old widget
  if( FOW.cur_wid >= 0 ) focus_call_reset( FOW.cur_wid );
  set_current_widget( p, 1 );
}
*/

int next_widget(int p)
{
    Widget *w;
    int n = m_len( FOW.wid) ;
    if( n == 0 ) return -1; // no focus list
    if( p < 0 ) p=-1;
    p++; // next widget
    if( p < n ) {
	w =(Widget*) mls( FOW.wid, p );
	if( *w != 0 ) return p;
    } else p=n;
    p--;
      // find start of focus list
    while( p > 0 )
	{
	    w =(Widget*) mls( FOW.wid, p-1 );
	    if( *w == 0 ) break;
	    p--;
	}

    return p;
}


int prev_widget(int p)
{
  Widget *w;
  int n = m_len( FOW.wid) ;

  if( n == 0 ) return -1; // no focus list
  if( p <= 0 ) { p=0; goto find_list_end; } // no previous widget

  p--;
  w =(Widget*) mls( FOW.wid, p );
  if ( *w != 0 ) return p;

  p++; // p points to start of list
  // find end of focus list
 find_list_end:
  while( p < (n-1) )
      {
	  w =(Widget*) mls( FOW.wid, p+1 );
	  if( *w == 0 ) break;
	  p++;
      }
  return p;
}

void set_widget(int p)
{
    int n = m_len( FOW.wid) ;
    // set focus to widget number p
    if( FOW.cur_wid == p || p < 0 || p >= n ) return;
    // deactivate old widget
    if( FOW.cur_wid >= 0 ) focus_call_reset( FOW.cur_wid );
    // activate new widget
    set_current_widget( p, 1 );
}


int isManaged(Widget w)
{
    if(! XtIsManaged(w) ) return 0;
    TRACE(trace_focus, "Widget %s found managed", XtName(w) );

    while( (w = XtParent(w)) && strcmp( XtName(w),"gb"))
	if( ! XtIsManaged(w) ) {
	    TRACE(trace_focus,"NOT manged Widget %s", XtName(w));
	    return 0;
	}
	else TRACE(trace_focus, "Parent Widget %s found managed", XtName(w) );
    return 1;
}


void focus_prev(void)
{
    Widget w;
    int p = FOW.cur_wid;

    while(1) {
	p=prev_widget(p);
	if( p < 0 || p == FOW.cur_wid ) return;
	w =*(Widget*) mls( FOW.wid, p );
	if(isManaged(w)) break;
    }

    set_widget(p);
}

#include <X11/IntrinsicP.h>
void dump_focus_group(int p)
{
    Widget w;
    while( p > 0 ) {
	w =*(Widget*) mls( FOW.wid, p-1 );
	if( !w ) break;
	p--;
    }
    while( p < m_len(FOW.wid) ) {
	w =*(Widget*) mls( FOW.wid, p );
	if( !w ) break;
	p++;
	// CoreRec cc = (CoreRec) w;
	puts( (char*) w->core.name );
    }

}

void focus_next(void)
{
    Widget w;
    int p = FOW.cur_wid;

    while(1) {
	p=next_widget(p);
	if( p < 0 || p == FOW.cur_wid ) {
	    return;
	}
	w =*(Widget*) mls( FOW.wid, p );
	if(isManaged(w)) break;
    }

    set_widget(p);
}


/*
void focus_prev(void)
{
  Widget *w;
  int n = m_len( FOW.wid) ;
  int p = FOW.cur_wid;

  if( n == 0 ) return; // no focus list
  if( p <= 0 ) { p=0; goto find_list_end; } // no previous widget

  p--; // prev widget
  w =(Widget*) mls( FOW.wid, p );
  if( *w != 0 ) goto set_focus;
  p++; // p points to start of list

  // find end of focus list
 find_list_end:
  while( p < (n-1) )
    {
      w =(Widget*) mls( FOW.wid, p+1 );
      if( *w == 0 ) break;
      p++;
    }

  // set focus to widget number p
 set_focus:
  if( FOW.cur_wid == p || p < 0 || p >= n ) return;
  // deactivate old widget
  if( FOW.cur_wid >= 0 ) focus_call_reset( FOW.cur_wid );
    // activate new widget
  set_current_widget( p, 1 );
}
*/


/* verändern des focus und aktivieren/deaktivieren des ausgewählten widgets */
void focus_do( wheel_cmd cmd )
{
  switch( cmd ) {
  default:
  case WHEEL_NOCOMMAND:
    break;
  case WHEEL_LEFT:
    focus_prev();
    break;
  case WHEEL_RIGHT:
    focus_next();
    break;
  case WHEEL_FIRE:
    focus_set();
    focus_start_timer();
    break;
  }
}


void focus_key_event(Widget w, void* client, XEvent *ev, Boolean *cont)
{
  Modifiers dummy;
  KeySym keysym;
  char *keyName;
  wheel_cmd cmd = WHEEL_NOCOMMAND;

  if( ev->type != KeyPress ) return;

  printf( "Key: %u\n", ev->xkey.keycode );
  XtTranslateKeycode(XtDisplay(w),          /* Convert to keysym     */
		     ev->xkey.keycode, ev->xkey.state, &dummy, &keysym);

  keyName =  XKeysymToString(keysym);

  printf("  KeySym = %x (%s)\n", (uint)keysym, keyName );


  // emulate wheel
  if( strcmp(keyName, "Left") == 0 )       cmd = WHEEL_LEFT;
  else if( strcmp(keyName, "Right") == 0 ) cmd = WHEEL_RIGHT;
  else if( strcmp(keyName, "space") == 0 ) cmd = WHEEL_FIRE;
  else {
    *cont = True; /* continue to dispatch */
    return;
  }

  *cont=False;
  focus_do( cmd );
}


wheel_cmd string_to_wheel_cmd( char *key )
{
  wheel_cmd cmd = WHEEL_NOCOMMAND;
  if( is_empty(key) ) return cmd;
  while( isspace(*key) ) key++;
  if( strncasecmp(key, "dec", 3) == 0 )
    cmd = WHEEL_LEFT;
  else if( strncasecmp(key, "inc", 3) == 0 )
    cmd = WHEEL_RIGHT;
  else if( strncasecmp(key, "fire", 4) == 0 )
    cmd = WHEEL_FIRE;
  return cmd;
}


char *wheel_cmd_to_str(wheel_cmd cmd)
{
  switch(cmd) {
  default:
  case WHEEL_NOCOMMAND: return "NO_COMMAND";
  case WHEEL_LEFT:      return "Left";
  case WHEEL_RIGHT:     return "Right";
  case WHEEL_FIRE:      return "Fire";
  case WHEEL_KEYPAD:    return "KP_<X>";
  }
}


/* execute wheel command |cmd|
   Zuerst wird geprüft ob das aktuelle Widget dieses Command brauchen kann,
   falls nicht wird geprüft ob der focus verändert werden soll.
   Hat das Widget das Kommando angenommen wird der
   ein Timer gestartet.
   Nach ablauf des Timers wird "reset" an das aktuelle widget
   gesendet. Das sollte zu einem "disarm" führen.
*/
void focus_exec_wheel_command( wheel_cmd cmd )
{
  int taken = 0;
  Widget crsr=focus_get_widget();

  TRACE(trace_focus, "" );

  if( crsr ) {

    TRACE(trace_focus,"check if widget is wheel");
    if( XtIsSubclass( crsr, wheelWidgetClass ) ) {
      TRACE(trace_focus,"check if widget wants %s", wheel_cmd_to_str(cmd));
      taken = wheel_exec_command(crsr, cmd, 0);
      if( taken )  {
        focus_start_timer();
        TRACE(trace_focus,"widget took key");
        return;
      }
    }
  }
  focus_timeout = 0; /* disable timer */
  if( ! taken )  focus_do(cmd); // we take it
}

/* execute wheel commmand |key|
   key: Dec, Inc, Fire
*/

void focus_cmd( char *key )
{
  wheel_cmd cmd;
  cmd = string_to_wheel_cmd( key );
  focus_exec_wheel_command( cmd );
}


static void focus_tracker_action(Widget w, XEvent* e, String* s, Cardinal* n)
{
  int i;
  TF();
  if( *n == 0 ) return;
  for(i=0; i< *n; i++ ) {
    TRACE(trace_focus, "action: %s", s[i] );
    if( strcasecmp( s[i],  "ENTER" )==0 )
      focus_push(w);
    if( strcasecmp( s[i],  "SET" ) == 0 )
      focus_start_timer();
  }
}



static void focus_translations_init(XtAppContext ctx)
{
  static XtActionsRec actionTable[] = {
    { "focus_tracker",focus_tracker_action }
  };

  static char focus_translation_str[] =
    "<EnterWindow>: focus_tracker(ENTER) highlight()\n"
    "<Btn1Down>: focus_tracker(ENTER SET) notify()";
  //    "\n<LeaveWindow>: focus_tracker(LEAVE) reset()  ";

  XtAppAddActions(ctx, actionTable, 1 );
  focus_translations = XtParseTranslationTable(focus_translation_str);
}


void focus_override_translations(Widget w)
{
  static char translations_init = 0;
  if( !translations_init ) {
    focus_translations_init(XtWidgetToApplicationContext(w));
    translations_init = 1;
  }

  XtOverrideTranslations( w, focus_translations );
}


void focus_add(Widget w)
{
  focus_override_translations(w);
  focus_put(w);
}


void focus_keypad_input( int val )
{
  if( FOW.input_widget && FOW.keypad_input )
    wheel_exec_command( FOW.input_widget, WHEEL_KEYPAD, val );
}

void focus_numkeys_input_cb(Widget w, void *data, void *class )
{
  int key = (int) class;
  if( key == XK_KP_Enter ) focus_pop();
  else focus_keypad_input( key );
}

/*  Wheel/Keyboard Input Action Handler
    Bound to MainFrame Translations

    Dec
    Inc
    Fire
*/
void focus_keyboard( Widget w, XEvent*ev, String *s, Cardinal* n)
{
  if( n && *n && s && *s ) focus_cmd( *s );
}

/* Focus Callback zur Anbindung an einzelne buttons
   Inc,Dec,Fire - führende leerzeichen werden entfernt
*/
void focus_cb( Widget w, void *u, void *c )
{
  char *cmd_str = u;
  focus_cmd( cmd_str );
}

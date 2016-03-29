#include "SelectW.h"
#include "converters-xft.h"
#include "sig_xt.h"
#include "common_sigxt.h"
#include "ini_read2.h"
#include "focus_group.h"

/****************************************************
 * resourcen, todo!                                 *
 ****************************************************/
#define TICKS_W 15
#define BAR_BORDER 1

#ifndef TF
#  define TF() TRACE(2,"")
#endif

#ifndef IsEmpty
#  define IsEmpty(s) ((s)==NULL || *(s) == 0) 
#endif

#undef OFFSET
// calculate (offset)address of field inside our widget structure
#define OFFSET(field) XtOffsetOf(SelectWidget_t, select.field)


/****************************************************
 *                                                  *
 *	implementation                              *
 *                                                  *
 ****************************************************/

static void class_initialize(void);

static void initialize(Widget request, Widget cnew,
		       ArgList args, Cardinal *num_args);

static void destroy(Widget w);

static Boolean set_values(Widget current, Widget request, Widget cnew,
			  ArgList args, Cardinal *num_args);

static void expose(Widget w, XEvent *event, Region region);


/* wheel interface */
/* signal interface */
static int exec_command(Widget w, int cmd, int val);
static int sig_recv( Widget w, Widget from, int type, void *class, void *data );

/* helpers */
static void draw_value(Widget w, Boolean clear );
static int set_select_value(Widget w, int val);

/****************************************************
 *	init our class                              *
 **************************t*************************/

/* {name, class, type, size, offset, default_type, default_addr}, */
static XtResource resources[] = {
  { XtNlabel, XtCLabel, XtRString, sizeof(String),
    OFFSET(label), XtRString, "PAW" },

  {XtNlabelFont, XtCFace, XtRXftFont, sizeof (XftFont *),
   OFFSET (label_font), XtRString, "Sans-20"},
  {XtNvalueFont, XtCFace, XtRXftFont, sizeof (XftFont *),
   OFFSET (value_font), XtRString, "Sans-30"},

  {XtNtextColor, XtCForeground, XtRXftColor, sizeof (XftColor),
   OFFSET (text_col), XtRString, XtDefaultForeground },
  {XtNgraphColor, XtCForeground, XtRPixel, sizeof (Pixel),
   OFFSET (graph_col), XtRString, XtDefaultForeground },  
  {XtNrulerColor, XtCForeground, XtRPixel, sizeof (Pixel),
   OFFSET (ruler_col), XtRString, "white" },    

  {XtNselectMax, XtCWidth, XtRInt, sizeof (int),
   OFFSET (select_max), XtRImmediate, (XtPointer)100 },    
  {XtNselectMin, XtCWidth, XtRInt, sizeof (int),
   OFFSET (select_min), XtRImmediate, (XtPointer)0 },    
  {XtNselectVal, XtCWidth, XtRInt, sizeof (int),
   OFFSET (select_val), XtRImmediate, (XtPointer)1 },

  {XtNvalue, XtCValue, XtRInt, sizeof (int),
   OFFSET (value), XtRImmediate, (XtPointer)1 },

  {XtNlocked, XtCValue, XtRBoolean, sizeof (Boolean),
   OFFSET (locked), XtRImmediate, (XtPointer) False },

  { XtNlabelStr,  XtCLabel, XtRString, sizeof(String),
    OFFSET(labelStr), XtRString, NULL},
  { XtNunitStr,  XtCLabel, XtRString, sizeof(String),
    OFFSET(unitStr), XtRString, NULL},
  { XtNchapterStr,  XtCLabel, XtRString, sizeof(String),
    OFFSET(chapterStr), XtRString, NULL},
  { XtNvarnameStr,  XtCLabel, XtRString, sizeof(String),
    OFFSET(varnameStr), XtRString, NULL},
  { XtNvalueChange, XtCCallback, XtRCallback, sizeof(XtPointer),
    OFFSET(value_change_cb_list), XtRImmediate, (XtPointer) NULL}

};


static void HighlightAction(Widget w, XEvent* e, String* s, Cardinal* n);
static void ResetAction(Widget w, XEvent* e, String* s, Cardinal* n);
static void NotifyAction(Widget w, XEvent* e, String* s, Cardinal* n);

static XtActionsRec actions[] =
{
  {"highlight",	HighlightAction},
  {"reset",     ResetAction},
  {"notify",    NotifyAction}
};

static char translations[] =
  "<BtnDown>:           notify()     \n"
  "<EnterWindow>:       highlight()  \n"
  "<LeaveWindow>:       reset()";

SelectWidgetClass_t SelectWidgetClassInstance = {
  { /* wheel */
    (WidgetClass)&wheelClassRec,	/* superclass */
    "Select",				/* class_name */
    sizeof(SelectWidget_t),		/* widget_size */
    class_initialize,			/* class_initialize */
    NULL,				/* class_part_initialize */
    False,				/* class_inited */
    initialize,				/* initialize */
    NULL,				/* initialize_hook */
    XtInheritRealize,			/* realize */
    actions,				/* actions */
    XtNumber(actions),			/* num_actions */
    resources,				/* resources */
    XtNumber(resources),		/* num_resources */
    NULLQUARK,				/* xrm_class */
    True,				/* compress_motion */
    True,				/* compress_exposure */
    True,				/* compress_enterleave */
    False,				/* visible_interest */
    destroy,			        /* destroy */
    NULL,				/* resize */
    expose,				/* expose */
    set_values,				/* set_values */
    NULL,				/* set_values_hook */
    XtInheritSetValuesAlmost,		/* set_values_almost */
    NULL,				/* get_values_hook */
    NULL,				/* accept_focus */
    XtVersion,				/* version */
    NULL,				/* callback_private */
    translations,			/* translations */
    XtInheritQueryGeometry,		/* query_geometry */
    XtInheritDisplayAccelerator,	/* display_accelerator */
    NULL,				/* extension */
  },

  /* Wheel */
  {
    exec_command,			/* wheel_exec_command_t */
  },

  /* Select */
  {
    NULL,				/* extension */
  }
};

/****************************************************
 *	THIS is the interface to the APPLICATION    *
 ****************************************************/
WidgetClass  selectWidgetClass = (WidgetClass)&SelectWidgetClassInstance;



/****************************************************
 *	I M P L E M E N T A T I O N                 *
 ****************************************************/

#ifndef WTRACE
#define WTRACE(w) TRACE(2,"%s", w->core.name )
#endif

static void class_initialize(void)
{
  converters_xft_init();
}


void calc_layout( Widget w, int *width, int *height )
{
  SelectWidget mw = (SelectWidget)w;
  SelectPart   *mp = & mw->select;
  Display *dpy = XtDisplay(w);
  XGlyphInfo extents;
  char *buf; 

  buf = mp->labelStr;
  XftTextExtentsUtf8(dpy, mp->label_font, 
		     (FcChar8*)buf, strlen(buf), &extents );

  *height = mp->label_font->height;
  *width  = extents.xOff;

  buf = mls(mp->display_value,0);
  XftTextExtentsUtf8(dpy, mp->value_font, 
		     (FcChar8*)buf, strlen(buf), &extents );

  *height += mp->value_font->height;
  *width  = Max( extents.xOff, *width );

  *height += 8;
  *width  += 8;
}



/* initialize our widget, calculate size and,
   if not already set, set core.width and core.height 
   there is no window, do not use XtWindow(w), until
   the widget is realized.
   initialized well be called twice! 
*/
static void initialize(Widget request, Widget cnew,
		       ArgList args, Cardinal *num_args)
{

  int width, height;
  SelectWidget mw = (SelectWidget)cnew;
  SelectPart   *mp = & mw->select; 
  TRACE(2,"%s:", mw->core.name);

  /* name of this widget */
  if( mp->label == 0 || mp->label[0] == 0 ) mp->label = mw->core.name;

  mp->select_key = rc_get_key( mp->chapterStr, mp->varnameStr );
  mp->select_min = 0;
  if( mp->select_key < 1 ) {
    WARN( "%s keine daten: %s %s", 
	  cnew->core.name, mp->chapterStr, mp->varnameStr );
  } else 
    mp->select_max = m_len(mp->select_key)-2; // name
  
  // force update
  mp->display_value = m_create(20,1);
  mp->select_val++; 
  set_select_value(cnew,mp->select_val-1); 

  calc_layout(cnew, &width, &height);
  
  /* set current widget size. Widget can be resized later */
  if (mw->core.height == 0)
    mw->core.height = height;

  if (mw->core.width == 0)
    mw->core.width = width;  

  /* if( mp->focus_group && *(mp->focus_group)) focus_add2( cnew, mp->focus_group ); */
}

static void destroy(Widget w)
{
  SelectWidget mw = (SelectWidget)w;
  SelectPart   *mp = & mw->select; 
  TF();  
  if( mp->draw ) { XftDrawDestroy( mp->draw ); mp->draw=0; }
  if( mp->display_value ) { m_free(mp->display_value); mp->display_value=0; }
}


/** find nearest PAIR (display_val, value) 
    update: display_value, value, select_val
*/
static int set_new_value(Widget w, int val)
{  
    SelectWidget mw = (SelectWidget)w;
    SelectPart   *mp = & mw->select;

    if( val == mp->value ) return 0;

    int sel = mp->select_val;

    rc_find_nearest_pair( mp->select_key, val, 
			  & mp->select_val, & mp->display_value,  &mp->value ); 

    return( sel != mp->select_val ) ;
}

/** set new select value 
    select_val: index auf die string-liste
    select_min, select_max: 
    (display_val,value): anzuzeigender string und zugeordneter wert
    returns: 
    1 - wert hat sich geändert 
    0 - wert ist unverändert 
*/
static int set_select_value(Widget w, int val)
{
  SelectWidget mw = (SelectWidget)w;
  SelectPart   *mp = & mw->select; 

  if( val < mp->select_min ) val = mp->select_min;
  else if( val > mp->select_max ) val = mp->select_max ;

  if( val != mp->select_val ) {
    mp->select_val=val;
    TRACE(2,"%s: Value Changed", w->core.name );
    rc_get_pair( mp->select_key, mp->select_val, & mp->display_value, &mp->value );
    return 1;
  }
  return 0;
}


static Boolean set_values(Widget current, Widget request, Widget cnew,
			  ArgList args, Cardinal *num_args)
{
  SelectWidget mw = (SelectWidget)cnew;
  SelectPart   *mp = & mw->select; 
  int i;
  Boolean redisplay = FALSE;
  SelectPart *cur = &(((SelectWidget)current)->select);

  TF();
  for(i=0;i<*num_args;i++) {
    /* printf( "Name=%s Value=%d\n", args[i].name, (int) args[i].value );    */
    if( strcmp(args[i].name, XtNlabelStr )==0) redisplay=True;
    else if( strcmp(args[i].name, XtNselectVal )==0) {
      int val = mp->select_val; // get new value
      mp->select_val_cur = val;
      mp->select_val = cur->select_val; // reset to old value
      if( set_select_value(cnew,val) ) // safe set new value 
	draw_value(cnew, True); 
    }
    else if( strcmp(args[i].name, XtNvalue )==0) {
	int wants = mp->value;
	mp->value = cur->value; // reset to old value
	if( set_new_value(cnew, wants) ) {
	    draw_value(cnew, True); 
            mp->select_val_cur = mp->select_val;
        }
    }
  }
  return redisplay;
}

/* set new select value and redraw the select if neccessary */
static void update_select_value(Widget w, int val)
{
  SelectWidget mw = (SelectWidget)w;
  SelectPart   *mp = & mw->select; 

  if( set_select_value(w,val) )
    {
      draw_value(w, True);
      XtCallCallbacks( w, XtNvalueChange, (XtPointer) &mp->select_val );
    }
}


/* 
   STATE_NORMAL:
      select_val_cur == select_val

   STATE_ARMED:
      select_val kann geändert werden

      mit NotifyAction wird select_val_cur <== select_val
      der eingestellte wert übernommen
      
*/
static void arm_widget_with_timeout( Widget w, Boolean enable_timeout )
{
  SelectWidget mw = (SelectWidget)w;
  SelectPart   *mp = & mw->select; 
  if( mw->wheel.state == STATE_ARMED ) return;
  mp->select_val_cur = mp->select_val; // save value
  mw->wheel.state = STATE_ARMED;
  expose(w,NULL,NULL);

  /* make virtual keyboard visible */

  vis_setup_slider_t msg; 
  msg.min = mp->select_min; 
  msg.max = mp->select_max; 
  msg.cur = mp->select_val; 
  msg.show_scale = 0;
  sig_send( w, VIS_SETUP_SLIDER, &msg );  
}

/* widget zurücksetzen ?? */
static void disarm_widget( Widget w )
{
  SelectWidget mw = (SelectWidget)w;
  SelectPart   *mp = & mw->select; 
  if( mw->wheel.state != STATE_ARMED ) return;
  mw->wheel.state = STATE_SELECTED;
  expose(w,NULL,NULL);
  /* disable virtual keyboard */
  sig_send( w, VIS_REMOVE_SLIDER, NULL  ); 
}

static void select_widget( Widget w )
{
  SelectWidget mw = (SelectWidget)w;
  if( mw->wheel.state == STATE_SELECTED ) return;
  mw->wheel.state = STATE_SELECTED;
  expose(w,NULL,NULL);
}

static void HighlightAction(Widget w, XEvent* e, String* s, Cardinal* n)
{
  SelectWidget mw = (SelectWidget)w;
  SelectPart   *mp = & mw->select; 
  TF();
  if( mp->locked ) return; 
  select_widget(w);
}

/* back to normal state */
static void ResetAction(Widget w, XEvent* e, String* s, Cardinal* n)
{ 
  SelectWidget mw = (SelectWidget)w;   
  SelectPart   *mp = & mw->select; 
  TRACE(2,"%s: reset, cur:%d old:%d", w->core.name,
        mp->select_val, mp->select_val_cur );

  set_select_value( w,mp->select_val_cur); // reset value 

  if( mw->wheel.state == STATE_ARMED ) 
    disarm_widget(w);
  else if(mw->wheel.state == STATE_SELECTED ) {
    mw->wheel.state = STATE_NORMAL;
    expose(w,NULL,NULL);
  } 
}

/* falls widget "armed" ist,
   callback aufrufen und den zum angezeigtem text gehörenden wert übergeben

   falls widget "selected" oder "normal" ist, in den modus "armed" umschalten.
*/
static void fire(Widget w)
{
  SelectWidget mw = (SelectWidget)w;
  SelectPart   *mp = & mw->select; 
  
  if( mp->locked ) return;

  if( mw->wheel.state == STATE_ARMED ) {
    mp->select_val_cur = mp->select_val; // saved value == current value
    XtCallCallbacks( w, XtNcallback, (XtPointer) & mp->value );
    disarm_widget(w);
  } else 
    arm_widget_with_timeout( w, True );  
}


static void NotifyAction(Widget w, XEvent *event, String *params, Cardinal *n)
{ 
  SelectWidget mw = (SelectWidget)w;
  SelectPart   *mp = & mw->select; 
  if( mp->locked ) return; 
  fire(w);
}


static int exec_command(Widget w, int cmd, int val)
{
  SelectWidget mw = (SelectWidget)w;
  SelectPart   *mp = & mw->select; 

  WTRACE(w);

  if( mp->locked ) return 0; 

  if(mw->wheel.state != STATE_ARMED ) {
    if( cmd != WHEEL_FIRE ) return 0; /* give back this event to the focus mgr */ 
    arm_widget_with_timeout(w, True);
    return 1; /* we have processed this event */
  } 

  /* we are armed */
  switch(cmd)
    {
    case WHEEL_LEFT:
      update_select_value(w, mp->select_val -1 );
      return 1;

    case WHEEL_RIGHT:
      update_select_value(w, mp->select_val+1);
      return 1;

    case WHEEL_FIRE:
      fire(w); // widget "is armed": execute callback
      return 1;
    }

  return 0; /* dont know how to handle this event, pass it to focus mgr */
}

int sig_recv(Widget w, Widget from, int type, void *class, void *data )
{
  TRACE(2,"selectW signal received");
  return 0;
}

/* area is cleared */
static void draw_header(Widget w)
{  
  Display *dpy = XtDisplay(w);
  SelectWidget mw = (SelectWidget)w;
  SelectPart   *mp = & mw->select; 
  int x,y;
  XGlyphInfo extents;
  char *buf = mp->labelStr;
  XftColor *c;

  XftTextExtentsUtf8(dpy, mp->label_font, 
		     (FcChar8*)buf, strlen(buf), &extents );
  
  y = mp->label_font->ascent;
  x = 0;
  c = & mw->wheel.xft_col[ mw->wheel.state ];
  
  XftDrawStringUtf8( mp->draw,c, mp->label_font, 
		     x, y, (FcChar8*)buf, strlen(buf));
}

static void draw_value(Widget w, Boolean clear )
{
  Display *dpy = XtDisplay(w);
  SelectWidget mw = (SelectWidget)w;
  SelectPart   *mp = & mw->select; 
  char *buf;
  XGlyphInfo extents;
  int x,y;
  draw_area_t *d = & mp->val_area;

  if(! mp->draw ) return; /* not initialized yet */

  if( mp->display_value>0 && m_len(mp->display_value) )
    buf = mls(mp->display_value,0);
  else buf="Empty";

  if( clear ) {
    XFillRectangle(dpy, XtWindow(w), 
		   mw->wheel.gc[ mw->wheel.state ],
		   d->x, d->y, d->w, d->h );
  }


  XftTextExtentsUtf8(dpy, mp->value_font, 
		     (FcChar8*)buf, strlen(buf), &extents );
  
  x = d->w / 2 + d->x; 
  x -= extents.xOff / 2;
  y = d->y + mp->value_font->height;
  XftColor *c = & mw->wheel.xft_col[ mw->wheel.state ];
  XftDrawStringUtf8( mp->draw,c, mp->value_font, 
		     x, y, (FcChar8*)buf, strlen(buf));
}

static void expose( Widget w, XEvent *event, Region region )
{
  Display *dpy = XtDisplay(w);
  int screen = DefaultScreen(dpy);
  SelectWidget mw = (SelectWidget)w;
  SelectPart   *mp = & mw->select; 
  draw_area_t *d = & mp->val_area;

  if( !XtIsRealized(w)) return;	/* if there is no window, do not draw into void */
  if( !mp->draw ) { 
    mp->draw = XftDrawCreate( dpy, XtWindow(w),
			      DefaultVisual(dpy, screen), None);
    if( !mp->draw ) ERR("XftDrawCreate returns NULL");
  }
  if( mw->wheel.state >= 3 ) { TRACE(2,"illg. state"); mw->wheel.state=0; }
  TRACE(2,"wheel state: %d", mw->wheel.state );
  XFillRectangle(dpy, XtWindow(w), mw->wheel.gc[ mw->wheel.state ],
		 0, 0, w->core.width, w->core.height );
  d->x = 0; 
  d->w = mw->core.width;
  d->y = mp->label_font->height +1;

  d->h = mw->core.height - d->y  + 1;


  draw_header(w);
  draw_value(w, False );
}

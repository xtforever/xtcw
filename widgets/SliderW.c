#include "SliderW.h"
#include "converters-xft.h"
#include "sig_xt.h"
#include "common_sigxt.h"

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
#define OFFSET(field) XtOffsetOf(SliderWidget_t, slider.field)


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
static int exec_command(Widget w, int cmd);
static int sig_recv( Widget w, Widget from, int type, void *class, void *data );



/****************************************************
 *	init our class                              *
 **************************t*************************/

/* {name, class, type, size, offset, default_type, default_addr}, */
static XtResource resources[] = {
  { XtNlabel, XtCLabel, XtRString, sizeof(String),
    OFFSET(label), XtRString, "PAW" },

  {XtNlabelFont, XtCFace, XtRXftFont, sizeof (XftFont *),
   OFFSET (label_font), XtRString, "Sans-20"},
  {XtNtickFont, XtCFace, XtRXftFont, sizeof (XftFont *),
   OFFSET (tick_font), XtRString, "Sans-8"},

  {XtNtextColor, XtCForeground, XtRXftColor, sizeof (XftColor),
   OFFSET (text_col), XtRString, XtDefaultForeground },
  {XtNgraphColor, XtCForeground, XtRPixel, sizeof (Pixel),
   OFFSET (graph_col), XtRString, XtDefaultForeground },  
  {XtNrulerColor, XtCForeground, XtRPixel, sizeof (Pixel),
   OFFSET (ruler_col), XtRString, "white" },    

  {XtNsliderMax, XtCWidth, XtRInt, sizeof (int),
   OFFSET (slider_max), XtRImmediate, (XtPointer)100 },    
  {XtNsliderMin, XtCWidth, XtRInt, sizeof (int),
   OFFSET (slider_min), XtRImmediate, (XtPointer)0 },    
  {XtNsliderVal, XtCWidth, XtRInt, sizeof (int),
   OFFSET (slider_val), XtRImmediate, (XtPointer)1 },
  {XtNshowScale, XtCWidth, XtRInt, sizeof (int),
   OFFSET (show_scale), XtRImmediate, (XtPointer)1 },
};

static void HighlightAction(Widget w, XEvent* e, String* s, Cardinal* n);
static void ResetAction(Widget w, XEvent* e, String* s, Cardinal* n);
static void NotifyAction(Widget w, XEvent* e, String* s, Cardinal* n);
static void SliderAction(Widget w, XEvent* e, String* s, Cardinal* n);

static XtActionsRec actions[] =
{
    /*{name,		procedure},*/
  {"slider",	SliderAction},
  {"highlight",	HighlightAction},
  {"reset",     ResetAction},
  {"notify",    NotifyAction},
};

static char translations[] =
  "<MotionNotify>:      slider()      \n"
  "<BtnDown>:           notify()     \n"
  "<EnterWindow>:       highlight()  \n"
  "<LeaveWindow>:       reset()        ";

SliderWidgetClass_t SliderWidgetClassInstance = {
  { /* wheel */
    (WidgetClass)&WheelWidgetClassInstance,	/* superclass */
    "Slider",				/* class_name */
    sizeof(SliderWidget_t),		/* widget_size */
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
    sig_recv,				/* sig_receive_t */
    NULL,				/* extension */
  },

  /* Slider */
  {
    NULL,				/* extension */
  }
};

/****************************************************
 *	THIS is the interface to the APPLICATION    *
 ****************************************************/
WidgetClass  sliderWidgetClass = (WidgetClass)&SliderWidgetClassInstance;



/****************************************************
 *	I M P L E M E N T A T I O N                 *
 ****************************************************/

static void class_initialize(void)
{
  converters_xft_init();
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

  SliderWidget mw = (SliderWidget)cnew;
  SliderPart   *mp = & mw->slider; 
  XtGCMask	valuemask;
  XGCValues	myXGCV;
  TRACE(2,"%s:", mw->core.name);

  /* name of this widget */
  if( mp->label == 0 || mp->label[0] == 0 ) mp->label = mw->core.name;

  /* set color to draw the graph */
  myXGCV.foreground = mp->graph_col;  valuemask = GCForeground;
  mp->graph_gc = XtGetGC(cnew, valuemask, &myXGCV);

  /* set color to draw the ruler */
  myXGCV.foreground = mp->ruler_col;
  mp->ruler_gc = XtGetGC(cnew, valuemask, &myXGCV);
  
  /* set fill color to clear the window */
  myXGCV.foreground = cnew->core.background_pixel; 
  mp->bg_gc = XtGetGC(cnew, valuemask, &myXGCV);

  int extra_height = 0;
  if( mp->tick_font ) extra_height = mp->tick_font->height;
  extra_height += 5;
  mp->slider_height = extra_height; 

  TRACE(2,"SliderMax: %d\n", mp->slider_max );

  /* set current widget size. Widget can be resized later */
  if (mw->core.height == 0)
    mw->core.height = mp->slider_height + extra_height;
  if (mw->core.width == 0)
    mw->core.width = 50;  
}

static void destroy(Widget w)
{
  SliderWidget mw = (SliderWidget)w;
  SliderPart   *mp = & mw->slider; 
  TF();  
  if( mp->draw ) { XftDrawDestroy( mp->draw ); mp->draw=0; }
  XtReleaseGC(w, mp->bg_gc );
  XtReleaseGC(w, mp->graph_gc );
  XtReleaseGC(w, mp->ruler_gc );
}

static Boolean set_values(Widget current, Widget request, Widget cnew,
			  ArgList args, Cardinal *num_args)
{
  //  SliderWidget mw = (SliderWidget)cnew;
  //  SliderPart   *mp = & mw->slider; 
  int i;
  Boolean redisplay = FALSE;

  TF();
  for(i=0;i<*num_args;i++) {
    printf( "Name=%s Value=%d\n", args[i].name, (int) args[i].value );    
    if( strcmp(args[i].name, XtNlabel )==0) redisplay=True;
    else if( strcmp(args[i].name, XtNsliderVal )==0) {
      redisplay=True;
    };
  } 

  return redisplay;
}



void draw_slider(Widget w)
{
  Display *dpy = XtDisplay(w);
  int bx,by,bw,bh;
  SliderWidget mw = (SliderWidget)w;
  SliderPart   *mp = & mw->slider; 
  

  if( mp->slider_val > mp->slider_max )
    mp->slider_val = mp->slider_max;
  else if(mp->slider_val < mp->slider_min)     
    mp->slider_val = mp->slider_min;

  bh = mp->slider_height;
  bw = w->core.width;
  bx = 0;
  by = w->core.height - mp->slider_height;

  float dx = bw * (mp->slider_val-mp->slider_min) * 1.0 / (mp->slider_max - mp->slider_min);
  bw = dx;

  printf( "max:%d val:%d pos_x:%d\n", mp->slider_max, mp->slider_val, bw );

  if( bw ) 
    XFillRectangle( XtDisplay(w), XtWindow(w), mp->graph_gc,
		    bx,by,bw,bh );
  
  if( bw < w->core.width ) {
    XFillRectangle(dpy, XtWindow(w), mw->wheel.gc[ mw->wheel.state ],
		   bw, by, w->core.width, bh );
  }
}

static void draw_ticks(Widget w)
{
  char buf[100];
  Display *dpy = XtDisplay(w);
  int bx,by,bw,bh,i,v;
  SliderWidget mw = (SliderWidget)w;
  SliderPart   *mp = & mw->slider; 

  bx=0;
  by=0;
  bw=w->core.width;
  bh=w->core.height - mp->slider_height -2;

  XDrawLine( dpy, XtWindow(w), mp->graph_gc,
	     0,bh, w->core.width, bh );

  // baseline for font
  by = bh - mp->tick_font->descent;

  for(i=1;i<=9;i++) { 
    v = (mp->slider_max - mp->slider_min) * 1.0 / 10 * i + mp->slider_min + 0.5;
    sprintf(buf,"%d", v );
    bx = bw*1.0 / 10 * i + 0.5;
    // vertical line from baseline to top of widget
    XDrawLine( dpy, XtWindow(w), mp->graph_gc,
	       bx,bh-1, bx, 0 );
    bx++;
    
    XftDrawStringUtf8( mp->draw, &mp->text_col, mp->tick_font, bx, by, (FcChar8*)buf, strlen(buf));
  }
  

}



static void expose(Widget w, XEvent *event, Region region)
{
  Display *dpy = XtDisplay(w);
  int screen = DefaultScreen(dpy);
  SliderWidget mw = (SliderWidget)w;
  SliderPart   *mp = & mw->slider; 
  TRACE(2,"%s %u %u", mw->core.name, mw->core.width, mw->core.height );
  if( !XtIsRealized(w)) return;	/* if there is no window, do not draw into void */
  // paint 
  if( !mp->draw ) { // we couldn't init without a window
    // this can only be executed after the widget is realized.
    // we could put this function into realize(), but this would add complexity.
    mp->draw = XftDrawCreate( dpy, XtWindow(w),
			      DefaultVisual(dpy, screen), None);
    if( !mp->draw ) ERR("XftDrawCreate returns NULL");
  }

  /* check widget state and change background color */
  // mw->core.background_pixel = mw->wheel.pixel[ mw->wheel.state + 3]; 
  // TRACE(2,"Slider set background: %06x", mw->wheel.pixel[ mw->wheel.state + 3] );
  /*  XClearArea (dpy, XtWindow(w),
	      0, 0, w->core.width, w->core.height, False); 
  */
  if( mw->wheel.state >= 3 ) { TRACE(2,"illg. state"); mw->wheel.state=0; }
  XFillRectangle(dpy, XtWindow(w), mw->wheel.gc[ mw->wheel.state ],
		 0, 0, w->core.width, w->core.height );

  draw_slider(w);
  if( mp->show_scale ) draw_ticks(w);
  return;
}

/* set new slider value and redraw the slider if neccessary */
static void update_slider_value(Widget w, int val)
{
  SliderWidget mw = (SliderWidget)w;
  SliderPart   *mp = & mw->slider; 

  if( val < mp->slider_min ) val = mp->slider_min;
  else if( val > mp->slider_max ) val = mp->slider_max;

  if( val != mp->slider_val ) {
    mp->slider_val=val;
    draw_slider(w);
    // XtCallCallbacks( w, XtNvalueChange, (XtPointer) mp->slider_val );
    sig_send(w, VIS_SLIDER_CHANGE, & mp->slider_val );
  }
}


/* calculate slider_val from coordinate x
   set new value
   and update slider
*/
static void coord_to_slider_val(Widget w, int x )
{
  SliderWidget mw = (SliderWidget)w;
  SliderPart   *mp = & mw->slider; 
  int val;

  if( x <= 0 ) {
    val = 0;
    goto ok;
  }

  if( x >= mw->core.width ) {
    val = mp->slider_max;
    goto ok;
  }
  
  float pos = x * 1.0 / mw->core.width;
  pos = pos * (mp->slider_max - mp->slider_min) + mp->slider_min;
  val = pos + 0.5;
  
 ok:
  update_slider_value(w, val);
}


/* 
   STATE_NORMAL:
      slider_val_cur == slider_val

   STATE_ARMED:
      slider_val kann geändert werden

      mit NotifyAction wird slider_val_cur <== slider_val
      der eingestellte wert übernommen

      
*/
static void arm_widget_with_timeout( Widget w, Boolean enable_timeout )
{
  SliderWidget mw = (SliderWidget)w;
  SliderPart   *mp = & mw->slider; 
  if( mw->wheel.state == STATE_ARMED ) return;
  mp->slider_val_cur = mp->slider_val; // save value
  mw->wheel.state = STATE_ARMED;
  expose(w,NULL,NULL);

  /* make virtual keyboard visible */
  /* vis_setup_slider_t msg; */
  /* msg.min = mp->slider_min; */
  /* msg.max = mp->slider_max; */
  /* msg.cur = mp->slider_val; */
  /* sig_send( w, VIS_SETUP_SLIDER, &msg );  */
}

static void disarm_widget( Widget w )
{
  SliderWidget mw = (SliderWidget)w;
  SliderPart   *mp = & mw->slider; 
  if( mw->wheel.state != STATE_ARMED ) return;
  mw->wheel.state = STATE_NORMAL;
  mp->slider_val = mp->slider_val_cur; // reset value
  expose(w,NULL,NULL);
  
  /* disable virtual keyboard */
  /*  sig_send( w, VIS_REMOVE_SLIDER, NULL  ); */
}

static void HighlightAction(Widget w, XEvent* e, String* s, Cardinal* n)
{
  TF();
  arm_widget_with_timeout( w, True );
}

static void ResetAction(Widget w, XEvent* e, String* s, Cardinal* n)
{  
  TF();
  disarm_widget(w);
}

static void NotifyAction(Widget w, XEvent *event, String *params, Cardinal *n)
{
  SliderWidget mw = (SliderWidget)w;
  SliderPart   *mp = & mw->slider; 
  
  if( mw->wheel.state == STATE_ARMED ) {
    mp->slider_val_cur = mp->slider_val; // saved value == current value
    XtCallCallbacks( w, XtNcallback, (XtPointer) mp->slider_val );
    disarm_widget(w);
    sig_send( w, VIS_FIRE, NULL );
  } else 
    arm_widget_with_timeout( w, True );
}

static void SliderAction(Widget w, XEvent* event, String* s, Cardinal* n)
{
  SliderWidget mw = (SliderWidget)w;
  int x,y; 
  (void)y;

  switch (event->type) {
  case MotionNotify:
    x = event->xmotion.x;
    y = event->xmotion.y;
    if(mw->wheel.state == STATE_ARMED) {
      coord_to_slider_val(w,x);
    }
    break;
  default:
    printf("Type: %d\n", event->type );
  };
}

static int exec_command(Widget w, int cmd)
{
  SliderWidget mw = (SliderWidget)w;
  SliderPart   *mp = & mw->slider; 

  if(mw->wheel.state != STATE_ARMED ) {
    if( cmd != WHEEL_FIRE ) return 0; /* give back this event to the focus mgr */ 
    arm_widget_with_timeout(w, True);
    return 1; /* we have processed this event */
  } 

  /* we are armed */
  switch(cmd)
    {
    case WHEEL_LEFT:
      update_slider_value(w, mp->slider_val -1 );
      return 1;

    case WHEEL_RIGHT:
      update_slider_value(w, mp->slider_val+1);
      return 1;

    case WHEEL_FIRE:
      XtCallCallbacks( w, XtNcallback, (XtPointer)mp->slider_val);
      disarm_widget(w);
      return 1;
    }

  return 0; /* dont know how to handle this event, pass it to focus mgr */
}

int sig_recv(Widget w, Widget from, int type, void *class, void *data )
{
  TRACE(2,"sliderW signal received");
  return 0;
}

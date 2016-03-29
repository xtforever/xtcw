#include <stdio.h>
#include <stdint.h>
#include <X11/Xft/Xft.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Label.h>


#include "Ctrl.h"
#include "mls.h"
#include "sig_xt.h"
#include "common_sigxt.h"


#define Max(a,b) (a>b ? a : b)
#define IsEmpty(s) ((s)==NULL || *(s) == 0)
#ifndef TF
#  define TF() TRACE(2,"")
#endif

static void calc_widget_size( Widget w, Dimension *width, Dimension *height );
static void do_layout(Widget w);
static void calc_str_position( clabel_t *cl );

static XtResource resources[] = {
#define offset(field) XtOffsetOf(CtrlWidget_t, ctrl.field)
	/* {name, class, type, size, offset, default_type, default_addr}, */

  { XtNisflip, XtCIsflip, XtRBoolean, sizeof(Boolean), offset(isflip),
    XtRImmediate, (XtPointer)False },

  { XtNnormalFg, XtCForeground, 
    XtRPixel, sizeof(Pixel), offset(pixel[0]), 
    XtRString, XtDefaultForeground },

  { XtNnormalBg, XtCBackground, 
    XtRPixel, sizeof(Pixel),offset(pixel[3]), 
    XtRString, XtDefaultBackground },

  { XtNselectedFg, XtCSelectedFg, 
    XtRPixel, sizeof(Pixel), offset(pixel[1]), 
    XtRString, XtDefaultForeground },
  
  { XtNselectedBg, XtCSelectedBg, 
    XtRPixel, sizeof(Pixel), offset(pixel[4]), 
    XtRString, XtDefaultBackground },

  { XtNarmedFg, XtCArmedFg, 
    XtRPixel, sizeof(Pixel), offset(pixel[2]), 
    XtRString, XtDefaultForeground },
  
  { XtNarmedBg, XtCArmedBg, 
    XtRPixel, sizeof(Pixel),offset(pixel[5]), 
    XtRString, XtDefaultBackground },

  { XtNvspace, XtCBorderWidth, 
    XtRDimension, sizeof(Dimension), offset(vspace),
    XtRImmediate, (XtPointer) 1 },

  { XtNhspace, XtCBorderWidth, 
    XtRDimension, sizeof(Dimension), offset(hspace),
    XtRImmediate, (XtPointer) 1 },

  { XtNlabelFont,  XtCXftFont, XtRString, sizeof(String),
   offset(fontnames[LABEL]),XtRString, NULL},
  { XtNunitFont,  XtCXftFont, XtRString, sizeof(String),
   offset(fontnames[UNIT]),XtRString, NULL},
  { XtNtextFont,  XtCXftFont, XtRString, sizeof(String),
   offset(fontnames[TEXT]),XtRString, NULL},

  { XtNlabelStr,  XtCLabel, XtRString, sizeof(String),
   offset(str[LABEL]),XtRString, NULL},
  { XtNunitStr,  XtCLabel, XtRString, sizeof(String),
   offset(str[UNIT]),XtRString, NULL},
  { XtNtextStr,  XtCLabel, XtRString, sizeof(String),
   offset(str[TEXT]),XtRString, NULL},

  {XtNlabelGravity, XtCGravity, XtRGravity, sizeof(XtGravity),
   offset(clabel[LABEL].gravity), XtRImmediate, (XtPointer)CenterGravity},
  {XtNunitGravity, XtCGravity, XtRGravity, sizeof(XtGravity),
   offset(clabel[UNIT].gravity), XtRImmediate, (XtPointer)CenterGravity},
  {XtNtextGravity, XtCGravity, XtRGravity, sizeof(XtGravity),
   offset(clabel[TEXT].gravity), XtRImmediate, (XtPointer)CenterGravity},

  {XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
         offset(notify_cb_list), XtRImmediate, (XtPointer) NULL},


#undef offset
};

static void initialize(Widget, Widget, ArgList, Cardinal*);
static void destroy(Widget w);
static XtGeometryResult query_geometry(Widget w, XtWidgetGeometry *intended, XtWidgetGeometry *preferred);
static Boolean set_values(Widget current, Widget req, Widget new, ArgList args, Cardinal *num_args);
static void realize(Widget w, XtValueMask *mask, XSetWindowAttributes *attr);
static void resize(Widget w);
static void expose( Widget w, XEvent *event, Region region );

static void highlight_act(Widget w, XEvent* e, String* s, Cardinal* n);
static void reset_act(Widget w, XEvent* e, String* s, Cardinal* n);
static void notify_act(Widget w, XEvent* e, String* s, Cardinal* n);
static void key_act(Widget w, XEvent* e, String* s, Cardinal* n);
static void increment_act(Widget w, XEvent* e, String* s, Cardinal* n);
static void decrement_act(Widget w, XEvent* e, String* s, Cardinal* n);
static void paint( Widget w, int select );
static void redraw_labels(Widget w);

static XtActionsRec actions[] =
{
  {"highlight",	      highlight_act},
  {"reset",           reset_act},
  {"notify",          notify_act},
  {"key",	      key_act},
};

static char translations[] =
  "<BtnDown>:	        notify()";

struct ctrl_widget_function_st CTRL_WIDGET_CLASS = {
  /* core */
  {
    (WidgetClass)&widgetClassRec,	/* superclass */
    "Ctrl",				/* class_name */
    sizeof(CtrlWidget_t),		/* widget_size */
    NULL,				/* class_initialize */
    NULL,				/* class_part_initialize */
    False,				/* class_inited */
    initialize,				/* initialize */
    NULL,				/* initialize_hook */
    realize,				/* realize */
    actions,				/* actions */
    XtNumber(actions) ,			/* num_actions */
    resources,				/* resources */
    XtNumber(resources),		/* num_resources */
    NULLQUARK,				/* xrm_class */
    True,				/* compress_motion */
    True,				/* compress_exposure */
    True,				/* compress_enterleave */
    False,				/* visible_interest */
    destroy,			        /* destroy */
    resize,				/* resize */
    expose,				/* expose */
    set_values,				/* set_values */
    NULL,				/* set_values_hook */
    XtInheritSetValuesAlmost,		/* set_values_almost */
    NULL,				/* get_values_hook */
    NULL,				/* accept_focus */
    XtVersion,				/* version */
    NULL,				/* callback_private */
    translations,			/* translations */
    query_geometry,			/* query_geometry */
    XtInheritDisplayAccelerator,	/* display_accelerator */
    NULL,				/* extension */
  },
  /* ctrl */
  {
    NULL,				/* extension */
  }
};

WidgetClass ctrlWidgetClass = (WidgetClass)& CTRL_WIDGET_CLASS;


static void resize(Widget w)
{
  TF();
  // layout based on the new width and height of the widget's
  // window.
  do_layout(w);
  TRACE(2, "%s %u %u", ((CtrlWidget)w)->core.name, XtWidth(w), XtHeight(w));
}

static void
make_set_values_request(Widget w, unsigned int width, unsigned int height)
{
    CtrlWidget smw = (CtrlWidget)w;
    Arg arglist[2];
    Cardinal num_args = 0;
    
    if (!smw->ctrl.recursive_set_values) {
        if (XtWidth(smw) != width || XtHeight(smw) != height) {
            smw->ctrl.recursive_set_values = True;
            XtSetArg(arglist[num_args], XtNwidth, width);   num_args++;
            XtSetArg(arglist[num_args], XtNheight, height); num_args++;
            XtSetValues(w, arglist, num_args);
        }
        else if (XtIsRealized((Widget)smw))
            expose((Widget)smw, NULL, NULL);
    }
    smw->ctrl.recursive_set_values = False;
}


static void alloc_xft_color( Widget w, Pixel fg, XftColor *col )
{
  Display *dpy = XtDisplay(w);
  XRenderColor xre_color;
  TRACE(2,"%X",fg);
  xre_color.red = (fg >> 8) & 0xff00;
  xre_color.green = (fg) & 0xff00;
  xre_color.blue = (fg <<8) & 0xff00;
  xre_color.alpha = 0xffff;
  
  XftColorAllocValue(dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
		     None, &xre_color, col );
}


static void free_xft_color(Widget w, XftColor *col)
{
  Display *dpy = XtDisplay(w);
  TF();
  XftColorFree(dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
	       None, col );
}
 


static int update_labels(Widget w)
{
  int i, ent;
  CtrlPart *c = & ((CtrlWidget)w)->ctrl;
  TF();

  ent=0;
  for(i=0; i<MAX_CLABEL;i++) {
    if( strncmp( c->clabel[i].str, c->msg.str[i], MAX_CLABEL_STR ) ) {
      strncpy( c->clabel[i].str, c->msg.str[i], MAX_CLABEL_STR );
      ent |= (1 << i);
    }
  }
  return ent;
}

static void realize(Widget w, XtValueMask *mask, XSetWindowAttributes *attr)
{
  CtrlPart *c = & ((CtrlWidget)w)->ctrl;
  Display *dpy = XtDisplay(w);
  int i;
  Dimension width, height,w1,h1;
  TF();

  /*
  if( c->isflip ) {
    c->msg.cmd=MSG_UPDATE;
    XtCallCallbacks( w, XtNcallback, (XtPointer) &c->msg );
    update_labels(w);
    calc_widget_size(w, &width, &height);
    if( width > XtWidth(w) || height > XtHeight(w) ) {
      TRACE(2,"resizing widget");
      for(i=0;i<2;i++) {
	XtGeometryResult geo = XtMakeResizeRequest( w, width, height, &w1, &h1 );
	if( geo == XtGeometryAlmost ) { width=w1; height=h1; }
	else break;
      }
      XtWidth(w) = width;
      XtHeight(w) = height;
    }
  }
  */

  do_layout(w);
  (*XtClass(w)->core_class.superclass->core_class.realize)(w, mask,attr); 
  c->draw = XftDrawCreate(dpy, XtWindowOfObject(w),
                          DefaultVisual(dpy, DefaultScreen(dpy)), None);


  TRACE(2, "%s %u %u", ((CtrlWidget)w)->core.name, XtWidth(w), XtHeight(w));

}

static void destroy(Widget w)
{
  int i;
  CtrlPart *c = & ((CtrlWidget)w)->ctrl;
  Display *dpy = XtDisplay(w);
  clabel_t *cl;
  TF();
  if(! c->draw ) return; 

  XftDrawDestroy( c->draw ); c->draw =0; 
  for(i=0;i<3;i++) {  
    XftColorFree(dpy, DefaultVisual(dpy, DefaultScreen(dpy)), None, c->col+i );
    XtReleaseGC( w, c->gc[i] ); c->gc[i] =0;
  }

  for(i=0;i<MAX_CLABEL;i++) {
    cl = c->clabel+i;

    if( cl->font ) { 
      XftFontClose(dpy, cl->font ); cl->font = 0; 
    }

  }
}


static void Change_Font(Widget w, int n, char *s)
{
  CtrlWidget cw = (CtrlWidget)w;
  CtrlPart *c = & cw->ctrl;
  Display *dpy = XtDisplay(w);
  clabel_t *cl = c->clabel+n;
  XftFont *font;
  TF();
  TRACE(2,"%s",s);

  if( IsEmpty(s) ) s="Sans-12";
  font = XftFontOpenName(dpy, DefaultScreen(dpy), s );
  if(!font ) 
    XtAppError(XtWidgetToApplicationContext(w), "CtrlWidget: unknown font" );
  else {
    if( cl->font ) XftFontClose( dpy, cl->font);
    cl->font = font;
  }
}

/*
static void set_widget_size( FlipWidget tw )
{
  FlipPart *tkb = & tw->flip;  
  Display *dpy = XtDisplay((Widget)tw);
  if( !dpy || !tw->flip.font ) {
    tw->core.width=tw->core.height=1;
    WARN("widget is not initialized");
    return;
  }

  char *s = tkb->label;
  XftTextExtentsUtf8(dpy, tkb->font, (FcChar8*)  
		     s, strlen(s), & tkb->extents );
  tw->core.width = tkb->extents.xOff;
  tw->core.height = tkb->font->height;  
}
*/


static void do_layout(Widget w)
{
  CtrlWidget cw = (CtrlWidget)w;
  CtrlPart *c = & cw->ctrl;
  Display *dpy = XtDisplay(w);
  clabel_t *cl,*cu,*ct;
  char *s;
  Dimension total_w=0, total_h=0, stretch=0;
  TF();

  total_w = XtWidth(w);
  total_h = XtHeight(w);

  // top-left
  cl = c->clabel+LABEL;
  s=cl->str;
  if( IsEmpty(s) || !cl->font ) {
    cl->width=0; cl->height=0;
    cl->x=0; cl->y=0;
  }
  else {
    cl->x=0; cl->y = 0; 
    cl->height = cl->font->height;
    cl->width = cl->extents.xOff;
  }

  // top-right
  cu = c->clabel+UNIT;
  s=cu->str;
  if( IsEmpty(s) || !cu->font ) {
    cu->width=0; cu->height=0;
    cu->x=0; cu->y=0;
  }
  else {
    cu->width = cu->extents.xOff;
    cu->x = total_w - cu->width;
    cu->y = 0; 
    cu->height = cu->font->height;
  }
  
  int top_h = Max(cl->height, cu->height );

  // center bottom
  ct = c->clabel+TEXT;
  s=ct->str;
  if( ct->font && !IsEmpty(s)) {
    ct->width = total_w;
    ct->height = total_h - top_h;
    ct->x = 0;
    ct->y = top_h;
    calc_str_position( ct );
  }
  else {
    ct->width = ct->height = ct->x=ct->y=0;
  }

}

static void calc_widget_size(Widget w, Dimension *width, Dimension *height )
{
  CtrlWidget cw = (CtrlWidget)w;
  CtrlPart *c = & cw->ctrl;
  Display *dpy = XtDisplay(w);
  clabel_t *cl,*cu,*ct;
  char *s;
  Dimension total_w=0, total_h=0, stretch=0;
  TF();


  total_w = total_h = 0;

  // top-left
  cl = c->clabel+LABEL;
  s=cl->str;
  if( ! IsEmpty(s) && cl->font ) {
    XftTextExtentsUtf8(dpy, cl->font, (FcChar8*)s,  
		       strlen(s), & cl->extents );
    total_h = cl->font->height;
    total_w = cl->extents.xOff;
  }
  total_w += c->hspace +1;
  
  // top-right
  cu = c->clabel+UNIT;
  s=cu->str;
  if(! IsEmpty(s)  && cu->font ) {
    XftTextExtentsUtf8(dpy, cu->font, (FcChar8*)s,  
                       strlen(s), & cu->extents );
    total_w += cu->extents.xOff;    
    total_h = Max( cu->font->height, total_h );
  }
  total_h += c->vspace;
  
  // center bottom
  ct = c->clabel+TEXT;
  s=ct->str;
  if(! IsEmpty(s)  && ct->font ) {
    XftTextExtentsUtf8(dpy, ct->font, (FcChar8*)s,  
                       strlen(s), & ct->extents );
    
    total_w = Max( total_w,ct->extents.xOff ); 
    total_h += ct->font->height;
  }

  *width = total_w;
  *height = total_h;

  TRACE(2,"%s %u %u", cw->core.name, total_w, total_h );

}


static GC get_gc(Widget w, Pixel fg)
{
  XGCValues     values;
  TRACE(2,"background fill color: %x", fg );

  values.foreground   = fg;
  return XtGetGC(w, GCForeground, &values);
}

/* render text
   position - 0 left, 1-center
*/
static void draw_str( Widget w, int clear_area, enum CLABEL n )
{
  CtrlWidget cw = (CtrlWidget)w;
  CtrlPart *c = & cw->ctrl;
  Display *dpy = XtDisplay(w);
  clabel_t *cl = c->clabel + n;
  char *s;
  Dimension x,y;

  TF();

  if( ! cl->width || ! cl->height ) return; 

  if( clear_area ) {
    TRACE(2, "%s %d", cw->core.name, c->msg.state );

    XFillRectangle( XtDisplay(w), XtWindow(w), c->gc[c->msg.state], 
		    cl->x, cl->y, cl->width, cl->height );  
  }

  s = cl->str;
  if( IsEmpty(s) ) return;

  x = cl->x;
  y = cl->y + cl->font->ascent;
  if( cl->position == 1 ) // center label
    {
      XftTextExtentsUtf8(dpy, cl->font, (FcChar8*)s,  
		     strlen(s), & cl->extents );
      x+=(cl->width - cl->extents.xOff) / 2; 

      calc_str_position( cl );

      TRACE(2, "%s (%u %u) (%u %u)", cw->core.name,
	    cl->x, cl->y,
	    cl->str_x, cl->str_y );
      

      XftDrawStringUtf8(c->draw, c->col+c->msg.state, cl->font, 
			cl->x + cl->str_x,
			cl->y + cl->str_y,
			(FcChar8*)s,strlen(s) ); 
      return;
    }

  XftDrawStringUtf8(c->draw, c->col+c->msg.state, cl->font, x,y,
		    (FcChar8*)s,strlen(s) ); 
}
 


#define SafeStrcpy( dst, src, n ) strncpy( dst, src && *src ? src : "", n );


/* Initializes widget instance
 *
 * Parameters:
 *	request - requested widget
 *	w	- the widget
 *	args	- arguments
 *	num_args - number of arguments
 *
 */
static void initialize(Widget request, Widget w, ArgList args, Cardinal *num_args)
{
  CtrlWidget cw = (CtrlWidget)w;
  CtrlPart *c = & cw->ctrl;
  // Display *dpy = XtDisplay(w);
  int i;
  Dimension width,height;
  TF();  
  c->draw  = 0;

  CtrlWidget cw1 = (CtrlWidget)request;
  CtrlPart *c1 = & cw1->ctrl;

  // foreach state
  for(i=0;i<3;i++) {
    c->gc[i] = get_gc( w, c->pixel[3+i] );
    alloc_xft_color( w, c->pixel[i], c->col+i );
  }

  // foreach label
  for(i=0;i<MAX_CLABEL;i++) {
    Change_Font(w, i, c->fontnames[i] );
  }
  
  c->msg.state = STATE_NORMAL;
  c->msg.cmd   = MSG_UPDATE;
  c->msg.value  = 0;

  if( IsEmpty( c->str[TEXT] )) c->str[TEXT] = cw->core.name;

  for(i=0;i<MAX_CLABEL;i++) {
    SafeStrcpy( c->clabel[i].str, c->str[i], MAX_CLABEL_STR ); 
    c->clabel[i].position=0;
  }
  c->clabel[TEXT].position=1;


  calc_widget_size(w, &width,&height );

  if (cw->core.height == 0)
    cw->core.height = height;
  if (cw->core.width == 0)
    cw->core.width = width;
  
  // (*XtClass(w)->superclass->core_class.resize)(w);
}


// falls select == 0 wird kein label gezeichnet
// falls select = ((1 << MAX_CLABEL)-1) -
//   wird der hintergrund komplett gefüllt und alle labels werden gezeichnet
// sonst wird nur das geänderte label gezeichnet
//
static void paint(Widget w, int select )
{
#define ALL_LABELS ((1 << MAX_CLABEL)-1)
  CtrlWidget cw = (CtrlWidget)w;
  CtrlPart *c = & cw->ctrl;
  TF();

  int i,clear_area = 1;
  if( (select & ALL_LABELS)==ALL_LABELS ) {
    TRACE(2, "%s %d", cw->core.name, c->msg.state );
    XFillRectangle( XtDisplay(w), XtWindow(w), c->gc[c->msg.state], 
                    0, 0, XtWidth(w), XtHeight(w) );
    clear_area=0; 
  }
  for(i=0;i<MAX_CLABEL;i++) {
    if( select &  ( 1 << i ) ) draw_str(w,clear_area,i);
  }
}

static void expose( Widget w, XEvent *event, Region region )
{  
  CtrlWidget cw = (CtrlWidget)w;
  CtrlPart *c = & cw->ctrl;
  TF();
  /*
   * Check to see whether or not the widget's window is mapped.  You can't
   * draw into a window that is not mapped.  Realizing a widget doesn't 
   * mean its mapped, but this call will work for most Window Managers.
   */
  if (!XtIsRealized(w))
    return;
  
  paint( w, ((1 << MAX_CLABEL)-1) ); // paint all

}

static void highlight_act(Widget w, XEvent* e, String* s, Cardinal* n)
{
  CtrlWidget cw = (CtrlWidget)w;
  CtrlPart *c = & cw->ctrl;  
  TF();
  if( c->msg.state == STATE_SELECTED ) return;
  c->msg.state = STATE_SELECTED;
  expose(w, NULL, 0);
}

static void reset_act(Widget w, XEvent* e, String* s, Cardinal* n)
{
  CtrlWidget cw = (CtrlWidget)w;
  CtrlPart *c = & cw->ctrl;  
  TF();
  if( c->msg.state == STATE_NORMAL ) return;
  c->msg.state = STATE_NORMAL;
  expose(w, NULL, 0);
  /* disable virtual keyboard */
  sig_send( w, VIS_REMOVE_SLIDER, NULL  ); 
}

static void notify_act(Widget w, XEvent *event, String *params, Cardinal *n)
{
  CtrlWidget cw = (CtrlWidget)w;
  CtrlPart *c = & cw->ctrl;  
  TF();

  if( ! c->isflip ) {
    c->msg.cmd = MSG_FIRE;
    XtCallCallbacks( w, XtNcallback, (XtPointer) & c->msg );
  }
  else {
    if( c->msg.state == STATE_ARMED ) {
      c->msg.state = STATE_SELECTED;
      c->msg.cmd = MSG_FIRE;
      XtCallCallbacks( w, XtNcallback, (XtPointer) & c->msg );
    }
    else {
      c->msg.state = STATE_ARMED;
      /* make virtual keyboard visible */
      vis_setup_slider_t msg;
      msg.min = c->msg.min_value;
      msg.max = c->msg.max_value;
      msg.cur = c->msg.value;
      sig_send( w, VIS_SETUP_SLIDER, &msg );
    }
  }

  expose(w, NULL, 0);
}


static XtGeometryResult query_geometry(Widget w, XtWidgetGeometry *intended, 
				       XtWidgetGeometry *preferred)
{  
  Dimension width,height;
  CtrlWidget cw = (CtrlWidget)w;
  CtrlPart *c = & cw->ctrl;  

  if( c->isflip ) { 
    c->msg.cmd=MSG_UPDATE;
    XtCallCallbacks( w, XtNcallback, (XtPointer) &c->msg );
    update_labels(w);
  }

  calc_widget_size(w, &width, &height);
  preferred->request_mode = CWWidth | CWHeight;
  // preferred->width = Max( width, cw->core.width );
  // preferred->height = Max( height, cw->core.height );
  preferred->width = width;
  preferred->height = height;
  
  TRACE(2, "%s %u %u", cw->core.name,   preferred->width,
	preferred->height );



  if (  ((intended->request_mode & (CWWidth | CWHeight))
         == (CWWidth | CWHeight)) &&
        intended->width == preferred->width &&
        intended->height == preferred->height)
    return XtGeometryYes;
  else if (preferred->width == w->core.width &&
           preferred->height == w->core.height)
    return XtGeometryNo;
  else
    return XtGeometryAlmost;
}

static Boolean set_values(Widget current, Widget req, Widget new, ArgList args, Cardinal *num_args)
{
  TF();
  int i;
  Dimension width, height,w1,h1;

  /* TODO: check set value and redraw labels 

  c->msg.cmd=MSG_UPDATE;
  XtCallCallbacks( w, XtNcallback, (XtPointer) &c->msg );
  update_labels(w);
  redraw_labels(new); return False;
  */

  calc_widget_size(new, &width, &height);
  TRACE(2,"cur: (%u,%u) new: (%u %u)", XtWidth(new), XtHeight(new), 
        width, height );

  if( width != XtWidth(new) || height != XtHeight(new) ) {
    for(i=0;i<2;i++) {
      XtGeometryResult geo = XtMakeResizeRequest( new, width, height, &w1, &h1 );
      if( geo == XtGeometryAlmost ) { width=w1; height=h1; }
      else break;
    }   
    XtWidth(new) = width;
    XtHeight(new) = height;
  }

  return TRUE;
}

static void key_act(Widget w, XEvent* e, String* s, Cardinal* n)
{
  char *s1;
  s1 = *n && s && *s && **s ? *s : "" ;
  TRACE(2,"key:%s", s1);
}

static void redraw_labels(Widget w)
{
  int ent = update_labels(w);   // check which label has changed
  TRACE(2,"%u", ent );
  paint( w, ent );              // repaint changed labels 
}

static void increment_act(Widget w, XEvent* e, String* s, Cardinal* n)
{
  TF();
  CtrlWidget cw = (CtrlWidget)w;
  CtrlPart *c = & cw->ctrl;  
  c->msg.cmd = MSG_INC;
  XtCallCallbacks( w, XtNcallback, (XtPointer) & c->msg );
  redraw_labels(w);
}

static void decrement_act(Widget w, XEvent* e, String* s, Cardinal* n)
{
  TF();
  CtrlWidget cw = (CtrlWidget)w;
  CtrlPart *c = & cw->ctrl;  
  c->msg.cmd = MSG_DEC;
  XtCallCallbacks( w, XtNcallback, (XtPointer) & c->msg );
  redraw_labels(w);
}


// returns TRUE if widget wants this wheel event
int wheel_input(Widget w, char *key)
{
  TF();
  Cardinal null;
  CtrlWidget cw = (CtrlWidget)w;
  CtrlPart *c = & cw->ctrl;  

  if( ! XtIsSubclass( w, ctrlWidgetClass ) ) return 0;

  if(! c->isflip ) {
    if( strcmp(key,"Fire") == 0 ) {
      notify_act(w, NULL,NULL, &null );
      return 1;
    }
    return 0;
  }
  
  if( c->msg.state == STATE_ARMED ) 
    {
      if( strcmp(key,"Inc") == 0 ) increment_act(w,NULL,NULL, &null );
      else if( strcmp(key,"Dec") == 0 ) decrement_act(w,NULL,NULL, &null );
      else if( strcmp(key,"Fire") == 0 ) notify_act(w, NULL,NULL, &null );
      return 1;
    }
  
  return 0;
}



static void calc_str_position( clabel_t *cl )
{
  int x,y;
  int sw = cl->extents.xOff;
  int sh = cl->font->height;
  int bw = cl->width;
  int bh = cl->height;

  int eh = bh - sh;
  int ew = bw - sw;

  x=0; 
  y=0;

  switch( cl->gravity ) {

  case WestGravity:
    y = eh / 2;
    break;
  case NorthWestGravity:
    break;
  case CenterGravity: 
    x = ew / 2;
    y = eh / 2;
    break;

  case NorthGravity:
    x = ew / 2;
    break;

  case SouthGravity:
    y = eh;
    x = ew / 2;
    break ;
  
  case EastGravity:
    x = ew;
    y = eh / 2;
    break;

  case NorthEastGravity: 
    x = ew;
    break;

  case SouthEastGravity:
    x = ew;
    y = eh;
    break;

  case SouthWestGravity: 
    y=eh;
  
  }

  y += cl->font->ascent;
  cl->str_x = x;
  cl->str_y = y;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <locale.h>

#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/Xos.h>
#include <X11/CoreP.h>
#include <X11/Xft/Xft.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/XawInit.h>

#include "converters-xft.h"
#include "xftdef.h"
#include "MiniW.h"

// calculate (offset)address of field inside our widget structure
#define OFFSET(field) XtOffsetOf(MiniWidget_t, mini.field)
/****************************************************
 *                                                  *
 *	implementation                              *
 *                                                  *
 ****************************************************/

static void initialize(Widget request, Widget cnew,
		       ArgList args, Cardinal *num_args);

static void destroy(Widget w);

static Boolean set_values(Widget current, Widget request, Widget cnew,
			  ArgList args, Cardinal *num_args);

static void expose(Widget w, XEvent *event, Region region);

static XtGeometryResult query_geometry(Widget w, XtWidgetGeometry *intended,
				       XtWidgetGeometry *preferred);


/****************************************************
 *	init our class                              *
 ****************************************************/

/* {name, class, type, size, offset, default_type, default_addr}, */
static XtResource resources[] = {
  { XtNlabel, XtCLabel, XtRString, sizeof(String),
    OFFSET(label), XtRString, "Hello World" },
  { XtNwidth, XtCWidth, XtRInt, sizeof(int),
    OFFSET(w), XtRImmediate, (XtPointer)50 },
  { XtNheight, XtCHeight, XtRInt, sizeof(int),
    OFFSET(h), XtRImmediate, (XtPointer)25 },
  { XtNxftFont, XtCXftFont, XtRXftFont, sizeof (XftFont *),
    OFFSET (font), XtRString, "Sans-20"},
  { XtNforeground, XtCForeground, XtRXftColor, sizeof (XftColor),
    OFFSET (color), XtRString, XtDefaultForeground }
};

MiniWidgetClass_t MINI_WIDGET_CLASS = {
  { /* core */
    (WidgetClass)&widgetClassRec,	/* superclass */
    "Mini",				/* class_name */
    sizeof(MiniWidget_t),		/* widget_size */
    NULL,				/* class_initialize */
    NULL,				/* class_part_initialize */
    False,				/* class_inited */
    initialize,				/* initialize */
    NULL,				/* initialize_hook */
    XtInheritRealize,			/* realize */
    NULL,				/* actions */
    0,					/* num_actions */
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
    NULL,				/* translations */
    query_geometry,			/* query_geometry */
    XtInheritDisplayAccelerator,	/* display_accelerator */
    NULL,				/* extension */
  },
  /* Mini */
  {
    NULL,				/* extension */
  }
};

/****************************************************
 *	THIS is the interface to the APPLICATION    *
 ****************************************************/
WidgetClass  miniWidgetClass = (WidgetClass)&MINI_WIDGET_CLASS;



/****************************************************
 *	I M P L E M E N T A T I O N                 *
 ****************************************************/

static void calc_widget_size(MiniWidget mw, int *w, int *h)
{
  MiniPart   *mp = & mw->mini; 
  Display *dpy = XtDisplay(mw);

  XftTextExtentsUtf8(dpy, mp->font, (FcChar8*)  
		     mp->label, strlen(mp->label), & mp->extents );
  
  *w = mp->extents.width; // also extents.xOff
  *h = mp->font->height;
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
  TF();
  MiniWidget mw = (MiniWidget)cnew;
  MiniPart   *mp = & mw->mini; 
  Display *dpy = XtDisplay(mw);
  int screen_num = DefaultScreen(dpy);

  /* init our private/public members */
  if( mp->label == 0 || mp->label[0] == 0 ) mp->label = mw->core.name;
  
  calc_widget_size(mw, &mp->w, &mp->h);

  /* set current widget size, can be resized later with set_values */
  if (mw->core.height == 0)
    mw->core.height = mp->h;
  if (mw->core.width == 0)
    mw->core.width = mp->w;  
}

static void destroy(Widget w)
{
  MiniWidget mw = (MiniWidget)w;
  MiniPart   *mp = & mw->mini; 
  Display *dpy = XtDisplay(mw);
  int screen_num = DefaultScreen(dpy);

  TF();

  XftDrawDestroy( mp->draw );

}


/* set widget size to *w / *h. ask our manager widget for 
   possible sizes. if the manager offers a compromise use
   this values.
 */
static void reset_widget_size( MiniWidget mw, int *w, int *h )
{
  int i;  
  Dimension width,height,h1,w1;
  width=*w; height=*h;

  TRACE(2,"Req %d %d", *w, *h );
    
  for(i=0;i<2;i++) {
    XtGeometryResult geo = XtMakeResizeRequest( (Widget)mw, 
						width, height, &w1, &h1 );
    if( geo == XtGeometryAlmost ) { width=w1; height=h1; }
    else break;
  }   
  mw->core.width = *w = width;
  mw->core.height = *h = height;
  TRACE(2,"Grant %d %d", *w, *h );
}

static Boolean set_values(Widget current, Widget request, Widget cnew,
			  ArgList args, Cardinal *num_args)
{
  MiniWidget mw = (MiniWidget)cnew;
  MiniPart   *mp = & mw->mini; 
  int i;
  int re_layout = 0;

  TF();
  for(i=0;i<*num_args;i++) {
    printf( "Name=%s Value=%d\n", args[i].name, (int) args[i].value );    
    if( strcmp(args[i].name, XtNlabel )==0) re_layout=1;
  } 

  if( re_layout ) {
    calc_widget_size( mw, & mp->w, &mp->h );
    reset_widget_size(  mw, & mp->w, & mp->h );
  }

  return re_layout ? TRUE : FALSE; // should we RE-DISPLAY this WIDGET ?
}


static void expose(Widget w, XEvent *event, Region region)
{
  Display *dpy = XtDisplay(w);
  int screen = DefaultScreen(dpy);
  MiniWidget mw = (MiniWidget)w;
  MiniPart   *mp = & mw->mini; 
  TF();
  if( !XtIsRealized(w)) return;	/* if there is no window, do not draw into void */
  // paint 
  if( !mp->draw ) { // we couldn't init without a window
    // this can only be executed after the widget is realized.
    // we could put this function into realize(), but this would add complexity.
    mp->draw = XftDrawCreate( dpy, XtWindow(w),
			      DefaultVisual(dpy, screen), None);
    if( !mp->draw ) ERR("XftDrawCreate returns NULL");
  }
  
  XClearArea (dpy, XtWindow(w),
	      0, 0, w->core.width, w->core.height,
	      False);

  XftDrawStringUtf8 (mp->draw,
		     &mp->color,
		     mp->font,
		     0, mp->font->ascent,
		     (FcChar8 *)mp->label, strlen(mp->label) ); 
}

/* tell the manager widget, that we want at least the size ( mp->w, mp->h ). 
 */
static XtGeometryResult query_geometry(Widget w, XtWidgetGeometry *intended,
				       XtWidgetGeometry *preferred)
{
    MiniWidget mw = (MiniWidget)w;
    MiniPart   *mp = & mw->mini; 
    TF();

    preferred->request_mode = CWWidth | CWHeight;
    preferred->width = mp->w;
    preferred->height = mp->h;

    if (((intended->request_mode & (CWWidth | CWHeight)) == (CWWidth | CWHeight))
	&& intended->width == preferred->width
	&& intended->height == preferred->height)
	return (XtGeometryYes);
      else if (preferred->width == mw->core.width && preferred->height == mw->core.height)
	return (XtGeometryNo);

    return (XtGeometryAlmost);
}

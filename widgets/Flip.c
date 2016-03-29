/*

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <X11/Xft/Xft.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Label.h>

#include "FlipP.h"
#include "mls.h"

static XtResource resources[] = {
#define offset(field) XtOffsetOf(FlipRec, flip.field)
	/* {name, class, type, size, offset, default_type, default_addr}, */

  { XtNnormalFg, XtCColor, XtRPixel, sizeof(Pixel),
    offset(pixel[0]), XtRString, XtDefaultForeground },
  { XtNnormalBg, XtCColor, XtRPixel, sizeof(Pixel),
    offset(pixel[3]), XtRString, XtDefaultBackground },

  { XtNselectedFg, XtCColor, XtRPixel, sizeof(Pixel),
    offset(pixel[1]), XtRString, XtDefaultForeground },
{ XtNselectedBg, XtCColor, XtRPixel, sizeof(Pixel),
    offset(pixel[4]), XtRString, XtDefaultBackground },

  { XtNarmedFg, XtCColor, XtRPixel, sizeof(Pixel),
    offset(pixel[2]), XtRString, XtDefaultForeground },
  { XtNarmedBg, XtCColor, XtRPixel, sizeof(Pixel),
    offset(pixel[5]), XtRString, XtDefaultBackground },

  {XtNentryList,  XtCLabel, XtRString, sizeof(String),
   offset(label), XtRString, (XtPointer) NULL},
  
  { XtNxftFont,  XtCXftFont, XtRString, sizeof(String),
   offset(xftfontname),XtRString, NULL},

  {XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
         offset(notify_cb_list), XtRImmediate, (XtPointer) NULL},
  {XtNfocusCB, XtCCallback, XtRCallback, sizeof(XtPointer),
         offset(focus_cb_list), XtRImmediate, (XtPointer) NULL},
  {XtNupdateCB, XtCCallback, XtRCallback, sizeof(XtPointer),
         offset(update_cb_list), XtRImmediate, (XtPointer) NULL},

#undef offset
};


/*
 * Class Methods
 */
static void FlipInitialize(Widget, Widget, ArgList, Cardinal*);
static void FlipDestroy(Widget w);
static XtGeometryResult
QueryGeometry(Widget w, XtWidgetGeometry *intended, XtWidgetGeometry *preferred);
static Boolean set_values(Widget current, Widget req, Widget new, ArgList args, Cardinal *num_args);


/*
 * Prototypes

 */
// static Bool FlipFunction(FlipWidget, int, int, Bool);


/*
 * Actions
 */
static void TemplateAction(Widget w, XEvent* e, String* s, Cardinal* n);
static void HighlightAction(Widget w, XEvent* e, String* s, Cardinal* n);
static void ResetAction(Widget w, XEvent* e, String* s, Cardinal* n);
static void NotifyAction(Widget w, XEvent* e, String* s, Cardinal* n);
static void FocusOutAction(Widget w, XEvent* e, String* s, Cardinal* n);
static void FocusInAction(Widget w, XEvent* e, String* s, Cardinal* n);
static void KeyAction(Widget w, XEvent* e, String* s, Cardinal* n);


static XtActionsRec actions[] =
{
    /*{name,		procedure},*/
    {"template",	TemplateAction},
    {"highlight",	HighlightAction},
    {"reset",           ResetAction},
    {"notify",          NotifyAction},
    {"focus_in",	FocusInAction},
    {"focus_out",	FocusOutAction},
    {"key",		KeyAction},
    
};

static char translations[] =
  "<BtnDown>:	        notify()   \n"
  "<EnterWindow>:       highlight() focus_in() \n"
  //  "<Key>:               key() \n"
  "<LeaveWindow>:       reset() focus_out() ";

static void expose( Widget w, XEvent *event, Region region );

#define Superclass	(&widgetClassRec)
FlipClassRec flipClassRec = {
  /* core */
  {
    (WidgetClass)Superclass,		/* superclass */
    "Flip",				/* class_name */
    sizeof(FlipRec),		/* widget_size */
    NULL,				/* class_initialize */
    NULL,				/* class_part_initialize */
    False,				/* class_inited */
    FlipInitialize,			/* initialize */
    NULL,				/* initialize_hook */
    XtInheritRealize,			/* realize */
    actions,				/* actions */
    XtNumber(actions) ,			/* num_actions */
    resources,				/* resources */
    XtNumber(resources),		/* num_resources */
    NULLQUARK,				/* xrm_class */
    True,				/* compress_motion */
    True,				/* compress_exposure */
    True,				/* compress_enterleave */
    False,				/* visible_interest */
    FlipDestroy,			/* destroy */
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
    QueryGeometry,		/* query_geometry */
    XtInheritDisplayAccelerator,	/* display_accelerator */
    NULL,				/* extension */
  },
  /* flip */
  {
    NULL,				/* extension */
  }
};

WidgetClass flipWidgetClass = (WidgetClass)&flipClassRec;


static char *Get_Label(Widget w)
{
  FlipWidget tw = (FlipWidget)w;  
  return ( tw->flip.label && tw->flip.label[0] ) ? tw->flip.label : "None";
}


static GC Get_GC(Widget w, Pixel fg)
{
  XGCValues     values;

  values.foreground   = fg;
  return XtGetGC(w, GCForeground, &values);
}

/*
 * Implementation
 */
/*
 * Function:
 *	FlipInitialize
 *
 * Parameters:
 *	request - requested widget
 *	w	- the widget
 *	args	- arguments
 *	num_args - number of arguments
 *
 * Description:
 *	Initializes widget instance.
 */
/*ARGSUSED*/


static void col_alloc( Widget w, int num )
{
  Display *dpy = XtDisplay(w);
  XRenderColor xre_color;
  FlipWidget tw = (FlipWidget)w;

  Pixel fg = tw->flip.pixel[num]; // foreground
  xre_color.red = (fg >> 8) & 0xff00;
  xre_color.green = (fg) & 0xff00;
  xre_color.blue = (fg <<8) & 0xff00;
  xre_color.alpha = 0xffff;
  
  XftColorAllocValue(dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
                       None, &xre_color, &tw->flip.xftCol[num]);
  // background
  tw->flip.gc[num] = Get_GC(w, tw->flip.pixel[num+3] );
}

static void col_free(Widget w, int num)
{
  Display *dpy = XtDisplay(w);
  FlipWidget tw = (FlipWidget)w;

  XftColorFree(dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
	       None, &tw->flip.xftCol[num]);
  XtReleaseGC( w,tw->flip.gc[num] );
  tw->flip.gc[num]=0;
}

static void FlipPostInit(Widget w)
{
  int i;
  FlipWidget tw = (FlipWidget)w;
  Display *dpy = XtDisplayOfObject(w);

  tw->flip.draw = XftDrawCreate(dpy, XtWindowOfObject(w),
                         DefaultVisual(dpy, DefaultScreen(dpy)), None);
  
  for(i=0;i<3;i++) {
    col_alloc(w,i);  
    TRACE(2,"col %d. %lx %lx", i, tw->flip.pixel[i], tw->flip.pixel[i+3] );
  }

  char *s = Get_Label(w);
  XftTextExtentsUtf8(dpy, tw->flip.font, (FcChar8*)  s, 
                     strlen( s ), &tw->flip.extents);
}

static void FlipDestroy(Widget w)
{
  FlipWidget tw = (FlipWidget)w;
  Display *dpy = XtDisplayOfObject(w);
  int i;

  if(tw->flip.draw) {
    TRACE(2,"");
    // XtFree( tw->flip.label ); tw->flip.label=0;

    for(i=0;i<3;i++) {    
      col_free(w,i);
    }
    XftDrawDestroy( tw->flip.draw ); tw->flip.draw =0;
    XftFontClose(dpy, tw->flip.font ); tw->flip.font = 0;
  }
}

static void Change_Label_Size(Widget w)
{
  Display *dpy = XtDisplayOfObject(w);
  FlipWidget tw = (FlipWidget)w;
  FlipPart *tkb = & tw->flip;
  char *s = Get_Label(w);

  XftTextExtentsUtf8(dpy, tkb->font, (FcChar8*)  s, 
                   strlen(s), & tkb->extents );
}



static void Change_Font(Widget w, char *s)
{
  Display *dpy = XtDisplayOfObject(w);
  FlipWidget tw = (FlipWidget)w;
  FlipPart *tkb = & tw->flip;
  
  TRACE(2,"%s",s);

  if( (!s) || (!*s) ) s="Sans-12";
  XftFont *font = XftFontOpenName(dpy, DefaultScreen(dpy), s );
  if(!font ) XtAppError(XtWidgetToApplicationContext(w), "Flip: unknown font" );
  else {
    if( tkb->font ) XftFontClose( dpy, tkb->font);
    tkb->font = font;
  }
}

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


#define IsEmpty(s) ((s)==NULL || *(s) == 0)

static void
FlipInitialize(Widget request, Widget w, ArgList args, Cardinal *num_args)
{
  FlipWidget tw = (FlipWidget)w;
    // Display *dpy = XtDisplayOfObject(w);

  tw->flip.state = normal;
  tw->flip.draw = NULL; // noch ist das window dieses widgets nicht vorhanden
  tw->flip.value = 0;


    if (IsEmpty(tw->flip.label) )
        tw->flip.label = XtNewString(tw->core.name);
    else
        tw->flip.label = XtNewString(tw->flip.label);

    Change_Font(w,  tw->flip.xftfontname );
    Change_Label_Size(w); 

    if (tw->core.height == 0)
      tw->core.height = tw->flip.font->height +2;
    if (tw->core.width == 0)
      tw->core.width = tw->flip.extents.xOff +2;    
}

static void PaintLabel(Widget w)
{   
  FlipWidget tw = (FlipWidget)w;
  FlipPart *tkb = & tw->flip;
  int width,height,x,y;
  GC gc;
  XftColor *col;

  width=tw->core.width;
  height=tw->core.height;  
  gc = tkb->gc[ tkb->state ];
  col = tkb->xftCol+ tkb->state;

  TRACE(2,"tkb->pixel: F:%lx B:%lx\nLabel:%s", 
	tkb->pixel[tkb->state],
	tkb->pixel[tkb->state+3],
	tkb->label );

  // center label
  x = (width - tkb->extents.xOff) / 2;
  y = (height - (tkb->font->ascent + tkb->font->descent ))/2 + tkb->font->ascent;
  XFillRectangle( XtDisplay(w), XtWindow(w), gc, 0,0, width, height );  
  char *s=Get_Label(w);
  XftDrawStringUtf8(tkb->draw, col, tkb->font, x,y, (FcChar8*)s,strlen(s) ); 
}


static void expose( Widget w, XEvent *event, Region region )
{
  FlipWidget tw = (FlipWidget) w;
  TRACE(2, "%s Expose w=%d h=%d\n", tw->flip.label, tw->core.width, tw->core.height );
  
  // jetzt ist ein günstiger moment alle initialisierungen vorzunehmen
  if( tw->flip.draw == NULL ) 
      FlipPostInit(w);
  PaintLabel(w); 
}

static void TemplateAction(Widget w, XEvent* e, String* s, Cardinal* n)
{
  puts("key");
}


//
// focus_in == 1
//

static void FocusInAction(Widget w, XEvent* e, String* s, Cardinal* n)
{
  XtCallCallbacks( w, XtNfocusCB, (XtPointer) 1 );
}

static void FocusOutAction(Widget w, XEvent* e, String* s, Cardinal* n)
{
  XtCallCallbacks( w, XtNfocusCB, (XtPointer) 0 );
}

static void HighlightAction(Widget w, XEvent* e, String* s, Cardinal* n)
{
  FlipPart *tkb = & ((FlipWidget)w)->flip;
  puts("hi");
  tkb->state = selected;
  PaintLabel(w);
}

static void ResetAction(Widget w, XEvent* e, String* s, Cardinal* n)
{
  FlipPart *tkb = & ((FlipWidget)w)->flip;
  tkb->state = normal;
  puts("reset");
  PaintLabel(w);
}

static void NotifyAction(Widget w, XEvent *event, String *params, Cardinal *n)
{
  FlipPart *tkb = & ((FlipWidget)w)->flip;
  if( tkb->state == armed ) { 
    tkb->state = selected;
    // unregister timer callback
    TRACE(2,"selected, value committed");
  }
  else {
    tkb->state = armed;
    tkb->old_value = tkb->value;
    // save old value, for timer timeout, register timer
    TRACE(2,"armed");
  }
  PaintLabel(w);
  uint32_t ret;
  ret = (tkb->state << 16) + tkb->value;
  XtCallCallbacks( w, XtNcallback, (XtPointer) ret );
}

#define Max(a,b) (a>b ? a : b)
static XtGeometryResult
QueryGeometry(Widget w, XtWidgetGeometry *intended, XtWidgetGeometry *preferred)
{  
  FlipPart *tkb = & ((FlipWidget)w)->flip;
  FlipWidget tw = (FlipWidget) w;
  TRACE(2,"");
  preferred->request_mode = CWWidth | CWHeight;
  // tkb->extents.xOff ist größer als extents.width
  preferred->width = Max( tkb->extents.xOff, tw->core.width );
  preferred->height = Max( tkb->font->height, tw->core.height );
  
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
  FlipWidget tw = (FlipWidget)new;
  FlipPart *tkb_new = & ((FlipWidget)new)->flip;
  FlipPart *tkb_current = & ((FlipWidget)current)->flip;
  int i = *num_args;

  TRACE(2,"");
  while( i-- ) {
    if( strcmp( args[i].name, XtNlabel )==0 )  goto label_change;
    if( strcmp( args[i].name, XtNxftFont )==0 )  goto font_change;
    if( strcmp( args[i].name, XtNnormalFg ) == 0 ) goto normalfg_change;
    continue;

  normalfg_change:
    col_free( new, normal );
    col_alloc( new, normal );
    continue;

  label_change:
    XtFree( tkb_current->label );
    tkb_current->label = 0;
    tkb_new->label = XtNewString( tkb_new->label );
    set_widget_size( tw );
    continue;
    
  font_change:
    Change_Font(new,  tkb_new->xftfontname );
    set_widget_size( tw );
    continue;
  }
  return TRUE;
}

static void set_new_label(Widget w)
{
  char *s1;
  int i;
  FlipPart *tkb = & ((FlipWidget)w)->flip;
  Dimension width,height, w1,h1;
  FlipWidget tw = (FlipWidget)w;

  s1=Get_Label(w);
  XftTextExtentsUtf8(XtDisplay(w), tkb->font, (FcChar8*)  
		     s1, strlen(s1), & tkb->extents );
  width = tkb->extents.xOff;
  height = tkb->font->height;

  if( width > tw->core.width || height > tw->core.height )
    {
      for(i=0;i<2;i++) {
	XtGeometryResult geo = XtMakeResizeRequest( w, width, height, &w1, &h1 );
	if( geo == XtGeometryAlmost ) { width=w1; height=h1; }
	else break;
      }
    }
}


static void update_value( Widget w)
{
  FlipPart *tkb = & ((FlipWidget)w)->flip;
  XtCallCallbacks( w, XtNupdateCB, (XtPointer) & tkb->list );  
  set_new_label(w);
}

static void increment(Widget w)
{
  FlipPart *tkb = & ((FlipWidget)w)->flip;
  // label neu berechnen, größe anpassen, neu zeichnen
  tkb->value++;
  update_value(w);
  expose(w,NULL,NULL);
  TRACE(2,"");
}

static void decrement(Widget w)
{
  FlipPart *tkb = & ((FlipWidget)w)->flip;
  tkb->value--;
  update_value(w);
  expose(w,NULL,NULL);
  TRACE(2,"");
}

static void KeyAction(Widget w, XEvent* e, String* s, Cardinal* n)
{
  char *s1;
  s1 = *n && s && *s && **s ? *s : "" ;
  TRACE(2,"key:%s", s1);

  FlipWheelInput(w,s1);
}

// returns TRUE if widget wants this wheel event
int FlipWheelInput(Widget w, char *key)
{
  Cardinal null;
  FlipPart *tkb = & ((FlipWidget)w)->flip;

  if( tkb->state == armed ) {
    if( strcmp(key,"Inc") == 0 ) increment(w);
    else if( strcmp(key,"Dec") == 0 ) decrement(w);
    else if( strcmp(key,"Fire") == 0 ) NotifyAction(w, NULL,NULL, &null );
    return 1;
  }
  return 0;
}

void FlipSetList( Widget w, int mlist )
{
  FlipPart *tkb = & ((FlipWidget)w)->flip;
  FlipList_t *fl = & tkb->list;
  if( fl->mlist ) m_free(fl->mlist);
  fl->mlist = mlist;
  fl->crsr = 0;
}

FlipList_t *FlipGetList( Widget w )
{
  FlipPart *tkb = & ((FlipWidget)w)->flip;
  return & tkb->list;
}

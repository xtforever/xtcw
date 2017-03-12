#include "xutil.h"
#include <math.h>
#include "mls.h"
#include <assert.h>
#if 0
#include <cairo/cairo.h>
#include <cairo/cairo-xlib-xrender.h>
#include  <cairo/cairo-xlib.h>
#endif
/* utilities */
static int xft_font_angle( const char *name,int *n );


void xft_calc_string_size( Widget X, XftFont *fn, char *s, int *w, int *h )
{
  XGlyphInfo extents;
  Display *dpy = XtDisplay(X);
  XftTextExtentsUtf8(dpy, fn, (FcChar8*)  s,
		       strlen( s ), &extents);
  if( w ) *w = extents.xOff;
  if( h ) *h = fn->ascent + fn->descent;
}

int xft_textwidth( Display *dpy, XftFont *font, char *s )
{
  XGlyphInfo extents;
  XftTextExtentsUtf8(dpy, font, (FcChar8*)  s,
		       strlen( s ), &extents);
  return extents.xOff;
}

/** fill the struct xftColor with values computed from Pixel fg
 */
Boolean xft_color_alloc(Widget w, Pixel fg, XftColor *xftColor )
{
  pixel_to_xftcolor(w,fg,xftColor);
  return True;
}


/*
  FcChar8* txt;
  XGlyphInfo extents;
  XftTextExtentsUtf8(s->display, s->font, txt, strlen(txt), &extents);
  XftDrawStringUtf8(draw, color, font, extents.x, extents.y,  txt, strlen(txt) );
*/

/** open font with Xft. Angle is Degree (0-360). */
typedef Boolean bool;
#ifndef M_PI
# define M_PI 3.14159265359
#endif

XftFont*xft_fontopen(Display *dpy, int screen, char* name, bool core, int angle)
{
  XftFont *font; // the font we will return;
  XftPattern *fnt_pat;
  FcChar8 *picked_name;
  XftPattern *match_pat;  // the best available match on the system
  XftResult match_result; // the result of our matching attempt

  xft_font_angle( name, &angle );

  fnt_pat = XftNameParse (name);
  if( !fnt_pat ) {
    TRACE(2,"Name '%s' not parsed", name );
    return 0;
  }

  // rotate font if angle!=0
  if (angle !=0) {
    XftMatrix m;
    XftMatrixInit(&m);
    XftMatrixRotate(&m,cos(M_PI*angle/180.),sin(M_PI*angle/180.));
    XftPatternAddMatrix (fnt_pat, XFT_MATRIX,&m);
  }

  if (core) {
    XftPatternAddBool(fnt_pat, XFT_CORE, FcTrue);
    XftPatternAddBool(fnt_pat, XFT_RENDER, FcFalse);
  }

  // query the system to find a match for this font
  match_pat = XftFontMatch(dpy,screen, fnt_pat, &match_result);
  XftPatternDestroy( fnt_pat );
  if( !match_pat ) {
    TRACE(2,"Name '%s' not matched", name );
    return 0;
  }

#if 0
  // diagnostic to print the "full name" of the font we matched.
  picked_name =  FcNameUnparse(match_pat);
  printf("Match: %s\n", picked_name);
  free(picked_name);
#endif

  // open the matched font
  font = XftFontOpenPattern(dpy, match_pat);

#if 0
  // diagnostic to print the "full name" of the font we actually opened.
  picked_name =  FcNameUnparse(font->pattern);
  printf("Open : %s\n", picked_name);
  free(picked_name);
#endif

  if (!font)
    XftPatternDestroy (match_pat);

  //  XftPatternDestroy(match_pat);
  //  FontConfig will destroy this resource for us. We must not!

  return font;
}

/** set *n if and only if "angle=<value>" is given */
static int xft_font_angle( const char *name,int *n )
{
  if( !n ) return -1;
  char *p = strstr( name, "angle=" );
  if( !p ) return -1;
  p+=7; *n=0; unsigned ch;
  while( (ch = (*p++)-'0') < 10 ) *n=(10* (*n))+ch;
  printf("Angle=%d\n", *n );
  return 0;
}


int load_pixmap_from_file(Widget w, char *name, Pixmap *pixmap, Pixmap *mask )
{
    XpmAttributes attr;
    load_pixmap_from_file_attr(w,name,pixmap,mask,&attr);
}

int load_pixmap_from_file_attr(Widget w, char *name, Pixmap *pixmap, Pixmap *mask, XpmAttributes *attr)
{
  int status;
  // XpmAttributes attributes;

  attr->valuemask = 0;
  status = XpmReadFileToPixmap(XtDisplay(w),DefaultRootWindow(XtDisplay(w)),
			       name, pixmap, mask, attr);
  if (status != XpmSuccess) {
    fprintf(stderr, "File: '%s' XpmError: %s\n", name, XpmGetErrorString(status));
    return -1;
  }

  return 0;
}

inline static uint get_component( Pixel px, ulong m )
{
    ulong p = px;
    p &= m;

    while( m && !(m&1) ) { m>>=1; p >>=1; }
    return p;
}

#if 0
void cairo_set_source_rgb_from_pixel(Widget self, cairo_t *c, Pixel px)
{
    Display *dpy = XtDisplay(self);
    Visual *v = DefaultVisual(dpy,DefaultScreen(dpy));


    uint r,g,b;

    r = get_component( px, v->red_mask );
    g = get_component( px, v->green_mask );
    b = get_component( px, v->blue_mask );

    cairo_set_source_rgb(c, r / 255.0 ,g / 255.0, b / 255.0 );
}
#endif


static short
maskbase (unsigned long m)
{
    short        i;

    if (!m)
        return 0;
    i = 0;
    while (!(m&1))
    {
        m>>=1;
        i++;
    }
    return i;
}
static short
masklen (unsigned long m)
{
    unsigned long y;

    y = (m >> 1) &033333333333;
    y = m - y - ((y >>1) & 033333333333);
    return (short) (((y + (y >> 3)) & 030707070707) % 077);
}



void pixel_to_xftcolor( Widget w, Pixel px, XftColor *result )
{
    Display *dpy = XtDisplay(w);
    Visual *visual = DefaultVisual(dpy,DefaultScreen(dpy));
    int         red_shift, red_len;
    int         green_shift, green_len;
    int         blue_shift, blue_len;

    red_shift = maskbase (visual->red_mask);
    red_len = masklen (visual->red_mask);
    green_shift = maskbase (visual->green_mask);
    green_len = masklen (visual->green_mask);
    blue_shift = maskbase (visual->blue_mask);
    blue_len = masklen (visual->blue_mask);

    int r,g,b,a;
    r = get_component( px, visual->red_mask );
    g = get_component( px, visual->green_mask );
    b = get_component( px, visual->blue_mask );
    a = 0xffff;

    result->pixel = px;
    result->color.red = r << (16-red_len);
    result->color.green = g << (16-green_len);
    result->color.blue = b << (16-blue_len);
    result->color.alpha = a;
}



void WDrawLine( Widget w, GC gc, int x0,int y0,int x1, int y1 )
{
    XDrawLine( XtDisplay(w), XtWindow(w), gc, x0,y0,x1,y1 );
}

void XtDrawLine( Widget w, GC gc, int x0,int y0,int x1, int y1 )
{
    XDrawLine( XtDisplay(w), XtWindow(w), gc, x0,y0,x1,y1 );
}

void XtDrawRectangle( Widget w, GC gc, XRectangle *r )
{
        XDrawRectangle( XtDisplay(w),XtWindow(w), gc,
                        r->x,r->y, r->width, r->height );
}
void XtFillRectangle( Widget w, GC gc, XRectangle *r )
{
        XFillRectangle( XtDisplay(w),XtWindow(w), gc,
                        r->x,r->y, r->width, r->height );
}
Pixmap XtCreatePixmap(Widget w, int width, int height)
{
    Display *dpy = XtDisplay(w);
    return XCreatePixmap(dpy, XtWindow(w), width, height,
                         DefaultDepth(dpy, DefaultScreen(dpy)));
}

XftDraw *XtXftDrawCreate( Widget w, Drawable d)
{
    Display *dpy = XtDisplay(w);
    XftDraw *draw = XftDrawCreate(dpy, d,
                                  DefaultVisual(dpy, DefaultScreen(dpy)), None);
    assert( draw );
    return draw;
}

void XtCopyArea(Widget w, Drawable src, Drawable dst, int src_x, int src_y,
                int width, int height,
                int dst_x, int dst_y )
{
        Display *dpy = XtDisplay(w);
        GC copy_gc = DefaultGC(dpy,DefaultScreen(dpy));

        XCopyArea(dpy,src,dst,copy_gc, src_x,src_y, width,height, dst_x,dst_y );
}


void ManageWidget( Widget w, int managed )
{
  if( managed ) XtManageChild(w); else XtUnmanageChild(w);
}

Bool rect_is_inside( XRectangle *r, int x, int y )
{
    return (x >= r->x) && x < (r->x+r->width)
        && (y >= r->y) && y < (r->y+r->height);
}

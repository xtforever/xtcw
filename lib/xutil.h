#ifndef XUTIL_H
#define XUTIL_H

#include <X11/Intrinsic.h>
#include <X11/Xft/Xft.h>
#include <X11/xpm.h>
//#include <cairo/cairo.h>

#define WBUILD_HAS_CHANGED(x) ($##x!=$old$##x)
#define XT()  if( method_trace ) fprintf( stderr, "%s: %s\n", XtName(self), __func__ )

#define XTFUNC() fprintf( stderr, "%s: %s\n", XtName(self), __func__ )
int xft_textwidth( Display *dpy, XftFont *font, char *s );
Boolean xft_color_alloc(Widget w, Pixel fg, XftColor *xftColor );
XftFont* xft_fontopen(Display *dpy, int screen, char* name, Boolean core, int angle);

int load_pixmap_from_file_attr(Widget w, char *name, Pixmap *pixmap, Pixmap *mask, XpmAttributes *attr);
int load_pixmap_from_file(Widget w, char *name, Pixmap *pixmap, Pixmap *mask);

void xft_calc_string_size( Widget X, XftFont *fn, char *s, int *w, int *h );
//void cairo_set_source_rgb_from_pixel(Widget self, cairo_t *c, Pixel px);
void pixel_to_xftcolor( Widget w, Pixel px, XftColor *result );
void WDrawLine( Widget w, GC gc, int x0,int y0,int x1, int y1 );
void XtDrawLine( Widget w, GC gc, int x0,int y0,int x1, int y1 );
void XtDrawRectangle(Widget w, GC gc, XRectangle *r);
void XtFillRectangle(Widget w, GC gc, XRectangle *r);
Pixmap XtCreatePixmap(Widget w, int width, int height);
XftDraw *XtXftDrawCreate(Widget w, Drawable d);
void XtCopyArea(Widget w, Drawable src, Drawable dst, int src_x, int src_y,
                int width, int heigt,
                int dst_x, int dst_y );

void ManageWidget( Widget w, int managed );
Bool rect_is_inside( XRectangle *r, int x, int y );

#endif

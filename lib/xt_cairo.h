#ifndef XT_CAIRO_H
#define XT_CAIRO_H

#include <X11/Intrinsic.h>
#include <X11/Xft/Xft.h>
#include <X11/xpm.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib-xrender.h>
#include  <cairo/cairo-xlib.h>


typedef struct xtcairo_st {
    cairo_surface_t *cs;
    Pixmap p;
    uint w,h;
    Display *dpy;
    /* resources from widget */
    Pixel fg,bg;
    uint lineWidth;
} xtcairo_t;

xtcairo_t* xtcairo_create_pixmap( Widget s, uint w, uint h );
void xtcairo_destroy_pixmap(xtcairo_t *cs);
void xtcairo_draw_line(xtcairo_t *cs, uint x, uint y, uint w, uint h);
void xtcairo_draw_circle(xtcairo_t *cs, uint x, uint y, float radius, Boolean filled);
void cairo_set_source_rgb_from_pixel(Widget self, cairo_t *c, Pixel px);

#endif

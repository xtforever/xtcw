#include "xt_cairo.h"
#include <math.h>


inline static uint get_component( Pixel px, ulong m )
{
    ulong p = px;
    p &= m;

    while( m && !(m&1) ) { m>>=1; p >>=1; }
    return p;
}


void cairo_set_source_rgb_from_pixel_dpy(Display *dpy, cairo_t *c, Pixel px)
{
    Visual *v = DefaultVisual(dpy,DefaultScreen(dpy));
    uint r,g,b;
    r = get_component( px, v->red_mask );
    g = get_component( px, v->green_mask );
    b = get_component( px, v->blue_mask );
    cairo_set_source_rgb(c, r / 255.0 ,g / 255.0, b / 255.0 );
}

void cairo_set_source_rgb_from_pixel(Widget self, cairo_t *c, Pixel px)
{
    Display *dpy = XtDisplay(self);
    cairo_set_source_rgb_from_pixel_dpy(dpy,c,px);
}



xtcairo_t* xtcairo_create_pixmap( Widget s, uint w, uint h )
{
    struct xtcairo_st *c = malloc(sizeof(xtcairo_t));
    c->w = w; c->h=h;
    c->dpy = XtDisplay(s);
    c->p=XCreatePixmap(c->dpy, XtWindow(s), w,h,
                         DefaultDepth(c->dpy, DefaultScreen(c->dpy)));
    c->cs=cairo_xlib_surface_create(c->dpy,c->p,
                                   DefaultVisual(c->dpy, DefaultScreen(c->dpy)),
                                   w,h );

    XtVaGetValues(s, "foreground", &c->fg, "background", &c->bg,
                  "lineWidth", &c->lineWidth, NULL );
    return c;
}

void xtcairo_destroy_pixmap(xtcairo_t *cs)
{
    XFreePixmap(cs->dpy, cs->p);
    cairo_surface_destroy(cs->cs);
    memset(cs,0,sizeof *cs);
    free(cs);
}

void xtcairo_draw_line(xtcairo_t *cs, uint x, uint y, uint x1,uint y1)
{
    cairo_t *c;
    c=cairo_create(cs->cs);
    cairo_set_line_width (c, cs->lineWidth);
    cairo_set_source_rgb_from_pixel_dpy(cs->dpy,c,cs->fg);
    cairo_move_to(c, x, y);
    cairo_line_to(c,x1,y1);
    cairo_close_path(c);
    cairo_stroke (c);
    cairo_show_page(c);
    cairo_destroy(c);
}

void xtcairo_draw_circle(xtcairo_t *cs, uint x, uint y, float radius, Boolean filled)
{
    cairo_t *c;
    c=cairo_create(cs->cs);
    cairo_arc (c, x, y, radius, 0.0  * (M_PI/180.0), 360.0  * (M_PI/180.0) );
    cairo_set_line_width (c, cs->lineWidth);
    cairo_set_source_rgb_from_pixel_dpy(cs->dpy,c,cs->fg);
    if( filled ) cairo_fill(c); else cairo_stroke (c);
    cairo_show_page(c);
    cairo_destroy(c);
}

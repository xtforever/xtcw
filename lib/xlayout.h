#ifndef XLAYOUT_H
#define XLAYOUT_H

#include <X11/Intrinsic.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <stdlib.h>
#include "mls.h"

typedef struct xlprint {
    Display *dpy;
    XftDraw *draw;
    XftFont *fnt;
    XftColor *fg,*bg;
} xlprint;

typedef struct xlprint_int {
    struct xlprint *dev;
    XRectangle r;
    int buf, glyphs;
    XGlyphInfo extents;
    char *format;
} xlprint_int;

/* measure default size, alloc buffers */
void xlprint_init( struct xlprint_int *p, ... );
/* measure string size, get glyphs */
void xlprint_render( struct xlprint_int *p, ... );

/* draw background, draw glyphs right-adjusted : Buggy - needs second rectange */
void xlprint_draw( struct xlprint_int *p );
void xlprint_draw2( XRectangle *backgr, struct xlprint_int *p );
void xlprint_draw3( XRectangle *backgr, struct xlprint_int *p );
/* free buffers */
void xlprint_free( struct xlprint_int *p );

/* draw filled rectangle with xrandr */
void draw_rect( XftDraw *draw,  XftColor *col, XRectangle *r );

void get_glyphs(Display	    *dpy,
		XftFont	    *pub,
		int         m_glyphs,
		int         m );
void draw_glyphs( XftDraw *draw, XftColor *col, XftFont *fnt, int x, int y,  int glyphs );
#endif

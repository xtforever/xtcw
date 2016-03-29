#ifndef XPRINT2_H
#define XPRINT2_H

#include <X11/Xft/Xft.h>
#include <X11/Xmu/Converters.h>
#include <X11/Intrinsic.h>


enum LabelPosition  {
    LabelNorthEast = 1,
    LabelEast = 2,
    LabelSouthEast = 3, 
    LabelSouth =4,
    LabelSouthWest =5,
    LabelWest =6,
    LabelNorthWest =7,
    LabelCenter = 8,
    LabelNorth = 9,

    LabelGravity = 15,
    
    LabelRightOf = 16,
    LabelLeftOf  = 32,
    LabelAlignH  = 48,

    LabelBelow   = 64,
    LabelAbove   = 128,
    LabelAlignV  = 192
};


typedef struct rect_st
{
    int x,y,w,h;
} rect_t;
 
struct xp2_attr {
    XftColor *fg,*bg;
    XftFont  *font;
};

struct xp2_label {
    int attr;
    int glyph;
    rect_t r;
    int text_width, text_height; 
    int child;
    int other_label;
    int place;
};

typedef struct xprint2 {
    Display *dpy;
    XftDraw *draw;
    rect_t win;			/* coord. inside window */
    int x,y;		        /* coord. visible area  */
    int attr,label;
} xp2_t;

xp2_t* xp2_init(Display *dpy);
void xp2_set_drawable( xp2_t *x, XftDraw *draw );
void xp2_set_window( xp2_t *xp, int x, int y,int w, int h ); 
/* NorthWestGravity, CenterGravity, ... 
   AlignLeft, AlignRight, AlignBottom, AlignTop
   ALignBetween,
*/
int xp2_place(xp2_t*x, int r, int r1, int where );
void placer( rect_t *r0, rect_t *r1, int where );


int xp2_rem(xp2_t*x, int r );

int xp2_print(xp2_t*x, int r );

int xp2_label( xp2_t *x, int num, int attr );
int xp2_textbox(xp2_t*x, int r, char *s, int attr );
void xp2_free(xp2_t*x);
int xp2_new_attr( xp2_t *x, XftColor *fg,  XftColor *bg, XftFont *font );
void xp2_update_label_str(xp2_t *x, int num, char *s, int measure );
void xp2_update_label(xp2_t *x, int num, int str, int measure );
int xp2_new_label( xp2_t *x, int attr );
void xp2_draw_label(xp2_t*x, int label_num, int clear_background );
rect_t *xp2_get_rect(xp2_t *x, int num);
void xp2_get_bounds( xp2_t *x, rect_t *bounds );

/* create a label */
int xp2_setup_label( xp2_t *x, char *str, int attr, int other_label, int where );
void xp2_layout_label( xp2_t *x, int label );
/* create attribute */
void xp2_attr( xp2_t *x, int num, XftColor *fg,  XftColor *bg, XftFont *font ); 
void xp2_measure_label( xp2_t *x, int num );
#endif


#ifndef LFONT_H
#define LFONT_H

#include "mls.h"
#include <X11/Intrinsic.h>
#include <X11/Xft/Xft.h>

enum lfont_names { bold, italic, regular, LFONT_MAX };

struct lfont_st {
    int init;
    Display *dpy;
    XftDraw *draw;
    XftFont *xft_font_def[LFONT_MAX];
    XftColor xft_color_def;
    char *def_font_name;
};


int lfont_init(Display *dpy, char *family);
struct lfont_st *lfont_get(int lfont_hdl);
void lfont_free(int lfont_hdl);
void lfont_destroy(void);
void lfont_set_drawable( int lfont_hdl, Drawable d );
void lfont_set_color( int lfont_hdl, XftColor *color );
void lfont_draw_text( int lfont_hdl,
                      enum lfont_names font,
                      int x, int y,
                      char *string, int len,
                      int *ret_x, int *ret_y);

int lfont_height(int lf, enum lfont_names font );

#endif

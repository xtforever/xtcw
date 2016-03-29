#ifndef FONT_H
#define FONT_H

#ifdef USE_CONFIG_H
#include "config.h"
#endif

#include "mls.h"
#include <X11/Xft/Xft.h>
#include <X11/Intrinsic.h>



extern int trace_font;

XftFont *font_load(Display *dpy, int screen, char *name);
XftFont *font_unicode_find(char *name, int unicode, int *glyphindex );
void font_cache_flush(void);

int font_test(Widget w);
#endif

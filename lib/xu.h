#ifndef XU_H
#define XU_H

#include <X11/Xft/Xft.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/xpm.h>
#include "IconLabel.h"
#include "mls.h"

struct X11_CONFIG {
  Widget main_w;
  XtAppContext   app;
  Display *display; 
  Visual *visual; 
  Window window;
  GC gc;
  int screen_num;
  Screen *screen;
  int depth;
  Colormap cmap; 
  XftFont *small_font, *large_font, *normal_font, *special_font;
  XftColor text_color; 
  Pixel bg,fg;
  Atom MyAtom;
} X11;

#define args_free(args) do { m_free(args); args=0; } while(0)
#define args_clear(args) m_clear(args)

int args_put( int args, ... );
int args_vput( int args,va_list var );
void xu_init(Widget top);
Widget xu_mkiconlabel(Widget w, char *name, int args);
Widget xu_iconlabel(Widget w, char *name, ... ); 
#endif

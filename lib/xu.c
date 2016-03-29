#include "xu.h"

XftFont *xu_openfont(const char *pattern )
{
  XftFont *font = XftFontOpenName( X11.display, X11.screen_num, pattern );
  if( !font ) {
    fprintf(stderr,"ERROR kann font %s nicht laden. Nehme fixed-8\n", pattern);
    font = XftFontOpenName( X11.display, X11.screen_num, "fixed-8" );
    if( !font ) {
      fprintf(stderr,"ERROR kann fixed-8 nicht laden. quit\n");
      exit( EXIT_FAILURE );
    }
  }
  return font;
}


void xu_init(Widget top)
{
  // get X11 defaults
  X11.main_w  = top;
  X11.display = XtDisplay(X11.main_w);
  X11.screen_num  = DefaultScreen(X11.display);
  X11.screen      = DefaultScreenOfDisplay(X11.display);
  X11.window    = DefaultRootWindow(X11.display);
  X11.visual  = DefaultVisual(X11.display,X11.screen_num); 
  X11.depth   = DefaultDepth(X11.display,X11.screen_num);
  X11.cmap    = DefaultColormap(X11.display,X11.screen_num);
  X11.MyAtom  = XInternAtom (X11.display, "MyAtom",     False);
  X11.small_font = xu_openfont( "fixed-8" ); 
  X11.special_font = xu_openfont( "sans-12" ); 
  X11.normal_font = xu_openfont( "WenQuanYi Zen Hei Sharp-30" );  
  X11.large_font = xu_openfont( "times-28" ); 
  X11.gc = DefaultGC(X11.display, X11.screen_num );
}

int args_put( int m, ... )
{
  va_list var;
  va_start(var, m);
  m=args_vput(m,var);
  va_end(var);
  return m;
}

int args_vput( int m,va_list var )
{
  Arg arg;
  char *attr;
  
  if( !m ) m=m_create(10,sizeof(Arg));

  for(attr = va_arg(var, String) ; attr != NULL;
                        attr = va_arg(var, String)) {
    arg.name=attr;
    arg.value=va_arg(var,XtArgVal);
    m_put(m,&arg);
  }

  return m;
}


Widget xu_mkiconlabel(Widget w, char *name, int args )
{
  int status;
  XpmAttributes attributes;
  Pixmap pixmap, mask;
  Widget icon;
  ArgList al=NULL;
  Cardinal an=0;
  int free_args = ! args;

  attributes.valuemask = 0;
  status = XpmReadFileToPixmap(X11.display,X11.window,
			       name, &pixmap, &mask, &attributes);
  if (status != XpmSuccess) {
    fprintf(stderr, "XpmError: %s\n", XpmGetErrorString(status));
  }
  else {
    args = args_put(args, XtNpixmap, pixmap,
          XtNmask,   mask,
          XtNisBitmap, False,
          XtNshape, True,
          NULL );
  }

  if( args ) {
    an = (Cardinal) m_len(args);
    if( an ) al =  (ArgList) mls(args,0);
  }

  icon = XtCreateManagedWidget(name, iconLabelWidgetClass,w, al,an  ); 
    
  if( free_args ) m_free(args);

  return icon;

}


Widget xu_iconlabel(Widget w, char *name, ...  )
{
  va_list var;
  va_start(var, name);
  int m = args_vput(0,var);
  Widget icon = xu_mkiconlabel(w,name, m );
  args_free(m);
  va_end(var);
  return icon;
}

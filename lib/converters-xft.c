#include "converters-xft.h"

#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/xpm.h>
#include <X11/Xft/Xft.h>
#include <X11/Xmu/Converters.h>
#include <X11/Xaw/XawInit.h>
#include "mls.h"
#include "xutil.h"

static XtConvertArgRec xftColorConvertArgs[] = {
    {XtWidgetBaseOffset, (XtPointer)XtOffsetOf(WidgetRec, core.screen),
     sizeof(Screen *)},
    {XtWidgetBaseOffset, (XtPointer)XtOffsetOf(WidgetRec, core.colormap),
     sizeof(Colormap)}
};

#define	donestr(type, value, tstr) \
	{							\
	    if (toVal->addr != NULL) {				\
		if (toVal->size < sizeof(type)) {		\
		    toVal->size = sizeof(type);			\
		    XtDisplayStringConversionWarning(dpy, 	\
			(char*) fromVal->addr, tstr);		\
		    return False;				\
		}						\
		*(type*)(toVal->addr) = (value);		\
	    }							\
	    else {						\
		static type static_val;				\
		static_val = (value);				\
		toVal->addr = (XPointer)&static_val;		\
	    }							\
	    toVal->size = sizeof(type);				\
	    return True;					\
	}

static void
XmuFreeXftColor (XtAppContext app, XrmValuePtr toVal, XtPointer closure,
		 XrmValuePtr args, Cardinal *num_args)
{
    Screen	*screen;
    Colormap	colormap;
    XftColor	*color;

    if (*num_args != 2)
    {
	XtAppErrorMsg (app,
		       "freeXftColor", "wrongParameters",
		       "XtToolkitError",
		       "Freeing an XftColor requires screen and colormap arguments",
		       (String *) NULL, (Cardinal *)NULL);
	return;
    }

    screen = *((Screen **) args[0].addr);
    colormap = *((Colormap *) args[1].addr);
    color = (XftColor *) toVal->addr;
    XftColorFree (DisplayOfScreen (screen),
		  DefaultVisual (DisplayOfScreen (screen),
				 XScreenNumberOfScreen (screen)),
		  colormap, color);
}

static Boolean
XmuCvtStringToXftColor(Display *dpy,
		       XrmValue *args, Cardinal *num_args,
		       XrmValue *fromVal, XrmValue *toVal,
		       XtPointer *converter_data)
{
    char	    *spec;
    XRenderColor    renderColor;
    XftColor	    xftColor;
    Screen	    *screen;
    Colormap	    colormap;

    if (*num_args != 2)
    {
	XtAppErrorMsg (XtDisplayToApplicationContext (dpy),
		       "cvtStringToXftColor", "wrongParameters",
		       "XtToolkitError",
		       "String to render color conversion needs screen and colormap arguments",
		       (String *) NULL, (Cardinal *)NULL);
	return False;
    }

    screen = *((Screen **) args[0].addr);
    colormap = *((Colormap *) args[1].addr);

    spec = (char *) fromVal->addr;
    if (strcasecmp (spec, XtDefaultForeground) == 0)
    {
	renderColor.red = 0;
	renderColor.green = 0;
	renderColor.blue = 0;
	renderColor.alpha = 0xffff;
    }
    else if (strcasecmp (spec, XtDefaultBackground) == 0)
    {
	renderColor.red = 0xffff;
	renderColor.green = 0xffff;
	renderColor.blue = 0xffff;
	renderColor.alpha = 0xffff;
    }
    else if (!XRenderParseColor (dpy, spec, &renderColor))
	return False;
    if (!XftColorAllocValue (dpy,
			     DefaultVisual (dpy,
					    XScreenNumberOfScreen (screen)),
			     colormap,
			     &renderColor,
			     &xftColor))
	return False;

    donestr (XftColor, xftColor, XtRXftColor);
}

static void
XmuFreeXftFont (XtAppContext app, XrmValuePtr toVal, XtPointer closure,
		XrmValuePtr args, Cardinal *num_args)
{
    Screen  *screen;
    XftFont *font;

    if (*num_args != 1)
    {
	XtAppErrorMsg (app,
		       "freeXftFont", "wrongParameters",
		       "XtToolkitError",
		       "Freeing an XftFont requires screen argument",
		       (String *) NULL, (Cardinal *)NULL);
	return;
    }

    screen = *((Screen **) args[0].addr);
    font = *((XftFont **) toVal->addr);
    if (font)
	XftFontClose (DisplayOfScreen (screen), font);
}

static Boolean
XmuCvtStringToXftFont(Display *dpy,
		      XrmValue *args, Cardinal *num_args,
		      XrmValue *fromVal, XrmValue *toVal,
		      XtPointer *converter_data)
{
    char    *name;
    XftFont *font;
    Screen  *screen;

    if (*num_args != 1)
    {
	XtAppErrorMsg (XtDisplayToApplicationContext (dpy),
		       "cvtStringToXftFont", "wrongParameters",
		       "XtToolkitError",
		       "String to XftFont conversion needs screen argument",
		       (String *) NULL, (Cardinal *)NULL);
	return False;
    }

    screen = *((Screen **) args[0].addr);
    name = (char *) fromVal->addr;

    font = XftFontOpenName(
                           dpy,
                           XScreenNumberOfScreen (screen),
                           name );
    /*
    font = xft_fontopen (dpy,
                         XScreenNumberOfScreen (screen),
                         name, False, 0);
    */
    if (font)
    {
	donestr (XftFont *, font, XtRXftFont);
    }
    XtDisplayStringConversionWarning(dpy, (char *) fromVal->addr, XtRXftFont);
    return False;
}

static XtConvertArgRec xftFontConvertArgs[] = {
    {XtWidgetBaseOffset, (XtPointer)XtOffsetOf(WidgetRec, core.screen),
     sizeof(Screen *)},
};

static void CvtStringToPixmap(
    XrmValue*           args,
    Cardinal*           num_args,
    XrmValuePtr         fromVal,
    XrmValuePtr         toVal
    )
{
    static Pixmap	pmap;
    Pixmap shapemask;
    char *name = (char *)fromVal->addr;
    Screen *screen;
    Display *dpy;

    TRACE(1,"convert STRING to PIXMAP\n");

    if (*num_args != 1)
     XtErrorMsg("wrongParameters","cvtStringToPixmap","XtToolkitError",
             "String to pixmap conversion needs screen argument",
              (String *)NULL, (Cardinal *)NULL);

    if (strcmp(name, "None") == 0) {
        pmap = None;
    } else {
	screen = *((Screen **) args[0].addr);
	dpy = DisplayOfScreen(screen);

	int status = XpmReadFileToPixmap(dpy, RootWindowOfScreen(screen), name, &pmap, &shapemask, NULL);
        if (status != XpmSuccess) {
            fprintf(stderr, "File: '%s' XpmError: %s\n", name, XpmGetErrorString(status));
            return;
        }

    }

    (*toVal).size = sizeof(Pixmap);
    (*toVal).addr = (XPointer) &pmap ;
}

static XtConvertArgRec scrnConvertArg[] = {
  {XtBaseOffset, (XtPointer) XtOffset(Widget, core.screen),
   sizeof(Screen *)}
};

void converters_xft_init(void)
{
  static Boolean first_time = True;

  if (first_time == False)
    return;

  first_time = False;

  XawInitializeWidgetSet();

  XtAddConverter( XtRString, XtRPixmap, CvtStringToPixmap,
	      	    scrnConvertArg, XtNumber(scrnConvertArg));

  XtSetTypeConverter (XtRString, XtRXftColor,
		      XmuCvtStringToXftColor,
		      xftColorConvertArgs, XtNumber(xftColorConvertArgs),
		      XtCacheByDisplay, XmuFreeXftColor);

  XtSetTypeConverter (XtRString, XtRXftFont,
		      XmuCvtStringToXftFont,
		      xftFontConvertArgs, XtNumber(xftFontConvertArgs),
		      XtCacheByDisplay, XmuFreeXftFont);
}

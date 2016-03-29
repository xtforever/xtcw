#ifndef CONVERTERS_XFT_H
#define CONVERTERS_XFT_H

#ifndef XtRXftColor
#define XtRXftColor "XftColor"
#endif

#ifndef XtRXftFont
#define XtRXftFont "XftFont"
#endif

//
// Add String converters for:
// XtRXftColor			XftColor color
// XtRXftFont			XftFont *font
// XtRPixmap			Pixmap - wird von Xaw bereitgestellt
//
void converters_xft_init(void);

#endif

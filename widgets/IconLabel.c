/********************************************************************\
**                             _________________________________    **
**   A n t h o n y            |________    __    __    _________|   **
**                                     |  |o_|  |o_|  |             **
**           T h y s s e n           __|   __    __   |__           **
**                                __|   __|  |  |  |__   |__        **
**  `` Dragon Computing! ''    __|   __|     |  |     |__   |__     **
**                            |_____|        |__|        |_____|    **
**                                                                  **
\********************************************************************/
/*
** IconLabel.c - IconLabel Widget   Display a Pixmap  with optional label
**
** Author: Anthony Thyssen
**         Griffith University
**         anthony@cit.gu.edu.au
**
** Date:   March 30, 1996
**
** This Widget was created due to problems with using the original X
** Consortium Label Widget for displaying Bitmap and Pixmap in my
** XbmBrowser program.  I designed this widget to basically display a
** pixmap and optionally a title or filename centered underneath this
** image.
** 
** FEATURES
**    1/  Pixmap displayed correctly (a flag indicates to treat it as a bitmap)
**    2/  Shaped Windows using the bitmap mask given
**    3/  Optional Label centered either above or below the image
**    4/  Numerous inter-dimension controls for good user control
** 
*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "IconLabelP.h"          /* Private header file */
#include <X11/Xmu/Drawing.h>
#include <X11/xpm.h>

/* shape extension stuff */
#include <X11/extensions/shape.h>



#define streq(a,b) (strcmp( (a), (b) ) == 0)

/*---------------------------------------------------------------------------*/
/*-------------------------- Subroutine Declaration -------------------------*/

static void Initialize();
static void Realize();
static void Redisplay();
static void Resize();
static Boolean SetValues();
static void Destroy();
static XtGeometryResult QueryGeometry();

/*---------------------------------------------------------------------------*/
/*-------------------------------- Resources --------------------------------*/

#define offset(field) XtOffsetOf(IconLabelRec, icon_label.field)
static XtResource resources[] = {
/* miscellanous */
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
        offset(foreground), XtRString, XtDefaultForeground},
    {XtNfont,  XtCFont, XtRFontStruct, sizeof(XFontStruct *),
        offset(font),XtRString, XtDefaultFont},
    {XtNshape, XtCShape, XtRBoolean, sizeof(Boolean),
        offset(shape), XtRImmediate, (XtPointer)False},
    {XtNresize, XtCResize, XtRBoolean, sizeof(Boolean),
        offset(resize), XtRImmediate, (XtPointer)True},
/* pixmap stuff */
    {XtNpixmapName, XtCLabel, XtRString, sizeof(String),
        offset(pixmapName), XtRImmediate, (XtPointer)None},
    {XtNpixmap, XtCPixmap, XtRPixmap, sizeof(Pixmap),
        offset(pixmap), XtRImmediate, (XtPointer)None},
    {XtNmask, XtCMask, XtRBitmap, sizeof(Pixmap),
        offset(mask), XtRImmediate, (XtPointer)None},
    {XtNisBitmap, XtCIsBitmap, XtRBoolean, sizeof(Boolean),
        offset(is_bitmap), XtRImmediate, (XtPointer)False},
/* label stuff */
    {XtNlabel,  XtCLabel, XtRString, sizeof(String),
        offset(label), XtRString, NULL},
    {XtNlabelTop, XtCLabelTop, XtRBoolean, sizeof(Boolean),
        offset(label_top), XtRImmediate, (XtPointer)False},
/* internal space */
    {XtNinternalWidth, XtCWidth, XtRDimension,  sizeof(Dimension),
        offset(internal_width), XtRImmediate, (XtPointer)2},
    {XtNinternalHeight, XtCHeight, XtRDimension, sizeof(Dimension),
        offset(internal_height), XtRImmediate, (XtPointer)1},
    {XtNlabelGap, XtCHeight, XtRDimension, sizeof(Dimension),
        offset(label_gap), XtRImmediate, (XtPointer)1},
/* extra information pointer */
    {XtNinfoPtr, XtCInfoPtr, XtRPointer, sizeof(XtPointer),
        offset(info_ptr), XtRImmediate, (XtPointer)NULL},
 
};
#undef offset


/*---------------------------------------------------------------------------*/
/*---------------------- Class Structure Initialization ---------------------*/

#define SuperClass ((WidgetClass)&simpleClassRec)

IconLabelClassRec iconLabelClassRec = {
  { /* CoreClass 
    **  S -  self contained,   C v  down chain (super->sub),
    **  P    private Xt only,  C ^  up chained (sub->super),  ?  unknowen
    */
    SuperClass,                         /* superclass            C ^ (link) */  
    "IconLabel",                        /* class_name            P   (data) */
    sizeof(IconLabelRec),               /* widget_size           P   (data) */
    NULL,                               /* class_initialize      S - (exe)  */
    NULL,                               /* class_part_initialize C v (exe)  */
    FALSE,                              /* class_inited          P   (flag) */
    Initialize,                         /* initialize            C v (exe)  */
    NULL,                               /* initialize_hook       C v (exe)  */
    Realize,                            /* realize               S - (exe)  */
    NULL,                               /* actions               C ^ (srch) */
    0,                                  /* num_actions           C ^ (srch) */
    resources,                          /* resources             C v (comp) */
    XtNumber(resources),                /* num_resources         C v (comp) */
    NULLQUARK,                          /* xrm_class             P   (???)  */
    TRUE,                               /* compress_motion       S   (flag) */
    TRUE,                               /* compress_exposure     S   (flag) */
    TRUE,                               /* compress_enterleave   S   (flag) */
    FALSE,                              /* visible_interest      S   (flag) */
    Destroy,                            /* destroy               C ^ (exe)  */
    Resize,                             /* resize                S - (exe)  */
    Redisplay,                          /* expose                S - (exe)  */
    SetValues,                          /* set_values            C v (exe)  */
    NULL,                               /* set_values_hook       C v (exe)  */
    XtInheritSetValuesAlmost,           /* set_values_almost     S - (exe)  */
    NULL,                               /* get_values_hook       C v (exe)  */
    NULL,                               /* accept_focus          S - (exe)  */
    XtVersion,                          /* version               S - (data) */
    NULL,                               /* callback_private      P   (???)  */
    NULL,                               /* tm_table              S - (data) */
    QueryGeometry,                      /* query_geometry        S - (exe)  */
    XtInheritDisplayAccelerator,        /* display_accelerator   ?   (data) */
    NULL                                /* extension             S   (link) */
  },
  { /* SimpleClass */
    XtInheritChangeSensitive            /* change_sensitive  */ 
  },
  { /* IconLabelClass */
    0                                   /* field not used    */
  }
};

WidgetClass iconLabelWidgetClass = (WidgetClass)&iconLabelClassRec;

/*---------------------------------------------------------------------------*/
/*---------------------------- Private Procedures ---------------------------*/

static void
GetGCs(ilw)
  IconLabelWidget ilw;
/* Create the required drawing GCs */
{
  XGCValues     values;

  /* main drawing GC */
  values.foreground = ilw->icon_label.foreground;
  values.background = ilw->core.background_pixel;
  values.font       = ilw->icon_label.font->fid;
  values.graphics_exposures = False;

  ilw->icon_label.normal_GC = XtGetGC( (Widget)ilw,
      (unsigned) GCForeground | GCBackground | GCFont | GCGraphicsExposures,
      &values);

  /* grey insensitive GC -- and its stipple */
  values.fill_style = FillTiled;
  values.tile       = XmuCreateStippledPixmap( XtScreen((Widget)ilw),
       ilw->icon_label.foreground, ilw->core.background_pixel, ilw->core.depth);

  ilw->icon_label.stipple = values.tile;
  ilw->icon_label.gray_GC = XtGetGC( (Widget)ilw,
       (unsigned) GCForeground | GCBackground | GCFont | GCTile |
                  GCFillStyle | GCGraphicsExposures, &values);
}


static void
ReleaseGCs(ilw)
  IconLabelWidget ilw;
/* Release the required drawing GCs */
{
  XtReleaseGC( (Widget)ilw, ilw->icon_label.normal_GC );
  XtReleaseGC( (Widget)ilw, ilw->icon_label.gray_GC);
  XmuReleaseStippledPixmap( XtScreen((Widget)ilw), ilw->icon_label.stipple );
}


static void
SetPixmapDimensions(ilw)
  IconLabelWidget ilw;
{
  XRectangle  *rect = &ilw->icon_label.pm_rect;

  if (ilw->icon_label.pixmap != None) {
    Window w;
    int x, y;
    unsigned int width, height, bw, depth;

    if (XGetGeometry(XtDisplay(ilw), ilw->icon_label.pixmap,
                  &w, &x, &y, &width, &height, &bw, &depth)) {
      rect->width  = (unsigned short) width;
      rect->height = (unsigned short) height;
      ilw->icon_label.pm_depth = depth;
      return;
    }
  } 

  /* failed -- no dimensions */
  rect->width  = 0;
  rect->height = 0;
  ilw->icon_label.pm_depth = 0;

}


static void
SetLabelDimensions(ilw)
  IconLabelWidget ilw;
{
  XRectangle  *rect = &ilw->icon_label.lb_rect;
  XFontStruct *fs = ilw->icon_label.font;


  rect->width  = 0;
  rect->height = 0;
  ilw->icon_label.label_len = 0;

  if( ilw->icon_label.label != NULL ) {
    ilw->icon_label.label_len = strlen(ilw->icon_label.label);
    rect->width = (unsigned short)
         XTextWidth(fs, ilw->icon_label.label, (int) ilw->icon_label.label_len)
         + 2 * ilw->icon_label.internal_width;
    rect->height = (unsigned short)
         fs->max_bounds.ascent + fs->max_bounds.descent + 1;
  } 
}


static Dimension
PreferredWidth(ilw)
  IconLabelWidget ilw;
/* Figure out the prefered width of the widget */
{
  IconLabelPart *ilwp = &ilw->icon_label;

  /* width of the parts -- No pixmap then its width is 0 */
  Dimension p = ilwp->pm_rect.width + 2 * ilwp->internal_width;
  Dimension l = ilwp->lb_rect.width;

  return ( ilwp->label != NULL && l>p ) ? l : p;
}


static Dimension
PreferredHeight(ilw)
  IconLabelWidget ilw;
/* Figure out the prefered height of the widget */
{
  IconLabelPart *ilwp = &ilw->icon_label;
  Dimension h = ilwp->internal_height;

  /* NOTE: height is the same regardless of if label at top or
  ** the bottom of widget. The comments pretend it is at bottom */

  if ( ilwp->label != NULL ) {
    h += ilwp->lb_rect.height;   /* label present */
    if ( ilwp->pm_depth > 0 )
      h += ilwp->label_gap;      /* label & pixmap -- add gap space */
  } else {
    h += ilwp->internal_height;  /* no label internal space instead */
  }
  if ( ilwp->pm_depth > 0 )   /* pixmap present add it and top space */
    h += ilwp->pm_rect.height;

  return h;
}


static void
SetWindowShape(ilw)
  IconLabelWidget ilw;
/* set the shape masks for this widget */
{
  IconLabelPart *ilwp = &ilw->icon_label;

  if ( !ilwp->shape ) {
    if ( ilwp->shape_on ) {
      ilwp->shape_on = False;  /* don't do this all the time */
      /* turn off shaped window entirely */
      XShapeCombineMask(XtDisplay((Widget)ilw), XtWindow((Widget)ilw),
               ShapeBounding, 0, 0, (Pixmap)None, ShapeSet);
    }
  } else {
    ilwp->shape_on = True;

    /* Set region appropriate for pixmap and mask */
    if ( ilwp->pm_depth > 0 ) {
      if ( ilwp->mask != None )
        XShapeCombineMask(XtDisplay((Widget)ilw), XtWindow((Widget)ilw),
                ShapeBounding, (int)ilwp->pm_rect.x, (int)ilwp->pm_rect.y,
                ilwp->mask, ShapeSet );
      else 
        XShapeCombineRectangles(XtDisplay((Widget)ilw), XtWindow((Widget)ilw),
                ShapeBounding, 0, 0, &ilwp->pm_rect, 1, ShapeSet, Unsorted );
    }
    else /* no pixmap found */
      XShapeCombineRectangles( XtDisplay((Widget)ilw), XtWindow((Widget)ilw),
                ShapeBounding, 0, 0, &ilwp->pm_rect, 0, ShapeSet, Unsorted );

    /* Add a label region if needed */
    if ( ilwp->label != NULL ) {
      XShapeCombineRectangles(XtDisplay((Widget)ilw), XtWindow((Widget)ilw),
                ShapeBounding, 0, 0, &ilwp->lb_rect, 1, ShapeUnion, Unsorted );
    }
  }
}

static void
PositionParts(ilw, width, height)
  IconLabelWidget ilw;
  Dimension width, height;
/* position the parts in the current window size */
{
  IconLabelPart *ilwp = &ilw->icon_label;

  /* X positions are easy */
  ilwp->pm_rect.x = (width - ilwp->pm_rect.width) / 2;
  ilwp->lb_rect.x = (width - ilwp->lb_rect.width) / 2;
  if( ilwp->lb_rect.x < 0 ) ilwp->lb_rect.x = 0;

  /* Y positions depends on label -- This is halved later. */
  ilwp->pm_rect.y = height - ilwp->pm_rect.height - ilwp->internal_height;

  if( ilwp->label == NULL ) {              /* No label */
    ilwp->pm_rect.y += ilwp->internal_height; /* +ve -- work it out! */
    ilwp->pm_rect.y /= 2; /* now halve it */
  } else {                                 /* Label present */
    ilwp->pm_rect.y -=  ilwp->lb_rect.height + ilwp->label_gap;
    ilwp->pm_rect.y /= 2; /* now halve it */
    
    if( ilwp->label_top ) {               /* Label is at top */
      ilwp->lb_rect.y = 0;
      ilwp->pm_rect.y += ilwp->lb_rect.height + ilwp->label_gap;
    } else {                              /* Label is at bottom */
      ilwp->lb_rect.y = height - ilwp->lb_rect.height;
      ilwp->pm_rect.y += ilwp->internal_height;
    }
  }

  if (XtIsRealized((Widget)ilw))
    SetWindowShape(ilw);
}

/*---------------------------------------------------------------------------*/
/*------------------------------ Widget Methods -----------------------------*/

#if 0  /* not required */
static void 
ClassInitialize()
/* Class Initialization (Self Contained)
** Install type converters
*/
{
}
#endif


static void load_pixmap_from_file(IconLabelWidget  ilw)
{
  Widget w = (Widget) ilw;
  int status;
  XpmAttributes attributes;
  char *name = ilw->icon_label.pixmapName; 
  Pixmap 
    *pixmap =  &ilw->icon_label.pixmap, 
    *mask = &ilw->icon_label.mask;

  attributes.valuemask = 0;
  status = XpmReadFileToPixmap(XtDisplay(w),DefaultRootWindow(XtDisplay(w)),
			       name, pixmap, mask, &attributes);
  if (status != XpmSuccess) {
    fprintf(stderr, "XpmError: %s\n", XpmGetErrorString(status));
  }
  
}


/* ARGSUSED */
static void
Initialize(req, new, args, num_args)
  Widget   req;      /* widget user originally requested */
  Widget   new;      /* widget modified by superclases */
  ArgList  args;     /* resource list from Creation call */
  Cardinal num_args;
/* Initialize this widget (Chained Downward : Super -> Sub)
** Insure all public and private variables are reasonable.
*/
{
  IconLabelWidget  ilw = (IconLabelWidget)new;
  int x;  /* junk variable */

  if( ilw->icon_label.label != NULL )   /* make our own copy */
    ilw->icon_label.label = XtNewString(ilw->icon_label.label);

  ilw->icon_label.shape_on = False;
  if ( ilw->icon_label.shape && !XShapeQueryExtension(XtDisplay(new),&x,&x) )
    ilw->icon_label.shape = False;

  if( ilw->icon_label.pixmapName &&
      ilw->icon_label.pixmapName[0] ) 
    {
      load_pixmap_from_file(ilw); 
    }

  GetGCs(ilw);
  SetPixmapDimensions(ilw);
  SetLabelDimensions(ilw);

  /* set prefered size if not already set */
  if (ilw->core.width == 0)
    ilw->core.width = PreferredWidth(ilw);
  if (ilw->core.height == 0)
    ilw->core.height = PreferredHeight(ilw);

  /* call that widgets resize routine */
  (XtClass(new)->core_class.resize)(new);
}


static void
Realize(w, valueMask, attributes)
  Widget w;                /* realise the widget now */
  XtValueMask *valueMask;
  XSetWindowAttributes *attributes;
/* Reliase this Widget (Self Contained)
** set window attributes, and create the window
** and set the windows shape
*/
{
  /* Call the Realise of the superclass to create the window */
  (SuperClass->core_class.realize)(w, valueMask, attributes);
  
  /* Now set the Shape Extension Stuff */
  SetWindowShape( (IconLabelWidget)w );
}


/* ARGSUSED */
static void
Redisplay(w, event, region)
  Widget w;        /* widget to be (re)drawen */
  XEvent *event;   /* event which caused the redraw */
  Region region;   /* region to be drawn */
/* Redraw the Exposed Parts (Self Contained)
** part of the widget is now visible to the user so
** draw the parts required or all of it
*/
{
  IconLabelPart *ilwp = &((IconLabelWidget)w)->icon_label;
  GC  gc = XtIsSensitive(w) ? ilwp->normal_GC : ilwp->gray_GC;

#ifdef notdef
  if (region != NULL)
    XSetRegion(XtDisplay(w), gc, region);
#endif /*notdef*/

  /* --- label draw --- */
  /* this is done first so that pixmap can overwrite it if
  ** the widget is too small, and they overlap */
  if ( ilwp->label != NULL ) {   /* is label available? */
    XDrawString(XtDisplay(w), XtWindow(w), gc,
             (int)ilwp->lb_rect.x + ilwp->internal_width,
             (int)ilwp->lb_rect.y + ilwp->font->max_bounds.ascent,
             (char *)ilwp->label,  ilwp->label_len);
  }

  /* --- pixmap draw --- */
  if ( ilwp->pm_depth > 0 ) {  /* is a pixmap provided? */
    if ( ilwp->is_bitmap && ilwp->pm_depth == 1 ) { /* is it a bitmap */
      XCopyPlane(XtDisplay(w), ilwp->pixmap, XtWindow(w), gc,
                  0, 0, (int)ilwp->pm_rect.width, (int)ilwp->pm_rect.height,
                  (int)ilwp->pm_rect.x, (int)ilwp->pm_rect.y, 1L );
    } else {
      XCopyArea(XtDisplay(w), ilwp->pixmap, XtWindow(w), gc,
                  0, 0, (int)ilwp->pm_rect.width, (int)ilwp->pm_rect.height,
                  (int)ilwp->pm_rect.x, (int)ilwp->pm_rect.y );
    }
    if( ilwp->mask != None && ! ilwp->shape ) {  /* mask but no shape? */
      ; /* paint the mask in the background color! */
    }
  }

#ifdef notdef
  if (region != NULL)
    XSetClipMask(XtDisplay(w), gc, (Pixmap)None);
#endif /* notdef */
}


static void
Resize(w)
  Widget w;   /* widget to be resized */
/* Resize and Position Internals (Self Contained)
** recalculate internals based on new window position and size
*/
{
  PositionParts( (IconLabelWidget)w, w->core.width, w->core.height );
}


/* user_requests found */
#define PIXMAP     0
#define MASK       1
#define WIDTH      2
#define HEIGHT     3
#define NUM_CHECKS 4

/* ARGSUSED */
static Boolean 
SetValues(cur, req, new, args, num_args)
  Widget cur;     /* original before change resources */
  Widget req;     /* user requested changes to resources */
  Widget new;     /* resources modified by superclasses (modify this) */
  ArgList  args;  /* resource list from Creation call */
  Cardinal *num_args;
/* Set Public Variables (Chained Downward : Super -> Sub)
** Change the public variables and do whatever is needed
** to affect the changes ask for by the user
*/
{
  IconLabelPart *ilwp_cur = &((IconLabelWidget)cur)->icon_label;
  //  IconLabelPart *ilwp_req = &((IconLabelWidget)req)->icon_label;
  IconLabelPart *ilwp_new = &((IconLabelWidget)new)->icon_label;

  Boolean was_resized = False;
  Boolean redisplay = False;
  Boolean user_reqs[NUM_CHECKS];
  int i;

  /* Check the user set value requests for specific things */
  for (i = 0; i < NUM_CHECKS; i++)
    user_reqs[i] = FALSE;
  for (i = 0; i < *num_args; i++) {
    if (streq(XtNpixmap, args[i].name))
      user_reqs[PIXMAP] = TRUE;
    if (streq(XtNmask, args[i].name))
      user_reqs[MASK] = TRUE;
    if (streq(XtNwidth, args[i].name))
      user_reqs[WIDTH] = TRUE;
    if (streq(XtNheight, args[i].name))
      user_reqs[HEIGHT] = TRUE;
    if (streq(XtNpixmapName, args[i].name)) {
      user_reqs[PIXMAP] = TRUE;
      // TODO! unload pixmap_
      load_pixmap_from_file( (IconLabelWidget)new );
    }
  }

  /* Label changed? */
  if ( ilwp_cur->label != ilwp_new->label ) {
    XtFree( (char *)ilwp_cur->label );   /* free the old label (if present) */

    if ( ilwp_new->label != NULL ) {
      ilwp_new->label = XtNewString( ilwp_new->label ); /* allocate new */
    }
    was_resized = True;
  }

  /* change position of label? */
  if ( was_resized || ilwp_cur->font != ilwp_new->font ) {
    SetLabelDimensions((IconLabelWidget)new);
    was_resized = True;
  }

  /* Did the pixmap change? 
  ** Originally any pixmap change means redisplay -- don't do this
  ** maybe I should add a option which resets to false called
  ** redisplay -- IE: if true reset to false and re-display pixmap
  ** if ( user_reqs[PIXMAP] ) {  <=== Original If statment */
  if ( ilwp_cur->pixmap != ilwp_new->pixmap ) {
    SetPixmapDimensions((IconLabelWidget)new);
    was_resized = True;
    redisplay = True;
  }

  /* Set prefered width and height  IF
  **   resize is allowed and any sub-class or the user doesn't set it */
  if ( ilwp_new->resize && was_resized ) {
    if ( cur->core.height == req->core.height && !user_reqs[HEIGHT] )
      new->core.height = PreferredHeight((IconLabelWidget)new);
    if ( cur->core.width == req->core.width   && !user_reqs[WIDTH] )
      new->core.width = PreferredWidth((IconLabelWidget)new);
  }

  /* don't turn on shape ext if no shape ext is available */
  if ( !ilwp_cur->shape && ilwp_new->shape &&
                  !XShapeQueryExtension(XtDisplay(new),&i,&i) ) {
    ilwp_new->shape = False;
  }

  /* Drawing GCs changed? */
  if (    ilwp_cur->foreground       != ilwp_new->foreground
       || cur->core.background_pixel != new->core.background_pixel
       || ilwp_cur->font->fid        != ilwp_new->font->fid  ) {
    ReleaseGCs((IconLabelWidget)new);
    GetGCs((IconLabelWidget)new);
    redisplay = True;
  }

  /* Do we need to reposition parts */
  if (    ilwp_cur->internal_width  != ilwp_new->internal_width
       || ilwp_cur->internal_height != ilwp_new->internal_height
       || ilwp_cur->label_gap       != ilwp_new->label_gap
       || was_resized ) {
    /* Resize() will be called if geometry changes succeed
    ** So just resize against the current widget size for now */
    PositionParts((IconLabelWidget)new, cur->core.width, cur->core.height);
    redisplay = True;
  }
  else
  /* Do we just need to adjust the shape extension? */
  if (    ilwp_cur->mask  != ilwp_new->mask  /* || user_reqs[MASK] */
       || ilwp_cur->shape != ilwp_new->shape ) {
    if (XtIsRealized(new))
      SetWindowShape((IconLabelWidget)new);
    /* redisplay = True; -- we don't need to redisplay for shape change */
  }
  else
  /* Just need to redraw everything? */
  if (    XtIsSensitive(cur)  != XtIsSensitive(new)
       || ilwp_cur->is_bitmap != ilwp_new->is_bitmap ) {
    redisplay = True;
  }

  return  redisplay;  /* this includes was_resized */
}


static XtGeometryResult
QueryGeometry(w, intended, preferred)
  Widget w;                    /* widget for geometry request */
  XtWidgetGeometry *intended;  /* what is being requested */
  XtWidgetGeometry *preferred; /* what we would prefer to be */
/* Propose a new Size for Widget (Self Contained)
** Ask the widget what it things of a particular size
** and what it would prefer to be.
*/
{
  preferred->request_mode = CWWidth | CWHeight;
  preferred->height = PreferredHeight( (IconLabelWidget)w );
  preferred->width = PreferredWidth( (IconLabelWidget)w );

  if ( (intended->request_mode & (CWWidth|CWHeight)) == (CWWidth|CWHeight)
        && intended->width == preferred->width
        && intended->height == preferred->height)
    return XtGeometryYes;
  else if ( preferred->width == w->core.width &&
            preferred->height == w->core.height)
    return XtGeometryNo;  /* I don't know why but what the hey */
  else
    return XtGeometryAlmost;
}


static void
Destroy(w)
  Widget w; /* widget that is being destroyed */
/* Destroy our widget stuff (Chained Upward : Sub -> Super)
** Destroy and de-allocate anything we allocated for
** use within this widget.
*/
{
  IconLabelWidget ilw = (IconLabelWidget)w;

  XtFree( ilw->icon_label.label ); /* free our label name (if present) */
  ReleaseGCs( (IconLabelWidget)w );
}

/* ------------------------------------------------------------------------- */
/* ------------------------- Public Functions ------------------------------ */

void SetInfoPtr(w, info_ptr)
  Widget    w;
  XtPointer info_ptr;
/* Set the pointer to extra information provided.
** This information is can then be looked up fast as and when
** required by the program in response to user actions
** without needing to adjust any callback we don't provide
*/
{
  ((IconLabelWidget)w)->icon_label.info_ptr = info_ptr;
}

XPointer GetInfoPtr(w)
  Widget    w;
/* Get the pointer to extra information provided. */
{
  return ((IconLabelWidget)w)->icon_label.info_ptr;
}

/* ------------------------------------------------------------------------- */

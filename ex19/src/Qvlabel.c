/* Generated by wbuild
 * (generator version 3.3)
 */
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#line 35 "Qvlabel.widget"
#include "mls.h"
#line 36 "Qvlabel.widget"
#include "xutil.h"
#line 37 "Qvlabel.widget"
#include "micro_vars.h"
#line 38 "Qvlabel.widget"
#include "converters.h"
#line 39 "Qvlabel.widget"
#include "xtcw/WviewQV.h"
#line 40 "Qvlabel.widget"
#include "xtcw/Wlab.h"
#line 41 "Qvlabel.widget"
#include "xtcw/Gridbox.h"
#include <xtcw/QvlabelP.h>
static void _resolve_inheritance(
#if NeedFunctionPrototypes
WidgetClass
#endif
);
#line 10 "Qvlabel.widget"
static void initialize(
#if NeedFunctionPrototypes
Widget ,Widget,ArgList ,Cardinal *
#endif
);

static XtResource resources[] = {
#line 4 "Qvlabel.widget"
{XtNlabel,XtCLabel,XtRString,sizeof(((QvlabelRec*)NULL)->qvlabel.label),XtOffsetOf(QvlabelRec,qvlabel.label),XtRString,(XtPointer)NULL },
#line 5 "Qvlabel.widget"
{XtNvar,XtCVar,XtRQVar,sizeof(((QvlabelRec*)NULL)->qvlabel.var),XtOffsetOf(QvlabelRec,qvlabel.var),XtRImmediate,(XtPointer)0 },
#line 6 "Qvlabel.widget"
{XtNenable,XtCEnable,XtRQVar,sizeof(((QvlabelRec*)NULL)->qvlabel.enable),XtOffsetOf(QvlabelRec,qvlabel.enable),XtRImmediate,(XtPointer)0 },
};

QvlabelClassRec qvlabelClassRec = {
{ /* core_class part */
/* superclass   	*/  (WidgetClass) &gridboxClassRec,
/* class_name   	*/  "Qvlabel",
/* widget_size  	*/  sizeof(QvlabelRec),
/* class_initialize 	*/  NULL,
/* class_part_initialize*/  _resolve_inheritance,
/* class_inited 	*/  FALSE,
/* initialize   	*/  initialize,
/* initialize_hook 	*/  NULL,
/* realize      	*/  XtInheritRealize,
/* actions      	*/  NULL,
/* num_actions  	*/  0,
/* resources    	*/  resources,
/* num_resources 	*/  3,
/* xrm_class    	*/  NULLQUARK,
/* compres_motion 	*/  False ,
/* compress_exposure 	*/  FALSE ,
/* compress_enterleave 	*/  False ,
/* visible_interest 	*/  False ,
/* destroy      	*/  NULL,
/* resize       	*/  XtInheritResize,
/* expose       	*/  XtInheritExpose,
/* set_values   	*/  NULL,
/* set_values_hook 	*/  NULL,
/* set_values_almost 	*/  XtInheritSetValuesAlmost,
/* get_values+hook 	*/  NULL,
/* accept_focus 	*/  XtInheritAcceptFocus,
/* version      	*/  XtVersion,
/* callback_private 	*/  NULL,
/* tm_table      	*/  NULL,
/* query_geometry 	*/  XtInheritQueryGeometry,
/* display_acceleator 	*/  XtInheritDisplayAccelerator,
/* extension    	*/  NULL 
},
{ /* composite_class part */
XtInheritGeometryManager,
XtInheritChangeManaged,
XtInheritInsertChild,
XtInheritDeleteChild,
NULL
},
{ /* constraint_class part */
/* constraint_resources     */  NULL,
/* num_constraint_resources */  0,
/* constraint_size          */  sizeof(QvlabelConstraintRec),
/* constraint_initialize    */  NULL,
/* constraint_destroy       */  NULL,
/* constraint_set_values    */  NULL,
/* constraint_extension     */  NULL 
},
{ /* Gridbox_class part */
 /* dummy */  0
},
{ /* Qvlabel_class part */
 /* dummy */  0
},
};
WidgetClass qvlabelWidgetClass = (WidgetClass) &qvlabelClassRec;
static void _resolve_inheritance(class)
WidgetClass class;
{
  QvlabelWidgetClass c __attribute__((unused)) = (QvlabelWidgetClass) class;
  QvlabelWidgetClass super __attribute__((unused));
  static CompositeClassExtensionRec extension_rec = {
    NULL, NULLQUARK, XtCompositeExtensionVersion,
    sizeof(CompositeClassExtensionRec), True};
  CompositeClassExtensionRec *ext;
  ext = (XtPointer)XtMalloc(sizeof(*ext));
  *ext = extension_rec;
  ext->next_extension = c->composite_class.extension;
  c->composite_class.extension = ext;
  if (class == qvlabelWidgetClass) return;
  super = (QvlabelWidgetClass)class->core_class.superclass;
}
#line 10 "Qvlabel.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 10 "Qvlabel.widget"
static void initialize(Widget  request,Widget self,ArgList  args,Cardinal * num_args)
#else
#line 10 "Qvlabel.widget"
static void initialize(request,self,args,num_args)Widget  request;Widget self;ArgList  args;Cardinal * num_args;
#endif
#line 11 "Qvlabel.widget"
{
        if( ((QvlabelWidget)self)->qvlabel.label == NULL ) ((QvlabelWidget)self)->qvlabel.label=((QvlabelWidget)self)->core.name;
        XtVaCreateManagedWidget( ((QvlabelWidget)self)->qvlabel.label,
         wlabWidgetClass, self,
          XtNgridx,0,
          XtNgridy,0,
          XtNfill,0,
          XtNweightx,0,
          XtNweighty,0,
          NULL );

        XtVaCreateManagedWidget( "value",
          wviewQVWidgetClass, self,
          XtNgridx,1,
          XtNgridy,0,
          XtNfill,0,
          XtNweightx,0,
          XtNweighty,0,
          "var", ((QvlabelWidget)self)->qvlabel.var,
          "enable", ((QvlabelWidget)self)->qvlabel.enable,
          NULL );
}
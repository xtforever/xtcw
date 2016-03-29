#ifndef _XpAthena_h_
#define _XpAthena_h_


/* xaw3dxft: au1064@gmail.com 03.05.2013 */


/* SCCS_data: @(#) XpAthena.h	1.4 94/09/23 10:39:01
*/

/* Core, Object, RectObj, WindowObj, 
** Shell, OverrideShell, WMShell, VendorShell, TopLevelShell, ApplicationShell, 
** Constraint.
*/
#include <X11/Intrinsic.h>


/* include all the *.h files in heirarchical order */
#include <X11/Xaw/Simple.h>
#include <X11/Xaw/VendorEP.h>

/* Core */

/* Simple */
#include <X11/Xaw/Grip.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/StripChart.h>
#include <X11/Xaw/Text.h>

#include <X11/Xaw/Panner.h>

/* Label */
#include <X11/Xaw/Command.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Toggle.h>

/* Command */
#include <X11/Xaw/Repeater.h>


/* Sme */
#include <X11/Xaw/Sme.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>


/* Text */
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/TextSrc.h>
#include <X11/Xaw/AsciiSrc.h>
#include <X11/Xaw/TextSink.h>
#include <X11/Xaw/AsciiSink.h>

/* Composite and Constraint */
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Porthole.h>
#include <X11/Xaw/Tree.h>


/* Form */
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Viewport.h>

// Externals
#include "Table.h"

#endif /* _XpAthena_h_ */

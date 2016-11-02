static int down_x, down_y;

static void btndown(Widget w, XEvent* e, String* s, Cardinal* n)
{
    int x,y,x0=e->xbutton.x, y0= e->xbutton.y;
    Display *disp = XtDisplay(TopLevel);
    Window child_return;
    XTranslateCoordinates (disp, XtWindow(w), XtWindow(TopLevel),
                           x0, y0, & x, & y, & child_return);

    down_x = x;
    down_y = y;
}

static void btnmove(Widget w, XEvent* e, String* s, Cardinal* n)
{
    int x0=e->xbutton.x, y0= e->xbutton.y;

    Display *disp = XtDisplay(TopLevel);
    Window  win   = XtWindow(TopLevel);
    int screen    = DefaultScreen(disp);
    Window root   = RootWindow(disp,screen);

    int x,y;
    Window child_return;
    XTranslateCoordinates (disp, XtWindow(w), root,
                           x0, y0, & x, & y, & child_return);

    XMoveWindow(disp,win,x - down_x,y-down_y );
}



/** append to the list of client properties a property that
    tells the window manager to keep this window above others
*/
void make_stay_above(Widget top)
{
#define _NET_WM_STATE_REMOVE        0    // remove/unset property
#define _NET_WM_STATE_ADD           1    // add/set property
#define _NET_WM_STATE_TOGGLE        2    // toggle property

    Display *display = XtDisplay(top);
    int screen = DefaultScreen(display);
    Window root = RootWindow(display,screen);

    Atom wmStateAbove = XInternAtom( display, "_NET_WM_STATE_ABOVE", 1 );
    if( wmStateAbove != None ) {
        printf( "_NET_WM_STATE_ABOVE has atom of %ld\n", (long)wmStateAbove );
    } else {
        printf( "ERROR: cannot find atom for _NET_WM_STATE_ABOVE !\n" );
    }

    Atom wmNetWmState = XInternAtom( display, "_NET_WM_STATE", 1 );
    if( wmNetWmState != None ) {
        printf( "_NET_WM_STATE has atom of %ld\n", (long)wmNetWmState );
    } else {
        printf( "ERROR: cannot find atom for _NET_WM_STATE !\n" );
    }
    // set window always on top hint
    if( wmStateAbove == None ) return;

    XClientMessageEvent xclient;
    memset( &xclient, 0, sizeof (xclient) );
    //
    //window  = the respective client window
    //message_type = _NET_WM_STATE
    //format = 32
    //data.l[0] = the action, as listed below
    //data.l[1] = first property to alter
    //data.l[2] = second property to alter
    //data.l[3] = source indication (0-unk,1-normal app,2-pager)
    //other data.l[] elements = 0
    //
    xclient.type = ClientMessage;
    xclient.window = XtWindow(top); // GDK_WINDOW_XID(window);
    xclient.message_type = wmNetWmState; //gdk_x11_get_xatom_by_name_for_display( display, "_NET_WM_STATE" );
    xclient.format = 32;
    xclient.data.l[0] = _NET_WM_STATE_ADD; // add ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
    xclient.data.l[1] = wmStateAbove; //gdk_x11_atom_to_xatom_for_display (display, state1);
    xclient.data.l[2] = 0; //gdk_x11_atom_to_xatom_for_display (display, state2);
    xclient.data.l[3] = 0;
    xclient.data.l[4] = 0;
    //  gdk_wmspec_change_state( FALSE, window,
    //  gdk_atom_intern_static_string ("_NET_WM_STATE_BELOW"),
    //  GDK_NONE );
    XSendEvent( display,
                //mywin - wrong, not app window, send to root window!
                root, // !! DefaultRootWindow( display ) !!!
                False,
                SubstructureRedirectMask | SubstructureNotifyMask,
                (XEvent *)&xclient );
}

/** tell window manager to not decorated this shell window
 */
void make_borderless_window(Widget top )
{
    Window window = XtWindow(top);
    Display *display = XtDisplay(top);

    struct MwmHints {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
        long input_mode;
        unsigned long status;
    };

    enum {
        MWM_HINTS_FUNCTIONS = (1L << 0),
        MWM_HINTS_DECORATIONS =  (1L << 1),

        MWM_FUNC_ALL = (1L << 0),
        MWM_FUNC_RESIZE = (1L << 1),
        MWM_FUNC_MOVE = (1L << 2),
        MWM_FUNC_MINIMIZE = (1L << 3),
        MWM_FUNC_MAXIMIZE = (1L << 4),
        MWM_FUNC_CLOSE = (1L << 5)
    };
    struct MwmHints hints;
    Atom mwmHintsProperty = XInternAtom(display, "_MOTIF_WM_HINTS", 0);

    hints.flags = MWM_HINTS_DECORATIONS;
    hints.decorations = 0;
    XChangeProperty(display, window, mwmHintsProperty, mwmHintsProperty, 32,
                    PropModeReplace, (unsigned char *)&hints, 5);
}



static void winmove_translations(Widget w)
{
    static XtTranslations app_translations;
    static XtActionsRec action_table[] = {
        { "btndown",btndown }
        { "btnmove",btnmove }
    };

    static char translation_str[] =
        "<Btn1Motion>: btnmove\n"
        "<Btn1Down>: btndown";

    XtAppContext ctx = XtWidgetToApplicationContext(w);
    XtAppAddActions(ctx, action_table, 2 );
    app_translations = XtParseTranslationTable(translation_str);
    XtOverrideTranslations( w, app_translations );
}

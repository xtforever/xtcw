/* most important define */
#define APP_NAME "ex11"

/*
  TODO: create a conainer widget which never shrinks
        - gridbox wants to resize the shell widget

        connected (@R)

        @F playing                paused @P 1

        stopped @P 2
 */



#include "mls.h"
#include "ini_read2.h"

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xmu/Editres.h>
#include <X11/Vendor.h>
#include <X11/Xaw/XawInit.h>

#include <WcCreate.h>
#include <Xp.h>
#include "wcreg2.h"
#include <xtcw/register_wb.h>

#include "db.h"
#include "if_mpg123.h"

Widget TopLevel;
int DB;
int trace_main;

#define TRACE_MAIN 2
#define TRACE_MPG123 2

char *fallback_resources[] = {
    APP_NAME ".allowShellResize: False",
    "*WclResFiles:" APP_NAME ".ad\n",
    "*traceLevel: 2",
    NULL };

/* All Wcl applications should provide at least these Wcl options:
*/
static XrmOptionDescRec options[] = {
  { "-TraceLevel",	"*traceLevel",	XrmoptionSepArg, NULL },
  { "-ListenPort",      "*listenPort",  XrmoptionSepArg, NULL },
  WCL_XRM_OPTIONS
};

#define COLOR_NORMAL 0
#define COLOR_PLAY 1
#define COLOR_PAUSE 2

typedef struct BasicSetting {
    int traceLevel;
    char *listenPort;
    Widget widget_playlist;
    Widget widget_playsong;
    Widget widget_player_headline;
    Widget widget_playpos;

    int album_strs;
    int song_color;
    int album_color;
    int cur_song;
    int cur_album;
    int mode;
    int song_list;
} BasicSetting;

#define MODE_CONT 1

#define FLD(n)  XtOffsetOf(BasicSetting,n)
static XtResource basicSettingRes[] = {

  { NULL, NULL, XtRWidget, sizeof(Widget),
    FLD(widget_playlist), XtRString, "*playlist"
  },
  { NULL, NULL, XtRWidget, sizeof(Widget),
    FLD(widget_playsong), XtRString, "*playsong"
  },
  { NULL, NULL, XtRWidget, sizeof(Widget),
    FLD(widget_player_headline), XtRString, "*player_headline"
  },
  { NULL, NULL, XtRWidget, sizeof(Widget),
    FLD(widget_playpos), XtRString, "*playpos"
  },


  { "listenPort", "ListenPort", XtRString, sizeof(String),
   FLD(listenPort), XtRString, "10000"
  },

  { "traceLevel", "TraceLevel", XtRInt, sizeof(int),
   FLD(traceLevel), XtRImmediate, 0
  }



};
#undef FLD

struct BasicSetting SETTINGS;

void set_play_pos( Widget w, void *u, void *c )
{
    int pos = (int) c;
    TRACE(trace_main, "pos: %d", pos );
    /* position 0..10000 */
    m123_set_playpos(pos);
}

void update_play_pos(int pos)
{
    XtVaSetValues( SETTINGS.widget_playpos, XtNhandlePos,pos, NULL);
}

void test_cb( Widget w, void *u, void *c )
{
  printf("cb\n");
}

void quit_gui( Widget w, void *u, void *c )
{
    TRACE(1,"QUIT");
    XtAppSetExitFlag( XtWidgetToApplicationContext(w) );
}

static void wm_quit ( Widget w, XEvent *event, String *params,
		   Cardinal *num_params )
{
    TRACE(1,"WM_QUIT");
    XtAppSetExitFlag( XtWidgetToApplicationContext(w) );
}


/* --------------------------------------------------------------------------------------------------------------------


                        IMPLEMENTATION

  -------------------------------------------------------------------------------------------------------------------- */
static void set_color(int color);
static void play_song(int nr);

void show_songs(int album_nr )
{
    if( SETTINGS.cur_album == album_nr ) return;

    SETTINGS.cur_album = album_nr;
    SETTINGS.cur_song = -1;
    SETTINGS.song_color = -1;

    int p;
    struct d3_song *song;
    char *buf;
    int m = SETTINGS.song_list;
    struct d3_album *a = mls(DB,album_nr);
    int song_m = a->song_m;

    m_free_strings(m,1);
    m_foreach( song_m, p, song )
        {
            buf=0;
            asprintf( &buf, "%d\t%s", p,
                      (char*)m_buf(song->name) );
            m_put(m,&buf);
        }
    XtVaSetValues( SETTINGS.widget_playsong, "tableStrs", m, NULL );
    wlist4_clear_selection(SETTINGS.widget_playsong);

    XtVaSetValues( SETTINGS.widget_player_headline, "label",
                   d3_album_name( DB, album_nr ), NULL );
}


void song_toggle( Widget w, void *u, void *c )
{
    TRACE(trace_main,"");
    int song_nr = (int) c;

    int same = (song_nr == SETTINGS.cur_song);
    int state = m123_status();

    if( state == M123_PLAY ) {
        /* gleicher song ? */
        if( same ) { m123_pause(); return; }
        /* anderer song */
        set_color( COLOR_NORMAL );
        goto play_new_song;
    }

    if( state == M123_PAUSE ) {
        m123_pause();
        if( same ) {
            set_color(COLOR_PLAY);
            return;
        }
        /* anderer song */
        set_color(COLOR_NORMAL);
        goto play_new_song;
    }

    /* state == M123_STOP */
 play_new_song:
    play_song( song_nr );

}

static void play_song(int song_nr)
{
    struct d3_album *a = mls(DB,SETTINGS.cur_album );
    struct d3_song *song = mls(a->song_m, song_nr );

    SETTINGS.cur_song =  song_nr;
    SETTINGS.song_color = -1;

    char *path = m_buf(a->path);
    char *file = m_buf(song->file);
    char *buf = 0;
    asprintf( &buf, "%s/%s", path, file );
    m123_load(buf);
    TRACE(trace_main, buf );
    free(buf);
}

/* playlist becomes visible. */
void playlist_show( Widget w, void *u, void *c )
{
}

static void set_color(int color)
{
    int s = SETTINGS.cur_song;
    if( SETTINGS.song_color != color ) {
        TRACE(trace_main, "set color: nr:%d col:%d", s, color);
        wlist4_set_line_state(SETTINGS.widget_playsong, s, color );
        SETTINGS.song_color = color;
    }
}

static void set_album_color(int color)
{
    int s = SETTINGS.cur_album;
    if( SETTINGS.album_color != color ) {
        TRACE(trace_main, "set album color: nr:%d col:%d", s, color);
        wlist4_set_line_state(SETTINGS.widget_playlist, s, color );
        SETTINGS.album_color = color;
    }
}


void playlist_select( Widget w, void *u, void *c )
{
    int album_nr = (int) c;
    if( album_nr >= m_len(DB) ) return;
    puts( d3_album_name(DB, album_nr ) );
    puts( d3_album_interpret(DB, album_nr ) );

    if( SETTINGS.cur_album >= 0 ) {
        if( SETTINGS.cur_album != album_nr )
            set_album_color( COLOR_NORMAL );
    }

    show_songs( album_nr );
    SETTINGS.cur_album = album_nr;
    set_album_color( COLOR_PLAY );
}



static void print_state( int s )
{
    char *x[] ={ "wait", "stop","play","pause", "error" };
    if( s < 0 || s > 3 ) s=4;
    puts(x[s]);
}


void next_song(void)
{
    struct d3_album *a = mls(DB,SETTINGS.cur_album );
    int n = SETTINGS.cur_song +1;
    if( n < m_len(a->song_m) )
        {
            play_song(n);
        }
}


void m123_state_change(int old, int new)
{
    if( old == new ) { /* could be a postion update */
        if( new == M123_PLAY ) {
            update_play_pos( m123_playpos() );
        }
        return;
    }

    print_state( old );
    print_state( new );

    if( new == M123_PLAY ) {
        set_color( COLOR_PLAY );
        return;
    }

    if( new == M123_PAUSE ) {
        set_color( COLOR_PAUSE );
        return;
    }

    if( new == M123_STOP ) {
        set_color( COLOR_NORMAL );

        if( SETTINGS.mode == MODE_CONT )
            {
                next_song();
            }

        return;
    }
}




/******************************************************************************
**  Private Functions
******************************************************************************/
void show_album_list()
{
    int i;
    char *b;
    int s = SETTINGS.album_strs;
    if( !s ) s = SETTINGS.album_strs =
                 m_create(100,sizeof(char*));
    else
        m_free_strings(s,1);

    int cnt = m_len(DB);
    for(i=0;i<cnt;i++) {
        b=0; asprintf(&b,"%d\t%s\n\t%s", i+1,
                 d3_album_name(DB,i),
                 d3_album_interpret(DB,i) );
        m_put(s, &b );
    }
    XtVaSetValues( SETTINGS.widget_playlist, "tableStrs", s, NULL );

}


/*ARGSUSED*/
static void RegisterApplication ( Widget top )
{
    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
  RCB( top, quit_gui );
  RCB( top, playlist_select );
  RCB( top, song_toggle );
  RCB( top, playlist_show );
  RCB( top, set_play_pos );
}


static void syntax(void)
{
  puts( syntax_wcl );
  puts( "-TraceLevel <num>\n"
	"-ListenPort <num>\n" );
}


/* if the window manager closes the window, tell the
   window manager to send a message to xt. xt will
   call the wm_quit callback.
*/
void grab_window_quit(Widget top)
{
    XtAppContext app = XtWidgetToApplicationContext(top);
    /* if the user closes the window
       the Window Manager will call our quit() function
    */
    static XtActionsRec actions[] = {
        {"quit",	wm_quit}
    };
    XtAppAddActions
	(app, actions, XtNumber(actions));
    XtOverrideTranslations
	(TopLevel, XtParseTranslationTable ("<Message>WM_PROTOCOLS: quit()"));

    /* http://www.lemoda.net/c/xlib-wmclose/ */
    static Atom wm_delete_window;
    wm_delete_window = XInternAtom(XtDisplay(top), "WM_DELETE_WINDOW",
				   False);
    (void) XSetWMProtocols (XtDisplay(top), XtWindow(top),
                            &wm_delete_window, 1);
}



/******************************************************************************
*   MAIN function
******************************************************************************/
int main ( int argc, char **argv )
{
    trace_main = TRACE_MAIN;
    trace_m123 = TRACE_MPG123;
    SETTINGS.mode = MODE_CONT;

    XtAppContext app;
    signal(SIGPIPE, SIG_IGN); /* ignore broken pipe on write */
    m_init();
    DB = d3_open( "database.dat" );

    XtSetLanguageProc (NULL, NULL, NULL);
    XawInitializeWidgetSet();
    /*  -- Intialize Toolkit creating the application shell
     */
    Widget appShell = XtOpenApplication (&app, APP_NAME,
	     options, XtNumber(options),  /* resources: can be set from argv */
	     &argc, argv,
	     fallback_resources,
	     sessionShellWidgetClass,
	     NULL, 0
	   );

    /* enable Editres support */
    XtAddEventHandler(appShell, (EventMask) 0, True, _XEditResCheckMessages, NULL);

    XtAddCallback( appShell, XtNdieCallback, quit_gui, NULL );

    /* not parsed options are removed by XtOpenApplication
       the only entry left should be the program name
    */
    if (argc != 1) { m_destruct(); syntax(); exit(1); }

    TopLevel = appShell;


    /*  -- Register all application specific callbacks and widget classes
     */
    RegisterApplication ( appShell );

    /*  -- Register all Athena and Public widget classes, CBs, ACTs
     */
    XpRegisterAll ( app );

    /*  -- Create widget tree below toplevel shell using Xrm database
     */
    WcWidgetCreation ( appShell );

    /* get application resources and widget ptr */
    XtGetApplicationResources(	appShell, (XtPointer)&SETTINGS,
				basicSettingRes,
				XtNumber(basicSettingRes),
				(ArgList)0, 0 );
    /* init widgets */
    trace_level = SETTINGS.traceLevel;

    /*  init application functions
        All widgets are created, but not visible.
        functions can now communicate with widgets
    */

    /*  -- Realize the widget tree and enter the main application loop
     */
    XtRealizeWidget ( appShell );
    grab_window_quit( appShell );

    SETTINGS.cur_song = -1;
    SETTINGS.cur_album = -1;
    SETTINGS.song_color = -1;
    SETTINGS.song_list = m_create(20,sizeof(char*));

    show_album_list();
    m123_init(TopLevel, "localhost", "12300", m123_state_change );

    XtAppMainLoop ( app ); /* use XtAppSetExitFlag */
    XtDestroyWidget(appShell);
    m_free_strings(SETTINGS.song_list,0);
    m_free_strings(SETTINGS.album_strs,0);
    d3_close(DB);
    m123_destroy();
    m_destruct();
    return EXIT_SUCCESS;
}

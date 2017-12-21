/* this implements a lua server with a socket connection to a client,
   the data is slop encoded.
   this servers implements commands to start and stop a lua program
*/

#include "luaexec.h"
#include "slopnet.h"
#include "mls.h"
#include "command-parser.h"

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

static int EXIT_FLAG = 0;
static int SLN_CURRENT =-1; /* the connection, should be stored in lua state */
static void *LAST_CP_FUNC = 0;



static struct SCREEN_SIZE_st {
    int gw,gh, w,h, update;
} SCREEN_SIZE; /* zero initialized */




/* get milliseconds */
int get_time(void)
{
    struct timespec t;

    clock_gettime( CLOCK_MONOTONIC, &t );
    
    unsigned int ms = t.tv_nsec / 1000000U; /* 1E-9 to 1E-3 nano to milli */
    ms += t.tv_sec * 1000;
    return ms;
}

int elapsed_time(int time)
{
    unsigned int t1,t2;
    t1 = time;
    t2 = get_time();
    return t1-t2;
}


/* 50ms auf nachrichten warten
   alle nachrichten im MSG_STACK speichern
   sind mehr als 20 nachrichten im stack die älteste löschen
*/
#define COMM_TIMEOUT_MS 1000
#define MSG_STACK_MAX 20
static int MSG_STACK;
static int MSG_STACK_AVAIL = 0;
struct msg_st {
    int time;
    int msg;
};

static void add_msg(int stack, int msg)
{
    struct msg_st *d;

    if( m_len( stack ) >= MSG_STACK_MAX ) {
	d=mls(stack,0);
	m_free( d->msg );
	m_del( stack, 0);
    }
    
    d = m_add( stack);
    d->time = get_time();
    d->msg = m_dub(msg);
    MSG_STACK_AVAIL = 1;
}






static int luaexec_clrscr(lua_State* state)
{
  sln_printf(SLN_CURRENT, "CLRSCR");
  
 
  return 0;
}

static int luaexec_circle(lua_State* state)
{
  // The number of function arguments will be on top of the stack.
  int args = lua_gettop(state);
  if( args != 3 ) goto error;

  int x = lua_tonumber(state, 1);
  int y = lua_tonumber(state, 2);
  int r = lua_tonumber(state, 3);
  
  printf("circle %d %d %d\n", x, y, r );
  sln_printf(SLN_CURRENT, "CIRCLE:%d %d %d",x,y,r);
  
 error:
  return 0;
}

static int luaexec_gotoxy(lua_State* state)
{
  // The number of function arguments will be on top of the stack.
  int args = lua_gettop(state);
  if( args != 2 ) goto error;

  int x = lua_tonumber(state, 1);
  int y = lua_tonumber(state, 2);

  sln_printf(SLN_CURRENT, "GOTOXY:%d %d",x,y);
  
 error:
  return 0;
}

static int luaexec_write(lua_State* state)
{
  // The number of function arguments will be on top of the stack.
  int args = lua_gettop(state);
  if( args != 1 ) goto error;

  const char *s = lua_tostring(state, 1);

  sln_printf(SLN_CURRENT, "WRITE:%s",s);
  
 error:
  return 0;
}


static int luaexec_rectf(lua_State* state)
{
    const char *s = "";
    // The number of function arguments will be on top of the stack.
    int args = lua_gettop(state);
    if( args != 4 && args != 5 ) goto error;

    int x = lua_tonumber(state, 1);
    int y = lua_tonumber(state, 2);
    int w = lua_tonumber(state, 3);
    int h = lua_tonumber(state, 4);
    if( args == 5 ) s = lua_tostring(state,5);
  
    printf("rect %d %d %d %d %s\n", x, y, w,h,s  );
    sln_printf(SLN_CURRENT, "RECTF:%d %d %d %d %s",x,y,w,h, s);
  
 error:
  return 0;
}



static int l_row_push(lua_State *L)
{
    lua_createtable(L, 0, 4); /* creates and pushes new table on top of Lua stack */

    lua_pushstring(L, "jens" ); /* Pushes table value on top of Lua stack */
    lua_setfield(L, -2, "name");  /* table["name"] = row->name. Pops key value */

    lua_pushstring(L, "01.01.1990" );
    lua_setfield(L, -2, "date");

    lua_pushstring(L, "11.15.2.1" );
    lua_setfield(L, -2, "ip");

    lua_pushstring(L, "nbX54" );
    lua_setfield(L, -2, "custom");

    /* Returning one table which is already on top of Lua stack. */
    return 1;
}


const char* msg_compare( const char *s )
{
    int len = strlen(s);
    struct msg_st *d;    
    int p = m_len(MSG_STACK);
    while( p-- ) {
	d=mls(MSG_STACK,p);
	if( strncmp(mls(d->msg,0), s, len ) == 0 ) return mls(d->msg,len);
    }
    return NULL;
}


static int measure_screen(lua_State* L)
{
    if(! SCREEN_SIZE.update ) { 
	sln_printf( SLN_CURRENT, "MEASURE_SCREEN" );


	/* warte bis antwort oder timeout */
	
       while( sln_select_timeout(SLN_CURRENT, 2000) ) {
           if( SCREEN_SIZE.update ) break;

       }
     }
    /* creates and pushes new table on top of Lua stack */
    lua_createtable(L, 0, 4); 
    lua_pushinteger(L, SCREEN_SIZE.w ); /* Pushes table value on top of Lua stack */
    lua_setfield(L, -2, "width");  /* table["width"] = w */
    lua_pushinteger(L, SCREEN_SIZE.h ); /* Pushes table value on top of Lua stack */
    lua_setfield(L, -2, "height");  /* table["height"] = h */
    lua_pushinteger(L, SCREEN_SIZE.gw ); /* Pushes table value on top of Lua stack */
    lua_setfield(L, -2, "gwidth");  /* table["height"] = h */
    lua_pushinteger(L, SCREEN_SIZE.gh ); /* Pushes table value on top of Lua stack */
    lua_setfield(L, -2, "gheight");  /* table["height"] = h */

    return 1;     /* Returning one table on top of Lua stack. */
}


static int howdy(lua_State* state)
{
  // The number of function arguments will be on top of the stack.
  int args = lua_gettop(state);

  printf("howdy() was called with %d arguments:\n", args);

  for ( int n=1; n<=args; ++n) {
    printf("  argument %d: '%s'\n", n, lua_tostring(state, n));
  }

  
  // Push the return value on top of the stack. NOTE: We haven't popped the
  // input arguments to our function. To be honest, I haven't checked if we
  // must, but at least in stack machines like the JVM, the stack will be
  // cleaned between each function call.

  lua_pushnumber(state, 123);

  // Let Lua know how many return values we've passed
  return 1;
}


static void print_error(lua_State* state) {
  // The error message is on top of the stack.
  // Fetch it, print it and then pop it off the stack.
  const char* message = lua_tostring(state, -1);
  sln_printf(SLN_CURRENT,"ERR:%s", message);
  lua_pop(state, 1);
}




static int cmd_put(int msg,void *ctx)
{
    return 0;
}

static int cmd_get(int msg, void *ctx)
{
    TRACE(1,"");
    int sln = (intptr_t) ctx;
    sln_printf(sln, "HELLO" );
    return 0;
}    

/* parse client reply to MEASURE_SCREEN */
static int cmd_measure_screen(int msg, void *ctx)
{
    TRACE(1,"");
    int w,h,gw,gh;
    if( sscanf( mls(msg,15), "%d %d %d %d", &w, &h, &gw, &gh) == 4 ) {
	SCREEN_SIZE.w = w;
	SCREEN_SIZE.h = h;
	SCREEN_SIZE.gw = gw;
	SCREEN_SIZE.gh = gh;
	SCREEN_SIZE.update = 1;
	
    }
    else sln_printf(SLN_CURRENT, "ERR:MEASURE_SCREEN FORMAT PARAMETER");
    return 0;
}

static void execute_msg(int msg, void *ctx)
{

    TRACE(1,"");
    cp_func_t fn = cp_lookup(msg);
    
    if(fn) {
	LAST_CP_FUNC = fn;
	fn( (void*)(intptr_t)msg, ctx);
    } else WARN("unknow command %s", (char*) m_buf(msg) );
}

/**
 * this function is called if some data has arrived inside lua select loop 
 */
static void lua_input_cb(int error, int msg, int sln, void *ctx)
{
    if( error ) return;
    m_putc(msg,0);
    TRACE(1,"Msg: %s\n", (char*)mls(msg,0));
    execute_msg(msg,(void*)(intptr_t)sln);
}

static void init_commands()
{
    cp_init();
    cp_add( "PUT:", cmd_put );
    cp_add( "GET", cmd_get );

    /* reply from client */
    cp_add( "MEASURE_SCREEN:", cmd_measure_screen );
}

static int luaexecute(char* prog, int fd)
{
    TRACE(1,"executing lua code");
    int result;
    lua_State *state = luaL_newstate();

    // Make standard libraries available in the Lua object
    luaL_openlibs(state);

    /* register c functions */ 
    lua_register(state, "howdy", howdy);
    lua_register(state, "circle", luaexec_circle);
    lua_register(state, "clrscr", luaexec_clrscr);
    lua_register(state, "measure_screen", measure_screen);
    lua_register(state, "rectf", luaexec_rectf);
    lua_register(state, "write", luaexec_write);
    lua_register(state, "gotoxy", luaexec_gotoxy);


    /* setup a slop encoded connection inside lua */
    int sln = sln_init();
    SLN_CURRENT = sln;
    sln_accept( sln, fd, lua_input_cb, state );

    /* setup our command parser */
    init_commands();

    result = luaL_dostring(state, prog );

    if ( result != LUA_OK ) {
	print_error(state);
    }

    lua_close(state);
    sln_free_open_fd(sln); SLN_CURRENT=0;
    return result;
}


static char * PROG_READY = 0;

/**
 * this function is called if some data has arrived inside main select loop 
 */
static void program_input_cb(int error, int msg, int sln, void *ctx)
{
    if( error ) return;
    m_putc(msg,0);
    TRACE(1,"Msg: %s\n", (char*)mls(msg,0));

    if( strncmp( (char*)mls(msg,0), "PUT:", 4 )== 0 )
	{
	    PROG_READY = strdup(mls(msg,4)); 
	}
}


/* lua subprocess, execute lua and exit child */
void luaexec_main(int fd)
{

    /* for now, this is the state of the current connection */
    MSG_STACK = m_create(MSG_STACK_MAX, sizeof (struct msg_st));


    /* setup a slop encoded connection */
    int sln = sln_init();
    SLN_CURRENT = sln;
    sln_accept( sln, fd, program_input_cb, NULL );

    TRACE(1,"waiting for lua code");    
    /* exit flag can be set via external command */
    while( ! EXIT_FLAG )
	{
	    sln_select_timeout(sln, 200);
	    if( PROG_READY ) {
		sln_printf(sln, "RUN" );
		int result = luaexecute( PROG_READY, fd );
		/* tell client lua program is finished */
		sln_printf(sln, "EXIT:%d", result );
		free(PROG_READY);
		PROG_READY=0;
		TRACE(1,"waiting for lua code");
	    }
	}

    /* inform our client that we no longer receive messages */
    sln_printf(sln, "DEAD" );
    sln_destruct();
    m_free(MSG_STACK);
    m_destruct();
    exit(0);
}

	    

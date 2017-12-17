#include "luaexec.h"
#include "slopnet.h"

#include <stdio.h>
#include <assert.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>



int SLN_CURRENT; /* the connection, should be stored in lua state */


int luaexec_clrscr(lua_State* state)
{
  sln_printf(SLN_CURRENT, "CLRSCR");
  
 
  return 0;
}

int luaexec_circle(lua_State* state)
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



int l_row_push(lua_State *L)
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


int measure_screen(lua_State* state)
{
    return l_row_push(state);
    
}


int howdy(lua_State* state)
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


void print_error(lua_State* state) {
  // The error message is on top of the stack.
  // Fetch it, print it and then pop it off the stack.
  const char* message = lua_tostring(state, -1);
  puts(message);
  lua_pop(state, 1);
}

int luaexecute(char* prog, void *ctx)
{

    /* for now, ctx is the current connection */
    SLN_CURRENT = (intptr_t) ctx;
    
    int result;
    lua_State *state = luaL_newstate();

    // Make standard libraries available in the Lua object
    luaL_openlibs(state);

    /* register c functions */ 
    lua_register(state, "howdy", howdy);
    lua_register(state, "circle", luaexec_circle);
    lua_register(state, "clrscr", luaexec_clrscr);
    lua_register(state, "measure_screen", measure_screen);

    result = luaL_dostring(state, prog );

    if ( result != LUA_OK ) {
	print_error(state);
    }

    lua_close(state);
    return result;
}

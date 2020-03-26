#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace {

    void WithLua(lua_State* lua, const char* chunk, int nargs, int nresults) {
        // Load (compile) Lua script.
        //
        // The script will be placed at the top of the Lua stack as a "chunk"
        // (one unit of compilation, a block of code; think of it as a function
        // object, something you can immediately call and get a return value,
        // or store and call later if desired.
        (void)luaL_loadstring(lua, chunk);

        // Insert the chunk below the arguments being passed to it.
        lua_insert(lua, -nargs - 1);

        // Call the chunk to execute it.
        (void)lua_call(lua, nargs, nresults);
    }

}

int main(int argc, char* argv[]) {
    // Create the Lua instance.
    const auto lua = luaL_newstate();

    // Load standard Lua libraries.
    //
    // Temporarily disable the garbage collector as we load the
    // libraries, to improve performance
    // (http://lua-users.org/lists/lua-l/2008-07/msg00690.html).
    lua_gc(lua, LUA_GCSTOP, 0);
    luaL_openlibs(lua);
    lua_gc(lua, LUA_GCRESTART, 0);

    // Call Lua to obtain a function, and use the Lua registry
    // to stash it for our use later.
    WithLua(lua, R"lua(
        return function(x, y)
            return math.floor(x + y + 0.5)
        end
    )lua", 0, 1);
    const auto ourRegistryIndex = luaL_ref(lua, LUA_REGISTRYINDEX);

    // Recover the stashed Lua function from the Lua registry
    // and call it to perform a computation.
    (void)lua_rawgeti(lua, LUA_REGISTRYINDEX, ourRegistryIndex);
    lua_pushnumber(lua, 14.9);
    lua_pushnumber(lua, 27.3);
    (void)lua_call(lua, 2, 1);
    const auto answer = (int)lua_tointeger(lua, 1);
    lua_pop(lua, 1);
    (void)printf("The answer is %d.\n", answer);

    // Drop our Lua registry entry now that we no longer need it.
    luaL_unref(lua, LUA_REGISTRYINDEX, ourRegistryIndex);

    // All done!
    return EXIT_SUCCESS;
}

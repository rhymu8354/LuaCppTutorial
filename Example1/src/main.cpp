#include <stdio.h>
#include <stdlib.h>
#include <string>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace {

    int AddInLua(lua_State* lua) {
        // Load (compile) Lua script.
        //
        // The script will be placed at the top of the Lua stack as a "chunk"
        // (one unit of compilation, a block of code; think of it as a function
        // object, something you can immediately call and get a return value,
        // or store and call later if desired.
        (void)luaL_loadstring(lua, "return 15 + 27");

        // Call the chunk to execute it.
        //
        // Note: we don't pass any arguments, but we expect one return value.
        (void)lua_call(lua, 0, 1);

        // Pop the return value off the Lua stack and return it.
        const auto result = (int)lua_tointeger(lua, 1);
        lua_pop(lua, 1);
        return result;
    }

}

int main(int argc, char* argv[]) {
    // Create the Lua instance.
    const auto lua = luaL_newstate();

    // Use Lua to perform a simple calculation.
    const auto answer = AddInLua(lua);
    (void)printf("The answer is %d.\n", answer);

    // Destroy the Lua instance.
    lua_close(lua);

    // All done!
    return EXIT_SUCCESS;
}

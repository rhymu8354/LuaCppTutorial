#include <stdio.h>
#include <stdlib.h>
#include <string>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace {

    void* LuaAllocator(void* ud, void* ptr, size_t osize, size_t nsize) {
        if (nsize == 0) {
            free(ptr);
            return NULL;
        } else {
            return realloc(ptr, nsize);
        }
    }

    struct LuaReaderState {
        const std::string* chunk = nullptr;
        bool read = false;
    };

    const char* LuaReader(lua_State* lua, void* data, size_t* size) {
        LuaReaderState* state = (LuaReaderState*)data;
        if (state->read) {
            return NULL;
        } else {
            state->read = true;
            *size = state->chunk->length();
            return state->chunk->c_str();
        }
    }

    int LuaTraceback(lua_State* lua) {
        const char* message = lua_tostring(lua, 1);
        if (message == NULL) {
            if (!lua_isnoneornil(lua, 1)) {
                if (!luaL_callmeta(lua, 1, "__tostring")) {
                    lua_pushliteral(lua, "(no error message)");
                }
            }
        } else {
            luaL_traceback(lua, lua, message, 1);
        }
        return 1;
    }

    bool LoadScript(lua_State* lua, const std::string& script) {
        LuaReaderState luaReaderState;
        luaReaderState.chunk = &script;
        const auto loadResult = lua_load(lua, LuaReader, &luaReaderState, "=example", "t");
        return (loadResult == LUA_OK);
    }

    void DemonstrateLoadError(lua_State* lua) {
        if (LoadScript(lua, "syntax(error")) {
            (void)fprintf(stderr, "That should have been a syntax error!\n");
        } else {
            (void)printf(
                "Loading a script with syntax error returns this string: %s\n",
                lua_tostring(lua, -1)
            );
        }
        lua_pop(lua, 1);
    }

    void DemonstrateCallError(lua_State* lua) {
        // Push a custom message delegate onto the Lua stack that we
        // will use to process runtime errors.
        lua_pushcfunction(lua, LuaTraceback);

        // Load (compile) Lua script.
        (void)LoadScript(lua, "foobar()");

        // Call the chunk to execute it, but make it a "protected" call,
        // meaning we catch runtime errors if they happen.
        //
        // The last argument is the Lua stack index of where we placed
        // the custom message delegate what we want to use.
        const int luaPCallResult = lua_pcall(lua, 0, 0, -2);
        switch(luaPCallResult) {
            case LUA_OK:
                (void)fprintf(stderr, "That should have been a runtime error!\n");
                break;

            case LUA_ERRRUN:
                (void)printf(
                    "Calling a script with runtime error, we catch the "
                    "error which yields this string:\n"
                    "--------------------------------------------------------\n"
                    "%s\n"
                    "--------------------------------------------------------\n",
                    lua_tostring(lua, -1)
                );
                lua_pop(lua, 1);
                break;

            case LUA_ERRMEM:
                (void)fprintf(
                    stderr,
                    "This only happens if a memory allocation error occurs!\n"
                );
                lua_pop(lua, 1);
                break;

            case LUA_ERRERR:
                (void)fprintf(
                    stderr,
                    "This only happens if an error occurs "
                    "while running our custom message delegate!\n"
                );
                lua_pop(lua, 1);
                break;

            case LUA_ERRGCMM:
                (void)fprintf(
                    stderr,
                    "This only happens if an error occurs"
                    "during garbage collection!\n"
                );
                lua_pop(lua, 1);
                break;

            default:
                (void)fprintf(
                    stderr,
                    "Something very wrong happened; an unexpected result!\n"
                );
                lua_pop(lua, 1);
                break;
        }

        // Pop the custom message delegate off the Lua stack.
        lua_pop(lua, 1);
    }

    int AddAndRoundInLua(lua_State* lua, double a, double b) {
        // Load (compile) Lua script.
        //
        // This time we're going to pass two arguments.  The Lua code will add
        // them together, round the sum to the nearest integer using the
        // `math.floor` library function, and return the result.
        //
        // The ellipsis (...) is a special syntax in Lua which expands to
        // the actual arguments of the call.  Note that it's a direct
        // expansion, not a list or table, so `a = ...[1]` would not work
        // here (unless of course the first argument was a list and we
        // wanted to extract that list's first element).
        (void)LoadScript(
            lua,
            R"(
                local a, b = ...
                local ab = a + b
                return math.floor(ab + 0.5)
            )"
        );

        // Call the chunk to execute it.
        //
        // Note that we have to tell lua_call the number of arguments we
        // passed, and the number of return values we want to get back.
        lua_pushnumber(lua, a);
        lua_pushnumber(lua, b);
        (void)lua_call(lua, 2, 1);

        // Pop the return value off the Lua stack and return it.
        const auto result = (int)lua_tointeger(lua, 1);
        lua_pop(lua, 1);
        return result;
    }

}

int main(int argc, char* argv[]) {
    // Create the Lua instance.
    //
    // Here we use a custom allocator, giving us complete control over
    // the dynamic memory allocated by Lua.
    const auto lua = lua_newstate(LuaAllocator, NULL);

    // Load standard Lua libraries.
    //
    // Temporarily disable the garbage collector as we load the
    // libraries, to improve performance
    // (http://lua-users.org/lists/lua-l/2008-07/msg00690.html).
    lua_gc(lua, LUA_GCSTOP, 0);
    luaL_openlibs(lua);
    lua_gc(lua, LUA_GCRESTART, 0);

    // This will intentionally try to load a script with a syntax error, to
    // demonstrate how to handle it.
    DemonstrateLoadError(lua);

    // This will intentionally run a script which generates a runtime error, to
    // demonstrate how to handle it.
    DemonstrateCallError(lua);

    // Use Lua to perform a slightly more difficult calculation.
    const auto answer = AddAndRoundInLua(lua, 14.9, 27.3);
    (void)printf("The answer is %d.\n", answer);

    // Destroy the Lua instance.
    lua_close(lua);

    // All done!
    return EXIT_SUCCESS;
}

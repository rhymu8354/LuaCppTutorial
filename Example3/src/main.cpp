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

    void PushSimpleValue(lua_State* lua, int value) {
        // Create userdata value, which is essentially a pointer to
        // memory allocated by Lua that is shared between C++ and Lua.
        auto& udata = *(int*)lua_newuserdata(lua, sizeof(int));

        // Lua only allocates the memory.  It's up to us to initialize it
        // with values.  Since this is a simple value, just copy into it.
        udata = value;

        // Construct a simple metatable for the userdata which allows Lua
        // to call the userdata like a function, returning the value
        // stored inside the userdata.
        lua_newtable(lua);
        lua_pushstring(lua, "__call");
        lua_pushcfunction(lua, [](lua_State* lua){
            auto& udata = *(int*)lua_touserdata(lua, 1);
            lua_pushinteger(lua, udata);
            return 1;
        });
        lua_settable(lua, -3);
        lua_setmetatable(lua, -2);
    }

    struct SimpleStruct {
        int x = 0;
        int y = 0;
    };

    void PushSimpleStruct(lua_State* lua, const SimpleStruct& simpleStruct) {
        // Create userdata value, which is essentially a pointer to
        // memory allocated by Lua that is shared between C++ and Lua.
        auto& udata = *(SimpleStruct*)lua_newuserdata(lua, sizeof(SimpleStruct));

        // Lua only allocates the memory.  It's up to us to initialize it
        // with values.  Since this is a simple structure, just copy into it.
        udata = simpleStruct;

        // Construct a simple metatable for the userdata which allows Lua
        // to index the userdata like a table, returning the values
        // of the fields stored inside the structure.
        lua_newtable(lua);
        lua_pushstring(lua, "__index");
        lua_pushcfunction(lua, [](lua_State* lua){
            auto& udata = *(SimpleStruct*)lua_touserdata(lua, 1);
            const std::string key = lua_tostring(lua, 2);
            if (key == "x") {
                lua_pushinteger(lua, udata.x);
            } else if (key == "y") {
                lua_pushinteger(lua, udata.y);
            } else {
                lua_pushnil(lua);
            }
            return 1;
        });
        lua_settable(lua, -3);
        lua_setmetatable(lua, -2);
    }

    struct TestObject {
        int v;
        std::string s;
    };

    void PushOwnedObject(lua_State* lua, TestObject&& testObject) {
        // Create userdata value, which is essentially a pointer to
        // memory allocated by Lua that is shared between C++ and Lua.
        //
        // For an owned object, we will need to properly destroy
        // the object once its gets garbage-collected.
        auto udata = (TestObject*)lua_newuserdata(lua, sizeof(TestObject));

        // Lua only allocates the memory.  It's up to us to initialize it with
        // values.
        //
        // Since this is an object with a destructor (remember, the object
        // holds a `std::string`, which has a destructor), use placement-new
        // with rvalue-reference argument to move the given object into
        // the userdata.
        new (udata) TestObject(std::move(testObject));

        // Construct a metatable for the userdata which allows
        // Lua to interact with the object in two ways:
        // 1) The `__gc` metamethod tells Lua that this object has a
        //    "finalizer" or function which needs to be called to clean up the
        //    object before its memory is garbage-collected.  In our finalizer,
        //    destroy the owned C++ object by calling its destructor explicitly.
        // 2) The `__index` metamethod allows Lua to index the userdata like a
        //    table, returning the values of the fields stored inside the
        //    structure.
        lua_newtable(lua);
        lua_pushstring(lua, "__gc");
        lua_pushcfunction(lua, [](lua_State* lua){
            (void)printf("Releasing shared object.\n");
            auto udata = (TestObject*)lua_touserdata(lua, 1);
            udata->~TestObject();
            return 0;
        });
        lua_settable(lua, -3);
        lua_pushstring(lua, "__index");
        lua_pushcfunction(lua, [](lua_State* lua){
            auto& udata = *(TestObject*)lua_touserdata(lua, 1);
            const std::string key = lua_tostring(lua, 2);
            if (key == "v") {
                lua_pushinteger(lua, udata.v);
            } else if (key == "s") {
                lua_pushlstring(lua, udata.s.c_str(), udata.s.length());
            } else {
                lua_pushnil(lua);
            }
            return 1;
        });
        lua_settable(lua, -3);
        lua_setmetatable(lua, -2);
    }

    void PushSharedObject(lua_State* lua, const std::shared_ptr< TestObject >& testObject) {
        // Create userdata value, which is essentially a pointer to
        // memory allocated by Lua that is shared between C++ and Lua.
        //
        // For a shared object, we leverage `std::shared_ptr` to allow
        // Lua to own its own reference to the shared object.
        auto udata = (std::shared_ptr< TestObject >*)lua_newuserdata(
            lua,
            sizeof(std::shared_ptr< TestObject >)
        );

        // Lua only allocates the memory.  It's up to us to initialize it with
        // values.
        //
        // Since this is an object shared via `std::shared_ptr`, use
        // placement-new to make a new reference to shared object, storing
        // the reference inside the userdata.
        new (udata) std::shared_ptr< TestObject >(testObject);

        // Construct a metatable for the userdata which allows
        // Lua to interact with the object in two ways:
        // 1) The `__gc` metamethod tells Lua that this object has a
        //    "finalizer" or function which needs to be called to clean up the
        //    object before its memory is garbage-collected.  In our finalizer,
        //    destroy Lua's reference to the shared C++ object by calling the
        //    shared-pointer destructor explicitly.
        // 2) The `__index` metamethod allows Lua to index the userdata like a
        //    table, returning the values of the fields stored inside the
        //    structure.
        lua_newtable(lua);
        lua_pushstring(lua, "__gc");
        lua_pushcfunction(lua, [](lua_State* lua){
            (void)printf("Releasing shared object.\n");
            auto udata = (std::shared_ptr< TestObject >*)lua_touserdata(lua, 1);
            udata->~shared_ptr< TestObject >();
            return 0;
        });
        lua_settable(lua, -3);
        lua_pushstring(lua, "__index");
        lua_pushcfunction(lua, [](lua_State* lua){
            auto& udata = **(std::shared_ptr< TestObject >*)lua_touserdata(lua, 1);
            const std::string key = lua_tostring(lua, 2);
            if (key == "v") {
                lua_pushinteger(lua, udata.v);
            } else if (key == "s") {
                lua_pushlstring(lua, udata.s.c_str(), udata.s.length());
            } else {
                lua_pushnil(lua);
            }
            return 1;
        });
        lua_settable(lua, -3);
        lua_setmetatable(lua, -2);
    }

    void WithLua(lua_State* lua, const char* chunk, int nargs) {
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
        (void)lua_call(lua, nargs, 0);
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

    // Pass by value a simple integer value as a Lua "userdata" and give it
    // to a Lua script to use.
    PushSimpleValue(lua, 42);
    WithLua(lua, R"lua(
        local udata = ...
        print("Simple value: " .. udata())
    )lua", 1);

    // Pass by value a simple structure as a Lua "userdata" and give it
    // to a Lua script to use.
    SimpleStruct xy;
    xy.x = 4;
    xy.y = -7;
    PushSimpleStruct(lua, xy);
    WithLua(lua, R"lua(
        local udata = ...
        print("Simple struct: (x=" .. udata.x .. ", y=" .. udata.y .. ")")
    )lua", 1);

    // Move an object into Lua as a "userdata" which takes ownership
    // of the object, and give it to a Lua script to use.
    TestObject obj1;
    obj1.v = 99;
    obj1.s = "Hello,";
    PushOwnedObject(lua, std::move(obj1));
    WithLua(lua, R"lua(
        local udata = ...
        print("Owned object: (v=" .. udata.v .. ", s='" .. udata.s .. "')")
    )lua", 1);

    // Share an object with Lua using a "userdata" which shares ownership
    // of the object.  Then modify the object's state from C++, and
    // verify that Lua can see the changes.
    //
    // To make things even more interesting, release the C++ reference to
    // the shared object before asking Lua to use the object, and verify
    // Lua properly finalizes (disposes of during garbage collection)
    // its reference to the object by attaching a deleter to the shared
    // pointer that will print something once the actual object is destroyed.
    std::shared_ptr< TestObject > obj2{
        new TestObject(),
        [&](TestObject* p){
            delete p;
            (void)printf("The shared object was destroyed!\n");
        }
    };
    obj2->v = 77;
    obj2->s = "World!";
    PushSharedObject(lua, obj2);
    obj2->v = 78;
    obj2->s = "PogChamp!";
    obj2.reset();
    WithLua(lua, R"lua(
        local udata = ...
        print("Shared object: (v=" .. udata.v .. ", s='" .. udata.s .. "')")
    )lua", 1);

    // Destroy the Lua instance.
    //
    // Note that since Lua finalizes objects during garbage collection,
    // the shared object above might not actually be destroyed until
    // this point.
    (void)printf("About to destroy Lua instance...\n");
    lua_close(lua);
    (void)printf("Lua instance destroyed.\n");

    // All done!
    return EXIT_SUCCESS;
}

#include <string>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

/*md
## LuaConfig

To set config from a lua script, you need to call `setConfig(key, value)` in the lua script:

```lua
setConfig("print", "Call ZicBox print from Lua.")
setConfig("PLUGIN_COMPONENT", "Encoder2 ./plugins/components/build/libzic_Encoder2Component.so")
setConfig("LOAD_HOST", "config.cfg")
setConfig("VIEW", "Main")

-- Let's create an encoder function
function setEncoder (x, y, width, height, encoder_id, value)
  setConfig("COMPONENT", "Encoder2" .. x .. " " .. y .. " " .. width .. " " .. height)
  setConfig("ENCODER_ID", encoder_id)
  setConfig("VALUE", value)
end

setEncoder(10, 10, 100, 100, 1, "FM DECAY_1")

-- ...
```

*/
static int setConfigFn(lua_State* L)
{
    lua_getglobal(L, "callbackPtr");
    void (*callback)(char* command, char* params, const char* filename) = (void (*)(char* command, char* params, const char* filename))lua_touserdata(L, -1);

    lua_Debug ar;
    lua_getstack(L, 1, &ar);
    lua_getinfo(L, "S", &ar);
    const char* filename = ar.source + 1; // +1 to remove leading @
    // printf("yoyoyo %s\n", filename);

    const char* key = lua_tostring(L, 1);
    const char* value = lua_tostring(L, 2);

    callback((char*)key, (char*)value, filename);

    return 0;
}

void config(std::string filename, void (*callback)(char* command, char* params, const char* filename))
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    lua_pushlightuserdata(L, (void*)callback);
    lua_setglobal(L, "callbackPtr");
    lua_register(L, "setConfig", setConfigFn);

#ifdef IS_RPI
    lua_pushboolean(L, true);
#else
    lua_pushboolean(L, false);
#endif
    lua_setglobal(L, "IS_RPI");

    luaL_dofile(L, filename.c_str());
    lua_close(L);
}
}
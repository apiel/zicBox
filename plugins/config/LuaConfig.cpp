#include <string>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

static int setConfigFn(lua_State* L)
{
    lua_getglobal(L, "callbackPtr");
    void (*callback)(char* command, char* params, const char* filename) = (void (*)(char* command, char* params, const char* filename))lua_touserdata(L, -1);

    lua_Debug ar;
    lua_getstack(L, 1, &ar);
    lua_getinfo(L, "S", &ar);
    const char * filename = ar.source + 1; // +1 to remove leading @
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
    luaL_dofile(L, filename.c_str());
    lua_close(L);
}
}
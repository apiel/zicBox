#ifndef _LUA_CONFIG_H_
#define _LUA_CONFIG_H_

#include "log.h"

#include <string>
#include <vector>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

struct Var {
    std::string key;
    std::string value;
};

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

void luaConfig(std::string filename, void (*callback)(char* command, char* params, const char* filename), std::vector<Var> variables)
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    lua_pushlightuserdata(L, (void*)callback);
    lua_setglobal(L, "callbackPtr");
    lua_register(L, "zic", setConfigFn);

#ifdef IS_RPI
    lua_pushboolean(L, true);
#else
    lua_pushboolean(L, false);
#endif
    lua_setglobal(L, "IS_RPI");

    for (int i = 0; i < variables.size(); i++) {
        lua_pushstring(L, variables[i].key.c_str());
        lua_setglobal(L, variables[i].value.c_str());
    }

    int ret =luaL_dofile(L, filename.c_str());
    if (ret != 0) {
        logError("Lua error [0x%x]: %s", ret, lua_tostring(L, -1));
    }
    lua_close(L);
}
}

#endif
#ifndef WRAP_ENGINE_H_
#define WRAP_ENGINE_H_

#include <lua.hpp>
#include "engine.h"
#include "entity.h"
#include "scene.h"

///getTime()
int wrap_engine_getTime(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 0) {
        return luaL_argerror(L, 1, "getTime()");
    }
    DTRACETIME("getTime");
    lua_pushinteger(L, SDL_GetTicks());
    return 1;
}


///loadScript(name)
int wrap_engine_loadScript(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 1 || !lua_isstring(L, -1)) {
        return luaL_argerror(L, 1, "loadScript(name)");
    }
    const char *name=lua_tostring(L, -1);
    DTRACETIME("loadScript(%s)",name);
    engine_deferredRunScript(name);
    return 0;
}

///centerOnEntity(entity)
int wrap_engine_centerOnEntity(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 1 || luaL_getmetafield(L, -1, "id") == 0 || luaL_getmetafield(L, -2, "type") == WRAP_TYPE_ENTITY) {
        return luaL_argerror(L, 0, "valid entity needed");
    }
    int id=lua_tointeger(L, -2);
    Entity *e=scene_getEntityContainer()[id];
    DTRACETIME("centerOnEntity");
    engine_centerOnEntity(e);
    return 0;
}

///centerOnPlayer()
int wrap_engine_centerOnPlayer(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 0) {
        return luaL_argerror(L, 0, "centerOnPlayer()");
    }
    DTRACETIME("centerOnPlayer");
    engine_centerOnPlayer();
    return 0;
}

void wrap_wrapEngine(lua_State *L) {

    lua_newtable(L);
    lua_pushstring(L, "centerOnPlayer");
    lua_pushcfunction(L, wrap_engine_centerOnPlayer);
    lua_rawset(L, -3);
    lua_pushstring(L, "centerOnEntity");
    lua_pushcfunction(L, wrap_engine_centerOnEntity);
    lua_rawset(L, -3);
    lua_pushstring(L, "loadScript");
    lua_pushcfunction(L, wrap_engine_loadScript);
    lua_rawset(L, -3);
    lua_pushstring(L, "getTime");
    lua_pushcfunction(L, wrap_engine_getTime);
    lua_rawset(L, -3);


    lua_setglobal(L, "engine");
}

#endif /* WRAP_ENGINE_H_ */

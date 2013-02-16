#ifndef WRAP_PLAYER_H_
#define WRAP_PLAYER_H_

#include <lua.hpp>
#include "player.h"

///setSpawnPoint(x,y)
int wrap_player_setSpawnPoint(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 2 || !lua_isnumber(L, -1) || !lua_isnumber(L, -2)) {
        return luaL_argerror(L, 1, "setSpawnPoint(x, y)");
    }
    float x=lua_tonumber(L, -2);
    float y=lua_tonumber(L, -1);
    DTRACETIME("setSpawnPoint(%f,%f)",x,y);
    player_setSpawnPoint(x,y);
    return 0;
}

/// player.getPosition() -> {x,y}
int wrap_player_getPosition(lua_State *L) {
    b2Body *b=player_getPlayer()->getBody();
    lua_newtable(L);
    lua_pushnumber(L, b->GetPosition().x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, b->GetPosition().y);
    lua_setfield(L, -2, "y");
    return 1;
}

void wrap_wrapPlayer(lua_State *L) {

    lua_newtable(L);
    lua_pushstring(L, "setSpawnPoint");
    lua_pushcfunction(L, wrap_player_setSpawnPoint);
    lua_rawset(L, -3);
    lua_pushstring(L, "getPosition");
    lua_pushcfunction(L, wrap_player_getPosition);
    lua_rawset(L, -3);

    lua_setglobal(L, "player");

}

#endif /* WRAP_PLAYER_H_ */

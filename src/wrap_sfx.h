#ifndef WRAP_SFX_H_
#define WRAP_SFX_H_

#include <lua.hpp>
#include "sfx.h"
#include "scene.h"
#include "types.h"
#include "player.h"
#include "fs.h"
#include <cmath>

/// playedSound:setPosition(x, y)
int wrap_sfx_setPositionPlayedSound(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 3 || luaL_getmetafield(L, 1, "channel") == 0 || luaL_getmetafield(L, 1, "type") == 0 || lua_tointeger(L, -1) != WRAP_TYPE_PLAYEDSOUND
            || luaL_getmetafield(L, 1, "pid") == 0) {
        return 0; //no error handling here
    }
    int channel = lua_tointeger(L, -3);
    int playedMCId = lua_tointeger(L, -1);
    lua_pop(L,3);
    if (scene_getChannel2PlayedMCIDMap()[channel] != playedMCId) {
        return 0; //silently quit because channel is not owned by us
    }
    if (!lua_isnumber(L, 2))
        return luaL_argerror(L, 1, "number needed");
    if (!lua_isnumber(L, 3))
        return luaL_argerror(L, 2, "number needed");
    float x, y;
    x = lua_tonumber(L, 2);
    y = lua_tonumber(L, 3);
    b2Vec2 dir = player_getPlayer()->getBody()->GetPosition() - b2Vec2(x, y);
    float angle = (atan2(dir.y, dir.x) - M_PI / 2.0f) * 180.0f / M_PI;
    while (angle < 0.0f)
        angle += 360.0f;
    float dist = dir.Length()*10;
    if (dist < 0.0f)
        dist = 0.0f;
    if (dist > 255.0f)
        dist = 255.0f;
    Mix_SetPosition(channel, angle, dist);
    return 0;
}

/// playedSound:stop()
int wrap_sfx_stopPlayedSound(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 1 || luaL_getmetafield(L, 1, "channel") == 0 || luaL_getmetafield(L, 1, "type") == 0 || lua_tointeger(L, -1) != WRAP_TYPE_PLAYEDSOUND
            || luaL_getmetafield(L, 1, "pid") == 0) {
        return 0; //no error handling here because playedSound may be already inactive
    }
    int channel = lua_tointeger(L, -3);
    int playedMCId = lua_tointeger(L, -1);
    lua_pop(L,3);
    if (scene_getChannel2PlayedMCIDMap()[channel] != playedMCId) {
        return 0; //silently quit because channel is not owned by us
    }
    Mix_HaltChannel(channel);
    return 0;
}

/// sound:play(loops=0, volume=1.0) -> playedSound
int wrap_sfx_soundPlay(lua_State *L) {
    int n = lua_gettop(L);
    if (n < 1 || luaL_getmetafield(L, 1, "id") == 0 || luaL_getmetafield(L, 1, "type") == 0 || lua_tointeger(L, -1) != WRAP_TYPE_SOUND) {
        lua_pushstring(L, "sound:play(loops=0, volume=1.0)");
        return lua_error(L);
    }
    int mcid = lua_tointeger(L, -2);
    lua_pop(L, 2);
    int loops = luaL_optinteger(L, 2, 0);
    float volume = luaL_optnumber(L, 3, 1.0f);
    lua_pop(L, n);

    if (volume < 0.0f)
        volume = 0.0f;
    if (volume > 1.0f)
        volume = 1.0f;
    int channel = Mix_PlayChannel(-1, scene_getMCContainer()[mcid], loops);
    int playedMCId = scene_getUniquePlayedMCId();
    scene_getChannel2PlayedMCIDMap()[channel] = playedMCId;
    Mix_Volume(channel, MIX_MAX_VOLUME * volume);

    //bind
    lua_newtable(L);
    lua_pushstring(L, "stop");
    lua_pushcfunction(L,wrap_sfx_stopPlayedSound);
    lua_rawset(L, -3);
    lua_pushstring(L, "setPosition");
    lua_pushcfunction(L,wrap_sfx_setPositionPlayedSound);
    lua_rawset(L, -3);

    //create metatable
    lua_newtable(L);
    lua_pushstring(L, "pid");
    lua_pushinteger(L, playedMCId);
    lua_rawset(L, -3);
    lua_pushstring(L, "channel");
    lua_pushinteger(L, channel);
    lua_rawset(L, -3);
    lua_pushstring(L, "type");
    lua_pushinteger(L, WRAP_TYPE_PLAYEDSOUND);
    lua_rawset(L, -3);
    lua_setmetatable(L, -2);
    return 1;
}

/// sfx.createSound(filename)
int wrap_sfx_createSound(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 1 || !lua_isstring(L, -1)) {
        lua_pushstring(L, "createSound(filename)");
        return lua_error(L);
    }
    SDL_RWops *ops=fs_loadAsSDL_RWops(lua_tostring(L, -1));
    Mix_Chunk *mc = Mix_LoadWAV_RW(ops, 1);
    int mcid = scene_getUniqueMCId();
    scene_getMCContainer()[mcid] = mc;

    //bind
    lua_newtable(L);
    lua_pushstring(L, "play");
    lua_pushcfunction(L,wrap_sfx_soundPlay);
    lua_rawset(L, -3);

    //create metatable
    lua_newtable(L);
    lua_pushstring(L, "id");
    lua_pushinteger(L, mcid);
    lua_rawset(L, -3);
    lua_pushstring(L, "type");
    lua_pushinteger(L, WRAP_TYPE_SOUND);
    lua_rawset(L, -3);
    lua_setmetatable(L, -2);

    return 1;
}

void wrap_wrapSfx(lua_State *L) {

    lua_newtable(L);
    lua_pushstring(L, "createSound");
    lua_pushcfunction(L, wrap_sfx_createSound);
    lua_rawset(L, -3);

    lua_setglobal(L, "sfx");
}

#endif /* WRAP_SFX_H_ */

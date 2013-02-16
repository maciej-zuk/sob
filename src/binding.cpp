#include "binding.h"
#include "debug.h"
#include "physics.h"
#include "engine.h"
#include "player.h"
#include "scene.h"
#include "sfx.h"
#include <cstdlib>

static lua_State *L;
static bool binding_Active = false;

lua_State *binding_getLuaState() {
    return L;
}

static const luaL_Reg my_lualibs[] = { { "", luaopen_base },
// {LUA_LOADLIBNAME, luaopen_package},
        { LUA_TABLIBNAME, luaopen_table },
        // {LUA_IOLIBNAME, luaopen_io},
        // {LUA_OSLIBNAME, luaopen_os},
        { LUA_STRLIBNAME, luaopen_string }, { LUA_MATHLIBNAME, luaopen_math }, { LUA_DBLIBNAME, luaopen_debug }, { NULL, NULL } };

LUALIB_API void openCustomLuaLibs(lua_State *L) {
    const luaL_Reg *lib = my_lualibs;
    for (; lib->func; lib++) {
        lua_pushcfunction(L, lib->func);
        lua_pushstring(L, lib->name);
        lua_call(L, 1, 0);
    }
}

void binding_startUp() {
    if (binding_Active)
        return;
    binding_cleanEnvironment();
    binding_Active = true;
    DTRACETIME("clean");
}

void binding_cleanUp() {
    if (!binding_Active)
        return;
    binding_Active = false;
    if (L) {
        lua_close(L);
    }
    DTRACETIME("clean");
}

void binding_loadSceneFromFile(const char *name) {
    TRACETIME("loading %s",name);
    int ret;
    sfx_cleanEnvironment();
    binding_cleanEnvironment();
    scene_cleanUp();
    physics_cleanEnvironment();


    ret = luaL_loadfile(L, name);
    if (ret) {
        ETRACETIME("couldn't load file: %s", lua_tostring(L, -1));
        binding_Active = false;
        exit(1);
    }
    //lua_getglobal(L, "onWorldCreate");
    ret = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (ret) {
        ETRACETIME("failed to run script: %s", lua_tostring(L, -1));
        binding_Active = false;
        exit(1);
    }
    lua_getglobal(L, "onWorldCreate");
    ret = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (ret) {
        ETRACETIME("failed to run onWorldCreate: %s", lua_tostring(L, -1));
        binding_Active = false;
        exit(1);
    }

    player_getPlayer()->resetPhysics();
    engine_centerOnPlayer();

    lua_getglobal(L, "onWorldLoaded");
    ret = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (ret) {
        ETRACETIME("failed to run onWorldCreate: %s", lua_tostring(L, -1));
        binding_Active = false;
        exit(1);
    }
}

#include "wrap_physics.h"
#include "wrap_engine.h"
#include "wrap_player.h"
#include "wrap_entity.h"
#include "wrap_sfx.h"

void binding_cleanEnvironment() {
    if (L) {
        lua_close(L);
    }

    L = luaL_newstate();
    openCustomLuaLibs(L);
    luaL_loadstring(L, "dofile=nil;load=nil;loadfile=nil;loadstring=nil;nof=function()end;onWorldCreate=nof;onWorldLoaded=nof;");
    lua_pcall(L, 0, LUA_MULTRET, 0);

    lua_pushstring(L, "sob_deferredcalls");
    lua_newtable(L);
    lua_settable(L, LUA_REGISTRYINDEX);

    wrap_wrapPhysics(L);
    wrap_wrapEngine(L);
    wrap_wrapPlayer(L);
    wrap_wrapEntity(L);
    wrap_wrapSfx(L);

    DTRACETIME("env is clean");
}

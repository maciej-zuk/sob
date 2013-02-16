#include "callbacks.h"
#include "scene.h"
#include "binding.h"
#include "debug.h"

int deferredCall(lua_State*L) {
    int ix = 1;
    while (!lua_isnone(L, lua_upvalueindex(ix))) {
        lua_pushvalue(L, lua_upvalueindex(ix++));
    }
    ix--;
    if ((ix < 1) || (!lua_isfunction(L, (-1 * ix))))
        return luaL_error(L, "Bad Deferred Call");
    if (lua_pcall(L, ix - 1, 0, 0)) {
        return lua_error(L);
    }
    return 0;
}

void addDeferredCall(lua_State* L, int nargs) {
    luaL_checktype(L, -nargs, LUA_TFUNCTION);
    lua_pushcclosure(L, deferredCall, nargs);
    lua_getfield(L, LUA_REGISTRYINDEX, "sob_deferredcalls");
    int lastPos = lua_objlen(L, -1);
    lua_pushinteger(L, lastPos + 1);
    lua_pushvalue(L, -3);
    lua_rawset(L, -3);
    lua_pop(L, 2);
}

void callback_playedSound_finished(int channel){
    TRACETIME("callback playedSound finished %i",channel);
    lua_State *L = binding_getLuaState();
    lua_getfield(L, LUA_REGISTRYINDEX, "sob_playedSound");
    lua_pushinteger(L, channel);
    lua_gettable(L, -2);
    if(lua_istable(L,-1)){
        //clear metatable
        lua_pushnil(L);
        lua_setmetatable(L, -2);
        // we dont remove table here, because player may still want to use functions which now doesn't do anything
        // since mix_chunk from this channel is inactive
        //remove from registry
        lua_getfield(L, LUA_REGISTRYINDEX, "sob_playedSound");
        lua_pushinteger(L, channel);
        lua_pushnil(L);
        lua_rawset(L, -3);
        lua_pop(L, 1);
    }
    lua_pop(L, 2);
}

void callback_entity_onHit(Entity *entity1, Entity *entity2, float velocity) {
    DTRACETIME("callback entity onhit %s -> %s",entity1->getName().c_str(), entity2->getName().c_str());
    lua_State *L = binding_getLuaState();
    lua_getfield(L, LUA_REGISTRYINDEX, "sob_entity");
    lua_pushinteger(L, entity1->getId());
    lua_gettable(L, -2);
    if (lua_istable(L,-1)) {
        lua_getfield(L, -1, "onHit");
        // stack= onhit, e1table, sob_entity
        if (lua_isfunction(L, -1)) {
            lua_pushvalue(L, -2);
            lua_pushinteger(L, entity2->getId());
            // stack= e2id, e1table, onhit, e1table, sob_entity
            lua_gettable(L, -5);
            // stack= e2table, e1table, onhit, e1table, sob_entity
            lua_pushnumber(L, velocity);
            // stack= velocity, e2table, e1table, onhit, e1table, sob_entity
            addDeferredCall(L, 4);
        } else {
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 2);
}

void callback_entity_onTouch(Entity *entity) {
    DTRACETIME("callback entity ontouch %s", entity->getName().c_str());
    lua_State *L = binding_getLuaState();
    lua_getfield(L, LUA_REGISTRYINDEX, "sob_entity");
    lua_pushinteger(L, entity->getId());
    lua_gettable(L, -2);
    if (lua_istable(L,-1)) {
        lua_getfield(L, -1, "onTouch");
        // stack= ontouch, etable, sob_entity
        if (lua_isfunction(L, -1)) {
            lua_pushvalue(L, -2);
            // stack= etable, ontouch, etable, sob_entity
            addDeferredCall(L, 2);
        } else {
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 2);
}

void callback_entity_onLeave(Entity *entity) {
    DTRACETIME("callback entity onleave %s", entity->getName().c_str());
    lua_State *L = binding_getLuaState();
    lua_getfield(L, LUA_REGISTRYINDEX, "sob_entity");
    lua_pushinteger(L, entity->getId());
    lua_gettable(L, -2);
    if (lua_istable(L,-1)) {
        lua_getfield(L, -1, "onLeave");
        // stack= onleave, etable, sob_entity
        if (lua_isfunction(L, -1)) {
            lua_pushvalue(L, -2);
            // stack= etable, onleave, etable, sob_entity
            addDeferredCall(L, 2);
        } else {
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 2);
}

void callback_player_onHit(Entity *entity, float velocity) {
    DTRACETIME("callback player onhit %s",entity->getName().c_str());
    lua_State *L = binding_getLuaState();
    lua_getfield(L, LUA_REGISTRYINDEX, "sob_entity");
    lua_getfield(L, LUA_GLOBALSINDEX, "player");
    lua_getfield(L, -1, "onHit");
    // stack= onhit, player, sob_entity
    if (lua_isfunction(L, -1)) {
        lua_pushinteger(L, entity->getId());
        // stack= eid, onhit, player, sob_entity
        lua_gettable(L, -4);
        // stack= e2table, onhit, player, sob_entity
        lua_pushnumber(L, velocity);
        // stack= velocity, e2table, onhit, player, sob_entity
        addDeferredCall(L, 3);
    } else {
        lua_pop(L, 1);
    }
    lua_pop(L, 2);
}

void callback_player_onRopeHit(Entity *entity) {
    DTRACETIME("callback player onropehit %s",entity->getName().c_str());
    lua_State *L = binding_getLuaState();
    lua_getfield(L, LUA_REGISTRYINDEX, "sob_entity");
    lua_getfield(L, LUA_GLOBALSINDEX, "player");
    lua_getfield(L, -1, "onRopeHit");
    // stack= onhit, player, sob_entity
    if (lua_isfunction(L, -1)) {
        lua_pushinteger(L, entity->getId());
        // stack= eid, onhit, player, sob_entity
        lua_gettable(L, -4);
        // stack= e2table, onhit, player, sob_entity
        addDeferredCall(L, 2);
    } else {
        lua_pop(L, 1);
    }
    lua_pop(L, 2);
}

void callback_player_onJump() {
    DTRACETIME("callback player onjump");
    lua_State *L = binding_getLuaState();
    lua_getfield(L, LUA_GLOBALSINDEX, "player");
    lua_getfield(L, -1, "onJump");
    // stack= onhit, player
    if (lua_isfunction(L, -1)) {
        // stack= onshoot, player
        addDeferredCall(L, 1);
    } else {
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
}

void callback_entity_onTick(Entity *entity) {
    lua_State *L = binding_getLuaState();
    lua_getfield(L, LUA_REGISTRYINDEX, "sob_entity");
    lua_pushinteger(L, entity->getId());
    lua_gettable(L, -2);
    if (lua_istable(L,-1)) {
        lua_getfield(L, -1, "onTick");
        // stack= ontick, e1table, sob_entity
        if (lua_isfunction(L, -1)) {
            lua_pushvalue(L, -2);
            // stack= e1table, onhit, e1table, sob_entity
            addDeferredCall(L, 2);
        } else {
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 2);
}

void callback_processDeferredCalls() {
    lua_State *L = binding_getLuaState();
    lua_getfield(L, LUA_REGISTRYINDEX, "sob_deferredcalls");
    lua_pushnil(L);
#ifdef DEBUG
    bool printed=false;
#endif
    while (lua_next(L, -2) != 0) {
#ifdef DEBUG
        if(!printed)     DTRACETIME("calling deferred function");
       printed=true;
#endif

        if (lua_pcall(L, 0, 0, 0)) {
            ETRACETIME("error in callback %s", lua_tostring(L, -1));
            lua_pop(L,1);
        }
        //k, table
        lua_pushvalue(L, -1);
        lua_pushnil(L);
        // nil, k, k, table
        lua_rawset(L, -4);
        // k, table
    }
    lua_pop(L, 1);
    // TRACETIME("top %i",lua_gettop(L));
    // stack empty
}

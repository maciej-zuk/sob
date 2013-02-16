#ifndef WRAP_ENTITY_H_
#define WRAP_ENTITY_H_

#include <lua.hpp>
#include "entity.h"
#include "types.h"
#include "scene.h"

using std::list;

/**entity:destroy()
 */
int wrap_entity_destroy(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 1) {
        lua_pushstring(L, "entity:destroy()");
        return lua_error(L);
    }

    int entityid;

    if (luaL_getmetafield(L, -1, "id") == 0 || luaL_getmetafield(L, -2, "type") == 0 || lua_tointeger(L, -1) != WRAP_TYPE_ENTITY) {
        return luaL_argerror(L, 0, "valid entity needed");
    };
    entityid = lua_tointeger(L, -2);
    lua_pop(L, 2);

    Entity *e=scene_getEntityContainer()[entityid];
    scene_getEntityContainer().erase(entityid);
    delete e;

    //clear metatable
    lua_pushnil(L);
    lua_setmetatable(L, -2);

    //clear table
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
      lua_pushvalue(L, -2);
      lua_pushnil(L);
      lua_rawset(L,-5);
      lua_pop(L, 1);
    }

    //remove from registry
    lua_getfield(L,LUA_REGISTRYINDEX, "sob_entity");
    lua_pushinteger(L, entityid);
    lua_pushnil(L);
    lua_rawset(L,-3);

    return 0;
}



/// operator ==
int wrap_entity_eq(lua_State *L) {
    int n = lua_gettop(L);
    int id1, id2;
    if (n != 1 || luaL_getmetafield(L, -1, "id") == 0 || luaL_getmetafield(L, -2, "type") == WRAP_TYPE_ENTITY) {
        return luaL_argerror(L, 0, "valid entity needed");
    } else {
        id1 = lua_tointeger(L, -2);
        lua_pop(L, 3);
    }
    if (n != 1 || luaL_getmetafield(L, -1, "id") == 0 || luaL_getmetafield(L, -2, "type") == WRAP_TYPE_ENTITY) {
        return luaL_argerror(L, 1, "valid entity needed");
    } else {
        id2 = lua_tointeger(L, -2);
        lua_pop(L, 3);
    }
    lua_pushboolean(L, id1 == id2);
    return 1;
}

/// entity:setLinearVelocity(vx, vy)
int wrap_entity_setLinearVelocity(lua_State *L) {
    int n = lua_gettop(L);
    int id;
    float vx, vy;
    if (n != 3 || luaL_getmetafield(L, 1, "id") == 0 || luaL_getmetafield(L, 1, "type") == WRAP_TYPE_ENTITY) {
        return luaL_argerror(L, 0, "valid entity needed");
    }
    else{
        id = lua_tointeger(L, -2);
        lua_pop(L, 2);
    }
    if(!lua_isnumber(L, 2)){
        return luaL_argerror(L, 1, "number needed");
    }
    if(!lua_isnumber(L, 3)){
        return luaL_argerror(L, 2, "number needed");
    }
    vx=lua_tonumber(L, 2);
    vy=lua_tonumber(L, 3);
    b2Vec2 vel(vx, vy);
    Entity *e = scene_getEntityContainer()[id];
    b2Body *b = e->getBody();
    if(b){
        b->SetLinearVelocity(vel);
    }
    return 0;
}

/// entity:setAngularVelocity(omega)
int wrap_entity_setAngularVelocity(lua_State *L) {
    int n = lua_gettop(L);
    int id;
    float omega;
    if (n != 2 || luaL_getmetafield(L, 1, "id") == 0 || luaL_getmetafield(L, 1, "type") == WRAP_TYPE_ENTITY) {
        return luaL_argerror(L, 0, "valid entity needed");
    }
    else{
        id = lua_tointeger(L, -2);
        lua_pop(L, 2);
    }
    if(!lua_isnumber(L, 2)){
        return luaL_argerror(L, 1, "number needed");
    }
    omega=lua_tonumber(L, 2);
    Entity *e = scene_getEntityContainer()[id];
    b2Body *b = e->getBody();
    if(b){
        b->SetAngularVelocity(omega);
    }
    return 0;
}

/// entity:getPosition() -> {x,y}
int wrap_entity_getPosition(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 1 || luaL_getmetafield(L, -1, "id") == 0 || luaL_getmetafield(L, -2, "type") == WRAP_TYPE_ENTITY) {
        return luaL_argerror(L, 2, "valid entity needed");
    }
    int id = lua_tointeger(L, -2);
    Entity *e = scene_getEntityContainer()[id];
    lua_newtable(L);
    lua_pushnumber(L, e->getBody()->GetPosition().x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, e->getBody()->GetPosition().y);
    lua_setfield(L, -2, "y");
    return 1;
}

/// entity:getName()
int wrap_entity_getName(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 1 || luaL_getmetafield(L, -1, "id") == 0 || luaL_getmetafield(L, -2, "type") == WRAP_TYPE_ENTITY) {
        return luaL_argerror(L, 2, "valid entity needed");
    }
    int id = lua_tointeger(L, -2);
    lua_pushstring(L, scene_getEntityContainer()[id]->getName().c_str());
    return 1;
}

/**entity.createEntity(physics.body, name)
 *
 * this function may be deferred depending on physics solver
 */
int wrap_entity_createEntity(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 2) {
        lua_pushstring(L, "entity.createEntity(physics.body, name)");
        return lua_error(L);
    }
    if (!lua_isstring(L, -1)) {
        return luaL_argerror(L, 2, "string needed");
    }
    string name(lua_tostring(L, -1));
    lua_pop(L, 1);
    if (luaL_getmetafield(L, -1, "id") == 0 || luaL_getmetafield(L, -2, "type") == 0 || lua_tointeger(L, -1) != WRAP_TYPE_BODY) {
        return luaL_argerror(L, 2, "valid body needed");
    }
    int bodyid = lua_tointeger(L, -2);
    int id = scene_getUniqueEntityId();

    DTRACETIME("create entity");
    b2Body *body = scene_getBodyContainer()[bodyid];
    body->SetActive(true);
    Entity *entity = new Entity(body, name, id, bodyid);
    body->SetUserData(entity);
    scene_getEntityContainer()[id] = entity;

    /* destroying body */
    lua_getfield(L,LUA_REGISTRYINDEX, "sob_body");
    lua_pushinteger(L, bodyid);
    lua_gettable(L, -2);
    lua_pushinteger(L, bodyid);
    lua_pushnil(L);
    lua_rawset(L,-4);

    //clear metatable
    lua_pushnil(L);
    lua_setmetatable(L, -2);

    //clear table
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
      lua_pushvalue(L, -2);
      lua_pushnil(L);
      lua_rawset(L,-5);
      lua_pop(L, 1);
    }

    lua_pop(L, 1);

    //bind
    lua_newtable(L);
    lua_pushstring(L, "destroy");
    lua_pushcfunction(L,wrap_entity_destroy);
    lua_rawset(L, -3);
    lua_pushstring(L, "getPosition");
    lua_pushcfunction(L,wrap_entity_getPosition);
    lua_rawset(L, -3);
    lua_pushstring(L, "getName");
    lua_pushcfunction(L,wrap_entity_getName);
    lua_rawset(L, -3);
    lua_pushstring(L, "setLinearVelocity");
    lua_pushcfunction(L,wrap_entity_setLinearVelocity);
    lua_rawset(L, -3);
    lua_pushstring(L, "getAngularVelocity");
    lua_pushcfunction(L,wrap_entity_setAngularVelocity);
    lua_rawset(L, -3);


    //add to registry
    lua_getfield(L, LUA_REGISTRYINDEX, "sob_entity");
    lua_pushinteger(L, id);
    lua_pushvalue(L, -3);
    lua_rawset(L, -3);
    lua_pop(L,1);

    //create metatable
    lua_newtable(L);
    lua_pushstring(L, "id");
    lua_pushinteger(L, id);
    lua_rawset(L, -3);
    lua_pushstring(L, "type");
    lua_pushinteger(L, WRAP_TYPE_ENTITY);
    lua_rawset(L, -3);
    lua_pushstring(L, "__eq");
    lua_pushcfunction(L, wrap_entity_eq);
    lua_rawset(L, -3);
    lua_setmetatable(L, -2);

    return 1;
}

void wrap_wrapEntity(lua_State *L) {
    lua_pushstring(L, "sob_entity");
    lua_newtable(L);
    lua_settable(L, LUA_REGISTRYINDEX);

    lua_newtable(L);
    lua_pushstring(L, "createEntity");
    lua_pushcfunction(L, wrap_entity_createEntity);
    lua_rawset(L, -3);

    lua_setglobal(L, "entity");

}

#endif /* WRAP_PLAYER_H_ */

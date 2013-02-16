#include "entity.h"
#include "physics.h"
#include "debug.h"
#include "binding.h"
#include "scene.h"

Entity::Entity(b2Body *body, const string& name, int id, int bodyId) {
    DTRACETIME("create entity named %s, id=%i",name.c_str(),id);
    this->body = body;
    this->name = name;
    this->id = id;
    this->bodyId = bodyId;
}
Entity::~Entity() {
    DTRACETIME("delete entity named %s, id=%i",name.c_str(), id);
    b2World *world = physics_getWorld();
    if (body && world) {
        lua_State *L = binding_getLuaState();
        b2JointEdge *joint = body->GetJointList();
        while (joint) {
            b2Joint *tj = joint->joint;
            joint = joint->next;

            int *udata=(int*)tj->GetUserData();
            if(!udata)
                continue;
            int jointid = *udata;
            delete udata;
            world->DestroyJoint(tj);
            scene_getJointContainer().erase(jointid);

            lua_getfield(L, LUA_REGISTRYINDEX, "sob_joint");
            lua_pushinteger(L, jointid);
            lua_gettable(L, -2);
            if (lua_isnil(L, -1)){
                lua_pop(L, 2);
                continue;
            }
            //clear metatable
            lua_pushnil(L);
            lua_setmetatable(L, -2);

            //clear table
            lua_pushnil(L);
            while (lua_next(L, -2) != 0) {
                lua_pushvalue(L, -2);
                lua_pushnil(L);
                lua_rawset(L, -5);
                lua_pop(L, 1);
            }
            lua_pop(L, 1);

            //remove from registry
            lua_pushinteger(L, jointid);
            lua_pushnil(L);
            lua_rawset(L, -3);
            lua_pop(L, 1);
        }

        lua_getfield(L, LUA_REGISTRYINDEX, "sob_body");
        lua_pushinteger(L, bodyId);
        lua_gettable(L, -2);
        if (!lua_isnil(L, -1)) {
            //clear metatable
            lua_pushnil(L);
            lua_setmetatable(L, -2);

            //clear table
            lua_pushnil(L);
            while (lua_next(L, -2) != 0) {
                lua_pushvalue(L, -2);
                lua_pushnil(L);
                lua_rawset(L, -5);
                lua_pop(L, 1);
            }
            lua_pop(L, 1);

            //remove from registry
            lua_pushinteger(L, bodyId);
            lua_pushnil(L);
            lua_rawset(L, -3);
            lua_pop(L, 1);
        }
        else{
            lua_pop(L, 2);
        }
        scene_getBodyContainer().erase(bodyId);
        world->DestroyBody(body);
    }
}

int Entity::getId() {
    return id;
}

string &Entity::getName() {
    return name;
}

b2Body *Entity::getBody() {
    return body;
}

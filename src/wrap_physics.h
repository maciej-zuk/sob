#ifndef WRAP_PHYSICS_H_
#define WRAP_PHYSICS_H_

#include <lua.hpp>
#include <Box2D/Box2D.h>
#include "scene.h"
#include "types.h"
#include <list>

using std::list;

///internal
int _wrap_physics_createShape(lua_State *L, b2Shape *shape) {
    //bind
    lua_newtable(L);

    //add to registry
    int id = scene_getUniqueShapeId();
    lua_getfield(L, LUA_REGISTRYINDEX, "sob_shape");
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
    lua_pushinteger(L, WRAP_TYPE_SHAPE);
    lua_rawset(L, -3);
    lua_setmetatable(L, -2);

    scene_getShapeContainer()[id] = shape;
    return 1;
}

///createChainShape({x1, y1, x2, y2, ..., xn, yn})
int wrap_physics_createChainShape(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 1 || !lua_istable(L, -1)) {
        lua_pushstring(L, "createChainShape({x1, y1, x2, y2, ..., xn, yn})");
        return lua_error(L);
    }

    b2ChainShape *shape = new b2ChainShape;

    int len = lua_objlen(L, -1) / 2;
    b2Vec2 *verts = new b2Vec2[len];

    int i = 0;
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        float x = lua_tonumber(L, -1);
        float y = 0;
        lua_pop(L, 1);
        if (lua_next(L, -2) != 0) {
            y = lua_tonumber(L, -1);
            lua_pop(L,1);
            verts[i].x = x;
            verts[i].y = y;
            i++;
        } else {
            break;
        }
    }

    shape->CreateChain(verts, i);
    delete[] verts;

    return _wrap_physics_createShape(L, shape);
}

///createPolygonShape({x1, y1, x2, y2, ..., xn, yn})
int wrap_physics_createPolygonShape(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 1 || !lua_istable(L, -1)) {
        lua_pushstring(L, "createPolygonShape({x1, y1, x2, y2, ..., xn, yn})");
        return lua_error(L);
    }

    b2PolygonShape *shape = new b2PolygonShape;

    int len = lua_objlen(L, -1) / 2;
    b2Vec2 *verts = new b2Vec2[len];

    int i = 0;
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        float x = lua_tonumber(L, -1);
        float y = 0;
        lua_pop(L, 1);
        if (lua_next(L, -2) != 0) {
            y = lua_tonumber(L, -1);
            lua_pop(L,1);
            verts[i].x = x;
            verts[i].y = y;
            i++;
        } else {
            break;
        }
    }
    if (i < 3 || i > b2_maxPolygonVertices) {
        char error[255];
        sprintf(error, "invalid polygon vertices, there should be more than 2 and less than %i vertices", b2_maxPolygonVertices + 1);
        lua_pushstring(L, error);
        return lua_error(L);
    }
    shape->Set(verts, i);
    delete[] verts;

    return _wrap_physics_createShape(L, shape);
}

/// physics.createCircleShape(radius)
int wrap_physics_createCircleShape(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 1 || !lua_isnumber(L, -1)) {
        lua_pushstring(L, "physics.createCircleShape(radius)");
        return lua_error(L);
    }
    float radius = lua_tonumber(L, -1);
    b2CircleShape *shape = new b2CircleShape;

    shape->m_radius = radius;

    return _wrap_physics_createShape(L, shape);
}

/// physics.createFixtureDef(shape, physics.layer, density=1, friction=1, restitution=0)
int wrap_physics_createFixtureDef(lua_State *L) {
    int n = lua_gettop(L);
    if (n < 2 || n > 5) {
        lua_pushstring(L, "createFixtureDef(shape, physics.layer, density=1, friction=1, restitution=0)");
        return lua_error(L);
    }
    int shapeid;
    int layer;
    float density;
    float friction;
    float restitution;

    if (luaL_getmetafield(L, 1, "id") == 0 || luaL_getmetafield(L, 1, "type") == 0) {
        return luaL_argerror(L, 1, "valid shape needed");
    } else {
        shapeid = lua_tointeger(L, -2);
        lua_pop(L, 2);
    }

    if (!lua_isnumber(L, 2)) {
        return luaL_argerror(L, 2, "not a number");
    } else {
        layer = lua_tointeger(L, 2);
    }

    density = luaL_optnumber(L, 3, 1.0);
    friction = luaL_optnumber(L, 4, 1.0);
    restitution = luaL_optnumber(L, 5, 0.0);

    b2FixtureDef fixtureDef;
    fixtureDef.density = density;
    fixtureDef.friction = friction;
    fixtureDef.restitution = restitution;
    fixtureDef.filter.categoryBits = layer;
    fixtureDef.filter.maskBits = layer;
    fixtureDef.shape = scene_getShapeContainer()[shapeid];

    //bind
    lua_newtable(L);

    //add to registry
    int id = scene_getUniqueFixtureDefId();
    lua_getfield(L, LUA_REGISTRYINDEX, "sob_fixturedef");
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
    lua_pushinteger(L, WRAP_TYPE_FIXTUREDEF);
    lua_rawset(L, -3);
    lua_setmetatable(L, -2);

    scene_getFixtureDefContainer()[id] = fixtureDef;

    return 1;
}

/**body:addFixture(fixturedef)
 */
int wrap_physics_body_addFixture(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 2) {
        lua_pushstring(L, "body:addFixture(fixturedef)");
        return lua_error(L);
    }

    int bodyid;
    int fixdefid;

    if (luaL_getmetafield(L, -1, "id") == 0 || luaL_getmetafield(L, -2, "type") == 0 || lua_tointeger(L, -1) != WRAP_TYPE_FIXTUREDEF) {
        return luaL_argerror(L, 1, "valid fixturedef needed");
    } else {
        fixdefid = lua_tointeger(L, -2);
        lua_pop(L, 3);
    }

    if (luaL_getmetafield(L, -1, "id") == 0 || luaL_getmetafield(L, -2, "type") == 0 || lua_tointeger(L, -1) != WRAP_TYPE_BODY) {
        return luaL_argerror(L, 0, "valid body needed");
    } else {
        bodyid = lua_tointeger(L, -2);
        lua_pop(L, 3);
    }
    scene_getBodyContainer()[bodyid]->CreateFixture(&scene_getFixtureDefContainer()[fixdefid]);
    DTRACETIME("addFixture");
    return 0;
}

/**physics.createBody(physics.body_type, posx, posy, linearDumping=0.1, angularDumping=0.1)
 */

int wrap_physics_createBody(lua_State *L) {
    int n = lua_gettop(L);
    if (n < 3) {
        lua_pushstring(L, "createBody(physics.body_type, posx, posy)");
        return lua_error(L);
    }
    if (!lua_isnumber(L, 1)) {
        luaL_argerror(L, 1, "number needed");
    }
    if (!lua_isnumber(L, 2)) {
        luaL_argerror(L, 2, "number needed");
    }
    if (!lua_isnumber(L, 3)) {
        luaL_argerror(L, 3, "number needed");
    }
    float posy = lua_tonumber(L, 3);
    float posx = lua_tonumber(L, 2);
    int type = lua_tointeger(L, 1);
    float ldump = luaL_optnumber(L, 4, 0.1);
    float adump = luaL_optnumber(L, 5, 0.1);

    int id = scene_getUniqueBodyId();

    DTRACETIME("create body");
    b2World *world = physics_getWorld();

    b2BodyDef bodydef;
    switch (type) {
    case 1:
        bodydef.type = b2_kinematicBody;
        break;
    case 2:
        bodydef.type = b2_dynamicBody;
        break;
    default:
        bodydef.type = b2_staticBody;
        break;
    }
    bodydef.angularDamping = adump;
    bodydef.linearDamping = ldump;
    bodydef.position.Set(posx, posy);
    b2Body *body = world->CreateBody(&bodydef);
    body->SetActive(false);
    scene_getBodyContainer()[id] = body;

    //bind
    lua_newtable(L);
    lua_pushstring(L, "addFixture");
    lua_pushcfunction(L,wrap_physics_body_addFixture);
    lua_rawset(L, -3);

    //add to registry
    lua_getfield(L, LUA_REGISTRYINDEX, "sob_body");
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
    lua_pushinteger(L, WRAP_TYPE_BODY);
    lua_rawset(L, -3);
    lua_setmetatable(L, -2);

    return 1;
}

/**joint:destroy()
 */
int wrap_physics_joint_destroy(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 1) {
        lua_pushstring(L, "joint:destroy()");
        return lua_error(L);
    }

    int jointid;

    if (luaL_getmetafield(L, -1, "id") == 0 || luaL_getmetafield(L, -2, "type") == 0 || lua_tointeger(L, -1) != WRAP_TYPE_JOINT) {
        return luaL_argerror(L, 1, "valid joint needed");
    };
    jointid = lua_tointeger(L, -2);
    lua_pop(L, 2);

    b2Joint *joint = scene_getJointContainer()[jointid];
    scene_getJointContainer().erase(jointid);
    b2World *world = physics_getWorld();
    delete (int*) joint->GetUserData();
    world->DestroyJoint(joint);

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

    //remove from registry
    lua_getfield(L, LUA_REGISTRYINDEX, "sob_joint");
    lua_pushinteger(L, jointid);
    lua_pushnil(L);
    lua_rawset(L, -3);

    return 0;
}

///internal
int _wrap_physics_createJoint(lua_State *L, b2Joint *joint) {
    int id = scene_getUniqueJointId();
    scene_getJointContainer()[id] = joint;
    int *data = new int;
    *data = id;
    joint->SetUserData(data);

    //bind
    lua_newtable(L);
    lua_pushstring(L, "destroy");
    lua_pushcfunction(L,wrap_physics_joint_destroy);
    lua_rawset(L, -3);

    //add to registry
    lua_getfield(L, LUA_REGISTRYINDEX, "sob_joint");
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
    lua_pushinteger(L, WRAP_TYPE_JOINT);
    lua_rawset(L, -3);
    lua_setmetatable(L, -2);

    return 1;
}

/// createRevoluteJoint(entityA, entityB, {anchorX, anchorY}, motor=0, torque=1000, collideConnected=0)
int wrap_physics_createRevoluteJoint(lua_State *L) {
    int n = lua_gettop(L);
    if (n < 3) {
        lua_pushstring(L, "createRevoluteJoint(entityA, entityB, {anchorX, anchorY}, motor=0, torque=1000, collideConnected=0)");
        return lua_error(L);
    }

    int body1id;
    int body2id;
    float px, py;
    float motor;
    float torque;

    if (luaL_getmetafield(L, 1, "id") == 0 || luaL_getmetafield(L, 1, "type") == 0 || lua_tointeger(L, -1) != WRAP_TYPE_ENTITY) {
        return luaL_argerror(L, 1, "valid entity needed");
    } else {
        body1id = lua_tointeger(L, -2);
        lua_pop(L, 2);
    }

    if (luaL_getmetafield(L, 2, "id") == 0 || luaL_getmetafield(L, 2, "type") == 0 || lua_tointeger(L, -1) != WRAP_TYPE_ENTITY) {
        return luaL_argerror(L, 2, "valid entity needed");
    } else {
        body2id = lua_tointeger(L, -2);
        lua_pop(L, 2);
    }

    if (!lua_istable(L, 3)) {
        return luaL_argerror(L, 3, "valid table needed");
    } else {
        lua_getfield(L, 3, "x");
        lua_getfield(L, 3, "y");
        if (!lua_isnumber(L, -1) || !lua_isnumber(L, -2)) {
            return luaL_argerror(L, 3, "position should be definied as {['x']=x,['y']=y}");
        }
        px = lua_tonumber(L, -2);
        py = lua_tonumber(L, -1);
        lua_pop(L, 2);
    }
    bool collideConnected;
    bool motorEnabled = lua_isnumber(L, 4);
    motor = luaL_optnumber(L, 4, 0);
    torque = luaL_optnumber(L, 5, 1000);
    collideConnected = luaL_optinteger(L, 6, 0);

    b2Body *bA = scene_getBodyContainer()[body1id];
    b2Body *bB = scene_getBodyContainer()[body2id];
    b2RevoluteJointDef jd;
    b2Vec2 anchor(px, py);
    jd.Initialize(bA, bB, anchor);
    jd.collideConnected = collideConnected;
    jd.motorSpeed = motor;
    jd.maxMotorTorque = torque;
    jd.enableMotor = motorEnabled;
    b2World *world = physics_getWorld();
    b2Joint *joint = world->CreateJoint(&jd);
    return _wrap_physics_createJoint(L, joint);
}


/// createDistanceJoint(entityA, entityB, {anchor1X, anchor1Y}, {anchor2X, anchor2Y}, frq=10, dumping=0.5, collideConnected=0)
int wrap_physics_createDistanceJoint(lua_State *L) {
    int n = lua_gettop(L);
    if (n < 4) {
        lua_pushstring(L, "createDistanceJoint(entityA, entityB, {anchor1X, anchor1Y}, {anchor2X, anchor2Y}, frq=10, dumping=0.5, collideConnected=0)");
        return lua_error(L);
    }

    int body1id;
    int body2id;
    float p1x, p1y;
    float p2x, p2y;
    float frq;
    float dump;

    if (luaL_getmetafield(L, 1, "id") == 0 || luaL_getmetafield(L, 1, "type") == 0 || lua_tointeger(L, -1) != WRAP_TYPE_ENTITY) {
        return luaL_argerror(L, 1, "valid entity needed");
    } else {
        body1id = lua_tointeger(L, -2);
        lua_pop(L, 2);
    }

    if (luaL_getmetafield(L, 2, "id") == 0 || luaL_getmetafield(L, 2, "type") == 0 || lua_tointeger(L, -1) != WRAP_TYPE_ENTITY) {
        return luaL_argerror(L, 2, "valid entity needed");
    } else {
        body2id = lua_tointeger(L, -2);
        lua_pop(L, 2);
    }

    if (!lua_istable(L, 3)) {
        return luaL_argerror(L, 3, "valid table needed");
    } else {
        lua_getfield(L, 3, "x");
        lua_getfield(L, 3, "y");
        if (!lua_isnumber(L, -1) || !lua_isnumber(L, -2)) {
            return luaL_argerror(L, 3, "position should be definied as {['x']=x,['y']=y}");
        }
        p1x = lua_tonumber(L, -2);
        p1y = lua_tonumber(L, -1);
        lua_pop(L, 2);
    }
    if (!lua_istable(L, 4)) {
        return luaL_argerror(L, 4, "valid table needed");
    } else {
        lua_getfield(L, 4, "x");
        lua_getfield(L, 4, "y");
        if (!lua_isnumber(L, -1) || !lua_isnumber(L, -2)) {
            return luaL_argerror(L, 4, "position should be definied as {['x']=x,['y']=y}");
        }
        p2x = lua_tonumber(L, -2);
        p2y = lua_tonumber(L, -1);
        lua_pop(L, 2);
    }
    frq = luaL_optnumber(L, 5, 10.0f);
    dump = luaL_optnumber(L, 6, 0.5f);
    bool collideConnected = luaL_optinteger(L, 7, 0);

    b2Body *bA = scene_getBodyContainer()[body1id];
    b2Body *bB = scene_getBodyContainer()[body2id];
    b2DistanceJointDef jd;
    b2Vec2 anchor1(p1x, p1y);
    b2Vec2 anchor2(p2x, p2y);
    jd.Initialize(bA, bB, anchor1, anchor2);
    jd.collideConnected = collideConnected;
    jd.dampingRatio=dump;
    jd.frequencyHz=frq;
    b2World *world = physics_getWorld();
    b2Joint *joint = world->CreateJoint(&jd);
    return _wrap_physics_createJoint(L, joint);
}

void wrap_wrapPhysics(lua_State *L) {
    lua_pushstring(L, "sob_joint");
    lua_newtable(L);
    lua_settable(L, LUA_REGISTRYINDEX);
    lua_pushstring(L, "sob_shape");
    lua_newtable(L);
    lua_settable(L, LUA_REGISTRYINDEX);
    lua_pushstring(L, "sob_body");
    lua_newtable(L);
    lua_settable(L, LUA_REGISTRYINDEX);
    lua_pushstring(L, "sob_fixturedef");
    lua_newtable(L);
    lua_settable(L, LUA_REGISTRYINDEX);

    lua_newtable(L);
    lua_pushstring(L, "createBody");
    lua_pushcfunction(L, wrap_physics_createBody);
    lua_rawset(L, -3);
    lua_pushstring(L, "createChainShape");
    lua_pushcfunction(L, wrap_physics_createChainShape);
    lua_rawset(L, -3);
    lua_pushstring(L, "createPolygonShape");
    lua_pushcfunction(L, wrap_physics_createPolygonShape);
    lua_rawset(L, -3);
    lua_pushstring(L, "createCircleShape");
    lua_pushcfunction(L, wrap_physics_createCircleShape);
    lua_rawset(L, -3);

    lua_pushstring(L, "createFixtureDef");
    lua_pushcfunction(L, wrap_physics_createFixtureDef);
    lua_rawset(L, -3);
    lua_pushstring(L, "createRevoluteJoint");
    lua_pushcfunction(L, wrap_physics_createRevoluteJoint);
    lua_rawset(L, -3);
    lua_pushstring(L, "createDistanceJoint");
    lua_pushcfunction(L, wrap_physics_createDistanceJoint);
    lua_rawset(L, -3);

    lua_pushstring(L, "body_type_static");
    lua_pushinteger(L, 0);
    lua_rawset(L, -3);
    lua_pushstring(L, "body_type_kinetic");
    lua_pushinteger(L, 1);
    lua_rawset(L, -3);
    lua_pushstring(L, "body_type_dynamic");
    lua_pushinteger(L, 2);
    lua_rawset(L, -3);
    lua_pushstring(L, "layer_bg");
    lua_pushinteger(L, physics_collideBgCategory);
    lua_rawset(L, -3);
    lua_pushstring(L, "layer_fg");
    lua_pushinteger(L, physics_collideFgCategory);
    lua_rawset(L, -3);

    lua_setglobal(L, "physics");
}

#endif /* WRAP_PHYSICS_H_ */

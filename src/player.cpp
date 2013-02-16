#include "player.h"
#include "debug.h"
#include "physics.h"
#include "callbacks.h"
#include <SDL2/SDL.h>
#include "engine.h"

static bool player_Active = false;
static b2Vec2 spawnPoint(0, 0);

/*
 *  player
 */

Player::Player() {
    chainEntity = NULL;
    body = NULL;
    resetPhysics();
}

Player::~Player() {
    destroyPhysics();
}

void Player::resetPhysics() {
    b2World *world = physics_getWorld();
    destroyPhysics();
    b2BodyDef playerBodyDef;
    playerBodyDef.type = b2_dynamicBody;
    playerBodyDef.position = spawnPoint;
    playerBodyDef.angularDamping = 0.25f;
    playerBodyDef.linearDamping = 0.25f;
    body = world->CreateBody(&playerBodyDef);
    b2PolygonShape playerShape;
    playerShape.SetAsBox(1.0f, 1.0f);
    b2FixtureDef playerFixtureDef;
    playerFixtureDef.shape = &playerShape;
    playerFixtureDef.density = 1.0f;
    playerFixtureDef.friction = 0.5f;
    playerFixtureDef.restitution = 0.4f;
    playerFixtureDef.filter.categoryBits = physics_collideFgCategory;
    playerFixtureDef.filter.maskBits = physics_collideFgCategory;
    body->CreateFixture(&playerFixtureDef);
    body->SetUserData(this);
    plankShape.SetAsBox(0.15f, 1.0f);
    plankFixture.shape = &plankShape;
    plankFixture.density = 3.0f;
    plankFixture.friction = 0.5f;
    plankFixture.filter.categoryBits = physics_collideFgCategory;
    plankFixture.filter.maskBits = physics_collideFgCategory;

    chainEntity = new Entity(NULL, "_chain", -1, -1);
    ifCanJump = false;
    ifChainShoot = false;
    jumpCooldown=0;
    DTRACETIME("player reset");
}

void Player::destroyPhysics() {
    DTRACETIME("player physics destroyed");
    b2World *world = physics_getWorld();
    if (chainEntity) {
        delete chainEntity;
    }
    if (body && world) {
        world->DestroyBody(body);
    }
    body = NULL;
    chainEntity = NULL;
}

void Player::redrawOutput() {
    //todo
}

b2Body *Player::getBody() {
    return body;
}

Entity * Player::getChainEntity() {
    return chainEntity;
}

void Player::moveLeft(float speed) {
    if (body->GetLinearVelocity().x > -speed*speed) {
        body->ApplyLinearImpulse(b2Vec2(-speed, 0), body->GetPosition() + body->GetLocalCenter());
    }
}

void Player::moveRight(float speed) {
    if (body->GetLinearVelocity().x < speed*speed) {
        body->ApplyLinearImpulse(b2Vec2(speed, 0), body->GetPosition() + body->GetLocalCenter());
    }
}

bool Player::canJump(){
    return ifCanJump;
}

bool Player::ropeShoot(){
    return ifChainShoot;
}

bool Player::jump() {
    if (ifCanJump) {
        float vx = body->GetLinearVelocity().x;
        body->SetLinearVelocity(b2Vec2(vx, 30.0f));
        callback_player_onJump();
        jumpCooldown = 30;
        return true;
    }
    else return false;
}

void Player::update() {
    ifCanJump = false;
    if (jumpCooldown > 0) {
        jumpCooldown--;
        return;
    }
    physics_RayCastClosestCallback callback = physics_RayCastClosestCallback();
    physics_getWorld()->RayCast(&callback, body->GetPosition(), body->GetPosition() + b2Vec2(0.0f, -10.0f));
    if (callback.hit && !ifChainShoot) {
        float dist = (callback.m_point - body->GetPosition()).Length();
        if (dist < 1.42f) {
            ifCanJump = true;
        }
    }
}

void Player::shootChain(b2Vec2 direction) {
    b2World *world = physics_getWorld();
    chainBodyLen = 0;
    chainJointsLen = 0;
    physics_RayCastClosestCallback callback = physics_RayCastClosestCallback();
    b2Vec2 normD=direction;
    normD.Normalize();
    b2Body *playerBody = body;
    world->RayCast(&callback, playerBody->GetPosition(), playerBody->GetPosition() + direction);
    DTRACETIME("chain shoot");
    if (callback.hit && callback.m_fixture->GetFilterData().categoryBits != physics_collideBgCategory) {
        DTRACETIME("chain hit");
        void *ud = callback.m_fixture->GetBody()->GetUserData();
        if (ud)
            callback_player_onRopeHit((Entity*) ud);
        ifChainShoot = true;
        b2Vec2 pt = callback.m_point - normD;
        b2BodyDef plankBodyDef;
        plankBodyDef.type = b2_dynamicBody;
        plankBodyDef.position = pt;
        plankBodyDef.angularDamping = 0.25f;
        plankBodyDef.linearDamping = 0.25f;
        plankBodyDef.angle = atan2(normD.y, normD.x)+M_PI/2;
        b2Body *body = world->CreateBody(&plankBodyDef);
        body->CreateFixture(&plankFixture);
        body->SetUserData(chainEntity);
        chainBody[chainBodyLen++] = body;
        b2RevoluteJointDef jointDef;
        b2DistanceJointDef distanceJointDef;
        jointDef.collideConnected = false;
        distanceJointDef.collideConnected = false;

        jointDef.Initialize(body, callback.m_fixture->GetBody(), callback.m_point);
        chainJoints[chainJointsLen++] = world->CreateJoint(&jointDef);
        b2Body *prevBody = body;
        float odist = (playerBody->GetPosition() - pt).Length();
        pt -= 2.0f * normD;
        float dist = (playerBody->GetPosition() - pt).Length();
        bool lastPlank;
        if (odist > 5.0f) {
            lastPlank = true;
        } else {
            if ((odist - dist) > 1.99f)
                lastPlank = true;
            else
                lastPlank = false;
        }
        while (odist > 5.0f) {
            plankBodyDef.position = pt;
            body = world->CreateBody(&plankBodyDef);
            body->CreateFixture(&plankFixture);
            body->SetUserData(chainEntity);
            chainBody[chainBodyLen++] = body;
            distanceJointDef.Initialize(prevBody, body, 0.6f * prevBody->GetPosition() + 0.4f*body->GetPosition(), 0.4f * prevBody->GetPosition() + 0.6f*body->GetPosition());
            //jointDef.Initialize(prevBody, body, 0.5f * (prevBody->GetPosition() + body->GetPosition()));
            chainJoints[chainJointsLen++] = world->CreateJoint(&distanceJointDef);
            prevBody = body;
            pt -= 2.0f * normD;
            odist = dist;
            dist = (playerBody->GetPosition() - pt).Length();
        }
        float nw = dist / 2.0f;
        if (lastPlank && nw > 0.5f) {
            pt += 2 * normD;
            pt -= (1 + nw) * normD;
            b2PolygonShape plankShape;
            b2FixtureDef plankFixture;
            plankShape.SetAsBox(0.15, nw);
            plankFixture.shape = &plankShape;
            plankFixture.density = 3.0f;
            plankFixture.friction = 0.5f;
            plankFixture.filter.categoryBits = physics_collideFgCategory;
            plankFixture.filter.maskBits = physics_collideFgCategory;
            plankBodyDef.position = pt;
            body = world->CreateBody(&plankBodyDef);
            body->SetUserData(chainEntity);
            body->CreateFixture(&plankFixture);
            chainBody[chainBodyLen++] = body;
            //jointDef.Initialize(prevBody, body, prevBody->GetPosition() - normD);
            distanceJointDef.Initialize(prevBody, body, 0.6f * prevBody->GetPosition() + 0.4f*body->GetPosition(), 0.4f * prevBody->GetPosition() + 0.6f*body->GetPosition());
            chainJoints[chainJointsLen++] = world->CreateJoint(&distanceJointDef);
            prevBody = body;
        }
        float pbside = (1 + fabs(sin(playerBody->GetAngle() * 2.0f) * (sqrt(2.0f) - 1)));
        //jointDef.Initialize(prevBody, playerBody, playerBody->GetPosition() + pbside * normD);
        distanceJointDef.Initialize(prevBody, playerBody, 0.6f * prevBody->GetPosition() + 0.4f*playerBody->GetPosition(), playerBody->GetPosition() + pbside * normD);
        chainJoints[chainJointsLen++] = world->CreateJoint(&distanceJointDef);
    }
}

void Player::destroyChain() {
    b2World *world = physics_getWorld();
    for (int i = 0; i < chainBodyLen; i++) {
        if (chainBody[i]) {
            world->DestroyBody(chainBody[i]);
        }
    }
    chainBodyLen = 0;
    chainJointsLen = 0;
    ifChainShoot = false;
}

void Player::chainTrigger(bool toMouse) {
    if (ifChainShoot)
        destroyChain();
    else{
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        float realx, realy;
        b2Vec2 sc=body->GetPosition();//engine_getScreenCenter();
        realx=15*(float(mx)/ENGINE_SW-0.5)+sc.x;
        realy=15*(1-float(my)/ENGINE_SH-0.5)+sc.y;
        b2Vec2 direction=b2Vec2(realx, realy)-body->GetPosition();
        direction.Normalize();
        direction*=PLAYER_MAXCHAINLENGTH;
        if(toMouse)
            shootChain(direction);
        else
            shootChain();
    }
}

static Player *player = NULL;

/*
 *
 * rest of functions
 *
 */

Player * player_getPlayer() {
    return player;
}

void player_startUp() {
    if (player_Active)
        return;
    player_Active = true;
    player_cleanEnvironment();
    DTRACETIME("clean");
}

void player_cleanUp() {
    if (!player_Active)
        return;
    player_Active = false;
    if (player)
        delete player;
    DTRACETIME("clean");
}

void player_cleanEnvironment() {
    if (player)
        delete player;
    player = new Player;
    DTRACETIME("env is clean");
}

void player_setSpawnPoint(float x, float y) {
    spawnPoint.x = x;
    spawnPoint.y = y;
}

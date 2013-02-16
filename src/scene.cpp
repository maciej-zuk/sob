#include "scene.h"
#include "callbacks.h"

static map<int, Mix_Chunk*> mcContainer;
map<int, Mix_Chunk*> &scene_getMCContainer() {
    return mcContainer;
}
static int lastMCId = 0;
int scene_getUniqueMCId() {
    return lastMCId++;
}

static map<int, int> channel2PlayedMCIDMap;
map<int, int> &scene_getChannel2PlayedMCIDMap(){
    return channel2PlayedMCIDMap;
}
static int lastPlayedMCId = 0;
int scene_getUniquePlayedMCId(){
    return lastPlayedMCId++;
}



static map<int, b2Shape*> shapeContainer;
map<int, b2Shape*> &scene_getShapeContainer() {
    return shapeContainer;
}
static int lastShapeId = 0;
int scene_getUniqueShapeId() {
    return lastShapeId++;
}

static map<int, b2Joint*> jointContainer;
map<int, b2Joint*> &scene_getJointContainer() {
    return jointContainer;
}
static int lastJointId = 0;
int scene_getUniqueJointId() {
    return lastJointId++;
}

static map<int, b2FixtureDef> fixtureDefContainer;
map<int, b2FixtureDef> &scene_getFixtureDefContainer() {
    return fixtureDefContainer;
}
static int lastFixtureDefId = 0;
int scene_getUniqueFixtureDefId() {
    return lastFixtureDefId++;
}

static map<int, b2Body*> bodyContainer;
map<int, b2Body*> &scene_getBodyContainer() {
    return bodyContainer;
}
static int lastBodyId = 0;
int scene_getUniqueBodyId() {
    return lastBodyId++;
}

static map<int, Entity*> entityContainer;
map<int, Entity*> &scene_getEntityContainer() {
    return entityContainer;
}
static int lastEntityId = 0;
int scene_getUniqueEntityId() {
    return lastEntityId++;
}

void scene_redrawEntities() {

}

void scene_cleanUp() {
    map<int, b2Shape*>::iterator csci = shapeContainer.begin();
    for (; csci != shapeContainer.end(); csci++) {
        delete (*csci).second;
    }

    shapeContainer.clear();
    lastShapeId = 0;

    map<int, Entity*>::iterator eci = entityContainer.begin();
    for (; eci != entityContainer.end(); eci++) {
        delete (*eci).second;
    }

    entityContainer.clear();
    lastEntityId = 0;

    map<int, Mix_Chunk*>::iterator mci = mcContainer.begin();
    for (; mci != mcContainer.end(); mci++) {
        Mix_FreeChunk((*mci).second);
    }
    mcContainer.clear();
    channel2PlayedMCIDMap.clear();
    lastMCId = 0;
    lastPlayedMCId = 0;

    bodyContainer.clear();
    lastBodyId = 0;

    fixtureDefContainer.clear();
    lastFixtureDefId = 0;

    map<int, b2Joint*>::iterator jci = jointContainer.begin();
    for (; jci != jointContainer.end(); jci++) {
        delete (int*) (*jci).second->GetUserData();
    }

    jointContainer.clear();
    lastJointId = 0;

}

void scene_processOnTick() {
    map<int, Entity*>::iterator eci = entityContainer.begin();
    for (; eci != entityContainer.end(); eci++) {
        callback_entity_onTick((*eci).second);
    }
    callback_processDeferredCalls();
}

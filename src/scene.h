#ifndef SCENE_H_
#define SCENE_H_
#include <map>
#include <Box2D/Box2D.h>
#include "entity.h"
#include "sfx.h"
using std::map;

map<int, Mix_Chunk*> &scene_getMCContainer();
int scene_getUniqueMCId();
map<int, int> &scene_getChannel2PlayedMCIDMap();
int scene_getUniquePlayedMCId();

map<int, b2Shape*> &scene_getShapeContainer();
int scene_getUniqueShapeId();

map<int, b2FixtureDef> &scene_getFixtureDefContainer();
int scene_getUniqueFixtureDefId();

map<int, b2Body*> &scene_getBodyContainer();
int scene_getUniqueBodyId();

map<int, Entity*> &scene_getEntityContainer();
int scene_getUniqueEntityId();

map<int, b2Joint*> &scene_getJointContainer();
int scene_getUniqueJointId();


/**
 * @todo redraw output layer of all entities on scene
 */
void scene_redrawEntities();

/**
 * cleans up all entities on scene
 *
 * after this all containers are again ready to use
 */
void scene_cleanUp();

/** calls onTick entity for all entities on scene
 *
 */
void scene_processOnTick();

#endif /* SCENE_H_ */

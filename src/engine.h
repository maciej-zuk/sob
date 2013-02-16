#ifndef ENGINE_H_
#define ENGINE_H_


#ifdef RPI
#include <SDL2/SDL_opengles.h>
#include <SDL2/SDL.h>
#else
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>
#endif

#include <Box2D/Box2D.h>


#include "entity.h"

//todo loading from config
#define ENGINE_SW 800
#define ENGINE_SH 600
#define ENGINE_FPS 60

SDL_Window *engine_getWindow();
b2Vec2 engine_getScreenCenter();
/**
 * start the engine
 * @param argc, argv - forwarded from main()
 */
void engine_startUp(int argc, char ** argv);

/**
 * stop engine
 *
 * function is called automaticaly after main function returns
 */
void engine_cleanUp();

/**
 * run main loop of engine
 */
void engine_mainLoop();

/**
 * each frame, engine will center screen on given entity
 *
 * @bug after invalidating entity's body, engine behavior is undefinied (may work, or may not)
 * @param entity - Entity instance for centering
 */
void engine_centerOnEntity(Entity *entity);

/**
 * engine will center screen on player
 */
void engine_centerOnPlayer();

/**
 * @return true if engine has body given for screen centering
 */
bool engine_hasCenteringBody();

/** run script in proper time during main loop
 *  @param filename of script
 *  @todo change running script to loading scene
 */
void engine_deferredRunScript(const char *name);

#endif /* ENGINE_H_ */

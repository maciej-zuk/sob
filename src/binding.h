#ifndef BINDING_H_
#define BINDING_H_
#include <lua.hpp>

/**
 * @return current global lua state
 */
lua_State *binding_getLuaState();

/**
 * setup bindings
 */
void binding_startUp();

/**
 * clean up bindings
 */
void binding_cleanUp();

/**
 * restart lua environment, recreates lua state for scripting
 */
void binding_cleanEnvironment();

/**
 * run scene script from file
 * @param name - cstring with name of file to load
 */
void binding_loadSceneFromFile(const char *name);

#endif /* BINDING_H_ */

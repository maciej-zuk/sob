#ifndef FS_H_
#define FS_H_

#include <SDL2/SDL_rwops.h>

/**
 * load file as cstring
 *
 * returned pointer has to be freed by caller using delete[]
 */

char *fs_loadAsString(const char* fileName);

/**
 * load file as SDL_RWops
 *
 */

SDL_RWops *fs_loadAsSDL_RWops(const char* fileName);

/**
 * register directory
 */

void fs_registerDir(const char* dir);

/**
 * register zipfile
 */

bool fs_registerZip(const char* file);

/**
 * setup fs
 */
void fs_startUp();

/**
 * clean up fs
 */
void fs_cleanUp();

/**
 * restart fs
 */
void fs_cleanEnvironment();

#endif

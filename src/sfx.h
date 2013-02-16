#ifndef SFX_H_
#define SFX_H_

#include <SDL/SDL_mixer.h>
#define MAX_CHANNELS 256

/** start audio
 *
 */
void sfx_startUp();

/** clean audio
 *
 */
void sfx_cleanUp();

/** restart audio
 *
 */
void sfx_cleanEnvironment();

#endif /* SFX_H_ */

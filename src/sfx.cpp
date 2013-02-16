#include "sfx.h"
#include "debug.h"
#include "callbacks.h"
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_mixer.h>

static bool sfxActive = false;
/*
 * thx to sdl_mixer designers, not so much to do up here ;)
 */


void sfx_startUp() {
    int ret;
    DTRACETIME("sfx start");
    ret = Mix_Init( MIX_INIT_OGG);
    if((ret&MIX_INIT_OGG)!=MIX_INIT_OGG){
        ETRACETIME("Mix_Init: %s", Mix_GetError());
    }
    ret=Mix_OpenAudio(44100, AUDIO_S16, 2, 512);
    if(ret<0){
        ETRACETIME("Mix_OpenAudio: %s", Mix_GetError());
    }
    Mix_AllocateChannels(MAX_CHANNELS);
    sfxActive = true;
}

void sfx_cleanUp() {
    DTRACETIME("sfx clean");
    if (sfxActive) {
        Mix_CloseAudio();
        Mix_Quit();
        sfxActive = false;
    }
}

void sfx_cleanEnvironment() {
    Mix_HaltChannel(-1);
}

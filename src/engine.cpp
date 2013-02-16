#include "debug.h"
#include "engine.h"
#include "types.h"
#include "binding.h"
#include "scene.h"
#include "physics.h"
#include "player.h"
#include "callbacks.h"
#include "sfx.h"
#include "gleswrap.h"

#ifdef RPI
#include <bcm_host.h>
#endif

static SDL_Window *window = NULL;
static SDL_GLContext ctx;
static bool running = false;
static bool engine_Active = false;
static bool needLoadScript = false;
static char textBuffer[255];
int t0,t1,t2,t3,ns;
bool doCollectProfile;


SDL_Window *engine_getWindow() {
    return window;
}

static b2Vec2 screenCenter(0, 0);

static b2Body *centeringBody = NULL;
void centerOn(b2Vec2 where, float dt) {
    b2Vec2 direction = where - screenCenter;
    screenCenter.x += direction.x * dt;
    screenCenter.y += direction.y * dt;
}

b2Vec2 engine_getScreenCenter(){
    return screenCenter;
}

void engine_startUp(int argc, char ** argv) {
    DTRACE("engine_startUp");
    if (engine_Active){
        DTRACE("engine_Active!");
        return;
    }

#ifdef RPI
    bcm_host_init();
#endif
    SDL_Init(SDL_INIT_EVERYTHING);

#ifdef RPI
    window = SDL_CreateWindow("Sob",0,0,1,1, SDL_WINDOW_SHOWN);
    gleswrap_startUp(0);
#else
    window = SDL_CreateWindow("Sob",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,ENGINE_SW, ENGINE_SH, SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);
    ctx = SDL_GL_CreateContext(window);
#endif
    TRACE("SDL Platform: %s", SDL_GetPlatform());
    TRACE("SDL VD: %s", SDL_GetCurrentVideoDriver());

    SDL_ShowCursor(false);
    //SDL_WM_GrabInput(SDL_GRAB_ON);
    glEnable(GL_BLEND);
    glLineWidth(3);
    glClearColor(0.1f, 0.1f, 0.1f, 1);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, ENGINE_SW, ENGINE_SH);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#ifdef RPI
    glOrthof(viewport[0], viewport[2], viewport[1], viewport[3], -1, 1);
#else
    glOrtho(viewport[0], viewport[2], viewport[1], viewport[3], -1, 1);
#endif
    glMatrixMode(GL_MODELVIEW);
    engine_Active = true;

    sfx_startUp();
    physics_startUp();
    binding_startUp();
    player_startUp();
    atexit(engine_cleanUp);
    TRACETIME("engine clean");

    doCollectProfile=false;
    bool sceneLoaded=false;
    for(int i=1; i<argc; i++){
        if(strcmp("profile", argv[i])==0 && !doCollectProfile){
            doCollectProfile=true;
            TRACE("profile mode enable");
        }
        else{
            WTRACETIME("RUNNING FILE %s", argv[1]);
            binding_loadSceneFromFile(argv[1]);
            sceneLoaded=true;
        }
    }

    if (!sceneLoaded) {
        TRACE("");
        WTRACETIME("RUNNING DEMO");
        TRACE("");
        binding_loadSceneFromFile("liftExample.lua");
    }
}

void engine_cleanUp() {
    if (!engine_Active)
        return;
    engine_Active = false;
    sfx_cleanUp();
    scene_cleanUp();
    player_cleanUp();
    binding_cleanUp();
    physics_cleanUp();
    gleswrap_cleanUp();

    SDL_DestroyWindow(window);
    SDL_VideoQuit();
    SDL_Quit();
#if RPI
    bcm_host_deinit();
#endif
    TRACE("engine clean");
    if(doCollectProfile){
        TRACE("%i profile samples collected", ns);
        TRACE("sdl\t\tbox2d\t\tlua\t\topengl");
        TRACE("%fms\t%fms\t%fms\t%fms", float(t0)/ns, float(t1)/ns, float(t2)/ns, float(t3)/ns);
        TRACE("%f ms per frame (max framerate = %ffps)", float(t0+t1+t2+t3)/ns, 1000.0*ns/(t0+t1+t2+t3));
    }
}

void engine_mainLoop() {
    TRACETIME("entering main loop");
    running = true;
    unsigned char *kplist = SDL_GetKeyboardState(NULL);
    int lastTime = SDL_GetTicks() - 1;
    int timeToSleep = 1000.0 / ENGINE_FPS;
    t0=0;
    t1=0;
    t2=0;
    t3=0;
    ns=0;
    int s0,s1,s2,s3,s4;
    while (running) {
        if(doCollectProfile) s0=SDL_GetTicks();
        Player *player = player_getPlayer();
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                running = false;
                return;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_F1){
                    //SDL_WM_ToggleFullScreen(screen);
                }
                else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                    return;
                } else if (e.key.keysym.sym == SDLK_w || e.key.keysym.sym == SDLK_LCTRL) {
                    if(!player->jump())
                        player->chainTrigger();
                } else if (e.key.keysym.sym == SDLK_UP){
                    player->jump();
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if(e.button.button==SDL_BUTTON_RIGHT){
                    if(!player->jump())
                        player->chainTrigger(true);
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if(e.button.button==SDL_BUTTON_RIGHT){
                    if(player->ropeShoot())
                        player->chainTrigger();
                }
                break;
            case SDL_MOUSEMOTION:
                break;
                /*
                float motion=e.motion.xrel;
                if(motion>5) motion=5;
                if(motion<-5) motion=-5;
                if(motion>0) player->moveRight(motion);
                if(motion<0) player->moveLeft(-motion);
                */
            }
        }

        player->update();
        if (kplist[SDL_SCANCODE_A] || kplist[SDL_SCANCODE_LEFT])
            player->moveLeft();
        if (kplist[SDL_SCANCODE_D] || kplist[SDL_SCANCODE_RIGHT])
            player->moveRight();

        int newTime = SDL_GetTicks();
        float fps;
        if(newTime>lastTime) fps = 1000.0f / (newTime - lastTime);
        else fps=1000.0f;
        lastTime = newTime;

        if(doCollectProfile) s1 = SDL_GetTicks();
        physics_step(1.0 / ENGINE_FPS, 4, 4);
        if(doCollectProfile) s2 = SDL_GetTicks();
        callback_processDeferredCalls();
        scene_processOnTick();
        if(doCollectProfile) s3 = SDL_GetTicks();


        if (centeringBody) {
            centerOn(centeringBody->GetPosition(), 1 / 10.0);
        }

        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        glTranslatef(ENGINE_SW / 2.0f, ENGINE_SH / 2.0f, 0);
        glScalef(15, 15, 1);
        glTranslatef(-screenCenter.x, -screenCenter.y, 0);

        physics_redrawBodies();

        glLoadIdentity();
        GLfloat triangle[] = {
            100, 0, 0,
            0, 100, 0,
            0, 0, 0
        };
        GLfloat colors[] = {
            0.0f, 0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f, 1.0f
        };
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, triangle);
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_FLOAT, 0, colors);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);

        gleswrap_swap();


        if(doCollectProfile) {
            s4 = SDL_GetTicks();
            t0+=s1-s0;
            t1+=s2-s1;
            t2+=s3-s2;
            t3+=s4-s3;
            ns++;
        }
        if (needLoadScript) {
            needLoadScript = false;
            binding_loadSceneFromFile(textBuffer);
        }

#ifdef DEBUG
        lua_State *L = binding_getLuaState();
        if (lua_gettop(L)) {
            ETRACETIME("lua stack is not empty after loop, top=%i",lua_gettop(L));
        }
#endif

        if (fps > ENGINE_FPS)
            timeToSleep++;
        else if (timeToSleep > 10){
            timeToSleep--;
        }
        SDL_Delay(timeToSleep);
    }
}

void engine_centerOnEntity(Entity *entity) {
    if (entity) {
        centeringBody = entity->getBody();
        if (centeringBody)
            centerOn(centeringBody->GetPosition(), 1);
    }
}

void engine_centerOnPlayer() {
    centeringBody = player_getPlayer()->getBody();
    if (centeringBody)
        centerOn(centeringBody->GetPosition(), 1);
}

bool engine_hasCenteringBody() {
    return centeringBody != NULL;
}

void engine_deferredRunScript(const char *name) {
    needLoadScript = true;
    sprintf(textBuffer, "%s", name);
}

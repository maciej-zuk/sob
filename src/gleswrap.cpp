#ifdef RPI
#define GLES
#endif

#include "engine.h"
#include "gleswrap.h"
#include "debug.h"


#ifdef GLES
#include <EGL/eglplatform.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <SDL2/SDL_syswm.h>
#include <bcm_host.h>

char eglEStr[][32]={
    "EGL_SUCCESS",
    "EGL_NOT_INITIALIZED",
    "EGL_BAD_ACCESS",
    "EGL_BAD_ALLOC",
    "EGL_BAD_ATTRIBUTE",
    "EGL_BAD_CONFIG",
    "EGL_BAD_CONTEXT",
    "EGL_BAD_CURRENT_SURFACE",
    "EGL_BAD_DISPLAY",
    "EGL_BAD_MATCH",
    "EGL_BAD_NATIVE_PIXMAP",
    "EGL_BAD_NATIVE_WINDOW",
    "EGL_BAD_PARAMETER",
    "EGL_BAD_SURFACE",
    "EGL_CONTEXT_LOST"};


#else
#include <SDL2/SDL_opengl.h>
#endif


#ifdef GLES
static EGLDisplay g_eglDisplay = 0;
static EGLConfig g_eglConfig[16];
static EGLContext g_eglContext = 0;
static EGLSurface g_eglSurface = 0;
static EGL_DISPMANX_WINDOW_T nativeWindow;
static VCHI_INSTANCE_T vchi_instance;
static VCHI_CONNECTION_T *vchi_connections;
#endif

// consts
#define COLOURDEPTH_RED_SIZE  		5
#define COLOURDEPTH_GREEN_SIZE 		6
#define COLOURDEPTH_BLUE_SIZE 		5
#define COLOURDEPTH_DEPTH_SIZE		16

#ifdef GLES
static const EGLint g_configAttribs[] ={
      EGL_RED_SIZE, COLOURDEPTH_RED_SIZE,
      EGL_GREEN_SIZE, COLOURDEPTH_GREEN_SIZE,
      EGL_BLUE_SIZE, COLOURDEPTH_BLUE_SIZE,
      EGL_DEPTH_SIZE, COLOURDEPTH_DEPTH_SIZE,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_BIND_TO_TEXTURE_RGBA, EGL_TRUE,
      EGL_NONE
};
#endif


int gleswrap_startUp(SDL_Window *win)
{
#ifdef GLES
    int ret;
    WTRACE("Using GLES");

    WTRACE("eglGetDisplay");
    g_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_eglDisplay == EGL_NO_DISPLAY)
    {
        ETRACE("Unable to initialise EGL display.");
        return -2;
    }
    WTRACE("eglGetDisplay = %s",eglEStr[eglGetError()-0x3000]);
    WTRACE("eglInitialize");
    int mi, ma;
    ret=eglInitialize(g_eglDisplay, &ma, &mi);
    WTRACE("eglInitialize = %s",eglEStr[eglGetError()-0x3000]);
    if (!ret)
    {
        ETRACE("Unable to initialise EGL display.");
        return -3;
    }
    WTRACE("EGL Version: %i.%i", ma, mi);


    WTRACE("eglChooseConfig");
    EGLint numConfigsOut = 0;
    ret=eglChooseConfig(g_eglDisplay, g_configAttribs, g_eglConfig, 16, &numConfigsOut);
    WTRACE("eglChooseConfig = %s",eglEStr[eglGetError()-0x3000]);
    if (ret != EGL_TRUE || numConfigsOut == 0)
    {
        ETRACE("Unable to find appropriate EGL config.");
        return -4;
    }
    WTRACE("eglChooseConfig = %i", numConfigsOut);

    EGLint contextParams[] = {EGL_CONTEXT_CLIENT_VERSION, 1, EGL_NONE};
    g_eglContext = eglCreateContext(g_eglDisplay, g_eglConfig[0], EGL_NO_CONTEXT, contextParams);
    WTRACE("eglCreateContext = %s",eglEStr[eglGetError()-0x3000]);
    if (g_eglContext == EGL_NO_CONTEXT)
    {
        ETRACE("Unable to create GLES context!");
        return -7;
    }
    unsigned int w, h;
    graphics_get_display_size(0, &w, &h);


    VC_RECT_T dstRect;
    VC_RECT_T srcRect;
    dstRect.x = 0;
    dstRect.y = 0;
    dstRect.width = w;
    dstRect.height = h;
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.width = w << 16;
    srcRect.height = h << 16;
    DISPMANX_DISPLAY_HANDLE_T m_dispmanDisplay = vc_dispmanx_display_open(0);
    DISPMANX_UPDATE_HANDLE_T m_dispmanUpdate = vc_dispmanx_update_start(0);
    DISPMANX_ELEMENT_HANDLE_T m_dispmanElement = vc_dispmanx_element_add ( m_dispmanUpdate, m_dispmanDisplay, 0, &dstRect, 0,&srcRect, DISPMANX_PROTECTION_NONE, 0 ,0,DISPMANX_NO_ROTATE);
    nativeWindow.element = m_dispmanElement;
    nativeWindow.width =w;
    nativeWindow.height =h;
    // we now tell the vc we have finished our update
    vc_dispmanx_update_submit_sync( m_dispmanUpdate );


    WTRACE("eglCreateWindowSurface");
    g_eglSurface = eglCreateWindowSurface(g_eglDisplay, g_eglConfig[0], &nativeWindow, 0);
    WTRACE("eglCreateWindowSurface = %s",eglEStr[eglGetError()-0x3000]);
    if ( g_eglSurface == EGL_NO_SURFACE)
    {
        ETRACE("Unable to create EGL surface! err = %s",eglEStr[eglGetError()-0x3000]);
        return -6;
    }
/*
    WTRACE("eglBindAPI");
    eglBindAPI(EGL_OPENGL_ES_API);
    WTRACE("eglBindAPI = %s",eglEStr[eglGetError()-0x3000]);
*/
    WTRACE("eglMakeCurrent");
    if (eglMakeCurrent(g_eglDisplay,  g_eglSurface,  g_eglSurface, g_eglContext) == EGL_FALSE)
    {
        ETRACE("Unable to make GLES context current");
        return -8;
    }
    WTRACE("eglMakeCurrent = %s",eglEStr[eglGetError()-0x3000]);
    WTRACE("GLES done");
#else
    WTRACE("Using OpenGL");
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, COLOURDEPTH_RED_SIZE);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, COLOURDEPTH_GREEN_SIZE);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, COLOURDEPTH_BLUE_SIZE);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, COLOURDEPTH_DEPTH_SIZE);
#endif
    return 0;
}

void gleswrap_cleanUp()
{
#ifdef GLES
    eglMakeCurrent(g_eglDisplay, NULL, NULL, EGL_NO_CONTEXT);
    WTRACE("eglMakeCurrent = %s",eglEStr[eglGetError()-0x3000]);
    eglDestroySurface(g_eglDisplay, g_eglSurface);
    WTRACE("eglDestroySurface = %s",eglEStr[eglGetError()-0x3000]);
    eglDestroyContext(g_eglDisplay, g_eglContext);
    WTRACE("eglDestroyContext = %s",eglEStr[eglGetError()-0x3000]);
    g_eglSurface = 0;
    g_eglContext = 0;
    eglTerminate(g_eglDisplay);
    WTRACE("eglTerminate = %s",eglEStr[eglGetError()-0x3000]);
    g_eglDisplay = 0;    
#endif
}


int gleswrap_swap()
{
#ifdef GLES
    eglSwapBuffers(g_eglDisplay, g_eglSurface);
    DTRACE("eglSwapBuffers = %s",eglEStr[eglGetError()-0x3000]);
#else
    SDL_GL_SwapWindow(engine_getWindow());
#endif
}

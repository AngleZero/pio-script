#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#else
#include <glad/glad.h>
#endif

#include <string>
using namespace std::string_literals;

struct AppState {
    SDL_Window* wind;
    SDL_GLContext glCtx;
    bool shouldExit = false;
};

#define errif(cond, errorMsg) if (cond) { SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Erro!", ("O programa teve um problema ao inicializar.\n\n    \""s + errorMsg + "\"\n    - computador"s).c_str(), NULL); return SDL_APP_FAILURE; }

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    AppState* as = new AppState();
    *appstate = as;

    errif(!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO), "SDL3: "s + std::string(SDL_GetError()));

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    as->wind = SDL_CreateWindow("pio-script", 800, 450, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    errif(!as->wind, "SDL3: "s + std::string(SDL_GetError()));

    as->glCtx = SDL_GL_CreateContext(as->wind);
    errif(!as->glCtx, "SDL3_GL: "s + std::string(SDL_GetError()));

    #ifndef __EMSCRIPTEN__
    errif(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress), "Failed to initialize GLAD.");
    #endif

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    AppState* as = (AppState*)appstate;
    switch (event->type) {
        case SDL_EVENT_QUIT:
        as->shouldExit = true;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    AppState* as = (AppState*)appstate;

    if (as->shouldExit) return SDL_APP_SUCCESS;

    glClearColor(0, 0.5, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SDL_GL_SwapWindow(as->wind);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    AppState* as = (AppState*)appstate;
    if (as->glCtx) SDL_GL_DestroyContext(as->glCtx);
    if (as->wind) SDL_DestroyWindow(as->wind);
    delete as;
}
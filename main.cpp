// pio-script v0.0.0
// Por Arthur K :]

#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#ifdef __EMSCRIPTEN__
// No desktop usa OpenGL 330 core e na web usa OpenGL 300 es, vou ter que escrever o shader duas vezes :(
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
using namespace std::string_literals;

#include "runtime.h"

struct AppState {
    SDL_Window* wind;
    SDL_GLContext glCtx;
    bool shouldExit = false;

    uint32_t vao;
    uint32_t vbo;
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;

        Vertex(float x, float y, float z, float r, float g, float b): pos(x, y, z), color(r, g, b) {}
    };
    std::vector<Vertex> vertexes = std::vector<Vertex>{
        Vertex(-1, 0, 0, 1, 0, 0),
        Vertex(1, 0, 0, 0, 1, 0),
        Vertex(1, 1, 0, 0, 0, 1)
    };

    uint32_t shaderProg;

    glm::mat4 projMat;
    glm::mat4 viewMat;

    glm::vec3 camPos = glm::vec3(0, 0, 10);
    // Quaternions são usados pra prevenir gimbal lock e também é o que a Unity usa
    glm::quat camRot = glm::quat(glm::vec3());

    RuntimeData runtime;
};

// isso daqui é gambiarra
#define errif(cond, errorMsg) if (cond) { SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Erro!", ("O programa teve um problema ao inicializar.\n\n    \""s + errorMsg + "\"\n    - computador"s).c_str(), NULL); return SDL_APP_FAILURE; }

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    // Tem linguagem de programação em que "as" é uma palavra reservada, mas no C++ não tem esse "as"
    AppState* as = new AppState();

    // Esse negócio de ponteiro pra ponteiro é complicado
    *appstate = as;

    errif(!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO), "SDL3: "s + std::string(SDL_GetError()));

    // Inicializar janela do computador

#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    
    as->wind = SDL_CreateWindow("pio-script", 800, 450, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    errif(!as->wind, "SDL3: "s + std::string(SDL_GetError()));

    as->glCtx = SDL_GL_CreateContext(as->wind);
    errif(!as->glCtx, "SDL3_GL: "s + std::string(SDL_GetError()));

    // Gráficos

    #ifndef __EMSCRIPTEN__
    errif(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress), "Failed to initialize GLAD.");
    #endif

    // Não precisa usar o errif() aqui, n é tão ruim assim se isso daqui falhar
    if (!SDL_GL_SetSwapInterval(-1)) SDL_GL_SetSwapInterval(1);

    glGenVertexArrays(1, &as->vao);
    glBindVertexArray(as->vao);
    glGenBuffers(1, &as->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, as->vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AppState::Vertex), (void*)offsetof(AppState::Vertex, pos));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AppState::Vertex), (void*)offsetof(AppState::Vertex, color));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    uint32_t vertShader = glCreateShader(GL_VERTEX_SHADER);
    uint32_t fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    {
#ifdef __EMSCRIPTEN__
        const char* source =
R"glsl(     #version 300 es

            precision mediump float;

            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aColor;

            uniform mat4 vp;

            out vec3 color;

            void main() {
                gl_Position = vp * vec4(aPos, 1);
                color = aColor;
            }
        )glsl";
#else
        const char* source =
R"glsl(     #version 330 core

            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aColor;

            uniform mat4 vp;

            out vec3 color;

            void main() {
                gl_Position = vp * vec4(aPos, 1);
                color = aColor;
            }
        )glsl";
#endif
        glShaderSource(vertShader, 1, (const char *const *)&source, NULL);
        glCompileShader(vertShader);

        int success;
        char log[512];
        glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(vertShader, 512, NULL, log);
            errif(true, "GL: "s + std::string(log));
        }
    }

    {
#ifdef __EMSCRIPTEN__
        const char* source =
R"glsl(     #version 300 es

            precision mediump float;

            in vec3 color;
            out vec4 fColor;

            void main() {
                fColor = vec4(color, 1);
            }
        )glsl";
#else
        const char* source =
R"glsl(     #version 330 core

            in vec3 color;
            out vec4 fColor;

            void main() {
                fColor = vec4(color, 1);
            }
        )glsl";
#endif
        glShaderSource(fragShader, 1, (const char *const *)&source, NULL);
        glCompileShader(fragShader);

        int success;
        char log[512];
        glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(fragShader, 512, NULL, log);
            errif(true, "GL: "s + std::string(log));
        }
    }

    as->shaderProg = glCreateProgram();
    glAttachShader(as->shaderProg, vertShader);
    glAttachShader(as->shaderProg, fragShader);
    glLinkProgram(as->shaderProg);

    {
        int success;
        char log[512];
        glGetProgramiv(as->shaderProg, GL_LINK_STATUS, &success);

        if (!success) {
            glGetProgramInfoLog(as->shaderProg, 512, NULL, log);
            errif(true, "GL: "s + std::string(log));
        }
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    // Detectar input e request para fechar o aplicativo

    AppState* as = (AppState*)appstate;
    switch (event->type) {
        // Na versão web, pra fechar o aplicativo é só fechar a janela do Chrome, então provavelmente não vai
        // dar tempo de o programa executar SDL_Quit() e deletar o contexto gl, texturas, etc.
        case SDL_EVENT_QUIT:
        as->shouldExit = true;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    // Atualizar as coisas e renderizá-las a cada frame
    AppState* as = (AppState*)appstate;

    if (as->shouldExit) return SDL_APP_SUCCESS;

    glUseProgram(as->shaderProg);

    // Na projeção perspectiva os objetos mais ao fundo parecem menores, e na ortográfica parece tudo igual (dá
    // pra fazer umas ilusão de ótica legal com esse aí)
    int windW, windH;
    SDL_GetWindowSize(as->wind, &windW, &windH);
    as->projMat = glm::perspective(glm::radians(45.f), (float)windW / (float)windH, 0.1f, 2000.f);

    // Rotaciona as coisas por camRot e depois move por camPos, mas quando a câmera se move ou gira, os objetos
    // devem ir na direção oposta, então tem o glm::inverse(), que inverte o processo
    as->viewMat = glm::inverse(glm::translate(glm::toMat4(as->camRot), as->camPos));

    // vp -> projection * view
    // Quando multiplica duas matrizes a da direita é aplicada primeiro e a da esquerda é aplicada depois, confuso ein
    glUniformMatrix4fv(glGetUniformLocation(as->shaderProg, "vp"), 1, GL_FALSE, glm::value_ptr(as->projMat * as->viewMat));
    
    glClearColor(0, 0.5, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(as->vao);
    glBindBuffer(GL_ARRAY_BUFFER, as->vbo);

    glBufferData(GL_ARRAY_BUFFER, as->vertexes.size() * sizeof(AppState::Vertex), as->vertexes.data(), GL_STREAM_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, as->vertexes.size());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    SDL_GL_SwapWindow(as->wind);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    AppState* as = (AppState*)appstate;
    if (as->glCtx) SDL_GL_DestroyContext(as->glCtx);
    if (as->wind) SDL_DestroyWindow(as->wind);
    delete as;
}
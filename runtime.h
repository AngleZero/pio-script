// runtime.h - Complemento pro runtime.cpp

#include <vector>

struct RuntimeData {
    std::vector<char> bytecode;
    std::vector<void*> stack;
    // ponteiro pra ponteiro
    void** varSpace;
};

// void* appstate pra ficar igual às funções SDL_App
void RunByte(void* appstate);
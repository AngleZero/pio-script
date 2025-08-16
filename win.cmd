:: n faço ideia de como usa cmake então uso arquivos .cmd
clang main.cpp extrasrc/glad.c -Iinclude -Llib -lSDL3 -O3 -o build/windows/pio-script.exe && "build/windows/pio-script"
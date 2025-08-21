// runtime.cpp - Roda o bytecode do jogo

#include <vector>

void RunByte(void* appstate) {
    enum InsCode : char {
        INS_VARSPACE_SIZE, // Cria e dimensiona o espaço das variáveis. Deve ser executado uma vez no início do programa. Precisa de um int32 depois.
        
        INS_PUSH_F, // Coloca um float32 em cima (indexo maior) da pilha. No bytecode, precisa de um float32 depois.
        INS_GET_F, // Coloca o valor de uma variável float32 em cima (indexo maior) da pilha. No bytecode, precisa de um int32 depois.
        INS_SET_F, // Muda o valor de uma variável float32. No bytecode, precisa de um int32 depois e um float32 dado por INS_PUSH_F.
        INS_CALL_SYS, // Executa uma função do sistema. No bytecode, precisa de um byte depois e possívelmente valores dados por INS_PUSH_*.
        INS_JUMP // Pula para um ponto específico do bytecode sendo executado em vez de continuar o fluxo normal do programa. No bytecode, precisa de um int32 depois.
    }


}
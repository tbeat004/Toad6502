#include "../include/chip8.h"

void chip8::initialize() {
    // Initialize registers and memory once
    cpu.PC = 0x200; // Program counter starts at 0x200
    cpu.I = 0;      // Reset index register
    cpu.SP = 0;     // Reset stack pointer
    opcode = 0;   // Reset current opcode

    // Clear display
    for (int i = 0; i < 64 * 32; ++i) {
        display.pixels[i] = 0;
    }
    // Clear stack
    for (int i = 0; i < 16; ++i) {
        cpu.stack[i] = 0;
    }
    cpu.SP = 0;

    // Clear registers V0-VF
    for (int i = 0; i < 16; ++i) {
        cpu.V[i] = 0;
    }
    // Clear memory
    for (int i = 0; i < 4096; ++i) {
        memory.memory[i] = 0;
    }


    // Load fontset
    for (int i = 0; i < 80; ++i) {
        memory.memory[0x050 + i] = chip8_fontset[i];
    }
    // Reset timers
    cpu.DT = 0;
    cpu.ST = 0;

}
void chip8::loadGame(const char* filename) {}
void chip8::emulateCycle() {
    // Fetch Opcode 
    // Decode Opcode
    // Execute Opcode

    // Update timers
}
void chip8::setKeys() {}

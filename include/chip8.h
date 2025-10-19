#pragma once
#include <cstdint>
#include <array>
// Define globally usable types
using Byte = uint8_t;  // 8 bits
using Word = uint16_t; // 16 bits

/* System Memory Map
0x000 to 0x1FF - Reserved for the interpreter (contains font set in emu) 0 to 511
0x050 to 0x0A0 - Used for the built-in 4x5 pixel font set (0-F) 80 to 160
0x200 to 0xFFF - Program ROM and work RAM 512 - 4095
*/

struct Memory {
    Byte memory[4096]; // 4KB of memory
};

struct CPU {
    Byte V[16];    // General purpose registers
    Word I;        // Index register
    Word PC;       // Program counter

    Byte DT;      // Delay timer
    Byte ST;      // Sound timer

    Word stack[16]; 
    Byte SP;       // Only needs 0–15, so Byte is enough

    Byte keypad[16];
};

struct Graphics {
    std::array<Byte, 2048> pixels;
};

struct chip8 {
    Memory memory{};
    CPU cpu{};
    Graphics display{};
    Word opcode; // Current opcode

    // Fontset
    Byte chip8_fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0XF0, 0X80, 0XF0, 0X80, 0XF0  // F
    };

    // --Function Declarations--
    void initialize();
    void loadGame(const char* filename);
    void emulateCycle();
    void setKeys();
    
};

#include "../include/chip8.h"
#include <vector>
#include <ctime>
#include "raylib.h"
void chip8::initialize() {
    // Initialize registers and memory once
    cpu.PC = 0x200;
    cpu.I = 0;
    cpu.SP = 0;
    opcode = 0;

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
    
    // For random number generator
    srand(time(0));
}

void chip8::loadGame(const char* filename) {

    // Open the file as a binary stream
    FILE* pFile = fopen(filename, "rb");
    if (pFile == nullptr) {
        fprintf(stderr, "Failed to open ROM: %s\n", filename);
        return;
    }
    fseek(pFile, 0, SEEK_END);
    long fileSize = ftell(pFile);
    rewind(pFile);

    if (fileSize <= 0 || fileSize > (4096 - 512)) {
        fprintf(stderr, "ROM size is invalid: %s\n", filename);
        fclose(pFile);
        return;
    }

    std::vector<Byte> buffer(fileSize);

    // Read the ROM into the buffer
    size_t bytesRead = fread(buffer.data(), sizeof(Byte), fileSize, pFile);

    fclose(pFile);

    // Load the ROM into memory starting at 0x200
    for (long i = 0; i < fileSize; ++i) {
        memory.memory[0x200 + i] = buffer[i];
    }
}

void chip8::emulateCycle() {

    // Fetch opcode
    opcode = memory.memory[cpu.PC] << 8 | memory.memory[cpu.PC + 1];
    // Increase Program Counter
    cpu.PC += 2;
    // Set x, y for Vx, Vy
    Byte x = (opcode & 0x0F00) >> 8;
    Byte y = (opcode & 0x00F0) >> 4;

    switch (opcode & 0xF000) {
    case 0x0000: // 0nnn / 00E0 / 00EE
        switch (opcode & 0x00FF) {
        case 0x00E0: // CLS - Clears the display
            display.pixels.fill(0);
            break;

        case 0x00EE: // RET - Return from subroutine
            cpu.SP--;
            cpu.PC = cpu.stack[cpu.SP];
            break;

        default: // 0nnn - SYS addr (ignored by most modern interpreters)
            // SYS addr
            break;
        }
        break;

    case 0x1000: // 1nnn - JP addr - Jump to location nnn
        cpu.PC = opcode & 0x0FFF;
        break;

    case 0x2000: // 2nnn - CALL addr - Call subroutine at nnn
        cpu.stack[cpu.SP] = cpu.PC; // Store return address
        cpu.SP++;                       // Increment stack pointer
        cpu.PC = opcode & 0x0FFF;       // Jump to address NNN
        break;

    case 0x3000: // 3xkk - SE Vx, byte - Skip next instruction if Vx = kk
        if ((opcode & 0x00FF) == cpu.V[x]) {
            cpu.PC += 2;
        } 
        break;

    case 0x4000: // 4xkk - SNE Vx, byte - Skip next instruction if Vx != kk
        if ((opcode & 0x00FF) != cpu.V[x]) {
            cpu.PC += 2;
        }
        break;

    case 0x5000: // 5xy0 - SE Vx, Vy - Skip next instruction if Vx = Vy
        if (cpu.V[x] == cpu.V[y]) {
            cpu.PC += 2;
        } 
        break;

    case 0x6000: // 6xkk - LD Vx, byte - Set Vx = kk
        cpu.V[x] = opcode & 0x00FF;
        break;

    case 0x7000: // 7xkk - ADD Vx, byte - Set Vx = Vx + kk
        cpu.V[x] = (cpu.V[x] + (opcode & 0x00FF));
        break;

    case 0x8000: // 8xy[0–E] - ALU group
        switch (opcode & 0x000F) {
        case 0x0000: // 8xy0 - LD Vx, Vy
            cpu.V[x] = cpu.V[y];
            break;

        case 0x0001: // 8xy1 - OR Vx, Vy
            cpu.V[x] = cpu.V[x] | cpu.V[y];
            break;

        case 0x0002: // 8xy2 - AND Vx, Vy
            cpu.V[x] = cpu.V[x] & cpu.V[y];
            break;

        case 0x0003: // 8xy3 - XOR Vx, Vy
            cpu.V[x] = cpu.V[x] ^ cpu.V[y];
            break;

        case 0x0004: { // 8xy4 - ADD Vx, Vy
            Word sum = cpu.V[x] + cpu.V[y];
            if (sum > 255) {
                cpu.V[0xF] = 1;
            } else {
                cpu.V[0xF] = 0;
            }
            cpu.V[x] = sum & 0xFF;
            break;
        }

        case 0x0005: // 8xy5 - SUB Vx, Vy
            if (cpu.V[x] > cpu.V[y]) {
                cpu.V[0xF] = 1;
            } else {
                cpu.V[0xF] = 0;
            }
            cpu.V[x] -= cpu.V[y];
            break;

        case 0x0006: // 8xy6 - SHR Vx {, Vy}
            if ((cpu.V[x] & 0x1) == 1) {
                cpu.V[0xF] = 1;
            } else {
                cpu.V[0xF] = 0;
            }
            cpu.V[x] /= 2;
            break;
                
        case 0x0007: // 8xy7 - SUBN Vx, Vy
            if (cpu.V[y] > cpu.V[x]) {
                cpu.V[0xF] = 1;
            } else {
                cpu.V[0xF] = 0;
            }
            cpu.V[x] = cpu.V[y] - cpu.V[x];
            break;

        case 0x000E: // 8xyE - SHL Vx {, Vy}
            if ((cpu.V[x] & 0x8000) == 1) {
                cpu.V[0xF] = 1;
            } else {
                cpu.V[0xF] = 0;
            }
            cpu.V[x] *= 2;
            break;

        default:
            // Unknown 8xy? opcode
            break;
        }
        break;

    case 0x9000: // 9xy0 - SNE Vx, Vy
        if ((opcode & 0x000F) == 0x0000) {
            // SNE Vx, Vy
            if (cpu.V[x] != cpu.V[y]) {
                cpu.PC += 2;
            }
        }
        break; 

    case 0xA000: // Annn - LD I, addr - Set I = nnn
        cpu.I = opcode & 0x0FFF;
        break;

    case 0xB000: // Bnnn - JP V0, addr
        cpu.PC = (opcode & 0x0FFF) + cpu.V[0];
        break;

    case 0xC000: // Cxkk - RND Vx, byte
        cpu.V[x] = (rand() % 256) & (opcode & 0x00FF);
        break;

    case 0xD000: { // Dxyn - DRW Vx, Vy, nibble
        uint8_t Vx = cpu.V[x];
        uint8_t Vy = cpu.V[y];
        uint8_t height = opcode & 0x000F;
        cpu.V[0xF] = 0; // Reset collision flag
        // Loop through each row of the sprite
        for (int row = 0; row < height; row++) {
            Byte spriteByte =
                memory.memory[cpu.I + row]; // Read one byte from memory at I

            // Loop through each bit (8 pixels wide)
            for (int col = 0; col < 8; col++) {
                // Check if bit is set (from MSB to LSB, so start at bit 7)
                if ((spriteByte & (0x80 >> col)) != 0) {
                    // Calculate actual pixel position with wrap-around
                    int pixelX = (Vx + col) % 64;
                    int pixelY = (Vy + row) % 32;
                    int pixelIndex = pixelY * 64 + pixelX;

                    // Check collision: if the pixel is already on and we turn
                    // it off → set VF
                    if (display.pixels[pixelIndex] == 1) {
                        cpu.V[0xF] = 1;
                    }

                    // XOR draw — toggle pixel
                    display.pixels[pixelIndex] ^= 1;
                }
            }
        }
        break;
    }
    case 0xE000: // Ex9E / ExA1 - Key operations
        switch (opcode & 0x00FF) {
        case 0x009E: // SKP Vx
            if (cpu.keypad[cpu.V[x]]) {
                cpu.PC += 2;
            }
            break;

        case 0x00A1: // SKNP Vx
            if (!cpu.keypad[cpu.V[x]]) {
                    cpu.PC += 2;
                }
            break;

        default:
            // Unknown E instruction
            break;
        }
        break;

    case 0xF000: // Fx** - Misc/register/timer/memory ops
        switch (opcode & 0x00FF) {
        case 0x0007: // Fx07 - LD Vx, DT
            cpu.V[x] = cpu.DT;
            break;

        case 0x000A: { // Fx0A - LD Vx, K
            bool keyPressed = false;
            for (int i = 0; i < 16; i++) {
                if (cpu.keypad[i]) {
                    cpu.V[x] = i;
                    keyPressed = true;
                    break;
                }
            }
            if (!keyPressed) { cpu.PC -= 2;}
            break;
        }
        case 0x0015: // Fx15 - LD DT, Vx
            cpu.DT = cpu.V[x];
            break;

        case 0x0018: // Fx18 - LD ST, Vx
            cpu.ST = cpu.V[x];
            break;

        case 0x001E: // Fx1E - ADD I, Vx
            cpu.I += cpu.V[x];
            break;

        case 0x0029: // Fx29 - LD F, Vx
            cpu.I = cpu.V[x] * 5;
            break;

        case 0x0033: // Fx33 - LD B, Vx
            memory.memory[cpu.I] = cpu.V[x] / 100; // Hundreds
            memory.memory[cpu.I + 1] = (cpu.V[x] % 100) / 10; // Tens
            memory.memory[cpu.I + 2] = cpu.V[x] % 10; // Ones
            break;

        case 0x0055: // Fx55 - LD [I], Vx
            for (int k = 0; k <= x; k++) {
                memory.memory[cpu.I + k] = cpu.V[k];
            }
            cpu.I += x + 1;
            break;

        case 0x0065: // Fx65 - LD Vx, [I]
            for (int k = 0; k <= x; k++) {
                cpu.V[k] = memory.memory[cpu.I + k];
            }
            break;

        default:
            // Unknown F instruction
            break;
        }
        break;

    default:
        // Unknown opcode group
        break;
    }
}

/*              Input handling 

Original CHIP-8 Keypad      Qwerty PC Keyboard
┌───┬───┬───┬───┐           ┌───┬───┬───┬───┐
│ 1 │ 2 │ 3 │ C │           │ 1 │ 2 │ 3 │ 4 │
├───┼───┼───┼───┤           ├───┼───┼───┼───┤
│ 4 │ 5 │ 6 │ D │     =>    │ Q │ W │ E │ R │
├───┼───┼───┼───┤           ├───┼───┼───┼───┤
│ 7 │ 8 │ 9 │ E │           │ A │ S │ D │ F │
├───┼───┼───┼───┤           ├───┼───┼───┼───┤
│ A │ 0 │ B │ F │           │ Z │ X │ C │ V │
└───┴───┴───┴───┘           └───┴───┴───┴───┘
*/

void chip8::setKeys() {
    cpu.keypad[0x0] = IsKeyDown(KEY_X);
    cpu.keypad[0x1] = IsKeyDown(KEY_ONE);
    cpu.keypad[0x2] = IsKeyDown(KEY_TWO); 
    cpu.keypad[0x3] = IsKeyDown(KEY_THREE); 
    cpu.keypad[0x4] = IsKeyDown(KEY_Q);
    cpu.keypad[0x5] = IsKeyDown(KEY_W);
    cpu.keypad[0x6] = IsKeyDown(KEY_E); 
    cpu.keypad[0x7] = IsKeyDown(KEY_A);
    cpu.keypad[0x8] = IsKeyDown(KEY_S); 
    cpu.keypad[0x9] = IsKeyDown(KEY_D); 
    cpu.keypad[0xA] = IsKeyDown(KEY_Z); 
    cpu.keypad[0xB] = IsKeyDown(KEY_C); 
    cpu.keypad[0xC] = IsKeyDown(KEY_FOUR); 
    cpu.keypad[0xD] = IsKeyDown(KEY_R); 
    cpu.keypad[0xE] = IsKeyDown(KEY_F); 
    cpu.keypad[0xF] = IsKeyDown(KEY_V); 
}

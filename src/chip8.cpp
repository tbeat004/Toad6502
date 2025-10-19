#include "../include/chip8.h"
#include <vector>
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
void chip8::loadGame(const char* filename) {
    // Buffer to hold the ROM


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

     // Allocate memory to hold the ROM
    std::vector<Byte> buffer(fileSize);
      // Read the ROM into the buffer
    size_t bytesRead = fread(buffer.data(), sizeof(Byte), fileSize, pFile);



    // Close the file
    fclose(pFile);

    // Load the ROM into memory starting at 0x200
    for (long i = 0; i < fileSize; ++i) {
        memory.memory[0x200 + i] = buffer[i];
    }
}
void chip8::emulateCycle() {
    
    // Fetch opcode
    opcode = memory.memory[cpu.PC] << 8 | memory.memory[cpu.PC + 1];
    
   switch (opcode & 0xF000)
{
    case 0x0000: // 0nnn / 00E0 / 00EE
        switch (opcode & 0x00FF)
        {
            case 0x00E0: // CLS
                display.pixels.fill(0);
                cpu.PC += 2;
                break;

            case 0x00EE: // RET
                cpu.SP--;
                cpu.PC = cpu.stack[cpu.SP];
                break;

            default: // 0nnn - SYS addr (ignored by most modern interpreters)
                // SYS addr
                break;
        }
        break;

    case 0x1000: // 1nnn - JP addr
        cpu.PC = opcode & 0x0FFF;
        break;

    case 0x2000: // 2nnn - CALL addr
        cpu.stack[cpu.SP] = cpu.PC + 2;   // Store return address
        cpu.SP++;                        // Increment stack pointer
        cpu.PC = opcode & 0x0FFF;        // Jump to address NNN
        break;

    case 0x3000: // 3xkk - SE Vx, byte
        break;

    case 0x4000: // 4xkk - SNE Vx, byte
        break;

    case 0x5000: // 5xy0 - SE Vx, Vy
        if ((opcode & 0x000F) == 0x0000)
        {
            // SE Vx, Vy
        }
        break;

    case 0x6000: // 6xkk - LD Vx, byte
        cpu.V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        cpu.PC += 2;
        break;

    case 0x7000: // 7xkk - ADD Vx, byte
        cpu.V[(opcode & 0x0F00) >> 8] = (cpu.V[(opcode & 0x0F00) >> 8] +  (opcode & 0x00FF));
        cpu.PC += 2;
        break;

    case 0x8000: // 8xy[0–E] - ALU group
        switch (opcode & 0x000F)
        {
            case 0x0000: // 8xy0 - LD Vx, Vy
                break;

            case 0x0001: // 8xy1 - OR Vx, Vy
                break;

            case 0x0002: // 8xy2 - AND Vx, Vy
                break;

            case 0x0003: // 8xy3 - XOR Vx, Vy
                break;

            case 0x0004: // 8xy4 - ADD Vx, Vy
                break;

            case 0x0005: // 8xy5 - SUB Vx, Vy
                break;

            case 0x0006: // 8xy6 - SHR Vx {, Vy}
                break;

            case 0x0007: // 8xy7 - SUBN Vx, Vy
                break;

            case 0x000E: // 8xyE - SHL Vx {, Vy}
                break;

            default:
                // Unknown 8xy? opcode
                break;
        }
        break;

    case 0x9000: // 9xy0 - SNE Vx, Vy
        if ((opcode & 0x000F) == 0x0000)
        {
            // SNE Vx, Vy
        }
        break;

    case 0xA000: // Annn - LD I, addr
        cpu.I = opcode & 0x0FFF;
        cpu.PC += 2;
        break;

    case 0xB000: // Bnnn - JP V0, addr
        break;

    case 0xC000: // Cxkk - RND Vx, byte
        break;

    case 0xD000: {// Dxyn - DRW Vx, Vy, nibble
        uint8_t x = cpu.V[(opcode & 0x0F00) >> 8];
        uint8_t y = cpu.V[(opcode & 0x00F0) >> 4];
        uint8_t height = opcode & 0x000F;
        cpu.V[0xF] = 0; // Reset collision flag
        // Loop through each row of the sprite
        for (int row = 0; row < height; row++) {
            Byte spriteByte = memory.memory[cpu.I + row]; // Read one byte from memory at I

            // Loop through each bit (8 pixels wide)
            for (int col = 0; col < 8; col++) {
                // Check if bit is set (from MSB to LSB, so start at bit 7)
                if ((spriteByte & (0x80 >> col)) != 0) {
                    // Calculate actual pixel position with wrap-around
                    int pixelX = (x + col) % 64;
                    int pixelY = (y + row) % 32;
                    int pixelIndex = pixelY * 64 + pixelX;

                    // Check collision: if the pixel is already on and we turn it off → set VF
                    if (display.pixels[pixelIndex] == 1) {
                        cpu.V[0xF] = 1;
                    }

                    // XOR draw — toggle pixel
                    display.pixels[pixelIndex] ^= 1;
                }
            }
        }

        cpu.PC += 2; // Move to next instruction
        break;
    }
    case 0xE000: // Ex9E / ExA1 - Key operations
        switch (opcode & 0x00FF)
        {
            case 0x009E: // SKP Vx
                break;

            case 0x00A1: // SKNP Vx
                break;

            default:
                // Unknown E instruction
                break;
        }
        break;

    case 0xF000: // Fx** - Misc/register/timer/memory ops
        switch (opcode & 0x00FF)
        {
            case 0x0007: // Fx07 - LD Vx, DT
                break;

            case 0x000A: // Fx0A - LD Vx, K
                break;

            case 0x0015: // Fx15 - LD DT, Vx
                break;

            case 0x0018: // Fx18 - LD ST, Vx
                break;

            case 0x001E: // Fx1E - ADD I, Vx
                break;

            case 0x0029: // Fx29 - LD F, Vx
                break;

            case 0x0033: // Fx33 - LD B, Vx
                break;

            case 0x0055: // Fx55 - LD [I], Vx
                break;

            case 0x0065: // Fx65 - LD Vx, [I]
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
void chip8::setKeys() {}

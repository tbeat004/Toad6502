#include <iostream>
#include "raylib.h"
#include "../include/chip8.h"

int main() {
chip8 emulator;
emulator.initialize();
emulator.loadGame("../roms/test_opcode.ch8");
double cpuAccumulator = 0.0;
double timerAccumulator = 0.0;
const double CPU_INTERVAL = 1.0 / 700.0;   
const double TIMER_INTERVAL = 1.0 / 60.0;  

SetConfigFlags(FLAG_WINDOW_RESIZABLE);
InitWindow(640, 320, "CHIP-8 Emulator");  

RenderTexture2D chip8Texture = LoadRenderTexture(64, 32);  // CHIP-8 native resolution
SetTargetFPS(60);
SetConfigFlags(FLAG_WINDOW_RESIZABLE);
while (!WindowShouldClose()) {
    // Check for key presses
    emulator.setKeys();
    // 1. Run one or several CHIP-8 opcode cycles here (no drawing yet)
    double dt = GetFrameTime();
    cpuAccumulator += dt;
    timerAccumulator += dt;

    // --- Run CPU based on actual time ---
    while (cpuAccumulator >= CPU_INTERVAL) {
        emulator.emulateCycle();         // run 1 opcode
        cpuAccumulator -= CPU_INTERVAL;
    }

    // --- Run TIMERS based on actual time (60Hz) ---
    while (timerAccumulator >= TIMER_INTERVAL) {
        if (emulator.cpu.DT > 0) emulator.cpu.DT--;
        if (emulator.cpu.ST > 0) emulator.cpu.ST--;
        timerAccumulator -= TIMER_INTERVAL;
    }
    // 2. DRAW CHIP-8 SCREEN TO RENDER TEXTURE
    BeginTextureMode(chip8Texture);
        ClearBackground(BLACK);
        for (int i = 0; i < 2048; i++) {
            if (emulator.display.pixels[i]) {
                int x = (i % 64);             
                int y = (i / 64);             
                DrawRectangle(x, y, 1, 1, WHITE); 
            }
        }

    EndTextureMode();

    // 3. SCALE UP CHIP-8 TEXTURE TO REAL WINDOW
    BeginDrawing();
        ClearBackground(BLACK);
        Rectangle src = { 0.0f, 0.0f, (float)chip8Texture.texture.width, -(float)chip8Texture.texture.height };  // flip vertically for RenderTexture

        Rectangle dst = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };   // scale to window

        Vector2 origin = { 0.0f, 0.0f };

        // Draw scaled to the window
        DrawTexturePro(chip8Texture.texture, src, dst, origin, 0.0f, WHITE);
    EndDrawing();
}

CloseWindow();

    
    return 0;
}
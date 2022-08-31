#ifndef CHIP_9_CHIP8_H
#define CHIP_9_CHIP8_H

#include <cstdint>
#include <stack>
#include <unordered_map>
#include <chrono>
#include <array>
#include <vector>
#include "SDL.h"

class Chip8 {
    typedef std::chrono::high_resolution_clock Clock;
private:
    std::unordered_map<SDL_Keycode, uint8_t> keybindings {
            {SDL_SCANCODE_1, 1},
            {SDL_SCANCODE_2, 2},
            {SDL_SCANCODE_3, 3},
            {SDL_SCANCODE_4, 0xC},
            {SDL_SCANCODE_Q, 4},
            {SDL_SCANCODE_W, 5},
            {SDL_SCANCODE_E, 6},
            {SDL_SCANCODE_R, 0xD},
            {SDL_SCANCODE_A, 7},
            {SDL_SCANCODE_S, 8},
            {SDL_SCANCODE_D, 9},
            {SDL_SCANCODE_F, 0xE},
            {SDL_SCANCODE_Z, 0xA},
            {SDL_SCANCODE_X, 0},
            {SDL_SCANCODE_C, 0xB},
            {SDL_SCANCODE_V, 0xF},
    };
    std::array<uint8_t, 5 * 16> fontset = {
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
            0xF0, 0x80, 0xF0, 0x80, 0x80};
    std::array<uint8_t, 4096> memory{};
    uint16_t pc = 512;
    uint16_t instruction = 0x0000;
    std::array<uint8_t, 16> V{};
    uint16_t IR{};
    std::stack<uint16_t> stack{};


    uint8_t delayT = 0;
    uint8_t soundT = 0;

    bool redraw = false;

    uint16_t opcode() const;

    uint16_t operandX() const;

    uint16_t operandY() const;

    uint16_t operandN() const;

    uint16_t operandNN() const;

    uint16_t operandNNN() const;

    std::chrono::time_point<Clock> nextCycleTime = Clock::now();
    std::chrono::time_point<Clock> nextDrawTime = Clock::now();

    void cycle();

    void draw();

    std::array<bool, 64 * 32> display = {false};
    SDL_Window *win = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;
    std::array<bool, 16> keypad{};
    bool modernBehaviour = true;

public:
    int run(std::vector<unsigned char> rom, bool setBehaviour);
};

#endif //CHIP_9_CHIP8_H

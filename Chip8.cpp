#include "Chip8.h"
#include <algorithm>
#include <iostream>

const int WIN_WIDTH = 960;
const int WIN_HEIGHT = 480;

inline uint16_t Chip8::opcode() const {
    return (instruction & 0xF000) >> 12;
}

inline uint16_t Chip8::operandX() const {
    return (instruction & 0x0F00) >> 8;
}

inline uint16_t Chip8::operandY() const {
    return (instruction & 0x00F0) >> 4;
}

inline uint16_t Chip8::operandN() const {
    return instruction & 0x000F;
}

inline uint16_t Chip8::operandNN() const {
    return instruction & 0x00FF;
}

inline uint16_t Chip8::operandNNN() const {
    return instruction & 0x0FFF;
}

int Chip8::run(std::vector<unsigned char> rom, bool setBehaviour) {
    modernBehaviour = setBehaviour;
    std::copy(rom.begin(), rom.end(), memory.begin() + pc);
    std::copy(fontset.begin(), fontset.end(), memory.begin() + 0x050);
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }
    win = SDL_CreateWindow("chip9", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIN_WIDTH, WIN_HEIGHT,
                           SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(win, -1,
                                  SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    SDL_RenderSetLogicalSize(renderer, WIN_WIDTH, WIN_HEIGHT);
    SDL_RenderSetIntegerScale(renderer, static_cast<SDL_bool>(true));
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

    SDL_Event event;
    bool running = true;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if( event.type == SDL_KEYDOWN )
            {
                auto keycode = keybindings.find(event.key.keysym.scancode);
                if (keycode != keybindings.end()) keypad[keybindings[event.key.keysym.scancode]] = true;
            }
            else if (event.type == SDL_KEYUP) {
                auto keycode = keybindings.find(event.key.keysym.scancode);
                if (keycode != keybindings.end()) keypad[keybindings[event.key.keysym.scancode]] = false;
            }
        }

        if (Clock::now() >= nextCycleTime && !redraw) cycle();

        if (Clock::now() >= nextDrawTime) {
            nextDrawTime = Clock::now() + std::chrono::microseconds(16670);
            draw();
            if (delayT > 0) delayT--;
            if (soundT > 0) soundT--;
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}

void Chip8::cycle() {
    typedef std::chrono::microseconds us;
    instruction = ((uint16_t) memory[pc] << 8) | memory[pc + 1];
    pc += 2;
    switch (opcode()) {
        case 0:
            switch (operandNN()) {
                case 0x00E0:
                    nextCycleTime += us(109);
                    display.fill(false);
                    break;
                case 0x00EE:
                    pc = stack.top();
                    stack.pop();
                    nextCycleTime += us(105);
                    break;
            }
            break;
        case 1:
            pc = operandNNN();
            nextCycleTime += us(105);
            break;
        case 2:
            stack.push(pc);
            pc = operandNNN();
            nextCycleTime += us(105);
            break;
        case 3:
            if (V[operandX()] == operandNN()) {
                pc += 2;
            }
            nextCycleTime += us(55);
            break;
        case 4:
            if (V[operandX()] != operandNN()) {
                pc += 2;
            }
            nextCycleTime += us(55);
            break;
        case 5:
            if (V[operandX()] == V[operandY()]) {
                pc += 2;
            }
            nextCycleTime += us(73);
            break;
        case 9:
            if (V[operandX()] != V[operandY()]) {
                pc += 2;
            }
            nextCycleTime += us(73);
            break;
        case 6:
            V[operandX()] = operandNN();
            nextCycleTime += us(27);
            break;
        case 7:
            V[operandX()] += operandNN();
            nextCycleTime += us(45);
            break;
        case 8:
            switch (operandN()) {
                case 0:
                    V[operandX()] = V[operandY()];
                    nextCycleTime += us(200);
                    break;
                case 1:
                    V[operandX()] = V[operandX()] | V[operandY()];
                    nextCycleTime += us(200);
                    break;
                case 2:
                    V[operandX()] = V[operandX()] & V[operandY()];
                    nextCycleTime += us(200);
                    break;
                case 3:
                    V[operandX()] = V[operandX()] ^ V[operandY()];
                    nextCycleTime += us(200);
                    break;
                case 4:
                    if (V[operandX()] + V[operandY()] > 255) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[operandX()] = V[operandX()] + V[operandY()];
                    nextCycleTime += us(200);
                    break;
                case 5:
                    if (V[operandX()] > V[operandY()]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[operandX()] = V[operandX()] - V[operandY()];
                    nextCycleTime += us(200);
                    break;
                case 7:
                    if (V[operandX()] < V[operandY()]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[operandX()] = V[operandY()] - V[operandX()];
                    nextCycleTime += us(200);
                    break;
                case 6:
                    if (!modernBehaviour) {
                        V[operandX()] = V[operandY()];
                    }
                    V[0xF] = V[operandX()] & 1;
                    V[operandX()] >>= 1;
                    nextCycleTime += us(200);
                    break;
                case 0xE:
                    if (!modernBehaviour) {
                        V[operandX()] = V[operandY()];
                    }
                    V[0xF] = V[operandX()] & (1 << 3);
                    V[operandX()] <<= 1;
                    nextCycleTime += us(200);
                    break;
            }
            break;
        case 0xA:
            IR = operandNNN();
            nextCycleTime += us(55);
            break;
        case 0xB:
            if (modernBehaviour) {
                pc = V[operandX()] + operandNNN();
            } else {
                pc = V[0] + operandNNN();
            }
            nextCycleTime += us(105);
            break;
        case 0xC:
            V[operandX()] = rand() & operandNN();
            nextCycleTime += us(164);
            break;
        case 0xD: {
            uint8_t xpos = V[operandX()] % 64;
            uint8_t ypos = V[operandY()] % 32;
            uint8_t height = operandN();
            std::vector<bool> sprite{};
            sprite.resize(8 * height);
            for (int i = 0; i < height; i++) {
                auto row = static_cast<unsigned long long>(memory[i + IR]);

                for (int c = 0; row; (row >>= 1)) {
                    bool bit = row & 0x01;
                    sprite[i * 8 + 7 - c] = bit;
                    c++;
                }
            }
            V[0xF] = false;
            for (int i = ypos; i < std::min(ypos + height, 32); i++) {
                for (int k = xpos; k < std::min(xpos + 8, 64); k++) {
                    bool spriteBit = sprite[(i - ypos) * 8 + (k - xpos)];
                    if (spriteBit) {
                        if (display[i * 64 + k]) {
                            V[0xF] = true;
                        }
                        display[i * 64 + k] = !display[i * 64 + k];
                    }
                }
            }
            redraw = true;
            nextCycleTime += us(22734);
            break;
        }
        case 0xE: {
            uint8_t keycode = V[operandX()];
            if (keycode < 0xF) {
                switch (operandNN()) {
                    case 0x9E:
                        if (keypad[keycode]) pc += 2;
                        break;
                    case 0xA1:
                        if (!keypad[keycode]) pc += 2;
                        break;
                }
            }
            nextCycleTime += us(73);
            break;
        }
        case 0xF:
            switch (operandNN()) {
                case 0x07:
                    V[operandX()] = delayT;
                    nextCycleTime += us(45);
                    break;
                case 0x15:
                    delayT = V[operandX()];
                    nextCycleTime += us(45);
                    break;
                case 0x18:
                    soundT = V[operandX()];
                    nextCycleTime += us(45);
                    break;
                case 0x1E:
                    if (modernBehaviour) {
                        if (IR + V[operandX()] >= 0x1000) V[0xF] = 1;
                    }
                    IR += V[operandX()];
                    nextCycleTime += us(86);
                    break;
                case 0x0A: {
                    auto keycode = std::find (keypad.begin(), keypad.end(), true);
                    if (keycode == keypad.end()) pc -= 2;
                    else V[operandX()] = keycode - keypad.begin();
                    break;
                }
                case 0x29:
                    IR = 0x050 + (V[operandX()] & 0x0F) * 5;
                    nextCycleTime += us(91);
                    break;
                case 0x33: {
                    uint8_t number = V[operandX()];
                    for (int i = 2; i > -1; i--) {
                        memory[IR + i] = number % 10;
                        number = number / 10;
                    }
                    nextCycleTime += us(927);
                    break;
                }
                case 0x55: {
                    for (int i = 0; i <= operandX(); i++) {
                        if (modernBehaviour) {
                            memory[IR + i] = V[i];
                        } else {
                            memory[IR] = V[i];
                            IR++;
                        }
                    }
                    nextCycleTime += us(64 + operandX() * 64);
                    break;
                }
                case 0x65: {
                    for (int i = 0; i <= operandX(); i++) {
                        if (modernBehaviour) {
                            V[i] = memory[IR + i];
                        } else {
                            V[i] = memory[IR];
                            IR++;
                        }
                    }
                    nextCycleTime += us(64 + operandX() * 64);
                    break;
                }
            }
            break;
    }
}

void Chip8::draw() {
    redraw = false;
    uint32_t pixels[32 * 64];
    for (int i = 0; i < 32 * 64; i++) {
        if (display[i]) {
            pixels[i] = 0xFFFFFFFF;
        } else {
            pixels[i] = 0xFF000000;
        }
    }
    SDL_UpdateTexture(texture, nullptr, pixels, 64 * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

}
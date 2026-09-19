#include <cstdint>
#include <SDL2/SDL.h>

struct chip8 {
    uint8_t memory [4096]; //RAM
    uint8_t registers [16]; //holds a value temporarily(sticky notes) as hex 0toF
    uint16_t indexRegister; //points inside memory using an address
    uint16_t programCounter; //holds the address of the next instruction
    uint16_t stack [16]; //to remember where to go back after a subtask
    uint8_t display [64*32]; //display with 2048 pixels
    uint8_t keypad [16]; //16 keys, either 1 or 0 (pressed or not)
    uint8_t delayTimer; //used for pacing
    uint8_t soundTimer; //beeps if above 0
};

int main(){    chip8 mychip8 = {};
    const int SCALE = 10;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow ("mhsamk emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    mychip8.display[0] = 1;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    for(int i=0; i<2048; ++i){
        if(mychip8.display[i] == 1){
            int col = i % 64;
            int row = i / 64;
            SDL_Rect pixelRect = { col * SCALE, row * SCALE, SCALE, SCALE };
            SDL_RenderFillRect(renderer, &pixelRect);
        }
    }
    SDL_RenderPresent(renderer);
    mychip8.programCounter = 0x200;
    uint16_t opcode = (mychip8.memory [mychip8.programCounter << 8]) | (mychip8.memory [mychip8.programCounter + 1]);

    switch(opcode & 0xF000){
        case 0x0000 : {if (opcode == 0x00E0){for (int i=0; i<2048; ++i){mychip8.display[i] = 0;}}; break;}
        case 0x1000 : {mychip8.programCounter = opcode & 0x0FFF; break;}
        case 0x6000 : {uint8_t x = (opcode & 0x0F00) >> 8; uint8_t nn = opcode & 0x00FF; mychip8.registers[x] = nn; break;}
        case 0x7000 : {uint8_t x = (opcode & 0x0F00) >> 8; uint8_t nn = opcode & 0x00FF; mychip8.registers[x] += nn; break;}
        case 0xA000 : {mychip8.indexRegister = opcode & 0x0FFF; break;}
        case 0xD000 : {uint8_t x = (opcode & 0x0F00) >> 8; x = x % 64;
                       uint8_t y = (opcode & 0x00F0) >> 4; y = y % 32;
                       uint8_t n = (opcode & 0x000F); mychip8.registers[0xF] = 0;
                       for (int row=0; row<n; ++row) { uint8_t spritebyte = mychip8.memory[mychip8.indexRegister + row];
                        for (int col=0; col<8; ++col){ uint8_t pixel = (spritebyte >> (7 - col)) & 1;
                        if (pixel == 1){int index = (y + row) * 64 + (x + col);
                            if (mychip8.display[index] == 1){
                                mychip8.registers[0xF] = 1;
                            }   mychip8.display[index] ^= 1;
                        }}}}

    }


SDL_Delay(3000);
return 0;    
}
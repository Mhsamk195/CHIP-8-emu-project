#include <cstdint>
#include <iostream>
#include <SDL2/SDL.h>

struct chip8 {
    uint8_t memory [4096]; //RAM
    uint8_t registers [16]; //holds a value temporarily(sticky notes) as hex 0toF
    uint16_t indexRegister; //points inside memory using an address
    uint16_t PC; //holds the address of the next instruction
    uint16_t stack [16]; //to remember where to go back after a subtask
    uint8_t display [64*32]; //display with 2048 pixels
    uint8_t KP [16]; //16 keys, either 1 or 0 (pressed or not)
    uint8_t DT; //used for pacing
    uint8_t ST; //beeps if above 0
};

int main(){    chip8 emu = {};
  //////////////////////////////////////////////////////////
    const int SCALE = 10;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow ("mhsamk emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    emu.display[0] = 1;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    for(int i=0; i<2048; ++i){
        if(emu.display[i] == 1){
            int col = i % 64;
            int row = i / 64;
            SDL_Rect pixelRect = { col * SCALE, row * SCALE, SCALE, SCALE };
            SDL_RenderFillRect(renderer, &pixelRect);
        }
    }
    SDL_RenderPresent(renderer);
  //////////////////////////////////////////////////////////
 //starting point
    emu.PC = 0x200;
 //loop starts here
    bool running = true;
    while (running = true) {
 //fetch
    uint16_t opcode = (emu.memory [emu.PC << 8]) | emu.memory [emu.PC + 1];
    emu.PC += 2; //next instruction
 //decode+excute
    switch(opcode & 0xF000){
        case 0x0000 : {if (opcode == 0x00E0){for (int i=0; i<2048; ++i){emu.display[i] = 0;}}; break;}
        case 0x1000 : {emu.PC = opcode & 0x0FFF; break;}
        case 0x6000 : {uint8_t x = (opcode & 0x0F00) >> 8; uint8_t nn = opcode & 0x00FF; emu.registers[x] = nn; break;}
        case 0x7000 : {uint8_t x = (opcode & 0x0F00) >> 8; uint8_t nn = opcode & 0x00FF; emu.registers[x] += nn; break;}
        case 0xA000 : {emu.indexRegister = opcode & 0x0FFF; break;}
        case 0xD000 : {uint8_t x = (opcode & 0x0F00) >> 8; x = x % 64;
                       uint8_t y = (opcode & 0x00F0) >> 4; y = y % 32;
                       uint8_t n = (opcode & 0x000F); emu.registers[0xF] = 0;
                       for (int row=0; row<n; ++row) { uint8_t spritebyte = emu.memory[emu.indexRegister + row];
                        for (int col=0; col<8; ++col){ uint8_t pixel = (spritebyte >> (7 - col)) & 1;
                        if (pixel == 1){int index = (y + row) * 64 + (x + col);
                            if (emu.display[index] == 1){
                                emu.registers[0xF] = 1;
                            }   emu.display[index] ^= 1;
                        }}}}

    } } //loop ends here


SDL_Delay(3000);
return 0;    
}
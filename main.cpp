#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <SDL3/SDL.h>

struct chip8 {
    uint8_t memory [4096]; //RAM
    uint8_t registers [16]; //holds a value temporarily(sticky notes) as hex 0toF
    uint16_t indexRegister; //points inside memory using an address
    uint16_t PC; //holds the address of the next instruction
    uint16_t stack [16]; //to remember where to go back after a subtask
    uint8_t SP; //stack pointer
    uint8_t display [64*32]; //display with 2048 pixels
    uint8_t KP [16]; //16 keys, either 1 or 0 (pressed or not)
    uint8_t DT; //used for pacing
    uint8_t ST; //beeps if above 0
};

bool loadrom(chip8& emu,const std::string& filename){
    std::ifstream file(filename,std::ios::binary);
    if(!file){
        std::cerr<<"ERROR:ROM could not be loaded : "<<filename<<'\n'; return false;}
    file.read(reinterpret_cast<char*>(&emu.memory[0x200]),sizeof(emu.memory)-0x200);
    if(file.bad()){
        std::cerr<<"error occored while reading the ROM\n";
        return false;}
        return true;}

int main(){    chip8 emu = {};
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr<<"error:"
                 <<SDL_GetError()<<'\n';
        return 1;}

    SDL_Window* window=SDL_CreateWindow("mhsamk emulator",640,320,0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window,nullptr);
    if (window == nullptr || renderer == nullptr) {
        std::cerr<<"error:"
                 <<SDL_GetError()<<'\n';
                 SDL_Quit();
                 return 1;
    }

    emu.PC = 0x200;  //starting point
    emu.SP = 0;

    if(!loadrom(emu,"rom.ch8")){return 1;}

    bool running = true;
    while (running) {  //loop starts here
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        if(event.type==SDL_EVENT_QUIT){running=false;}}
 //fetch
    uint16_t opcode = (emu.memory [emu.PC] << 8) | emu.memory [emu.PC + 1];
    emu.PC += 2; //next instruction
 //decode+excute
    switch(opcode & 0xF000){
        case 0x0000 : {if (opcode == 0x00E0){for (int i=0; i<2048; ++i){emu.display[i] = 0;}}
                       else if(opcode==0x00EE){
                        if(emu.SP>0){
                            emu.SP--;
                            emu.PC=emu.stack[emu.SP];
                        }
                       } break;}
        case 0x1000 : {emu.PC = opcode & 0x0FFF; break;}
        case 0x6000 : {uint8_t x = (opcode & 0x0F00) >> 8; uint8_t nn = opcode & 0x00FF; emu.registers[x] = nn; break;}
        case 0x7000 : {uint8_t x = (opcode & 0x0F00) >> 8; uint8_t nn = opcode & 0x00FF; emu.registers[x] += nn; break;}
        case 0xA000 : {emu.indexRegister = opcode & 0x0FFF; break;}
        case 0xD000 : {uint8_t xRegister = (opcode & 0x0F00) >> 8;
                       uint8_t yRegister = (opcode & 0x00F0) >> 4;
                       uint8_t x = emu.registers[xRegister];
                       uint8_t y = emu.registers[yRegister];
                       uint8_t n = (opcode & 0x000F); emu.registers[0xF] = 0;
                       for (int row=0; row<n; ++row) { uint8_t spritebyte = emu.memory[emu.indexRegister + row];
                        for (int col=0; col<8; ++col){ uint8_t pixel = (spritebyte >> (7 - col)) & 1;
                        if (pixel == 1){
                            int px = (x + col) % 64;
                            int py = (y + row) % 32;
                            int index = py * 64 + px;
                            if (emu.display[index] == 1){
                                emu.registers[0xF] = 1;
                            }   emu.display[index] ^= 1;
                        }}} break;}
        case 0x2000 : {if(emu.SP<16){emu.stack[emu.SP]=emu.PC; emu.SP++; emu.PC= opcode & 0x0fff;} break;}
        case 0x3000 : {uint8_t x = (opcode&0x0F00)>>8; uint8_t v = opcode&0x00FF; if(emu.registers[x]==v){emu.PC+=2;} break;}
        case 0x4000 : {uint8_t x = (opcode&0x0F00)>>8; uint8_t v = opcode&0x00FF; if(emu.registers[x]!=v){emu.PC+=2;} break;}
        case 0x5000 : {if((opcode&0x000F)==0x0000){ uint8_t x = (opcode&0x0F00)>>8; uint8_t y = (opcode&0x00F0)>>4; if(emu.registers[x]==emu.registers[y]){emu.PC+=2;}} break;}
        case 0x8000 : {uint8_t x = (opcode & 0x0F00) >> 8; uint8_t y = (opcode&0x00F0)>>4; uint8_t type = opcode&0x000F;
                        switch(type){
                            case 0x0 : {emu.registers[x]=emu.registers[y]; break;}
                            case 0x1 : {emu.registers[x]|=emu.registers[y]; break;}
                            case 0x2 : {emu.registers[x]&=emu.registers[y]; break;}
                            case 0x3 : {emu.registers[x]^=emu.registers[y]; break;}
                            case 0x4 : {uint16_t sum = emu.registers[x] + emu.registers[y];
                                         emu.registers[0xF] = (sum > 0xFF) ? 1 : 0;
                                         emu.registers[x] = static_cast<uint8_t>(sum); break;}
                            case 0x5 : {emu.registers[0xF] = (emu.registers[x] >= emu.registers[y]) ? 1 : 0;
                                        emu.registers[x]=emu.registers[x]-emu.registers[y]; break;}
                            case 0x6 : {emu.registers[0xF]=emu.registers[x]&1; emu.registers[x]>>=1; break;}
                            case 0x7 : {emu.registers[0xF] = (emu.registers[y] >= emu.registers[x]) ? 1 : 0;
                                        emu.registers[x]=emu.registers[y]-emu.registers[x]; break;}
                            case 0xE : {emu.registers[0xF]=(emu.registers[x]>>7)&1;
                                        emu.registers[x]<<=1; break;}
                            default : {break;} break;
                        }}
        default : {std::cout << "Unknown opcode: 0x"<< std::hex << opcode << '\n'; break;}
            } 
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer,255,255,255,255);
    for(int y=0; y<32; ++y){
        for(int x=0; x<64; ++x){
            int index=y*64+x;
            if(emu.display[index] != 0){
                SDL_FRect pixel={
                    static_cast<float>(x*10),static_cast<float>(y*10),10.0f,10.0f};
                SDL_RenderFillRect(renderer,&pixel);
                }
            }
        }
        SDL_RenderPresent(renderer);
    SDL_Delay(2);
} //loop ends here
SDL_DestroyRenderer(renderer);
SDL_DestroyWindow(window);
SDL_Quit();
return 0;    
}
#include <cstdint>

struct chip8 {
    uint8_t memory [4096]; //RAM
    uint8_t registers [16]; //holds a value temporarily(sticky notes) as hex 0toF
    uint16_t indexRegister; //points inside memory using an address
    uint16_t programCounter; //holds the address of the next instruction
    uint16_t stack [16]; //to remember current task
    uint8_t display [64*32]; //display with 2048 pixels
    uint8_t keypad [16]; //16 keys, either 1 or 0 (pressed or not)
    uint8_t delayTimer; //used for pacing
    uint8_t soundTimer; //beeps if above 0
};

int main(){
    chip8 mychip8 = {};
    mychip8.programCounter = 0x200;
    



return 0;    
}
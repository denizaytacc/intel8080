#include <iostream>
#include "cpu.hpp"
#include "display.hpp"

int main (int argc, char *argv[]){
    // first argument: file name
    // second argument: program mode
    CPU cpu;
    if (argc == 3) {
        char *file_name = argv[1];
        int mode = strtol(argv[2], nullptr, 0);
        if (mode == 0) {
            cpu.load_rom(file_name);
            while (cpu.active) {
                cpu.execute();
            }
        }
        else if (mode == 1) {
            Display display;
            while(cpu.active) {
                SDL_PumpEvents();
                while (SDL_PollEvent(&display.event)) {
                    if (display.event.type == SDL_QUIT) {
                        display.destroy();
                        return 0;
                    }
                }
                if (display.state[SDL_SCANCODE_SPACE]) {
                            std::cout << "Key is being pressed." << std::endl ;
                        }
                
                cpu.execute();
            }
            
        } 
        else {
            std::cout << "Wrong mode number, 0 for console, 1 for graphics mode" << std::endl;
        }
    }
    else {
        std::cout << "Incorrect number of arguments" << std::endl;
    }
        
    return 0;    
   }    

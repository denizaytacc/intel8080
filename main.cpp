#include <iostream>
#include "cpu.hpp"

int main (int argc, char* argv[]){
    CPU cpu;
    cpu.load_rom("invaders");
    //std::cout << argc << std::endl;
    int i = 0;
    while (i < 100)    
    {    
        cpu.execute();    
        i += 1;
    }    
    return 0;    
   }    

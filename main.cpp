#include <iostream>
#include "cpu.hpp"
#include "display.hpp"

int main (int argc, char* argv[]){
    CPU cpu;
    cpu.load_rom("tests/8080PRE.COM");
    //Display screen;
    int i = 0;
    while (i < 1061)    
    {    
        cpu.execute();    
        i += 1;
    }    
    return 0;    
   }    

#include <iostream>
#include "cpu.hpp"

CPU::CPU() {
    a = 0;
    bc = 0;
    de = 0;
    hl = 0;

    flag_s = 0;
    flag_z = 0;
    flag_p = 0;
    flag_c = 0;
    flag_hc = 0;

    pc = 0;
    sp = 0;


}

bool CPU::get_parity(uint16_t n){
    bool parity = 0;
    while (n)
    {
        parity = !parity;
        n     = n & (n - 1);
    }    
    return parity;
}

bool get_halfcarry(int a, int b){
	return ((((a & 0xF) + (b & 0xF)) & 0x10) == 0x10);
}


void CPU::set_flags_sub(uint8_t op1, uint8_t op2, bool change_carry) {
    uint16_t answer = op1 - op2;
    // Sign flag
    if (answer & 0x80) {
        flag_s = 1;
    }
    else {
        flag_s = 0;
    }
    // Zero flag
    if ((answer & 0xff) == 0) {
        flag_z = 0;
    }
    else {
        flag_z = 0;
    }
    // Parity flag
    if (get_parity(answer)) {
        flag_p = 0;
    }
    else {
        flag_p = 1;
    }
    // Carry flag
    if (change_carry) {
        if (answer > 0xff) {
            flag_c = 0;
        }
        else {
            flag_c = 1;
        }
    }
    // Half carry flag, should it really be ?? 
    if (((((op1 & 0xF) + (op2 & 0xF)) & 0x10) == 0x10)) {
        flag_hc = 0;
    }

    else {
        flag_hc = 1;
    }

}


void CPU::set_flags_add(uint8_t op1, uint8_t op2, bool change_carry) {
    uint16_t answer = op1 + op2;
    // Sign flag
    if (answer & 0x80) {
        flag_s = 1;
    }
    else {
        flag_s = 0;
    }
    // Zero flag
    if ((answer & 0xff) == 0) {
        flag_z = 0;
    }
    else {
        flag_z = 0;
    }
    // Parity flag
    if (get_parity(answer)) {
        flag_p = 0;
    }
    else {
        flag_p = 1;
    }
    // Carry flag
    if (change_carry) {
        if (answer > 0xff) {
            flag_c = 0;
        }
        else {
            flag_c = 1;
        }
    }
    // Half carry flag
    if (((((op1 & 0xF) + (op2 & 0xF)) & 0x10) == 0x10)) {
        flag_hc = 0;
    }

    else {
        flag_hc = 1;
    }

}



void CPU::load_rom(const char* fileName) { 
    FILE *rom;
    rom = fopen(fileName, "rb");
    if ( rom == NULL )
    {
        std::cout << "The file failed to open." << std::endl;
    }
    fseek(rom, 0 ,SEEK_END);
    long bufferSize = ftell(rom);
    rewind(rom);

    char *buffer = (char*)malloc(sizeof(char) * bufferSize);
    if (buffer == NULL) 
    {
        std::cout << "Memory error" << std::endl;
    }

    size_t result = fread(buffer, 1, bufferSize, rom);
    if (result != bufferSize) 
    {
        std::cout << "Reading error" << std::endl;
    }
    for (int i = 0; i < bufferSize; i++) {
        memory[i] = buffer[i];
    }
    fclose(rom);
    free(buffer);
}

void CPU::execute() {
    opcode = memory[pc];
    unsigned char *code = &memory[pc];  

    switch (opcode) {
        case 0x00:
            std::cout << "NOP" << std::endl;
            pc += 1;

        case 0x01:
            std::cout << "LXI, B, d16" << std::endl;
            *b = memory[pc + 3];
            *c = memory[pc + 2];
            std::cout << "value of bc is: " << bc << std::endl; 
            if (*b == 0xd4) {
                std::cout << "Value of b is 0xd4" << std::endl;
            } 
            if (*c == 0xc3) {
                std::cout << "Value of c is 0xc3" << std::endl;
            }
            pc += 3;

        case 0x02:
            std::cout << "STAX B" << std::endl;
            bc = a;
            std::cout << "value of bc is: " << bc << std::endl; 
            pc += 1;

        case 0x03:
            std::cout << "INX BC" << std::endl;
            bc += 1;
            std::cout << "value of bc is: " << bc << std::endl; 
            pc += 1;

        case 0x04:
            std::cout << "INR B" << std::endl;
            set_flags_add(*b, 1, 0);
            *b += 1;
            pc += 1;

        case 0x05:
            std::cout << "DCR B" << std::endl;
            set_flags_sub(*b, 1, 0);
            *b -= 1;
            pc += 1;

        case 0x06:
            std::cout << "MVI B, D8" << std::endl;
            *b = memory[pc + 2];
            pc += 2;

        case 0x07:
            std::cout << "RLC" << std::endl;
            uint8_t temp = (a & 0x80) >> 7;
            a = a << 1;
            a = a | temp;
            flag_c = temp;
            pc += 1;
    }
}
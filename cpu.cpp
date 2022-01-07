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

// double check every rotate command later
void CPU::execute() {
    opcode = memory[pc];
    std::cout << "OPCODE: 0x" << std::hex << int(opcode) << std::dec << std::endl;
    switch (opcode) {

        // -------------------------0x-------------------------  

        case 0x00:
            std::cout << "NOP" << std::endl;
            pc += 1;
            break;

        case 0x01:
            std::cout << "LXI, B, d16" << std::endl;
            bc = (memory[pc + 1] | (memory[pc + 2] << 8));
            pc += 3;
            break; 

        case 0x02:
            std::cout << "STAX B" << std::endl;
            memory[bc] = a;
            pc += 1;
            break;

        case 0x03:
            std::cout << "INX B" << std::endl;
            bc += 1;
            pc += 1;
            break;

        case 0x04:
            std::cout << "INR B" << std::endl;
            set_flags_add(*b, 1, 0);
            *b += 1;
            pc += 1;
            break;

        case 0x05:
            std::cout << "DCR B" << std::endl;
            set_flags_sub(*b, 1, 0);
            *b -= 1;
            pc += 1;
            break;

        case 0x06:
            std::cout << "MVI B, D8: " << int(memory[pc + 1]) << std::endl;
            *b = memory[pc + 1];
            pc += 2;
            break;

        case 0x07:
            {
            std::cout << "RLC" << std::endl;
            uint8_t temp = (a & 0x80) >> 7;
            a = a << 1;
            a = a | temp;
            flag_c = temp;
            pc += 1;
            }
            break;

        case 0x09:
            {
            std::cout << "DAD B" << std::endl;
            flag_c = (hl + bc) > 0xffff;
            hl = hl + bc;
            pc += 1;
            break;
            }

        case 0x0a:
            std::cout << "LDAX B" << std::endl;
            a = memory[bc];
            pc += 1;
            break;

        case 0x0b:
            std::cout << "DCX B" << std::endl;
            bc -= 1 ;
            pc += 1;
            break;

        case 0x0c:
            std::cout << "INR C" << std::endl;
            set_flags_add(*c, 1, 0);
            *c -= 1;
            pc += 1;
            break;

        case 0x0d:
            std::cout << "DCR C" << std::endl;
            set_flags_sub(*c, 1, 0);
            *c -= 1;
            pc += 1;
            break;

        case 0x0e: 
            std::cout << "MVI C, D8" << std::endl;
            *c = memory[pc + 1];
            pc += 2;
            break;

        case 0x0f:
            {
            std::cout << "RRC" << std::endl;
            uint8_t temp = (a & 1);
            a = a >> 1;
            a = a | (temp << 7);
            flag_c = temp;
            pc += 1;
            }
            break;


        // -------------------------1x------------------------- 

        case 0x11:
            std::cout << "LXI D, D16" << std::endl;
            de = memory[pc + 1] | memory[pc + 2] << 8;
            pc += 3;
            break;

        case 0x12:
            std::cout << "STAX D" << std::endl;
            memory[de] = a;
            pc += 1;
            break;

        case 0x13:
            std::cout << "INX D" << std::endl;
            de += 1;
            pc += 1;
            break;

        case 0x14:
            std::cout << "INR D" << std::endl;
            set_flags_add(*d, 1, 0);
            *d += 1;
            pc += 1;
            break;

        case 0x15:
            std::cout << "DCR D" << std::endl;
            set_flags_sub(*d, 1, 0);
            *d -= 1;
            pc += 1;
            break;

        case 0x16:
            std::cout << "MVI D, D8" << std::endl;
            *d = memory[pc + 1];
            pc += 2;
            break;

        case 0x17:
            {
            std::cout << "RAL" << std::endl;
            unsigned char temp = (a & 0x80) >> 7;
            a = a << 1;
            a = a | flag_c;
            flag_c = temp;
            pc += 1;
            break;
            }

        case 0x19:
            {
            std::cout << "DAD D" << std::endl;
            flag_c = (hl + de) > 0xffff;
            hl = hl + de;
            pc += 1;
            break;
            }

        case 0x1a:
            std::cout << "LDAX D" << std::endl;
            a = memory[de];
            pc += 1;
            break;

        case 0x1b:
            std::cout << "DCX D" << std::endl;
            de -= 1;
            pc += 1;
            break;

        case 0x1c:
            std::cout << "INR E" << std::endl;
            set_flags_add(*e, 1, 0);
            *e += 1;
            pc += 1;
            break;

        case 0x1d:
            std::cout << "DCR E" << std::endl;
            set_flags_sub(*e, 1, 0);
            *e -= 1;
            pc += 1;
            break;

        case 0x1e:
            std::cout << "MVI E, D8" << std::endl;
            *e = memory[pc + 1];
            pc += 2;
            break;

        case 0x1f:
            {
            std::cout << "RAR" << std::endl;
            unsigned char temp = (a & 0x80) >> 7;
            a = a >> 1;
            a = a | (temp << 7);
            flag_c = temp;
            pc += 1;
            break;
            }

        // -------------------------2x------------------------- 
        
        
        case 0x21:
            std::cout << "LXI H,D16" << std::endl;
            hl = memory[pc + 1] | memory[pc + 2] << 8;
            pc += 3;
            break;

        // IMPLEMENT LATER
        case 0x22:
            {    
            uint16_t new_addr = memory[pc + 1] | memory[pc + 2] << 8;
            std::cout << "SHLD adr" << std::endl;
            memory[new_addr] = hl & 0xFF;
            memory[new_addr + 1] = hl >> 8;
            pc += 3;
            break;
            }

        case 0x23:
            std::cout << "INX H" << std::endl;
            hl += 1;
            pc += 1;
            break;

        case 0x24:
            std::cout << "INR H" << std::endl;
            set_flags_add(*h, 1, 0);
            *h += 1;
            pc += 1;
            break;

        case 0x25:
            std::cout << "DCR H" << std::endl;
            set_flags_sub(*h, 1, 0);
            *h -= 1;
            pc += 1;
            break;

        case 0x26:
            std::cout << "MVI H,D8" << std::endl;
            *h = memory[pc + 1];
            pc += 2;
            break;

        // Special??
        case 0x27:
            std::cout << "DAA" << std::endl;
            pc += 1;
            break;

        case 0x29:
            std::cout << "DAD H" << std::endl;
            flag_c = (hl + hl) > 0xffff; 
            hl += hl;
            pc += 1;
            break;

        case 0x2a:
            {
            std::cout << "LHLD adr" << std::endl;
            uint16_t new_addr = memory[pc + 1] | memory[pc + 2] << 8;
            hl = memory[new_addr] | memory[new_addr + 1] << 8;
            pc += 3;
            break;
            }

        case 0x2b:
            std::cout << "DCX H" << std::endl;
            hl -= 1;
            pc += 1;
            break;

        case 0x2c:
            std::cout << "INR L" << std::endl;
            set_flags_add(*l, 1, 0);
            *l += 1;
            pc += 1;
            break;

        case 0x2d:
            std::cout << "DCR L" << std::endl;
            set_flags_sub(*l, 1, 0);
            *l -= 1;
            pc += 1;
            break;

        case 0x2e:
            std::cout << "MVI L, D8" << std::endl;
            *l = memory[pc + 1];
            pc += 2;
            break;

        case 0x2f:
            std::cout << "CMA" << std::endl;
            a = ~a;
            pc += 1;
            break;









        // -------------------------3x------------------------- 

        case 0x31:
            std::cout << "LXI SP, D16" << std::endl;
            sp = memory[pc + 1] | memory[pc + 2] << 8;
            std::cout << "stack pointer is: " << sp << std::endl;
            pc += 3;
            break;

        case 0xc3:
            std::cout << "JMP adr";
            std::cout << "~ is: 0x" << std::hex << (memory[pc + 1] | (memory[pc + 2] << 8)) << std::dec << std::endl;
            pc = (memory[pc + 1] | (memory[pc + 2] << 8));
            break;

        case 0xcd:
            std::cout << "CALL adr~ 0x" << std::hex << (memory[pc + 1] | (memory[pc + 2] << 8)) << std::dec << std::endl;
            // Implement stack
            pc = (memory[pc + 1] | (memory[pc + 2] << 8));

        // End of switch    
        break;  


    }

}
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

void CPU::stack_push(uint16_t val) {
    memory[sp - 2] = val & 0xff;
    memory[sp - 1] = val >> 8;
    sp -= 2;
}

uint16_t CPU::stack_pop() {
    uint16_t val = memory[sp] | (memory[sp + 1] << 8);
    sp += 2;
    return val;
}

bool CPU::get_parity(uint16_t n){
    bool parity = 0;
    while (n)
    {
        parity = !parity;
        n     = n & (n - 1);
    }    
    return !parity;
}

bool CPU::get_carry(uint8_t op1, uint8_t op2) {
	int16_t answer = op1 + op2;
	int16_t carry = answer ^ op1 ^ op2;
	return (carry & (1 << 8)) != 0;
}

void CPU::set_flags_sub(uint8_t op1, uint8_t op2, bool change_carry) {
    uint16_t answer = op1 - op2;
    std::cout << std::hex << "Sub answer is: " << answer << std::dec << "/" << answer << std::endl;
    // Sign flag
    flag_s = ((answer & 0x80) != 0);
    // Zero flag
    flag_z = ((answer & 0xff) == 0);
    // Parity flag
    flag_p = get_parity(answer);
    // Carry flag
    if (change_carry) flag_c = !get_carry(op1, op2 * - 1);
    // Half carry flag
    flag_hc = ((((op1 & 0xF) + (-1 * op2 & 0xF)) & 0x10) == 0x10);//(((op1 & 0xF) - (op2 & 0xF)) < 0);

}

void CPU::set_flags_add(uint8_t op1, uint8_t op2, bool change_carry) {
    uint16_t answer = op1 + op2;
    std::cout << std::hex << "Add answer is: "  << answer << std::dec << std::endl;
    // Sign flag
    flag_s = ((answer & 0x80) != 0);
    // Zero flag
    flag_z = ((answer & 0xff) == 0);
    // Parity flag
    flag_p = get_parity(answer);
    // Carry flag
    if (change_carry) flag_c = get_carry(op1, op2);
    // Half carry flag
    flag_hc = ((((op1 & 0xF) + (op2 & 0xF)) & 0x10) == 0x10);

}

void CPU::debug() {
    std::cout << "Sign: " << int(flag_s) << std::endl;
    std::cout << "Zero: " << int(flag_z) << std::endl;
    std::cout << "Parity: " << int(flag_p) << std::endl;
    std::cout << "Carry: " << int(flag_c) <<std::endl;
    std::cout << "Half Carry: " << int(flag_hc) << std::endl;
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

        // -------------------------0x-------------------------  //

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


        // -------------------------1x------------------------- //

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

        // -------------------------2x------------------------- //
        
        
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




        // -------------------------3x------------------------- //


        case 0x31:
            std::cout << "LXI SP, D16" << std::endl;
            sp = memory[pc + 1] | memory[pc + 2] << 8;
            pc += 3;
            break;
            
        case 0x32:
            {
            std::cout << "STA adr" << std::endl;
            uint16_t new_addr = memory[pc + 1] | memory[pc + 2] << 8;
            memory[new_addr] = a;
            pc += 3;
            break;
            }

        case 0x33:
            std::cout << "INX SP" << std::endl;
            sp += 1;
            pc += 1;
            break;
            
        case 0x34:
            std::cout << "INR M" << std::endl;
            set_flags_add(memory[hl], 1, 0);
            memory[hl] += 1;
            pc += 1;
            break;
            
        case 0x35:
            std::cout << "DCR M" << std::endl;
            set_flags_sub(memory[hl], 1, 0);
            memory[hl] -= 1;
            pc += 1;
            break;
            
        case 0x36:
            std::cout << "MVI M,D8" << std::endl;
            memory[hl] = memory[pc + 1];
            pc += 2;
            break;
            
        case 0x37:
            std::cout << "STC" << std::endl;
            flag_c = 1;
            pc += 1;
            break;
            
            
        case 0x39:
            std::cout << "DAD SP" << std::endl;
            flag_c = (hl + sp) > 0xffff;
            hl = hl + sp;
            pc += 1;    
            break;
            
        case 0x3a:
            {
            std::cout << "LDA adr" << std::endl;
            uint16_t new_addr = memory[pc + 1] | memory[pc + 2] << 8;
            a = memory[new_addr];
            pc += 3;
            break;
            }

        case 0x3b:
            std::cout << "DCX SP" << std::endl;
            sp -= 1;
            pc += 1;
            break;
            
        case 0x3c:
            std::cout << "INR A" << std::endl;
            set_flags_add(a, 1, 0);
            a += 1;
            pc += 1;
            break;
            
        case 0x3d:
            std::cout << "DCR A" << std::endl;
            set_flags_sub(a, 1, 0);
            a -= 1;
            pc += 1;
            break;
            
        case 0x3e:
            std::cout << "MVI A,D8" << std::endl;
            a = memory[pc + 1];
            pc += 2;
            break;
            
        case 0x3f:
            std::cout << "CMC" << std::endl;
            flag_c = ~flag_c;
            pc += 1;
            break;


        // -------------------------4x------------------------- //

        case 0x40:
            std::cout << "MOV B, B" << std::endl; 
            *b = *b;
            pc += 1;
            break;

        case 0x41:
            std::cout << "MOV B, C" << std::endl; 
            *b = *c;
            pc += 1;
            break;

        case 0x42:
            std::cout << "MOV B, D" << std::endl; 
            *b = *d;
            pc += 1;
            break;

        case 0x43:
            std::cout << "MOV B, E" << std::endl; 
            *b = *e;
            pc += 1;
            break;

        case 0x44:
            std::cout << "MOV B, H" << std::endl; 
            *b = *h;
            pc += 1;
            break;

        case 0x45:
            std::cout << "MOV B, L" << std::endl; 
            *b = *l;
            pc += 1;
            break;

        case 0x46:
            std::cout << "MOV B, M" << std::endl; 
            *b = memory[hl];
            pc += 1;
            break;

        case 0x47:
            std::cout << "MOV B, A" << std::endl; 
            *b = a;
            pc += 1;
            break;

        case 0x48:
            std::cout << "MOV C, B" << std::endl; 
            *c = *b;
            pc += 1;
            break;

        case 0x49:
            std::cout << "MOV C, C" << std::endl; 
            *c = *c;
            pc += 1;
            break;

        case 0x4a:
            std::cout << "MOV C, D" << std::endl; 
            *c = *d;
            pc += 1;
            break;

        case 0x4b:
            std::cout << "MOV C, E" << std::endl; 
            *c = *e;
            pc += 1;
            break;

        case 0x4c:
            std::cout << "MOV C, H" << std::endl; 
            *c = *h;
            pc += 1;
            break;

        case 0x4d:
            std::cout << "MOV C, L" << std::endl; 
            *c = *l;
            pc += 1;
            break;

        case 0x4e:
            std::cout << "MOV C, M" << std::endl; 
            *c = memory[hl];
            pc += 1;
            break;

        case 0x4f:
            std::cout << "MOV C, A" << std::endl; 
            *c = a;
            pc += 1;
            break;

        // -------------------------5x------------------------- //

        case 0x50:
            std::cout << "MOV D, B" << std::endl; 
            *d = *b;
            pc += 1;
            break;

        case 0x51:
            std::cout << "MOV D, C" << std::endl; 
            *d = *c;
            pc += 1;
            break;

        case 0x52:
            std::cout << "MOV D, D" << std::endl; 
            *d = *d;
            pc += 1;
            break;

        case 0x53:
            std::cout << "MOV D, E" << std::endl; 
            *d = *e;
            pc += 1;
            break;

        case 0x54:
            std::cout << "MOV D, H" << std::endl; 
            *d = *h;
            pc += 1;
            break;

        case 0x55:
            std::cout << "MOV D, L" << std::endl; 
            *d = *l;
            pc += 1;
            break;

        case 0x56:
            std::cout << "MOV D, M" << std::endl; 
            *d = memory[hl];
            pc += 1;
            break;

        case 0x57:
            std::cout << "MOV D, A" << std::endl; 
            *b = a;
            pc += 1;
            break;

        case 0x58:
            std::cout << "MOV E, B" << std::endl; 
            *e = *b;
            pc += 1;
            break;

        case 0x59:
            std::cout << "MOV E, C" << std::endl; 
            *e = *c;
            pc += 1;
            break;

        case 0x5a:
            std::cout << "MOV E, D" << std::endl; 
            *e = *d;
            pc += 1;
            break;

        case 0x5b:
            std::cout << "MOV E, E" << std::endl; 
            *e = *e;
            pc += 1;
            break;

        case 0x5c:
            std::cout << "MOV E, H" << std::endl; 
            *e = *h;
            pc += 1;
            break;

        case 0x5d:
            std::cout << "MOV E, L" << std::endl; 
            *e = *l;
            pc += 1;
            break;

        case 0x5e:
            std::cout << "MOV E, M" << std::endl; 
            *e = memory[hl];
            pc += 1;
            break;

        case 0x5f:
            std::cout << "MOV E, A" << std::endl; 
            *e = a;
            pc += 1;
            break;


        // -------------------------6x------------------------- //
        
        
        case 0x60:
            std::cout << "MOV H, B" << std::endl; 
            *h = *b;
            pc += 1;
            break;
            
        case 0x61:
            std::cout << "MOV H, C" << std::endl; 
            *h = *c;
            pc += 1;
            break;

        case 0x62:
            std::cout << "MOV H, D" << std::endl; 
            *h = *d;
            pc += 1;
            break;

        case 0x63:
            std::cout << "MOV H, E" << std::endl; 
            *h = *e;
            pc += 1;
            break;

        case 0x64:
            std::cout << "MOV H, H" << std::endl; 
            *h = *h;
            pc += 1;
            break;

        case 0x65:
            std::cout << "MOV H, L" << std::endl; 
            *h = *l;
            pc += 1;
            break;

        case 0x66:
            std::cout << "MOV H, M" << std::endl; 
            *h = memory[hl];
            pc += 1;
            break;

        case 0x67:
            std::cout << "MOV H, A" << std::endl; 
            *h = a;
            pc += 1;
            break;

        case 0x68:
            std::cout << "MOV L, B" << std::endl; 
            *l = *b;
            pc += 1;
            break;

        case 0x69:
            std::cout << "MOV L, C" << std::endl; 
            *l = *c;
            pc += 1;
            break;

        case 0x6a:
            std::cout << "MOV L, D" << std::endl; 
            *l = *d;
            pc += 1;
            break;

        case 0x6b:
            std::cout << "MOV L, E" << std::endl; 
            *l = *e;
            pc += 1;
            break;

        case 0x6c:
            std::cout << "MOV L, H" << std::endl; 
            *l = *h;
            pc += 1;
            break;

        case 0x6d:
            std::cout << "MOV L, L" << std::endl; 
            *l = *l;
            pc += 1;
            break;

        case 0x6e:
            std::cout << "MOV L, M" << std::endl; 
            *l = memory[hl];
            pc += 1;
            break;

        case 0x6f:
            std::cout << "MOV L, A" << std::endl; 
            *l = a;
            pc += 1;
            break;


        // -------------------------7x------------------------- //

        case 0x70:
            std::cout << "MOV M, B" << std::endl; 
            memory[hl] = *b;
            pc += 1;
            break;
            
        case 0x71:
            std::cout << "MOV M, C" << std::endl; 
            memory[hl] = *c;
            pc += 1;
            break;

        case 0x72:
            std::cout << "MOV M, D" << std::endl; 
            memory[hl] = *d;
            pc += 1;
            break;

        case 0x73:
            std::cout << "MOV M, E" << std::endl; 
            memory[hl] = *e;
            pc += 1;
            break;

        case 0x74:
            std::cout << "MOV M, H" << std::endl; 
            memory[hl] = *h;
            pc += 1;
            break;

        case 0x75:
            std::cout << "MOV M, L" << std::endl; 
            memory[hl] = *l;
            pc += 1;
            break;

        case 0x76:
            std::cout << "HLT" << std::endl; 
            pc += 1;
            break;

        case 0x77:
            std::cout << "MOV M, A" << std::endl; 
            memory[hl] = a;
            pc += 1;
            break;

        case 0x78:
            std::cout << "MOV A, B" << std::endl; 
            a = *b;
            pc += 1;
            break;

        case 0x79:
            std::cout << "MOV A, C" << std::endl; 
            a = *c;
            pc += 1;
            break;

        case 0x7a:
            std::cout << "MOV A, D" << std::endl; 
            a = *d;
            pc += 1;
            break;

        case 0x7b:
            std::cout << "MOV A, E" << std::endl; 
            a = *e;
            pc += 1;
            break;

        case 0x7c:
            std::cout << "MOV A, H" << std::endl; 
            a = *h;
            pc += 1;
            break;

        case 0x7d:
            std::cout << "MOV A, L" << std::endl; 
            a = *l;
            pc += 1;
            break;

        case 0x7e:
            std::cout << "MOV A, M" << std::endl; 
            a = memory[hl];
            pc += 1;
            break;

        case 0x7f:
            std::cout << "MOV A, A" << std::endl; 
            a = a;
            pc += 1;
            break;



        // -------------------------8x------------------------- //



        case 0x80:
            std::cout << "ADD B" << std::endl;
            set_flags_add(a, *b, 1);
            a = a + *b;
            pc += 1;
            break;

        case 0x81:
            std::cout << "ADD C" << std::endl;
            set_flags_add(a, *c, 1);
            a = a + *c;
            pc += 1;
            break;
            
        case 0x82:
            std::cout << "ADD D" << std::endl;
            set_flags_add(a, *d, 1);
            a = a + *d;
            pc += 1;
            break;
            
        case 0x83:
            std::cout << "ADD E" << std::endl;
            set_flags_add(a, *e, 1);
            a = a + *e;
            pc += 1;
            break;
            
        case 0x84:
            std::cout << "ADD H" << std::endl;
            set_flags_add(a, *h, 1);
            a = a + *h;
            pc += 1;
            break;
            
        case 0x85:
            std::cout << "ADD L" << std::endl;
            set_flags_add(a, *l, 1);
            a = a + *l;
            pc += 1;
            break;
            
        case 0x86:
            std::cout << "ADD M" << std::endl;
            set_flags_add(a, memory[hl], 1);
            a = a + memory[hl];
            pc += 1;
            break;
            
        case 0x87:
            std::cout << "ADD A" << std::endl;
            set_flags_add(a, a, 1);
            a = a + a;
            pc += 1;
            break;
            
        case 0x88:
            std::cout << "ADC B" << std::endl;
            set_flags_add(a, *b + flag_c, 1);
            a = a + *b + flag_c;
            pc += 1;
            break;
            
        case 0x89:
            std::cout << "ADC C" << std::endl;
            set_flags_add(a, *c + flag_c, 1);
            a = a + *c + flag_c;
            pc += 1;
            break;
            
        case 0x8a:
            std::cout << "ADC D" << std::endl;
            set_flags_add(a, *d + flag_c, 1);
            a = a + *d + flag_c;
            pc += 1;
            break;
            
        case 0x8b:
            std::cout << "ADC E" << std::endl;
            set_flags_add(a, *e + flag_c, 1);
            a = a + *e + flag_c;
            pc += 1;
            break;
            
        case 0x8c:
            std::cout << "ADC H" << std::endl;
            set_flags_add(a, *h + flag_c, 1);
            a = a + *h + flag_c;
            pc += 1;
            break;

        case 0x8d:
            std::cout << "ADC L" << std::endl;
            set_flags_add(a, *b + flag_c, 1);
            a = a + *l + flag_c;
            pc += 1;
            break;
            
        case 0x8e:
            std::cout << "ADC M" << std::endl;
            set_flags_add(a, memory[hl] + flag_c, 1);
            a = a + memory[hl] + flag_c;
            pc += 1;
            break;
            
        case 0x8f:
            std::cout << "ADC A" << std::endl;
            set_flags_add(a, a + flag_c, 1);
            a = a + a + flag_c;
            pc += 1;
            break;


        // -------------------------9x------------------------- //

        case 0x90:
            std::cout << "SUB B" << std::endl;
            set_flags_sub(a, *b, 1);
            a = a - *b;
            pc += 1;
            break;

        case 0x91:
            std::cout << "SUB C" << std::endl;
            set_flags_sub(a, *c, 1);
            a = a - *c;
            pc += 1;
            break;
            
        case 0x92:
            std::cout << "SUB D" << std::endl;
            set_flags_sub(a, *d, 1);
            a = a - *d;
            pc += 1;
            break;
            
        case 0x93:
            std::cout << "SUB E" << std::endl;
            set_flags_sub(a, *e, 1);
            a = a - *e;
            pc += 1;
            break;
            
        case 0x94:
            std::cout << "SUB H" << std::endl;
            set_flags_sub(a, *h, 1);
            a = a - *h;
            pc += 1;
            break;
            
        case 0x95:
            std::cout << "SUB L" << std::endl;
            set_flags_sub(a, *l, 1);
            a = a - *l;
            pc += 1;
            break;
            
        case 0x96:
            std::cout << "SUB M" << std::endl;
            set_flags_sub(a, memory[hl], 1);
            a = a - memory[hl];
            pc += 1;
            break;
            
        case 0x97:
            std::cout << "SUB A" << std::endl;
            set_flags_sub(a, a, 1);
            a = a - a;
            pc += 1;
            break;
            
        case 0x98:
            std::cout << "SBB B" << std::endl;
            set_flags_sub(a, *b + flag_c, 1);
            a = a - (*b + flag_c);
            pc += 1;
            break;
            
        case 0x99:
            std::cout << "SBB C" << std::endl;
            set_flags_sub(a, *c + flag_c, 1);
            a = a - (*c + flag_c);// a = a + c + cy
            pc += 1;
            break;
            
        case 0x9a:
            std::cout << "SBB D" << std::endl;
            set_flags_sub(a, *d + flag_c, 1);
            a = a - (*d + flag_c);
            pc += 1;
            break;
            
        case 0x9b:
            std::cout << "SBB E" << std::endl;
            set_flags_sub(a, *e + flag_c, 1);
            a = a - (*e + flag_c);
            pc += 1;
            break;
            
        case 0x9c:
            std::cout << "SBB H" << std::endl;
            set_flags_sub(a, *h + flag_c, 1);
            a = a - (*h + flag_c);
            pc += 1;
            break;

        case 0x9d:
            std::cout << "SBB L" << std::endl;
            set_flags_sub(a, *l + flag_c, 1);
            a = a - (*l + flag_c);// a = a + l + cy
            pc += 1;
            break;
            
        case 0x9e:
            std::cout << "SBB M" << std::endl;
            set_flags_sub(a, memory[hl] + flag_c, 1);
            a = a - (memory[hl] + flag_c);
            pc += 1;
            break;
            
        case 0x9f:
            std::cout << "SBB A" << std::endl;
            set_flags_sub(a, a + flag_c, 1);
            a = a - (a + flag_c);
            pc += 1;
            break;


        // -------------------------ax------------------------- //


        case 0xa0:
            std::cout << "ANA B" << std::endl;
            flag_c = 0;
            flag_p = get_parity(a & *b);
            flag_z  = ((a & *b) == 0);
            flag_s  = (((a & *b) & 0x80) != 0);
            a = a & *b;
            break;    

        case 0xa1:
            std::cout << "ANA C" << std::endl;
            flag_c = 0;
            flag_p = get_parity(a & *c);
            flag_z  = ((a & *c) == 0);
            flag_s  = (((a & *c) & 0x80) != 0);
            a = a & *c;
            break;   

        case 0xa2:
            std::cout << "ANA D" << std::endl;
            flag_c = 0;
            flag_p = get_parity(a & *d);
            flag_z  = ((a & *d) == 0);
            flag_s  = (((a & *d) & 0x80) != 0);
            a = a & *d;
            break;  

        case 0xa3:
            std::cout << "ANA E" << std::endl;
            flag_c = 0;
            flag_p = get_parity(a & *e);
            flag_z  = ((a & *e) == 0);
            flag_s  = (((a & *e) & 0x80) != 0);
            a = a & *e;
            break;    

        case 0xa4:
            std::cout << "ANA H" << std::endl;
            flag_c = 0;
            flag_p = get_parity(a & *h);
            flag_z  = ((a & *h) == 0);
            flag_s  = (((a & *h) & 0x80) != 0);
            a = a & *h;
            break;   

        case 0xa5:
            std::cout << "ANA L" << std::endl;
            flag_c = 0;
            flag_p = get_parity(a & *l);
            flag_z  = ((a & *l) == 0);
            flag_s  = (((a & *l) & 0x80) != 0);
            a = a & *l;
            break;  

        case 0xa6:
            std::cout << "ANA M" << std::endl;
            flag_c = 0;
            flag_p = get_parity(a & memory[hl]);
            flag_z  = ((a & memory[hl]) == 0);
            flag_s  = (((a & memory[hl]) & 0x80) != 0);
            a = a & memory[hl];
            break;    

        case 0xa7:
            std::cout << "ANA A" << std::endl;
            flag_c = 0;
            flag_p = get_parity(a & a);
            flag_z  = ((a & a) == 0);
            flag_s  = (((a & a) & 0x80) != 0);
            a = a & a;
            break;   

        case 0xa8:
            std::cout << "XRA B" << std::endl;
            flag_c = 0;
            flag_p = get_parity((a ^ *b));
            flag_z  = ((a ^ *b) == 0);
            flag_s  = (((a ^ *b) & 0x80) != 0);
            a = a ^ *b;
            break;  

        case 0xa9:
            std::cout << "XRA C" << std::endl;
            flag_c = 0;
            flag_p = get_parity((a ^ *c));
            flag_z  = ((a ^ *c) == 0);
            flag_s  = (((a ^ *c) & 0x80) != 0);
            a = a ^ *c;
            break;    

        case 0xaa:
            std::cout << "XRA D" << std::endl;
            flag_c = 0;
            flag_p = get_parity((a ^ *d));
            flag_z  = ((a ^ *d) == 0);
            flag_s  = (((a ^ *d) & 0x80) != 0);
            a = a ^ *d;
            break;   

        case 0xab:
            std::cout << "XRA E" << std::endl;
            flag_c = 0;
            flag_p = get_parity((a ^ *e));
            flag_z  = ((a ^ *e) == 0);
            flag_s  = (((a ^ *e) & 0x80) != 0);
            a = a ^ *e;
            break;  

        case 0xac:
            std::cout << "XRA H" << std::endl;
            flag_c = 0;
            flag_p = get_parity((a ^ *h));
            flag_z  = ((a ^ *h) == 0);
            flag_s  = (((a ^ *h) & 0x80) != 0);
            a = a ^ *h;
            break;  

        case 0xad:
            std::cout << "XRA L" << std::endl;
            flag_c = 0;
            flag_p = get_parity((a ^ *l));
            flag_z  = ((a ^ *l) == 0);
            flag_s  = (((a ^ *l) & 0x80) != 0);
            a = a ^ *l;
            break;   

        case 0xae:
            std::cout << "XRA M" << std::endl;
            flag_c = 0;
            flag_p = get_parity((a ^ memory[hl]));
            flag_z  = ((a ^ memory[hl]) == 0);
            flag_s  = (((a ^ memory[hl]) & 0x80) != 0);
            a = a ^ memory[hl];
            break;    

         case 0xaf:
            std::cout << "XRA A" << std::endl;
            flag_c = 0;
            flag_p = get_parity(0);
            flag_z  = 0;
            flag_s  = 0;
            a = 0;
            break;             
        // -------------------------bx------------------------- //


        case 0xb0:
            std::cout << "ORA B" << std::endl;
            flag_c = 0;
            flag_p = get_parity(a | *b);
            flag_z  = ((a | *b) == 0);
            flag_s  = (((a | *b) & 0x80) != 0);
            a = a | *b;
            break;    

        case 0xb1:
            std::cout << "ORA C" << std::endl;
            flag_c = 0;
            flag_p = get_parity(a | *c);
            flag_z  = ((a | *c) == 0);
            flag_s  = (((a | *c) & 0x80) != 0);
            a = a | *c;
            break;   

        case 0xb2:
            std::cout << "ORA D" << std::endl;
            flag_c = 0;
            flag_p = get_parity(a | *d);
            flag_z  = ((a | *d) == 0);
            flag_s  = (((a | *d) & 0x80) != 0);
            a = a | *d;
            break;  

        case 0xb3:
            std::cout << "ORA E" << std::endl;
            flag_c = 0;
            flag_p = get_parity(a | *e);
            flag_z  = ((a | *e) == 0);
            flag_s  = (((a | *e) & 0x80) != 0);
            a = a | *e;
            break;    

        case 0xb4:
            std::cout << "ORA H" << std::endl;
            flag_c = 0;
            flag_p = get_parity(a | *h);
            flag_z  = ((a | *h) == 0);
            flag_s  = (((a | *h) & 0x80) != 0);
            a = a | *h;
            break;   

        case 0xb5:
            std::cout << "ORA L" << std::endl;
            flag_c = 0;
            flag_p = get_parity(a | *l);
            flag_z  = ((a | *l) == 0);
            flag_s  = (((a | *l) & 0x80) != 0);
            a = a | *l;
            break;  

        case 0xb6:
            std::cout << "ORA M" << std::endl;
            flag_c = 0;
            flag_p = get_parity(a | memory[hl]);
            flag_z  = ((a | memory[hl]) == 0);
            flag_s  = (((a | memory[hl]) & 0x80) != 0);
            a = a | memory[hl];
            break;    

        case 0xb7:
            std::cout << "ORA A" << std::endl;
            flag_c = 0;
            flag_p = get_parity(a | a);
            flag_z  = ((a | a) == 0);
            flag_s  = (((a | a) & 0x80) != 0);
            a = a | a;
            break;   

        case 0xb8:
            std::cout << "CMP B" << std::endl;
            set_flags_sub(a, *b, 1);
            break;  

        case 0xb9:
            std::cout << "CMP C" << std::endl;
            set_flags_sub(a, *c, 1);    
            break;    

        case 0xba:
            std::cout << "CMP D" << std::endl;
            set_flags_sub(a, *d, 1);
            break;   

        case 0xbb:
            std::cout << "CMP E" << std::endl;
            set_flags_sub(a, *e, 1);
            break;  

        case 0xbc:
            std::cout << "CMP H" << std::endl;
            set_flags_sub(a, *h, 1);
            break;  

        case 0xbd:
            std::cout << "CMP L" << std::endl;
            set_flags_sub(a, *l, 1);
            break;   

        case 0xbe:
            std::cout << "CMP M" << std::endl;
            set_flags_sub(a, memory[hl], 1);
            break;    

         case 0xbf:
            std::cout << "CMP A" << std::endl;
            set_flags_sub(a, a, 1);
            break;     


        // -------------------------cx------------------------- //


        case 0xc3:
            std::cout << "JMP adr";
            std::cout << "~ is: 0x" << std::hex << (memory[pc + 1] | (memory[pc + 2] << 8)) << std::dec << std::endl;
            pc = (memory[pc + 1] | (memory[pc + 2] << 8));
            break;

        case 0xcd:
            std::cout << "CALL adr~ 0x" << std::hex << (memory[pc + 1] | (memory[pc + 2] << 8)) << std::dec << std::endl;
            // Implement stack
            pc = (memory[pc + 1] | (memory[pc + 2] << 8));

        // -------------------------dx------------------------- //


        // -------------------------ex------------------------- //
        

        // -------------------------fx------------------------- //

        // End of switch    
        break;  


    }

}
#include <iostream>
#include "cpu.hpp"
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif


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

    pc = 0x100;
    sp = 0;
    local = 0;
    memory[0x5] = 0xd3;
    memory[0x6] = 0x01;
    memory[0x7] = 0xc9;
    int_enable = 0;
    active = 1;
    set_psw();

}

void CPU::set_psw() {
    psw = ((uint16_t) a) << 8;
    psw |= flag_s << 7;
    psw |= flag_z << 6;
    psw |= 0 << 5;
    psw |= flag_hc << 4;
    psw |= 0 << 3;
    psw |= flag_p << 2;
    psw |= 1 << 1;
    psw |= flag_c;
}

void CPU::stack_push(uint16_t val) {
    sp -= 2;
    memory[sp] = (val & 0xff);
    memory[sp + 1] = (val >> 8);

}

uint16_t CPU::stack_pop() {
    uint16_t val = memory[sp] | (memory[sp + 1] << 8);
    sp += 2;
    return val;
}

void CPU::cpm_print() {
    // REG_C - syscall number
    // REG_DE - parameters of syscall
    if (*c == 9) {  
        uint8_t *str = (&memory[de]);
        while (*str != '$') {
            std::cout << std::dec << char(*str);
            *str++;
        }
        std::cout << "\n";
    }
    else if (*c == 2) {
        std::cout << *c;
    }
    
}

bool CPU::get_parity(uint8_t n){
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

void CPU::set_flags_add(uint8_t op1, uint8_t op2, bool change_carry) {
    uint16_t answer = op1 + op2;
    // Sign flag
    flag_s = ((answer & 0x80) != 0);
    // Zero flag
    flag_z = ((answer & 0xff) == 0);
    // Parity flag
    flag_p = get_parity(uint8_t(answer));
    // Carry flag
    if (change_carry) flag_c = get_carry(op1, op2);
    // Half carry flag
    flag_hc = ((((op1 & 0xF) + (op2 & 0xF)) & 0x10) == 0x10);
}

void CPU::set_flags_sub(uint8_t op1, uint8_t op2, bool change_carry) {
    uint16_t answer = op1 - op2;
    // Sign flag
    flag_s = ((answer & 0x80) != 0);
    // Zero flag
    flag_z = ((answer & 0xff) == 0);
    // Parity flag
    flag_p = get_parity(uint8_t(answer));
    // Carry flag
    if (change_carry) {
        int16_t answer = op1 - op2;
        int16_t carry = answer ^ op1 ^ op2;
        flag_c = (carry & (1 << 8)) != 0;
        }
    // Half carry flag
    flag_hc = (~(op1 ^ answer ^ op2) & 0x10) != 0;
}


void CPU::set_flags_bitwise(uint8_t op1, uint8_t op2, int operation) {
    // 0 - AND
    // 1 - OR
    // 2 - X0R
    flag_c = 0;
    switch (operation) {
        case 0: 
            flag_p = get_parity((op1 & op2));
            flag_z  = ((op1 & op2) == 0);
            flag_s  = (((op1 & op2) & 0x80) != 0);
            flag_hc = ((((op1 & 0xF) & (op2 & 0xF)) & 0x10) == 0x10);
            break;

        case 1: 
            flag_p = get_parity((op1 | op2));
            flag_z  = ((op1 | op2) == 0);
            flag_s  = (((op1 | op2) & 0x80) != 0);
            flag_hc = ((((op1 & 0xF) | (op2 & 0xF)) & 0x10) == 0x10);
            break;

        case 2: 
            flag_p = get_parity((op1 ^ op2));
            flag_z  = ((op1 ^ op2) == 0);
            flag_s  = (((op1 ^ op2) & 0x80) != 0);
            flag_hc = ((((op1 & 0xF) ^ (op2 & 0xF)) & 0x10) == 0x10);
            break;
    }
}

void CPU::debug() {
    std::cout << std::hex << "PC: " << pc << " ";
    std::cout << "AF: " << psw << " ";
    std::cout << "BC: " << int(bc) << " ";
    std::cout << "DE: " << int(de) << " ";
    std::cout << "HL: " << int(hl) << " ";
    std::cout << "SP: " << int(sp) << " ";
    std::cout << "(" << int(memory[pc]) << " " << int(memory[pc + 1]) << " "<< int(memory[pc + 2]) << " " << int(memory[pc + 3]) << ")";
    std::cout << std::dec << "\n";
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
        memory[0x100 + i] = buffer[i];
    }
    fclose(rom);
    free(buffer);
}


void CPU::execute() {
    opcode = memory[pc];
    //debug();
    if(pc == 5){
        cpm_print();
    }
    switch (opcode) {

        // -------------------------0x-------------------------  //

        case 0x00:
            pc += 1;
            active = 0;
            break;

        case 0x01:
            bc = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            break; 

        case 0x02:
            memory[bc] = a;
            pc += 1;
            break;

        case 0x03:
            bc += 1;
            pc += 1;
            break;

        case 0x04:
            set_flags_add(*b, 1, 0);
            *b += 1;
            pc += 1;
            break;

        case 0x05:
            set_flags_sub(*b, 1, 0);
            *b -= 1;
            pc += 1;
            break;

        case 0x06:
            *b = memory[pc + 1];
            pc += 2;
            break;

        case 0x07:
            {
            uint8_t temp = (a & 0x80) >> 7;
            a = a << 1;
            a = a | temp;
            flag_c = temp;
            pc += 1;
            }
            break;

        case 0x08:
            pc += 1;
            break;

        case 0x09:
            flag_c = (hl + bc) > 0xffff;
            hl = hl + bc;
            pc += 1;
            break;

        case 0x0a:
            a = memory[bc];
            pc += 1;
            break;

        case 0x0b:
            bc -= 1 ;
            pc += 1;
            break;

        case 0x0c:
            set_flags_add(*c, 1, 0);
            *c += 1;
            pc += 1;
            break;

        case 0x0d:
            set_flags_sub(*c, 1, 0);
            *c -= 1;
            pc += 1;
            break;

        case 0x0e: 
            *c = memory[pc + 1];
            pc += 2;
            break;

        case 0x0f:
            {
            flag_c = a & 1;
            uint8_t temp = (a & 1);
            a = a >> 1;
            a = a | (temp << 7);
            pc += 1;
            }
            break;

        // -------------------------1x------------------------- //

        case 0x10:
            pc += 1;
            break;

        case 0x11:
            de = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            break;

        case 0x12:
            memory[de] = a;
            pc += 1;
            break;

        case 0x13:
            de += 1;
            pc += 1;
            break;

        case 0x14:
            set_flags_add(*d, 1, 0);
            *d += 1;
            pc += 1;
            break;

        case 0x15:
            set_flags_sub(*d, 1, 0);
            *d -= 1;
            pc += 1;
            break;

        case 0x16:
            *d = memory[pc + 1];
            pc += 2;
            break;

        case 0x17:
            {
            uint8_t temp = (a & 0x80) >> 7;
            a = a << 1;
            a = a | flag_c;
            flag_c = temp;
            pc += 1;
            }
            break;

        case 0x18:
            pc += 1;
            break;

        case 0x19:
            flag_c = (hl + de) > 0xffff;
            hl = hl + de;
            pc += 1;
            break;

        case 0x1a:
            a = memory[de];
            pc += 1;
            break;

        case 0x1b:
            de -= 1;
            pc += 1;
            break;

        case 0x1c:
            set_flags_add(*e, 1, 0);
            *e += 1;
            pc += 1;
            break;

        case 0x1d:
            set_flags_sub(*e, 1, 0);
            *e -= 1;
            pc += 1;
            break;

        case 0x1e:
            *e = memory[pc + 1];
            pc += 2;
            break;

        case 0x1f:
            {
            uint8_t temp = a & 1;
            a = a >> 1;
            a = a | (flag_c << 7);
            flag_c = temp;
            pc += 1;
            }
            break;

        // -------------------------2x------------------------- //
        
        case 0x20:
            pc += 1;
            break;

        case 0x21:
            hl = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            break;

        case 0x22:   
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            memory[local] = *l;
            memory[local + 1] = *h;
            pc += 3;
            break;

        case 0x23:
            hl += 1;
            pc += 1;
            break;

        case 0x24:
            set_flags_add(*h, 1, 0);
            *h += 1;
            pc += 1;
            break;

        case 0x25:
            set_flags_sub(*h, 1, 0);
            *h -= 1;
            pc += 1;
            break;

        case 0x26:
            *h = memory[pc + 1];
            pc += 2;
            break;

        case 0x27: // DAA
            local = 0;
            if ((a & 0xF) > 9 || flag_hc) {
                local += 0x06;
            }
            if (((a >> 4) >= 9 && (a & 0xF) > 9) || (a >> 4) > 9 || flag_c) {
                local += 0x60;
                flag_c = 1;
            }
            set_flags_add(a, local, 1);
            a += local;
            pc += 1;
            break;

        case 0x28:
            pc += 1;
            break;

        case 0x29:
            flag_c = (hl + hl) > 0xffff; 
            hl += hl;
            pc += 1;
            break;

        case 0x2a:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            hl = memory[local] | (memory[local + 1] << 8);
            pc += 3;
            break;

        case 0x2b:
            hl -= 1;
            pc += 1;
            break;

        case 0x2c:
            set_flags_add(*l, 1, 0);
            *l += 1;
            pc += 1;
            break;

        case 0x2d:
            set_flags_sub(*l, 1, 0);
            *l -= 1;
            pc += 1;
            break;

        case 0x2e:
            *l = memory[pc + 1];
            pc += 2;
            break;

        case 0x2f:
            a = ~a;
            pc += 1;
            break;

        // -------------------------3x------------------------- //

        case 0x30:
            pc += 1;
            break;

        case 0x31:
            sp = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            break;
            
        case 0x32:
            local = memory[pc + 1] | memory[pc + 2] << 8;
            memory[local] = a;
            pc += 3;
            break;

        case 0x33:
            sp += 1;
            pc += 1;
            break;
            
        case 0x34:
            set_flags_add(memory[hl], 1, 0);
            memory[hl] += 1;
            pc += 1;
            break;
            
        case 0x35:
            set_flags_sub(memory[hl], 1, 0);
            memory[hl] -= 1;
            pc += 1;
            break;
            
        case 0x36:
            memory[hl] = memory[pc + 1];
            pc += 2;
            break;
            
        case 0x37:
            flag_c = 1;
            pc += 1;
            break;
            
        case 0x38:
            pc += 1;
            break;

        case 0x39:
            flag_c = (hl + sp) > 0xffff;
            hl = hl + sp;
            pc += 1;    
            break;
            
        case 0x3a:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            a = memory[local];
            pc += 3;
            break;

        case 0x3b:
            sp -= 1;
            pc += 1;
            break;
            
        case 0x3c:
            set_flags_add(a, 1, 0);
            a += 1;
            pc += 1;
            break;
            
        case 0x3d:
            set_flags_sub(a, 1, 0);
            a -= 1;
            pc += 1;
            break;
            
        case 0x3e:
            a = memory[pc + 1];
            pc += 2;
            break;
            
        case 0x3f:
            flag_c = !flag_c;
            pc += 1;
            break;

        // -------------------------4x------------------------- //

        case 0x40:
            *b = *b;
            pc += 1;
            break;

        case 0x41:
            *b = *c;
            pc += 1;
            break;

        case 0x42:
            *b = *d;
            pc += 1;
            break;

        case 0x43:
            *b = *e;
            pc += 1;
            break;

        case 0x44:
            *b = *h;
            pc += 1;
            break;

        case 0x45:
            *b = *l;
            pc += 1;
            break;

        case 0x46:
            *b = memory[hl];
            pc += 1;
            break;

        case 0x47: 
            *b = a;
            pc += 1;
            break;

        case 0x48:
            *c = *b;
            pc += 1;
            break;

        case 0x49:
            *c = *c;
            pc += 1;
            break;

        case 0x4a:
            *c = *d;
            pc += 1;
            break;

        case 0x4b:
            *c = *e;
            pc += 1;
            break;

        case 0x4c:
            *c = *h;
            pc += 1;
            break;

        case 0x4d:
            *c = *l;
            pc += 1;
            break;

        case 0x4e:
            *c = memory[hl];
            pc += 1;
            break;

        case 0x4f:
            *c = a;
            pc += 1;
            break;

        // -------------------------5x------------------------- //

        case 0x50:
            *d = *b;
            pc += 1;
            break;

        case 0x51:
            *d = *c;
            pc += 1;
            break;

        case 0x52:
            *d = *d;
            pc += 1;
            break;

        case 0x53:
            *d = *e;
            pc += 1;
            break;

        case 0x54:
            *d = *h;
            pc += 1;
            break;

        case 0x55:
            *d = *l;
            pc += 1;
            break;

        case 0x56:
            *d = memory[hl];
            pc += 1;
            break;

        case 0x57:
            *d = a;
            pc += 1;
            break;

        case 0x58:
            *e = *b;
            pc += 1;
            break;

        case 0x59:
            *e = *c;
            pc += 1;
            break;

        case 0x5a:
            *e = *d;
            pc += 1;
            break;

        case 0x5b:
            *e = *e;
            pc += 1;
            break;

        case 0x5c:
            *e = *h;
            pc += 1;
            break;

        case 0x5d:
            *e = *l;
            pc += 1;
            break;

        case 0x5e:
            *e = memory[hl];
            pc += 1;
            break;

        case 0x5f:
            *e = a;
            pc += 1;
            break;

        // -------------------------6x------------------------- //
        
        case 0x60:
            *h = *b;
            pc += 1;
            break;
            
        case 0x61:
            *h = *c;
            pc += 1;
            break;

        case 0x62:
            *h = *d;
            pc += 1;
            break;

        case 0x63:
            *h = *e;
            pc += 1;
            break;

        case 0x64:
            *h = *h;
            pc += 1;
            break;

        case 0x65:
            *h = *l;
            pc += 1;
            break;

        case 0x66:
            *h = memory[hl];
            pc += 1;
            break;

        case 0x67:
            *h = a;
            pc += 1;
            break;

        case 0x68:
            *l = *b;
            pc += 1;
            break;

        case 0x69:
            *l = *c;
            pc += 1;
            break;

        case 0x6a:
            *l = *d;
            pc += 1;
            break;

        case 0x6b:
            *l = *e;
            pc += 1;
            break;

        case 0x6c:
            *l = *h;
            pc += 1;
            break;

        case 0x6d:
            *l = *l;
            pc += 1;
            break;

        case 0x6e:
            *l = memory[hl];
            pc += 1;
            break;

        case 0x6f:
            *l = a;
            pc += 1;
            break;

        // -------------------------7x------------------------- //

        case 0x70:
            memory[hl] = *b;
            pc += 1;
            break;
            
        case 0x71:
            memory[hl] = *c;
            pc += 1;
            break;

        case 0x72:
            memory[hl] = *d;
            pc += 1;
            break;

        case 0x73:
            memory[hl] = *e;
            pc += 1;
            break;

        case 0x74:
            memory[hl] = *h;
            pc += 1;
            break;

        case 0x75:
            memory[hl] = *l;
            pc += 1;
            break;

        case 0x76: // HLT
            exit(0);
            pc += 1;
            break;

        case 0x77:
            memory[hl] = a;
            pc += 1;
            break;

        case 0x78:
            a = *b;
            pc += 1;
            break;

        case 0x79:
            a = *c;
            pc += 1;
            break;

        case 0x7a:
            a = *d;
            pc += 1;
            break;

        case 0x7b: 
            a = *e;
            pc += 1;
            break;

        case 0x7c:
            a = *h;
            pc += 1;
            break;

        case 0x7d:
            a = *l;
            pc += 1;
            break;

        case 0x7e:
            a = memory[hl];
            pc += 1;
            break;

        case 0x7f:
            a = a;
            pc += 1;
            break;

        // -------------------------8x------------------------- //

        case 0x80:
            set_flags_add(a, *b, 1);
            a = a + *b;
            pc += 1;
            break;

        case 0x81:
            set_flags_add(a, *c, 1);
            a = a + *c;
            pc += 1;
            break;
            
        case 0x82:
            set_flags_add(a, *d, 1);
            a = a + *d;
            pc += 1;
            break;
            
        case 0x83:
            set_flags_add(a, *e, 1);
            a = a + *e;
            pc += 1;
            break;
            
        case 0x84:
            set_flags_add(a, *h, 1);
            a = a + *h;
            pc += 1;
            break;
            
        case 0x85:
            set_flags_add(a, *l, 1);
            a = a + *l;
            pc += 1;
            break;
            
        case 0x86:
            set_flags_add(a, memory[hl], 1);
            a = a + memory[hl];
            pc += 1;
            break;
            
        case 0x87:
            set_flags_add(a, a, 1);
            a = a + a;
            pc += 1;
            break;
            
        case 0x88:
            local = *b + flag_c;
            set_flags_add(a, local, 1);
            a = a + local;
            pc += 1;
            break;
            
        case 0x89:
            local = *c + flag_c;
            set_flags_add(a, local, 1);
            a = a + local;
            pc += 1;
            break;
            
        case 0x8a:
            local = *d + flag_c;
            set_flags_add(a, local, 1);
            a = a + local;
            pc += 1;
            break;
            
        case 0x8b:
            local = *e + flag_c;
            set_flags_add(a, local, 1);
            a = a + local;
            pc += 1;
            break;
            
        case 0x8c:
            local = *h + flag_c;
            set_flags_add(a, local, 1);
            a = a + local;
            pc += 1;
            break;

        case 0x8d:
            local = *l + flag_c;
            set_flags_add(a, local, 1);
            a = a + local;
            pc += 1;
            break;
            
        case 0x8e:
            local = memory[hl] + flag_c;
            set_flags_add(a, local, 1);
            a = a + local;
            pc += 1;
            break;
            
        case 0x8f:
            local = a + flag_c;
            set_flags_add(a, local, 1);
            a = a + local;
            pc += 1;
            break;

        // -------------------------9x------------------------- //

        case 0x90:
            set_flags_sub(a, *b, 1);
            a = a - *b;
            pc += 1;
            break;

        case 0x91:
            set_flags_sub(a, *c, 1);
            a = a - *c;
            pc += 1;
            break;
            
        case 0x92:
            set_flags_sub(a, *d, 1);
            a = a - *d;
            pc += 1;
            break;
            
        case 0x93:
            set_flags_sub(a, *e, 1);
            a = a - *e;
            pc += 1;
            break;
            
        case 0x94:
            set_flags_sub(a, *h, 1);
            a = a - *h;
            pc += 1;
            break;
            
        case 0x95:
            set_flags_sub(a, *l, 1);
            a = a - *l;
            pc += 1;
            break;
            
        case 0x96:
            set_flags_sub(a, memory[hl], 1);
            a = a - memory[hl];
            pc += 1;
            break;
            
        case 0x97:
            set_flags_sub(a, a, 1);
            a = a - a;
            pc += 1;
            break;
            
        case 0x98:
            local = *b + flag_c;
            set_flags_sub(a, local, 1);
            a = a - local;
            pc += 1;
            break;
            
        case 0x99:
            local = *c + flag_c;
            set_flags_sub(a, local, 1);
            a = a - local;
            pc += 1;
            break;
            
        case 0x9a:
            local = *d + flag_c;
            set_flags_sub(a, local, 1);
            a = a - local;
            pc += 1;
            break;
            
        case 0x9b:
            local = *e + flag_c;
            set_flags_sub(a, local, 1);
            a = a - local;
            pc += 1;
            break;
            
        case 0x9c:
            local = *h + flag_c;
            set_flags_sub(a, local, 1);
            a = a - local;
            pc += 1;
            break;

        case 0x9d:
            local = *l + flag_c;
            set_flags_sub(a, local, 1);
            a = a - local;
            pc += 1;
            break;
            
        case 0x9e:
            local = memory[hl] + flag_c;
            set_flags_sub(a, local, 1);
            a = a - local;
            pc += 1;
            break;
            
        case 0x9f:
            local = a + flag_c;
            set_flags_sub(a, local, 1);
            a = a - local;
            pc += 1;
            break;

        // -------------------------ax------------------------- //

        case 0xa0:
            set_flags_bitwise(a, *b, 0);
            a = a & *b;
            pc += 1;
            break;    

        case 0xa1:
            set_flags_bitwise(a, *c, 0);
            a = a & *c;
            pc += 1;
            break;   

        case 0xa2:
            set_flags_bitwise(a, *d, 0);
            a = a & *d;
            pc += 1;
            break;  

        case 0xa3:
            set_flags_bitwise(a, *e, 0);
            a = a & *e;
            pc += 1;
            break;    

        case 0xa4:
            set_flags_bitwise(a, *h, 0);
            a = a & *h;
            pc += 1;
            break;   

        case 0xa5:
            set_flags_bitwise(a, *l, 0);
            a = a & *l;
            pc += 1;
            break;  

        case 0xa6:
            set_flags_bitwise(a, memory[hl], 0);
            a = a & memory[hl];
            pc += 1;
            break;    

        case 0xa7:
            set_flags_bitwise(a, a, 0);
            a = a & a;
            pc += 1;
            break;   

        case 0xa8:
            set_flags_bitwise(a, *b, 2);
            a = a ^ *b;
            pc += 1;
            break;  

        case 0xa9:
            set_flags_bitwise(a, *c, 2);
            a = a ^ *c;
            pc += 1;
            break;    

        case 0xaa:
            set_flags_bitwise(a, *d, 2);
            a = a ^ *d;
            pc += 1;
            break;   

        case 0xab:
            set_flags_bitwise(a, *e, 2);
            a = a ^ *e;
            pc += 1;
            break;  

        case 0xac:
            set_flags_bitwise(a, *h, 2);
            a = a ^ *h;
            pc += 1;
            break;  

        case 0xad:
            set_flags_bitwise(a, *l, 2);
            a = a ^ *l;
            pc += 1;
            break;   

        case 0xae:
            set_flags_bitwise(a, memory[hl], 2);
            a = a ^ memory[hl];
            pc += 1;
            break;    

         case 0xaf:
            set_flags_bitwise(a, a, 2);
            a = a ^ a;
            pc += 1;
            break;           

        // -------------------------bx------------------------- //

        case 0xb0:
            set_flags_bitwise(a, *b, 1);
            a = a | *b;
            pc += 1;
            break;    

        case 0xb1:
            set_flags_bitwise(a, *c, 1);
            a = a | *c;
            pc += 1;
            break;   

        case 0xb2:
            set_flags_bitwise(a, *d, 1);
            a = a | *d;
            pc += 1;
            break;  

        case 0xb3:
            set_flags_bitwise(a, *e, 1);
            a = a | *e;
            pc += 1;
            break;    

        case 0xb4:
            set_flags_bitwise(a, *h, 1);
            a = a | *h;
            pc += 1;
            break;   

        case 0xb5:
            set_flags_bitwise(a, *l, 1);
            a = a | *l;
            pc += 1;
            break;  

        case 0xb6:
            set_flags_bitwise(a, memory[hl], 1);
            a = a | memory[hl];
            pc += 1;
            break;    

        case 0xb7:
            set_flags_bitwise(a, a, 1);
            a = a | a;
            pc += 1;
            break;   

        case 0xb8:
            set_flags_sub(a, *b, 1);
            pc += 1;
            break;  

        case 0xb9:
            set_flags_sub(a, *c, 1);  
            pc += 1;
            break;    

        case 0xba:
            set_flags_sub(a, *d, 1);
            pc += 1;
            break;   

        case 0xbb:
            set_flags_sub(a, *e, 1);
            pc += 1;
            break;  

        case 0xbc:
            set_flags_sub(a, *h, 1);
            pc += 1;
            break;  

        case 0xbd:
            set_flags_sub(a, *l, 1);
            pc += 1;
            break;   

        case 0xbe:
            set_flags_sub(a, memory[hl], 1);
            pc += 1;
            break;    

         case 0xbf:
            set_flags_sub(a, a, 1);
            pc += 1;
            break;     

        // -------------------------cx------------------------- //

        case 0xc0:
            pc += 1;
            if (!flag_z) {
                pc = stack_pop();
            }
            break;

        case 0xc1:
            bc = stack_pop();
            pc += 1;
            break;

        case 0xc2:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            if (!flag_z) {
                pc = local; 
            }
            break;

        case 0xc3:
            pc = memory[pc + 1] | (memory[pc + 2] << 8);
            break;

        case 0xc4:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            if (!flag_z) {
                stack_push(pc);
                pc = local;
            }
            break;

        case 0xc5:
            stack_push(bc);
            pc += 1;
            break;

        case 0xc6:
            set_flags_add(a, memory[pc + 1], 1);
            a = a + memory[pc + 1];
            pc += 2;
            break;

        case 0xc7:
            pc += 1;
            stack_push(pc);
            pc = 0x0000;
            break;

        case 0xc8:
            pc += 1;
            if (flag_z) {
                pc = stack_pop();
            }
            break;

        case 0xc9:
            pc = stack_pop();
            break;

        case 0xca:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            if (flag_z) {
                pc = local;
            }
            break;        

        case 0xcb:
            pc = memory[pc + 1] | (memory[pc + 2] << 8);
            break;

        case 0xcc:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            if (flag_z) {
                stack_push(pc);
                pc = local;
            }
            break;

        case 0xcd:
            {
            uint16_t addr = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            stack_push(pc);
            pc = addr;  
            }
            break;

        case 0xce:
            local = memory[pc + 1] + flag_c;
            set_flags_add(a, local, 1);
            a = a + local;
            pc += 2;
            break;  

        case 0xcf:
            pc += 1;
            stack_push(pc);
            pc = 0x0008;
            break;        

        // -------------------------dx------------------------- //

        case 0xd0:
            pc += 1;
            if (!flag_c) {
                pc = stack_pop();
            }
            break;

        case 0xd1:
            de = stack_pop();
            pc += 1;
            break;

        case 0xd2:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            if (!flag_c) {
                pc = local; 
            }
            break;

        case 0xd3: // OUT D8 TODO
            {
            uint8_t port = memory[pc + 1];
            if (port == 2) {
                shift_8 = a & 7;
            }
            else if (port == 4) {
                shift_16 >>= 8;
                shift_16 |= a << 8;
            }
            else {
                OutPort[port] = a;
            }
            pc += 2;
            break;
            }       

        case 0xd4:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            if (!flag_c) {
                stack_push(pc);
                pc = local;
            }
            break;

        case 0xd5:
            stack_push(de);
            pc += 1;
            break;

        case 0xd6:
            set_flags_sub(a, memory[pc + 1], 1);
            a = a - memory[pc + 1];
            pc += 2;
            break;

        case 0xd7:
            pc += 1;
            stack_push(pc);
            pc = 0x0010;
            break;

        case 0xd8:
            pc += 1;
            if (flag_c) {
                pc = stack_pop();
            }
            break;

        case 0xd9:
            pc = stack_pop();
            break;

        case 0xda:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            if (flag_c) {
                pc = local;
            }
            break;        

        case 0xdb: // IN D8 TODO
            std::cout << "I'm here" << std::endl;
            {
            uint8_t port = memory[pc + 1];
            if (port == 3) {
                a = shift_16 >> (8 - shift_8);
            }
            else {
                a = InPort[port];
            } 
            pc += 2;
            }
            break;

        case 0xdc:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            if (flag_c) {
                stack_push(pc);
                pc = local;
            }
            break;

        case 0xdd:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            stack_push(pc);
            pc = local;
            break;

        case 0xde:
            local = memory[pc + 1] + flag_c;
            set_flags_sub(a, local, 1);
            a = a - local;
            pc += 2;
            break;  

        case 0xdf:
            pc += 1;
            stack_push(pc);
            pc = 0x0018;
            break;        

        // -------------------------ex------------------------- //
        
        case 0xe0:
            pc += 1;
            if (!flag_p) {
                pc = stack_pop();
            }
            break;

        case 0xe1:
            hl = stack_pop();
            pc += 1;
            break;

        case 0xe2:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            if (!flag_p) {
                pc = local; 
            }
            break;

        case 0xe3:
            {
            uint8_t temp;
            temp = memory[sp];
            memory[sp] = *l;
            *l = temp;

            temp = memory[sp + 1];
            memory[sp + 1] = *h;
            *h = temp;
            pc += 1;
            }
            break;

        case 0xe4:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            if (!flag_p) {
                stack_push(pc);
                pc = local;
            }
            break;

        case 0xe5:
            stack_push(hl);
            pc += 1;
            break;

        case 0xe6:
            set_flags_bitwise(a, memory[pc + 1], 0);
            a = a & memory[pc + 1];
            pc += 2;
            break;

        case 0xe7:
            pc += 1;
            stack_push(pc);
            pc = 0x0020;
            break;

        case 0xe8:
            pc += 1;
            if (flag_p) {
                pc = stack_pop();
            }
            break;

        case 0xe9:
            pc = hl;
            break;

        case 0xea:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            if (flag_p) {
                pc = local;
            }
            break;        

        case 0xeb:
            local = hl;
            hl = de;
            de = local;
            pc += 1;
            break;   

        case 0xec:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            if (flag_p) {
                stack_push(pc);
                pc = local;
            }
            break;

        case 0xed:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            stack_push(pc);
            pc = local;
            break;

        case 0xee:
            set_flags_bitwise(a, memory[pc + 1], 2);
            a = a ^ memory[pc + 1];
            pc += 2;
            break;  

        case 0xef:
            pc += 1;
            stack_push(pc);
            pc = 0x0028;
            break;     

        // -------------------------fx------------------------- //

        case 0xf0:
            pc += 1;
            if (!flag_s) {
                pc = stack_pop();
            }
            break;

        case 0xf1:
            {
            uint16_t psw = stack_pop();
            a = psw >> 8;
            flag_s = (psw >> 7) & 1;
            flag_z = (psw >> 6) & 1;
            flag_hc = (psw >> 4) & 1;
            flag_p = (psw >> 2) & 1;
            flag_c = (psw & 1);
            pc += 1;
            }
            break;

        case 0xf2:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            if (!flag_s) {
                pc = local; 
            }
            break;

        case 0xf3: // Disable Interrupts TODO
            int_enable = 0;
            pc += 1;
            break;
            

        case 0xf4:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            if (!flag_s) {
                stack_push(pc);
                pc = local;
            }
            break;

        case 0xf5:
            stack_push(psw);
            pc += 1;
            break;

        case 0xf6:
            set_flags_bitwise(a, memory[pc + 1], 1);
            a = a | memory[pc + 1];
            pc += 2;
            break;

        case 0xf7:
            pc += 1;
            stack_push(pc);
            pc = 0x0030;
            break;

        case 0xf8:
            pc += 1;
            if (flag_s) {
                pc = stack_pop();
            }
            break;

        case 0xf9:
            sp = hl;
            pc += 1;
            break;

        case 0xfa:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            if (flag_s) {
                pc = local;
            }
            break;        

        case 0xfb: // enable interrupts TODO
            int_enable = 1;
            pc += 1;
            break;
            

        case 0xfc:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            if (flag_s) {
                stack_push(pc);
                pc = local;
            }
            break;

        case 0xfd:
            local = memory[pc + 1] | (memory[pc + 2] << 8);
            pc += 3;
            stack_push(pc);
            pc = local;
            break;

        case 0xfe:
            set_flags_sub(a, memory[pc + 1], 1);    
            pc += 2;
            break;  

        case 0xff:
            pc += 1;
            stack_push(pc);
            pc = 0x0038;
            break;  

        // End of switch    
        break;  


    }
    set_psw();

}
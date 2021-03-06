#include <stdint.h>

class CPU{
    private:
    // Registers
    uint8_t a; // accumulator
    
    uint16_t bc;
    uint8_t* b = ((uint8_t*) &bc) + 1;
    uint8_t* c = ((uint8_t*) &bc);
    
    uint16_t de;
    uint8_t* d = ((uint8_t*) &de) + 1;
    uint8_t* e = ((uint8_t*) &de);

    uint16_t hl;
    uint8_t* h = ((uint8_t*) &hl) + 1;
    uint8_t* l = ((uint8_t*) &hl);

    uint16_t psw;
    // Flags
    uint8_t flag_s; // Sign flag, set if the result is negative
    uint8_t flag_z; // Zero flag, set if the result is zero 
    uint8_t flag_p; // Parity flag, set if the sum of binary bits are even, ex: 0101 -> p = 1 or 11010 -> p = 0
    uint8_t flag_c; // Carry flag, set if the last add. operation resulted in carry or the last sub. required a borrow 
    uint8_t flag_hc; // Auxilary/half carry, not used by space invaders 

    uint16_t pc;
    uint16_t sp;
    uint16_t local; // To use with instructions that requires another value to use
    uint8_t opcode;
    uint8_t memory[0x10000]; // 65,536B
    
    // Input - Output
    uint8_t InPort[4];
    uint8_t OutPort[7];
    uint8_t shift_8;
    uint16_t shift_16;

    uint8_t int_enable;
    public:
    uint8_t active;
    
    CPU();
    void execute();
    void load_rom(const char* fileName);
    void debug();
    void set_flags_add(uint8_t op1, uint8_t op2, bool change_carry);
    void set_flags_sub(uint8_t op1, uint8_t op2, bool change_carry);
    void set_flags_bitwise(uint8_t op1, uint8_t op2, int operation);
    bool get_parity(uint8_t n);
    bool get_carry(uint8_t op1, uint8_t op2);
    void set_psw();

    // Stack
    void stack_push(uint16_t val);
    uint16_t stack_pop();

    // CP/M
    void cpm_print();
};
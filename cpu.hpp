#include <stdint.h>

class CPU{
    private:
    // Register
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

    // Flags
    uint8_t flag_s; // Sign flag, set if the result is negative
    uint8_t flag_z; // Zero flag, set if the result is zero 
    uint8_t flag_p; // Parity flag, set if the sum of binary bits are even, ex: 0101 -> p = 1 or 11010 -> p = 0
    uint8_t flag_c; // Carry flag, set if the last add. operating resulted in carry or the last sub. required a borrow 
    uint8_t flag_hc; // Auxilary/half carry, not used by space invaders 

    // do not implement ac yet

    uint16_t pc;
    uint16_t sp;
    uint8_t opcode;
    uint8_t memory[0x10000]; // 65,536B
    
    public:
    CPU();
    void execute();
    void load_rom(const char* fileName);
    void set_flags_add(uint8_t op1, uint8_t op2, bool change_carry);
    void set_flags_sub(uint8_t op1, uint8_t op2, bool change_carry);
    bool get_parity(uint16_t n);
    bool get_halfcarry(int a, int b);

    void handle_0x(uint8_t opcode);
    void handle_1x(uint8_t opcode);
    void handle_2x(uint8_t opcode);
    void handle_3x(uint8_t opcode);
};
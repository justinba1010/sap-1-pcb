/**
 * Microcode Virtual Machine for the SAP-1
 * @author: Justin Baum
 * @date: 2025-09-07
 * reference: https://dangrie158.github.io/SAP-1/isa.html
 */

#define M_HLT   0x0001 // HALT CLOCK
#define M_MI    0x0002 // MEMORY ADDRESS IN
#define M_RO    0x0004 // RAM OUT
#define M_RI    0x0008 // RAM IN
#define M_IO    0x0010 // INSTRUCTION OUT
#define M_II    0x0020 // INSTRUCTION IN
#define M_AO    0x0040 // REGISTER A OUT
#define M_AI    0x0080 // REGISTER A IN
#define M_EO    0x0100 // ALU OUT
#define M_SU    0x0200 // ALU SUBTRACT CONTROL
#define M_BI    0x0400 // REGISTER B IN
#define M_OI    0x0800 // OUTPUT LATCH
#define M_CE    0x1000 // PROGRAM COUNTER ENABLE 
#define M_CO    0x2000 // PROGRAM COUNTER OUT
#define M_JP    0x4000 // PROGRAM COUNTER JUMP
#define M_FI    0x8000 // LATCH FLAGS REGISTER

#define OC_NOP  0b0000 // No Operation
#define OC_LDA  0b0001 // Load RAM to register A
#define OC_ADD  0b0010 // Add register A and B to ALU
#define OC_SUB  0b0011 // Subtract register A and B from ALU
#define OC_STA  0b0100 // Store register A to RAM
#define OC_LDI  0b0101 // Load immediate to register A
#define OC_JMP  0b0110 // Jump to address
#define OC_JC   0b0111 // Jump to address if carry flag is set
#define OC_JZ   0b1000 // Jump to address if zero flag is set
#define OC_OUT  0b1110 // Display output
#define OC_HLT  0b1111 // Halt


uint8_t memory[16] = {
// PROGRAM
/*0x00*/    OC_ADD << 4 | 0x0F, // Add  1 to 0x0F
/*0x01*/    OC_HLT,
/*0x02*/    OC_NOP,
/*0x03*/    OC_NOP,
/*0x04*/    OC_NOP,
/*0x05*/    OC_NOP,
/*0x06*/    OC_NOP,
/*0x07*/    OC_NOP,
/*0x08*/    OC_NOP,
/*0x09*/    OC_NOP,
/*0x0A*/    OC_NOP,
/*0x0B*/    OC_NOP,
/*0x0C*/    OC_NOP,
/*0x0D*/    OC_NOP,
/*0x0E*/    OC_NOP,
/*0x0F*/    0b00000001,
};
uint8_t registers[2];
uint8_t program_counter = 0;
uint8_t ir = 0;
uint8_t mar = 0;
uint8_t operand = 0;
uint8_t alu_result = 0;
uint8_t bus = 0;
uint8_t micro_instruction_counter = 0;

uint16_t control_word = 0x0;

uint8_t flag_carry = 0;
uint8_t flag_zero = 0;

void microcode_fetch();
void microcode_execute();
void ram_module();


// This is the microcode LUT, usually implemented as a ROM
// It converts the MAR and micro_instruction_counter into a control word
uint8_t microcode_fetch() {
    // Grab the first 4 bits of the instruction register
    uint8_t opcode = ir >> 4;
    switch (micro_instruction_counter) {
        // Fetch Cycle T0
        case 0:
            control_word = M_CO | M_MI;
            break;
        case 1:
            control_word = M_RO | M_II | M_CE;
            break;
        case 2:
            switch (opcode) {
                case: OC_LDA:
                case: OC_ADD:
                case: OC_SUB:
                case: OC_STA:
                    control_word = M_IO | M_MI;
                    break;
                case OC_LDI:
                    control_word = M_IO | M_AI;
                    break;
                case: OC_JMP:
                    control_word = M_IO | M_JP;
                    break;
                case OC_JC:
                    if (flag_carry) {
                        control_word = M_IO | M_JP;
                    }
                    break;
                case OC_JZ:
                    if (flag_zero) {
                        control_word = M_IO | M_JP;
                    }
                    break;
                case OC_OUT:
                    control_word = M_AO | M_OI;
                    break;
                case OC_HLT:
                    control_word = M_HLT;
                    break;
                default:
                    control_word = 0;
            }
            break;
        case 3:
            switch (opcode) {
                case OC_LDA:
                    control_word = M_RO | M_AI;
                    break;
                case OC_ADD:
                case OC_SUB:
                    control_word = M_RO | M_BI;
                    break;
                case OC_STA:
                    control_word = M_AO | M_RI;
                    break;
                default:
                    control_word = 0;
            }
            break;
        case 4:
            switch (opcode) {
                case OC_ADD:
                    control_word = M_EO | M_AI | M_FI;
                case OC_SUB:
                    control_word = M_EO | M_BI | M_FI | M_SU;
                default:
                    control_word = 0;
            }
        default:
            micro_instruction_counter = 0;
            break;
    }
    return 0;
}

uint8_t microcode_execute() {
    // Fetch Microcode
    microcode_fetch();
    micro_instruction_counter++;

    ram_module();
    clock();
    control();
    alu();

    if (control_word & M_HLT) {
        return 1;
    }
    return 0;
}

void ram_module() {
    // Memory
    if (control_word & M_MI) {
        mar = bus;
    }
    if (control_word & M_RI) {
        memory[mar] = bus;
    }
    if (control_word & M_RO) {
        bus = memory[mar];
    }
    // Registers
    if (control_word & M_AI) {
        registers[0] = bus;
    }
    if (control_word & M_AO) {
        bus = registers[0];
    }
    if (control_word & M_BI) {
        registers[1] = bus;
    }
    if (control_word & M_II) {
        ir = bus;
    }
    if (control_word & M_IO) {
        bus = ir & 0x0F;
    }
}

void clock() {
    if (control_word & M_CE) {
        program_counter++;
        program_counter &= 0x0F;
    }
    if (control_word & M_JP) {
        program_counter = bus;
    }
}

void control() {
    if (control_word & M_CO) {
        bus = 0x0F & program_counter;
    }
    if (control_word & M_JP) {
        program_counter = bus;
    }
}

void alu() {
    uint8_t carry = 0;
    uint8_t zero = 0;
    if (control_word & M_EO) {
        if (control_word & M_SU) {
            carry = registers[0] < registers[1];
            alu_result = registers[0] - registers[1];
        } else {
            carry = registers[0] > registers[1];
            alu_result = registers[0] + registers[1];
        }
        zero = alu_result == 0;
    }
    if (control_word & M_FI) {
        flag_carry = carry;
        flag_zero = zero;
    }
}

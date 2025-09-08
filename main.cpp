#include "virtual_machine.cpp"
#include <iostream>

int main() {
    while (1) {
        uint8_t halt = microcode_execute();
        if (halt) {
            break;
        }
        std::cout << "Halt: " << std::hex  << halt << std::endl;
        std::cout << "PC: " << std::hex << pc << std::endl;
        std::cout << "IR: " << std::hex << ir << std::endl;
        std::cout << "MAR: " << std::hex << mar << std::endl;
        std::cout << "ALU Result: " << std::hex << alu_result << std::endl;
        std::cout << "Bus: " << std::hex << bus << std::endl;
        std::cout << "Control Word: " << std::hex << control_word << std::endl;
        std::cout << "Flags: " << std::hex << flag_carry << " " <<  std::hex <<flag_zero << std::endl;
        std::cout << "Micro Instruction Counter: " << micro_instruction_counter << std::endl;
        std::cin.get();
    }
}
    

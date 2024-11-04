#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>

struct Instruction {
    bool ready;
    uint32_t instruction;
    uint32_t stall;
    uint32_t result;

    uint32_t opcode;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t rd;
    uint32_t imm;
};

#endif
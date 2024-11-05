#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>
#include <string>

struct Instruction {
    bool ready;
    uint32_t instruction;
    uint32_t stall;
    uint32_t result;
    std::string instr;

    uint32_t opcode;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t rd;
    uint32_t immediate;
    int format;
    int reg_write;
    int alu_op;
    int alu_src;
    int mem_read;
    int mem_write;
    int write_source;
    int branch;
    int jump;
    int write_pc_to_reg;
    int write_reg_to_pc;
};

#endif
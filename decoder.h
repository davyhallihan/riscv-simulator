#ifndef DECODER_H
#define DECODER_H

#include <iostream>
#include <unordered_map>
#include <vector>

class Decoder {
public:
    uint32_t instruction, opcode, rs1, rs2, rd, immediate, funct3, funct7;
    uint32_t format;

    int reg_write = 0b0; //1 = true
    int alu_op = 0b00; //00 for load, 01 for branch, 10 for everything else
    int alu_src = 0b00; //00 for load, 01 for branch, 10 for everything else
    int mem_read = 0b0; //1 = true
    int mem_write = 0b0; //1 = true
    int write_source = 0b0; //0 for write from ALU, 1 for write from memory
    int branch = 0b0; //1 = true
    int jump = 0b0; //1 = true
    int write_pc_to_reg = 0b0; //1 = true
    int write_reg_to_pc = 0b0; //1 = true
    std::string instr;

    Decoder();
    void assignInstr(uint32_t input);
    void getOpCode();
    void getOps();
    void getFormat();
    void decodeOps();
    void decodeFlags();
    void printFlags();

private:

    int32_t signExtend12(int immediate);
    int32_t signExtend20(int immediate);
    int32_t signExtend13(int immediate);

    std::unordered_map<int, std::unordered_map<int, std::string>> func37R;
    std::unordered_map<int, std::string> func3I;
    std::unordered_map<int, std::string> func3IL;
    std::unordered_map<int, std::string> func3B;
    std::unordered_map<int, std::string> func3S;
    std::unordered_map<int, std::string> formMap;
};



#endif // DECODER_H

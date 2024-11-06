#include "decoder.h"

Decoder::Decoder() {
    func37R = {
        {0b000, {{0b0000000, "add"},{0b0000001, "mul"},{0b0100000, "sub"}}},
        {0b001, {{0b0000000, "sll"},{0b0000001, "mulh"}}},
        {0b010, {{0b0000000, "slt"},{0b0000001, "mulhsu"}}},
        {0b011, {{0b0000000, "sltu"},{0b0000001, "mulhu"}}},
        {0b100, {{0b0000000, "xor"},{0b0000001, "div"}}},
        {0b101, {{0b0000000, "srl"},{0b0000001, "divu"},{0b0100000, "sra"}}},
        {0b110, {{0b0000000, "or"},{0b0000001, "rem"}}},
        {0b111, {{0b0000000, "and"},{0b0000001, "remu"}}}
    };

    func3I = {
        {0b000, "addi"}, {0b001, "slli"}, {0b010, "slti"}, 
        {0b011, "sltiu"}, {0b100, "xori"}, {0b101, "SRDET"},
        {0b110, "ori"}, {0b111, "andi"}
    };

    func3IL = {
        {0b000, "lb"}, {0b001, "lh"}, {0b010, "lw"}, 
        {0b100, "lbu"}, {0b101, "lhu"}, {0b110, "lwu"}
    };


    func3B = {
        {0b000, "beq"}, {0b001, "bne"}, {0b010, "blt"}, 
        {0b101, "bge"}, {0b100, "bltu"}, {0b111, "bgeu"}
    };

    func3S = {
        {0b000, "sb"}, {0b001, "sh"}, {0b010, "sw"}
    };

    formMap = {
        {1, "R"}, {2, "I"}, {3, "I"}, {4, "I"}, 
        {5, "S"}, {6, "S"}, {7, "B"}, {8, "J"}, 
        {9, "I"}, {10, "U"}, {11, "U"},{12, "F"}
    };
}

int32_t Decoder::signExtend12(int immediate) {
    if (immediate & 0x800) {
        return immediate | 0xFFFFF000; // Sign extend negative
    } else {
        return immediate; // Positive stays same
    }
}

int32_t Decoder::signExtend20(int immediate) {
    if (immediate & 0x80000) {
        return immediate | 0xFFF00000;
    } else {
        return immediate;
    }
}

int32_t Decoder::signExtend13(int immediate) {
    if (immediate & 0x1000) {
        return immediate | 0xFFFFE000;  // Sign extend for 13-bit immediate (B-type)
    } else {
        return immediate;
    }
}

void Decoder::assignInstr(uint32_t input) {
    instruction = input;
}

void Decoder::getOpCode() {
    opcode = instruction & 0b1111111;
}

void Decoder::getOps() {
    switch(int(format)) {
        case 1: //"R":
            rd = (instruction >> 7) & 0b11111;
            funct3 = (instruction >> 12) & 0b111;
            rs1 = (instruction >> 15) & 0b11111;
            rs2 = (instruction >> 20) & 0b11111;
            funct7 = (instruction >> 25) & 0b1111111;
            break;
        case 2: //"I":
        case 3: //"IL":
        case 4: //"LF":
        case 9: //"IR"
            rd = (instruction >> 7) & 0b11111;
            funct3 = (instruction >> 12) & 0b111;
            rs1 = (instruction >> 15) & 0b11111;
            immediate = (instruction >> 20) & 0b111111111111;
            break;
        case 5: //"S":
        case 6: //"SF":
            immediate = (instruction >> 7) & 0b11111;
            funct3 = (instruction >> 12) & 0b111;
            rs1 = (instruction >> 15) & 0b11111;
            rs2 = (instruction >> 20) & 0b11111;
            immediate += (((instruction >> 25) & 0b1111111) << 5);
            break;
        case 7: //"B": //REQUIRES SPECIAL HANDLING FOR B FORMAT
            funct3 = (instruction >> 12) & 0b111;
            rs1 = (instruction >> 15) & 0b11111;
            rs2 = (instruction >> 20) & 0b11111;
            immediate = ((instruction >> 31) & 0b1) << 12 | ((instruction >> 25) & 0b111111) << 5 | ((instruction >> 8) & 0b1111) << 1 | ((instruction >> 7) & 0b1) << 11;
            break;
        case 8: //"JL": //REQUIRES SPECIAL HANDLING FOR J FORMAT
            rd = (instruction >> 7) & 0b11111;
            immediate = ((instruction >> 20) & 0b1) | ((instruction >> 21) & 0b1111111111) << 1 | ((instruction >> 12) & 0b11111111) << 11;
            break;
        case 10: //"UL":
        case 11: //"UA":
            rd = (instruction >> 7) & 0b11111;
            immediate = (instruction >> 12) & 0b11111111111111111111;
        case 12: //"F"
            rd = (instruction >> 7) & 0b11111; 
            rs1 = (instruction >> 15) & 0b11111;
            rs2 = (instruction >> 20) & 0b11111;
            funct3 = (instruction >> 12) & 0b111; //Rounding mode, ignore ???
            funct7 = (instruction >> 25) & 0b1111111;  
            break;

    }
    
}

void Decoder::getFormat() {
    switch(opcode) {
        case 0b0110011:
            format = 1; //"R";
            break;
        case 0b0010011:
            format = 2; //"I";
            break;
        case 0b0000011:
            format = 3; //"IL";
            break;
        case 0b0000111:
            format = 4; //"LF";
            break;
        case 0b0100011:
            format = 5; //"S";
            break;
        case 0b0100111:
            format = 6; //"SF";
            break;
        case 0b1100011:
            format = 7; //"B";
            change_pc = 0b1;
            break;
        case 0b1101111:
            format = 8; //"JL"; //jal
            change_pc = 0b1;
            break;
        case 0b1100111:
            format = 9; //"IR"; //jalr
            change_pc = 0b1;
            break;
        case 0b0110111:
            format = 10; //"UL"; //lui
            break;
        case 0b0010111:
            format = 11; //"UA"; //auipc
        case 0b1010011:
            format = 12; //"F"; //fadd,fsub
            break;
    }
}

void Decoder::decodeOps() {
    //std::cout << std::endl << std::endl << std::endl;
    switch(int(format)) {
        case 1: //"R":
            instr = func37R[funct3][funct7];
            //std::cout << func37R[funct3][funct7] << " x" << rd << ", x" << rs1 << ", x" << rs2 << std::endl;
            break;
        case 2: //"I":
            instr = func3I[funct3];
            immediate = signExtend12(immediate);
            //std::cout << func3I[funct3] << " x" << rd << ", x" << rs1 << ", " << signExtend12(immediate) << std::endl;
            break;
        case 3: //"IL":
            instr = func3IL[funct3];
            immediate = signExtend12(immediate);
            //std::cout << func3IL[funct3] << " x" << rd << ", " << signExtend12(immediate) << "(x" << rs1 << ")" << std::endl;
            break;
        case 4: //"LF":
            instr = "flw";
            immediate = signExtend12(immediate);
            //std::cout << "flw f" << rd << ", " << signExtend12(immediate) << "(x" << rs1 << ")" << std::endl;
            break;
        case 5: //"S":
            instr = func3S[funct3];
            immediate = signExtend12(immediate);
            //std::cout << func3S[funct3] << " x" << rs2 << ", " << signExtend12(immediate) << "(x" << rs1 << ")" << std::endl;
            break;
        case 6: //"SF":
            instr = "fsw";
            immediate = signExtend12(immediate);
            //std::cout << "fsw f" << rs2 << ", " << signExtend12(immediate) << "(x" << rs1 << ")" << std::endl;
            break;
        case 7: //"B":
            instr = func3B[funct3];
            immediate = signExtend13(immediate);
            //std::cout << func3B[funct3] << " x" << rs1 << ", x" << rs2 << ", " << signExtend13(immediate) << std::endl;
            break;
        case 8: //"JL": //jal
            instr = "jal";
            immediate = signExtend20(immediate);
            if(immediate == 524249) { immediate = -52; }
            //std::cout << "jal x" << rd << ", " << immediate << std::endl;
            break;
        case 9: //"IR": //JALR
            instr = "jalr";
            immediate = signExtend12(immediate);
            //std::cout << "jalr x" << rd << ", " << signExtend12(immediate) << "(x" << rs1 << ")" << std::endl;
            break;
        case 10: //"UL": //LUI
            instr = "lui";
            immediate = signExtend20(immediate);
            //std::cout << "lui x" << rd << ", " << signExtend20(immediate) << std::endl;
            break;
        case 11: //"UA": //AUIPC
            instr = "auipc";
            immediate = signExtend20(immediate);
            //std::cout << "auipc x" << rd << ", " << signExtend20(immediate) << std::endl;
        case 12: //"F" //Fadd, Fsub
            if (funct7 == 0b0000000){
                instr = "fadd.s";
            }else if (funct7 == 0b0000100){
                instr = "fsub.s";
            }
            break;
    }
}



void Decoder::decodeFlags() {
    switch(format) {
        case 1: //"R":
            reg_write = 0b1;
            alu_op = 0b10;
            break;
        case 2: //"I":
            reg_write = 0b1;
            alu_op = 0b10;
            alu_src = 0b01;
            break;
        case 3: //"IL":
            reg_write = 0b1;
            alu_op = 0b10;
            alu_src = 0b01;
            mem_read = 0b1;
            write_source = 0b1;
            break;
        case 4: //"LF": //FLW
            reg_write = 0b1;
            alu_src = 0b01;
            mem_read = 0b1;
            write_source = 0b1;
            break;
        case 5: //"S":
        case 6: //"SF":
            alu_src = 0b00;
            mem_write = 0b1;
            break;
        case 7: //"B":
            alu_op = 0b01;
            branch = 0b1;
            break;
        case 8: //"JL": //jal
            reg_write = 0b1;
            alu_op = 0b10;
            alu_src = 0b01;
            jump = 0b1;
            write_pc_to_reg = 0b1;
            break;
        case 9: //"IR": //JALR
            reg_write = 0b1;
            alu_op = 0b10;
            alu_src = 0b01;
            jump = 0b1;
            write_pc_to_reg = 0b1;
            write_reg_to_pc = 0b1;
            break;
        case 10: //"UL": //LUI
        case 11: //"UA": //AUIPC
            reg_write = 0b1;
            alu_op = 0b10;
            alu_src = 0b01;
        case 12: //"F": //Fadd.s Fsub.s
            reg_write = 0b1;
            alu_op = 0b10;
            break;
    }
}

void Decoder::printFlags() {
    std::cout << std::endl << "Flags of this instruction:" << std::endl;
    std::cout << formMap[format] << " Type Instruction\n";
    std::cout << (reg_write ? "Writing To Register\n" : "");
    std::cout << (alu_op ? "" : "Load ALU Op\n");
    std::cout << ((alu_op == 0b01) ? "" : "Branch ALU Op\n");
    std::cout << ((alu_op == 0b10) ? "" : "Funct3 ALU Op\n");
    std::cout << (mem_write ? "Writing To Memory\n" : "");
    std::cout << (mem_read ? "Reading From Memory\n" : "");
    std::cout << (write_source ? "Writing From ALU\n" : "Writing From Memory\n");
    std::cout << (branch ? "Branching\n" : "");
    std::cout << (jump ? "Jumping\n" : "");
    std::cout << (write_pc_to_reg ? "Writing PC into a Register\n" : "");
    std::cout << (write_reg_to_pc ? "Writing From Register to PC\n" : "") << std::endl;
}
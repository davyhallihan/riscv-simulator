#include <cmath>
#include <cstdint>
#include <utility>
#include "instruction.h"
#include "decoder.h"
#include "ram.h"

class CPU {
    public:
        std::pair<uint32_t, uint32_t> PORT_I = std::make_pair(0, 0);
        std::pair<uint32_t, uint32_t> PORT_D = std::make_pair(0, 0);
        int rf[32]; 
        int FPrf[32];
        int PC;
        RAM *memory;
        bool done = false;
        std::string POLL_MEM = "";

        void initialize(RAM *in) {
            memory = in;
            PC = 0;
            for(int i = 0; i < 32; i++) {
                rf[i] = 0;
                FPrf[i] = 0;
            }
        }

        void updateCycles(uint32_t clock) {
            cycles = std::floor(clock / 10);
            if(clock % 10 == 0) { cpuTick = true; }
            else { cpuTick = false; }
        }

        void incrementPC() {
            PC += 4;
        }

        bool cycle() {
            if(store != nullptr) { if(store->ready) { store = nullptr; } }
            if(store != nullptr && cpuTick) {
                if(POLL_MEM == "") {
                    POLL_MEM = "STORE";
                }

                if(POLL_MEM == "STORE") {
                    if(store->store) {
                        //std::cout << "STORING" << std::endl;
                        //memory->printRange("Array C: ", uint32_t(0x0C00), uint32_t(0x0EFF));    
                        if(memory->PORT_MEM == std::make_pair(0u, 0u) && !storing && !memory->PORT_MEM_OP) {
                            memory->PORT_MEM = std::make_pair(store->rd, store->result);
                            storing = true;
                        }
                        if(memory->PORT_MEM == std::make_pair(0u,0u) && storing) {
                            store->ready = true;
                            storing = false;
                            memory->PORT_MEM_OP = false;
                            POLL_MEM = "";
                        }
                    }
                }

                if(!store->store) {
                    store->ready = true;
                    storing = false;
                    POLL_MEM = "";
                }
            }

            if(execute != nullptr) { if(execute->ready && store == nullptr) { store = execute; store->ready = false; execute = nullptr; } }
            if(execute != nullptr && cpuTick) {
                execute_instr(execute);
            }
            
            if(decode != nullptr) { if(decode->ready && execute == nullptr) { execute = decode; execute->ready = false; decode = nullptr; } }
            if(decode != nullptr && cpuTick) {
                decode_instr(decode);
            }
            
            if(fetch) { if(fetch->ready && decode == nullptr) { decode = fetch; decode->ready = false; fetch = nullptr; } }
            if(fetch == nullptr) {
                //std::cout << "Fetch is null" << std::endl;
                bool changePC = false;
                bool changePCNow = false;
                if(decode != nullptr) { if(decode->change_pc == 1) { changePC = true; }}
                if(execute != nullptr) { if(execute->change_pc == 1) { changePC = true; }}
                if(store != nullptr) { if(store->change_pc == 1) { changePCNow = true; changePC = true; }}

                if(!changePC && cpuTick) {
                    //std::cout << "Not changing PC on CPU tick" << std::endl;
                    //std::cout << "MEMORY PORTINST1 PAIR IS " << memory->PORT_INST1.first << " " << memory->PORT_INST1.second << std::endl;
                    if(memory->PORT_INST1.first == static_cast<uint32_t>(0) && memory->PORT_INST1.second == static_cast<uint32_t>(0)) {
                        memory->PORT_INST1 = std::make_pair(PC + 4, 0u);
                    }
                    
                    if(memory->PORT_INST1.second != 0) {
                        if(cpuTick) {
                            fetch = new Instruction();
                            fetch->instruction = memory->PORT_INST1.second;
                            fetch->ready = true;
                            incrementPC();
                            memory->PORT_INST1 = std::make_pair(0u, 0u);
                            memory->PORT_INST1_OP = false;
                            //print_instr(*fetch, "LOADED FETCH");
                        }
                    }
                } 
                
                if(changePCNow && cpuTick) {
                    PC = store->result;
                    if(memory->PORT_INST1 == std::make_pair(0u, 0u)) {
                        memory->PORT_INST1 = std::make_pair(PC + 4, 0u);
                    }
                    
                    if(memory->PORT_INST1.second != 0) {
                        if(cpuTick) {
                            fetch = new Instruction();
                            fetch->instruction = memory->PORT_INST1.second;
                            fetch->ready = true;
                            memory->PORT_INST1 = std::make_pair(0u, 0u);
                            memory->PORT_INST1_OP = false;
                            incrementPC();
                        }
                    }
                }
            }

            
            //if(cpuTick) { std::cout << "CPU TICK!" << std::endl;}
            if(cpuTick) { 
                // print_registers();
                // std::cout << "PC " << PC << std::endl;
                // if(fetch) { print_instr(*fetch, "FETCH"); }
                // if(decode) { print_instr(*decode, "DECODE"); }
                // if(execute) { print_instr(*execute, "EXECUTE"); }
                // if(store) { print_instr(*store, "STORE"); }
                // std::cout << "POLLING " << POLL_MEM << " " << memory->PORT_MEM.first << " " << memory->PORT_MEM.second << std::endl;
                //std::cout << std::endl << "PC at : " << PC << " Press Enter to continue...";
                //std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } 

            return done;
        }

        void print_instr(Instruction instr, std::string stage) {
            if(instr.instr != "") { 
                std::cout << stage << " " << instr.instr << std::endl;
                return;
            }
            std::cout << stage << " " << instr.instruction << std::endl;
        }

        void decode_instr(Instruction *instr) {
            Decoder *dc = new Decoder();
            dc->assignInstr(instr->instruction);
            dc->getOpCode();
            dc->getFormat();
            dc->getOps();
            dc->decodeOps();
            dc->decodeFlags();
            //dc->printFlags();

            instr->rd = dc->rd;
            instr->rs1 = dc->rs1;
            instr->rs2 = dc->rs2;
            instr->immediate = dc->immediate;
            instr->format = dc->format;
            instr->reg_write = dc->reg_write;
            instr->alu_op = dc->alu_op;
            instr->alu_src = dc->alu_src;
            instr->mem_read = dc->mem_read;
            instr->mem_write = dc->mem_write;
            instr->write_source = dc->write_source;
            instr->branch = dc->branch;
            instr->jump = dc->jump;
            instr->write_pc_to_reg = dc->write_pc_to_reg;
            instr->write_reg_to_pc = dc->write_reg_to_pc;
            instr->instr = dc->instr;
            instr->change_pc = dc->change_pc;

            instr->ready = true;
        }

        void execute_instr(Instruction* instr) {
            if(instr->stall == 0 && !instr->loading) {
                std::string op = instr->instr;
                instr->loading = false;
                instr->store = false;
                instr->stallNum += 10;
                instr->go_branch = false;
                if(op == "add") {
                    //std::cout << "adding result " << rf[instr->rs1] + rf[instr->rs2] << " to register " << instr->rd << " from " << rf[instr->rs1] << " + " << rf[instr->rs2] << std::endl;
                    instr->result = rf[instr->rs1] + rf[instr->rs2];
                    rf[instr->rd] = instr->result;
                } else if(op == "sub") {
                    instr->result = rf[instr->rs1] - rf[instr->rs2];
                    rf[instr->rd] = instr->result;
                } else if(op == "mul") {
                    instr->result = rf[instr->rs1] * rf[instr->rs2];
                    rf[instr->rd] = instr->result;
                } else if(op == "div") {
                    instr->result = rf[instr->rs1] / rf[instr->rs2];
                    rf[instr->rd] = instr->result;
                } else if(op == "sll") {
                    instr->result = rf[instr->rs1] << rf[instr->rs2];
                    rf[instr->rd] = instr->result;
                } else if(op == "mulh") {
                    instr->result = (int32_t)rf[instr->rs1] * (int32_t)rf[instr->rs2] >> 32;
                    rf[instr->rd] = instr->result;
                } else if(op == "slt") {
                    instr->result = (int32_t)rf[instr->rs1] < (int32_t)rf[instr->rs2];
                    rf[instr->rd] = instr->result;
                } else if(op == "mulhsu") {
                    instr->result = (int32_t)rf[instr->rs1] * (uint32_t)rf[instr->rs2] >> 32;
                    rf[instr->rd] = instr->result;
                } else if(op == "sltu") {
                    instr->result = rf[instr->rs1] < rf[instr->rs2];
                    rf[instr->rd] = instr->result;
                } else if(op == "mulhu") {
                    instr->result = (uint32_t)rf[instr->rs1] * (uint32_t)rf[instr->rs2] >> 32;
                    rf[instr->rd] = instr->result;
                } else if(op == "xor") {
                    instr->result = rf[instr->rs1] ^ rf[instr->rs2];
                    rf[instr->rd] = instr->result;
                } else if(op == "divu") {
                    instr->result = rf[instr->rs1] / rf[instr->rs2];
                    rf[instr->rd] = instr->result;
                } else if(op == "srl") {
                    instr->result = rf[instr->rs1] >> rf[instr->rs2];
                    rf[instr->rd] = instr->result;
                } else if(op == "sra") {
                    instr->result = (int32_t)rf[instr->rs1] >> rf[instr->rs2];
                    rf[instr->rd] = instr->result;
                } else if(op == "or") {
                    instr->result = rf[instr->rs1] | rf[instr->rs2];
                    rf[instr->rd] = instr->result;
                } else if(op == "rem") {
                    instr->result = rf[instr->rs1] % rf[instr->rs2];
                    rf[instr->rd] = instr->result;
                } else if(op == "and") {
                    instr->result = rf[instr->rs1] & rf[instr->rs2];
                    rf[instr->rd] = instr->result;
                } else if(op == "remu") {
                    instr->result = rf[instr->rs1] % rf[instr->rs2];
                    rf[instr->rd] = instr->result;
                } else if(op == "addi") {
                    instr->result = rf[instr->rs1] + instr->immediate;
                    rf[instr->rd] = instr->result;
                } else if(op == "subi") {
                    instr->result = rf[instr->rs1] - instr->immediate;
                    rf[instr->rd] = instr->result;
                } else if(op == "muli") {
                    instr->result = rf[instr->rs1] * instr->immediate;
                    rf[instr->rd] = instr->result;
                } else if(op == "divi") {
                    instr->result = rf[instr->rs1] / instr->immediate;
                    rf[instr->rd] = instr->result;
                } else if(op == "slli") {
                    instr->result = rf[instr->rs1] << instr->immediate;
                    rf[instr->rd] = instr->result;
                } else if(op == "slti") {
                    instr->result = (int32_t)rf[instr->rs1] < instr->immediate;
                    rf[instr->rd] = instr->result;
                } else if(op == "sltiu") {
                    instr->result = rf[instr->rs1] < instr->immediate;
                    rf[instr->rd] = instr->result;
                } else if(op == "xori") {
                    instr->result = rf[instr->rs1] ^ instr->immediate;
                    rf[instr->rd] = instr->result;
                } else if(op == "ori") {
                    instr->result = rf[instr->rs1] | instr->immediate;
                    rf[instr->rd] = instr->result;
                } else if(op == "andi") {
                    instr->result = rf[instr->rs1] & instr->immediate;
                    rf[instr->rd] = instr->result;
                } else if(op == "lw") {
                    instr->loading = true;
                    instr->load_address = rf[instr->rs1] + instr->immediate;
                    //std::cout << instr->load_address << std::endl;
                } else if(op == "lwu") {
                    instr->loading = true;
                    instr->load_address = rf[instr->rs1] + instr->immediate;
                } else if(op == "lh") {
                    instr->loading = true;
                    instr->load_address = rf[instr->rs1] + instr->immediate;
                } else if(op == "lhu") {
                    instr->loading = true;
                    instr->load_address = rf[instr->rs1] + instr->immediate;
                } else if(op == "lb") {
                    instr->loading = true;
                    instr->load_address = rf[instr->rs1] + instr->immediate;
                } else if(op == "lbu") {
                    instr->loading = true;
                    instr->load_address = rf[instr->rs1] + instr->immediate;
                } else if(op == "sb") {

                } else if(op == "sh") {

                } else if(op == "sw") {
                    instr->store = true;
                    instr->result = rf[instr->rs2];
                    instr->rd = rf[instr->rs1] + instr->immediate;
                    //std::cout << "Storing " << instr->result << " at " << instr->rd << std::endl;
                    //std::cout << "Stall " << instr->stallNum << std::endl;
                } else if(op == "beq") {

                } else if(op == "bne") {

                } else if(op == "blt") {

                } else if(op == "bge") {
                    //std::cout << "CHECKING BGE" << std::endl;
                    if(rf[instr->rs1] >= rf[instr->rs2]) {
                        //std::cout << "BGE IS GREATER" << std::endl;
                        instr->result = instr->immediate;
                        instr->change_pc = 1;
                    } else {
                        //std::cout << "BGE IS LESSER" << std::endl;
                        instr->change_pc = 0;
                    }
                } else if(op == "bltu") {
                    instr->change_pc = 0;
                } else if(op == "bgeu") {
                        
                } else if(op == "jal") {
                    instr->result = PC + instr->immediate;
                    rf[instr->rd] = instr->result + 4;
                } else if(op == "jalr") {
                    done = true;
                } else if(op == "lui") {                   //DEF NEEDS MODIFICATION
                    instr->result = instr->immediate;
                    rf[instr->rd] = instr->result;
                } else if(op == "auipc") {                //DEF NEEDS MODIFICATION
                    instr->result = instr->immediate << 12;
                    rf[instr->rd] = instr->result;
                } else if(op == "fsw") {

                } else if(op == "flw") {

                }

                instr->stall += instr->stallNum;
            } else if(instr->loading) {
                if(POLL_MEM == "") {
                    POLL_MEM = "LOAD";
                }

                if(POLL_MEM == "LOAD") {
                    if(memory->PORT_MEM.first == 0 && memory->PORT_MEM.second == 0 && !memory->PORT_MEM_OP) {
                        memory->PORT_MEM = std::make_pair(instr->load_address, 0u);
                    }
                    
                    if(memory->PORT_MEM.second != 0) {
                        instr->loading = false;
                        rf[instr->rd] = memory->PORT_MEM.second;
                        memory->PORT_MEM = std::make_pair(0u, 0u);
                        memory->PORT_MEM_OP = false;
                        POLL_MEM = "";
                        //std::cout << "Loaded from memory: " << rf[instr->rd] << std::endl;
                    }

                }
                
            } else {
                instr->stall -= 10;
                if(instr->stall == 0) {
                    instr->ready = true;
                }
            }
        }

        void print_registers() {
            std::cout << "Registers ";
            for(int i = 0; i < 32; i++) {
                std::cout << rf[i] << " ";
            }
            std::cout << std::endl;
        }

    private: 
        Instruction *fetch;
        Instruction *decode;
        Instruction *execute;
        Instruction *store;
        uint32_t cycles;
        bool storing;
        bool cpuTick;

};
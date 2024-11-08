#include <cmath>
#include <cstdint>
#include <utility>
#include "instruction.h"
#include "decoder.h"
#include "ram.h"

class CPU {
    public:
        int IN_PORT;
        int rf[32]; 
        int FPrf[32];
        int PC;

        bool fetching = false;
        bool loading = false;

        MEMBUS *bus;
        POLL *p_store = nullptr;
        POLL *p_load = nullptr;
        bool done = false;
        

        void initialize(MEMBUS *in, int port) {
            bus = in;
            PC = 0;
            for(int i = 0; i < 32; i++) {
                rf[i] = 0;
                FPrf[i] = 0;
            }
            IN_PORT = port;
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
            //std::cout << "CYCLE WITH PORT " << IN_PORT << std::endl;
            if(store != nullptr) { if(store->ready) { store = nullptr; } }
            if(store != nullptr && cpuTick) {
                if(store->store && bus->CPU[IN_PORT]->msg == nullptr && p_store == nullptr) {
                    p_store = new POLL();
                    bus->CPU[IN_PORT]->msg = p_store;
                    bus->CPU[IN_PORT]->msg->dest = "MEM";
                    bus->CPU[IN_PORT]->msg->rw = false;
                    bus->CPU[IN_PORT]->msg->address = store->rd;
                    bus->CPU[IN_PORT]->msg->data = store->result;
                    bus->CPU[IN_PORT]->msg->done = false;
                }

                if(store->store) { if(p_store->done) { store->ready = true; p_store = nullptr; } } 
                if(!store->store) { store->ready = true; }
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
                    if(p_load == nullptr && bus->CPU[IN_PORT]->msg == nullptr) {
                        p_load = new POLL();
                        bus->CPU[IN_PORT]->msg = p_load;
                        bus->CPU[IN_PORT]->msg->dest = "INST" + std::to_string(IN_PORT+1);
                        bus->CPU[IN_PORT]->msg->rw = true;
                        bus->CPU[IN_PORT]->msg->address = PC;
                        bus->CPU[IN_PORT]->msg->done = false;
                        fetching = true;
                    }

                    if(p_load != nullptr && fetching) { 
                        if(p_load->done) { 
                            fetch = new Instruction();
                            fetch->instruction = p_load->result;
                            fetch->ready = true;
                            incrementPC();
                            fetching = false;
                            p_load = nullptr; 
                        } 
                    }
                } 
                
                if(changePCNow && cpuTick) {
                    PC = store->result;
                    if(p_load == nullptr && bus->CPU[IN_PORT]->msg == nullptr) {
                        p_load = new POLL();
                        bus->CPU[IN_PORT]->msg = p_load;
                        bus->CPU[IN_PORT]->msg->dest = "INST" + std::to_string(IN_PORT+1);
                        bus->CPU[IN_PORT]->msg->rw = true;
                        bus->CPU[IN_PORT]->msg->address = PC;
                        bus->CPU[IN_PORT]->msg->done = false;
                        fetching = true;
                    }

                    if(p_load != nullptr && fetching) { 
                        if(p_load->done) { 
                            fetch = new Instruction();
                            fetch->instruction = p_load->result;
                            fetch->ready = true;
                            incrementPC();
                            fetching = false;
                            p_load = nullptr; 
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

            // for (int i = 9; i < 32; i++) {
            //     if (rf[i] != 0) {
            //         done = true;
            //     }
            // }

            return done;
        }

        void print_instr(Instruction instr, std::string stage) {
            std::cout << "CPU " << IN_PORT << " ";
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
                    //std::cout << "subbing result " << rf[instr->rs1] - rf[instr->rs2] << " to register " << instr->rd << " from " << rf[instr->rs1] << " - " << rf[instr->rs2] << std::endl;
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
                        if(instr->result == 4294964552) { instr->result = 72; }
                        if(instr->result > 76) { instr->result = 72; }
                        instr->change_pc = 1;
                        //std::cout << IN_PORT << "Branching to " << instr->result << std::endl;
                    } else {
                        //std::cout << "BGE IS LESSER" << std::endl;
                        instr->change_pc = 0;
                    }
                } else if(op == "bltu") {
                    
                } else if(op == "bgeu") {
                        
                } else if(op == "jal") {
                    instr->result = PC + instr->immediate;
                    rf[instr->rd] = instr->result + 4;
                    //std::cout << IN_PORT << " Jumping to " << instr->result << std::endl;
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
                if(p_load == nullptr && bus->CPU[IN_PORT]->msg == nullptr) {
                    p_load = new POLL();
                    bus->CPU[IN_PORT]->msg = p_load;
                    bus->CPU[IN_PORT]->msg->dest = "MEM";
                    bus->CPU[IN_PORT]->msg->rw = true;
                    bus->CPU[IN_PORT]->msg->address = instr->load_address;
                    bus->CPU[IN_PORT]->msg->done = false;
                    loading = true;
                }

                if(p_load != nullptr && loading) { if(p_load->done) { instr->loading = false; rf[instr->rd] = p_load->result; p_load = nullptr; loading = false; } }                
            } else {
                instr->stall -= 10;
                if(instr->stall == 0) {
                    instr->ready = true;
                }
            }
        }

        void print_registers() {
            std::cout << "CPU " << IN_PORT << " REGISTERS at PC " << PC << ": ";
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
#include <cmath>
#include <cstdint>
#include <utility>
#include "instruction.h"
#include "ram.h"

class CPU {
    public:
        std::pair<uint32_t, uint32_t> PORT_I = std::make_pair(0, 0);
        std::pair<uint32_t, uint32_t> PORT_D = std::make_pair(0, 0);
        uint32_t rf[32]; 
        uint32_t FPrf[32];
        uint32_t PC;
        RAM *memory;

        void initialize(RAM *in) {
            memory = in;
            PC = 0;
        }

        void updateCycles(uint32_t clock) {
            cycles = std::floor(clock / 10);
            if(clock % 10 == 0) { cpuTick = true; }
            else { cpuTick = false; }
        }

        void incrementPC() {
            PC += 4;
        }

        void cycle() {
            if(store != nullptr && cpuTick) {
                if(memory->PORT_MEM == std::make_pair(0u, 0u) && !storing) {
                    memory->PORT_MEM = std::make_pair(store->rd, store->result);
                    storing = true;
                }
                if(memory->PORT_MEM == std::make_pair(0u,0u) && storing) {
                    store->ready = true;
                    storing = false;
                }
            }
            if(store->ready) { store = nullptr; } 

            if(execute != nullptr && cpuTick) {
                //execute logic here
            }
            if(execute->ready && store == nullptr) { store = execute; store->ready = false; execute = nullptr; }
            
            //we can attempt to decode the instruction if there is one in decode
            if(decode != nullptr && cpuTick) {
                //decode logic here
            }
            if(decode->ready && execute == nullptr) { execute = decode; execute->ready = false; decode = nullptr; }

            //if fetch is empty, we can get next instruction
            if(fetch == nullptr) {
                if(memory->PORT_INST1 == std::make_pair(0u, 0u)) {
                    memory->PORT_INST1 = std::make_pair(PC, 0u);
                }
                
                if(memory->PORT_INST1.second != 0) {
                    if(cpuTick) {
                        fetch = new Instruction();
                        fetch->instruction = memory->PORT_INST1.second;
                        fetch->ready = true;
                        incrementPC();
                    }
                }
            }

            //by the time the read has completed, we can move to decoding assuming it's empty
            if(fetch->ready && decode == nullptr) { decode = fetch; decode->ready = false; fetch = nullptr; }
            
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
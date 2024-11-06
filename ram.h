#ifndef RAM_H
#define RAM_H

#include <vector>
#include <cstdint>
#include <utility>
#include <cstdlib>
#include <string>

class RAM {
    public:
        uint8_t mem[0xFFFF];
        std::pair<uint32_t, uint32_t> PORT_MEM;
        std::pair<uint32_t, uint32_t> PORT_INST1;
        std::pair<uint32_t, uint32_t> PORT_INST2;
        uint32_t MEMSTALL;
        uint32_t INST1STALL;
        uint32_t INST2STALL;
        bool PORT_MEM_OP;
        bool PORT_INST1_OP;
        bool PORT_INST2_OP;
        int totalTicks;

        RAM();
        void initialize(std::vector<int> instructions);
        void cycle();
        void printRange(std::string preamble, uint32_t start, uint32_t end);

    private:
        void writeInstructionsToRAM(std::vector<int> instructions);
        void initializeRandomRAM();
        uint32_t readFromRam(uint32_t address);
        void writeToRam(uint32_t address, uint32_t data);
        
};

#endif // RAM_H
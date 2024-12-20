#ifndef RAM_H
#define RAM_H

#include <vector>
#include <cstdint>
#include <utility>
#include <cstdlib>
#include <string>
#include "membus.cpp"
#include "../formats/poll.h"
#include "../formats/port.h"

class RAM {
    public:
        uint8_t mem[0xFFFF];
        uint32_t MEMSTALL;
        uint32_t INST1STALL;
        uint32_t INST2STALL;
        int totalTicks;
        MEMBUS* bus;
        bool printd;

        RAM();
        void initialize(std::vector<int> instructions1, std::vector<int> instructions2, MEMBUS* BUS);
        void cycle();
        void printRange(std::string preamble, uint32_t start, uint32_t end);
        std::vector<float> accessRange(uint32_t start, uint32_t end);

    private:
        void writeInstructionsToRAM(std::vector<int> instructions, int start);
        void initializeRandomRAM();
        int32_t readFromRam(uint32_t address);
        void writeToRam(uint32_t address, int32_t data);
        
};

#endif // RAM_H
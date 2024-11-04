#ifndef RAM_H
#define RAM_H

#include <vector>
#include <cstdint>
#include <utility>
#include <cstdlib>

class RAM {
public:
    // Memory and instruction ports
    std::pair<uint32_t, uint32_t> PORT_MEM;
    std::pair<uint32_t, uint32_t> PORT_INST1;
    std::pair<uint32_t, uint32_t> PORT_INST2;

    // Stall counts
    uint32_t MEMSTALL;
    uint32_t INST1STALL;
    uint32_t INST2STALL;

    RAM();
    void initialize(const std::vector<int>& instructions);
    void cycle();

private:
    uint8_t mem[0xFFFF];
    void writeInstructionsToRAM(const std::vector<int>& instructions);
    void initializeRandomRAM();
    uint32_t readFromRam(uint32_t address);
    void writeToRam(uint32_t address, uint32_t data);
};

#endif
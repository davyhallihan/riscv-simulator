#include <ctime>
#include <vector>
#include <cstdint>
#include <utility>
#include <algorithm>
#include <iostream>
#include "ram.h"

#define WAIT 9

RAM::RAM() {
    for (int i = 0; i < 0xFFFF; i++) {
        mem[i] = 0;
    }

    PORT_MEM = std::make_pair(0u, 0u);
    PORT_INST1 = std::make_pair(0u, 0u);
    PORT_INST2 = std::make_pair(0u, 0u);
    MEMSTALL = 0;
    INST1STALL = 0;
    INST2STALL = 0;
    PORT_MEM_OP = false;
    PORT_INST1_OP = false;
    PORT_INST2_OP = false;
}

void RAM::initialize(std::vector<int> instructions) {
    writeInstructionsToRAM(instructions);
    std::srand(std::time(0));
    initializeRandomRAM();
    totalTicks = 10;
} 

void RAM::cycle() {
    if(totalTicks == 0) {
        if (PORT_MEM.first != 0 && PORT_MEM_OP == false) {
            if (PORT_MEM.second != 0) {
                if (MEMSTALL == 0) {
                    MEMSTALL = WAIT;
                } else {
                    MEMSTALL--;
                    if(MEMSTALL == 0) {
                        writeToRam(PORT_MEM.first, PORT_MEM.second);
                        PORT_MEM = std::make_pair(0, 0);
                        PORT_MEM_OP = true;
                    }
                }
            } else {
                if (MEMSTALL == 0) {
                    MEMSTALL = WAIT;
                } else {
                    MEMSTALL--;
                    if(MEMSTALL == 0) {
                        PORT_MEM.second = readFromRam(PORT_MEM.first);
                        PORT_MEM_OP = true;
                    }
                }
            }
        }

        //std::cout << "RAM INST1 stall is " << INST1STALL << std::endl;
        //std::cout << "RAM INST1 pair is " << PORT_INST1.first << " " << PORT_INST1.second << std::endl;
        if (PORT_INST1.first != 0 && PORT_INST1_OP == false) {
            if (PORT_INST1.second != 0) {
                if(INST1STALL == 0) {
                    INST1STALL = WAIT;
                } else {
                    INST1STALL--;
                    if(INST1STALL == 0) {
                        writeToRam(PORT_INST1.first - 4, PORT_INST1.second);
                        PORT_INST1 = std::make_pair(0, 0);
                        PORT_INST1_OP = true;
                    }
                }
            } else {
                if(INST1STALL == 0) {
                    INST1STALL = WAIT;
                } else {
                    INST1STALL--;
                    if(INST1STALL == 0) {
                        PORT_INST1.second = readFromRam(PORT_INST1.first - 4);
                        PORT_INST1_OP = true;
                    }
                }
            }
        }

        if (PORT_INST2.first != 0 && PORT_INST2_OP == false) {
            if (PORT_INST2.second != 0) {
                if(INST2STALL == 0) {
                    INST2STALL = WAIT;
                } else {
                    INST2STALL--;
                    if(INST2STALL == 0) {
                        writeToRam(PORT_INST2.first - 4, PORT_INST2.second);
                        PORT_INST2 = std::make_pair(0, 0);
                        PORT_INST2_OP = true;
                    }
                }
            } else {
                if(INST2STALL == 0) {
                    INST2STALL = WAIT;
                } else {
                    INST2STALL--;
                    if(INST2STALL == 0) {
                        PORT_INST2.second = readFromRam(PORT_INST2.first - 4);
                        PORT_INST2_OP = true;
                    }
                }
            }
        }
    } else {
        totalTicks--;
    }
}

void RAM::writeInstructionsToRAM(std::vector<int> instructions) {
    for (int i = 0; i < instructions.size(); i++) {
        writeToRam(i * 4, instructions[i]);
    }
}

void RAM::initializeRandomRAM() {
    for (uint32_t address = 0x400; address <= 0xFFFF; address++) {
        mem[address] = rand() % 256; // Random value between 0 and 255
    }
}

//Functions only for internal use and loading of instructions
uint32_t RAM::readFromRam(uint32_t address) {
    return *reinterpret_cast<uint32_t*>(&mem[address]);
}

void RAM::writeToRam(uint32_t address, uint32_t data) {
    *reinterpret_cast<uint32_t*>(&mem[address]) = data;
}
#include <ctime>
#include <vector>
#include <cstdint>
#include <utility>
#include <algorithm>
#include "ram.h"


class RAM {
    
    public: 
        //For All Pairs:
        //<address, result>
        //If address and result are filled, it is a write
        //If address only is filled, it is a read
        std::pair<uint32_t, uint32_t> PORT_MEM = std::make_pair(0, 0);
        std::pair<uint32_t, uint32_t> PORT_INST1 = std::make_pair(0, 0);
        std::pair<uint32_t, uint32_t> PORT_INST2 = std::make_pair(0, 0);
        uint32_t MEMSTALL = 0;
        uint32_t INST1STALL = 0;
        uint32_t INST2STALL = 0;


        void initialize(std::vector<int> instructions) {
            writeInstructionsToRAM(instructions);
            std::srand(std::time(0));
            initializeRandomRAM();
        } 

        void cycle() {
            if (PORT_MEM.first != 0) {
                if (PORT_MEM.second != 0) {
                    if (MEMSTALL == 0) {
                        MEMSTALL = 20;
                    } else {
                        MEMSTALL--;
                        if(MEMSTALL == 0) {
                            writeToRam(PORT_MEM.first, PORT_MEM.second);
                            PORT_MEM = std::make_pair(0, 0);
                        }
                    }
                } else {
                    if (MEMSTALL == 0) {
                        MEMSTALL = 20;
                    } else {
                        MEMSTALL--;
                        if(MEMSTALL == 0) {
                            PORT_MEM.second = readFromRam(PORT_MEM.first);
                        }
                    }
                }
            }

            if (PORT_INST1.first != 0) {
                if (PORT_INST1.second != 0) {
                    if(INST1STALL == 0) {
                        INST1STALL = 20;
                    } else {
                        INST1STALL--;
                        if(INST1STALL == 0) {
                            writeToRam(PORT_INST1.first, PORT_INST1.second);
                            PORT_INST1 = std::make_pair(0, 0);
                        }
                    }
                } else {
                    if(INST1STALL == 0) {
                        INST1STALL = 20;
                    } else {
                        INST1STALL--;
                        if(INST1STALL == 0) {
                            PORT_INST1.second = readFromRam(PORT_INST1.first);
                        }
                    }
                }
            }

            if (PORT_INST2.first != 0) {
                if (PORT_INST2.second != 0) {
                    if(INST2STALL == 0) {
                        INST2STALL = 20;
                    } else {
                        INST2STALL--;
                        if(INST2STALL == 0) {
                            writeToRam(PORT_INST2.first, PORT_INST2.second);
                            PORT_INST2 = std::make_pair(0, 0);
                        }
                    }
                } else {
                    if(INST2STALL == 0) {
                        INST2STALL = 20;
                    } else {
                        INST2STALL--;
                        if(INST2STALL == 0) {
                            PORT_INST2.second = readFromRam(PORT_INST2.first);
                        }
                    }
                }
            }
        }
        
    private: 
        uint8_t mem[0xFFFF];

        void writeInstructionsToRAM(std::vector<int> instructions) {
            for (int i = 0; i < instructions.size(); i++) {
                writeToRam(i * 4, instructions[i]);
            }
        }

        void initializeRandomRAM() {
            for (uint32_t address = 0x400; address <= 0xFFFF; address++) {
                mem[address] = rand() % 256; // Random value between 0 and 255
            }
        }


        //Functions only for internal use and loading of instructions
        uint32_t readFromRam(uint32_t address) {
            return *reinterpret_cast<uint32_t*>(&mem[address]);
        }

        void writeToRam(uint32_t address, uint32_t data) {
            *reinterpret_cast<uint32_t*>(&mem[address]) = data;
        }
};
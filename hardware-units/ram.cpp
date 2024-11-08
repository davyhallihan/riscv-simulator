#include <ctime>
#include <vector>
#include <cstdint>
#include <utility>
#include <algorithm>
#include <iostream>
#include "ram.h"
#include <string>
#include "../formats/poll.h"
#include "../formats/port.h"

#define WAIT 9

RAM::RAM() {
    for (int i = 0; i < 0xFFFF; i++) {
        mem[i] = 0;
    }

    MEMSTALL = 0;
    INST1STALL = 0;
    INST2STALL = 0;
}

void RAM::printRange(std::string preamble, uint32_t start, uint32_t end) {
    std::cout << preamble << " ";
    for (uint32_t i = start; i < end; i += 4) {
        int val = readFromRam(i);
        std::cout << val << " ";        
    }
    std::cout << std::endl;
}

void RAM::initialize(std::vector<int> instructions1, std::vector<int> instructions2, MEMBUS* commline) {
    writeInstructionsToRAM(instructions1, 0x000);
    writeInstructionsToRAM(instructions2, 0x100);
    std::srand(std::time(0));
    initializeRandomRAM();
    totalTicks = 10;
    bus = commline;
} 

void RAM::cycle() {
    if(totalTicks == 0) {
        if(bus->MEM->msg != nullptr) {
            if(MEMSTALL == 0) {
                MEMSTALL = WAIT;
            }

            MEMSTALL--;
            if(bus->MEM->msg->rw && MEMSTALL == 0) {
                bus->MEM->msg->result = readFromRam(bus->MEM->msg->address);
                bus->MEM->msg->done = true;
                bus->MEM->msg = nullptr;
            } else if(!bus->MEM->msg->rw && MEMSTALL == 0) {
                //std::cout << "Writing to RAM at address " << bus->MEM->msg->address << " data " << bus->MEM->msg->data << std::endl;
                writeToRam(bus->MEM->msg->address, bus->MEM->msg->data);
                bus->MEM->msg->done = true;
                bus->MEM->msg = nullptr;
            }
        }

        if(bus->INST1->msg != nullptr) {
            if(INST1STALL == 0) {
                INST1STALL = WAIT;
            }

            INST1STALL--;
            if(bus->INST1->msg->rw && INST1STALL == 0) {
                bus->INST1->msg->result = readFromRam(bus->INST1->msg->address);
                bus->INST1->msg->done = true;
                bus->INST1->msg = nullptr;
            } else if(!bus->INST1->msg->rw && INST1STALL == 0) {
                writeToRam(bus->INST1->msg->address, bus->INST1->msg->data);
                bus->INST1->msg->done = true;
                bus->INST1->msg = nullptr;
            }
        }

        if(bus->INST2->msg != nullptr) {
            if(INST2STALL == 0) {
                INST2STALL = WAIT;
            }

            INST2STALL--;
            if(bus->INST2->msg->rw && INST2STALL == 0) {
                bus->INST2->msg->result = readFromRam(bus->INST2->msg->address + 0x100);
                bus->INST2->msg->done = true;
                bus->INST2->msg = nullptr;
            } else if(!bus->INST2->msg->rw && INST2STALL == 0) {
                writeToRam(bus->INST2->msg->address + 0x100, bus->INST2->msg->data);
                bus->INST2->msg->done = true;
                bus->INST2->msg = nullptr;
            }
        }
        

    } else {
        totalTicks--;
    }
}

void RAM::writeInstructionsToRAM(std::vector<int> instructions, int start) {
    for (int i = 0; i < instructions.size(); i++) {
        writeToRam((i * 4) + start, instructions[i]);
    }
}

void RAM::initializeRandomRAM() {
    for (uint32_t address = 0x400; address <= 0xBFF; address += 4) {
        writeToRam(address, (rand() % 255) + 1);
        // if((address - 1) % 4) {
        //     mem[address] = rand() % 256; // Random value between 0 and 255
        // }
    }
}

//Functions only for internal use and loading of instructions
uint32_t RAM::readFromRam(uint32_t address) {
    return *reinterpret_cast<uint32_t*>(&mem[address]);
}

void RAM::writeToRam(uint32_t address, uint32_t data) {
    *reinterpret_cast<uint32_t*>(&mem[address]) = data;
}
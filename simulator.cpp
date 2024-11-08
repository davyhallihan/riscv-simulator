#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <bitset>
#include "hardware-units/ram.h"
#include "timing-utilities/clock.h"
#include "hardware-units/cpu.cpp"

CPUCLOCK *cpuclock = new CPUCLOCK();
RAM *memory = new RAM();
CPU *cpu1 = new CPU();
CPU *cpu2 = new CPU();
MEMBUS *bus = new MEMBUS();



std::vector<int> instructionsFromFile(const std::string& filename) {
    std::vector<int> instructions;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cout << "Error: Could not open file " << filename << std::endl;
        return instructions;
    }
    
    while(file) {
        std::string line;
        std::getline(file, line);

        //skip short lines
        if (line.size() < 32) { continue; }
        
        //erase whitespace
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        line = line.substr(0, 32);

        //convert binary string to int and pushback
        uint32_t instruction = std::stoul(line, nullptr, 2);
        instructions.push_back(instruction);
    }

    return instructions;
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 4) { std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl; return 1; }
    std::string filename1 = argv[1];
    std::string filename2 = argv[2];
    std::cout << "Initializing Simulator With Filenames: " << filename1 << " " << filename2 << std::endl;

    std::vector<int> instructions1 = instructionsFromFile(filename1);
    std::vector<int> instructions2 = instructionsFromFile(filename2);
    std::cout << "File 1, " << filename1 << ": " << instructions1.size() << " instructions" << std::endl;
    std::cout << "File 2, " << filename2 << ": " << instructions2.size() << " instructions" << std::endl;

    memory->initialize(instructions1, instructions2, bus);
    cpu1->initialize(bus, 0);
    cpu2->initialize(bus, 1);

    //UNIT TEST TO VERIFY INITIALIZATION WORKING PROPERLY
    // for(int i = 0; i < 4*21; i += 4) {
    //     uint32_t value = *reinterpret_cast<uint32_t*>(&memory->mem[i]);
    //     std::cout << std::bitset<32>(value) << " vs " << std::bitset<32>(instructions[i/4]) << std::endl;
    // }
    bool done1 = false;
    bool done2 = false;
    while(!done1) { //|| !done2) {
        if(!done1) {
            done1 = cpu1->cycle();
            cpu1->updateCycles(cpuclock->getClock());
        }
        if(!done2){
            done2 = cpu2->cycle();
            cpu2->updateCycles(cpuclock->getClock());
        }
        bus->cycle();
        cpuclock->increment();
        memory->cycle();
        if(!(cpuclock->getClock() % 100)) {
            //std::cout << cpu1->PC << " " << cpu2->PC << std::endl;
        }
    }

    std::cout << "Simulation Complete" << std::endl;
    memory->printRange("Array A: ", uint32_t(0x0400), uint32_t(0x0400 + 255*4));
    memory->printRange("Array B: ", uint32_t(0x0800), uint32_t(0x0800 + 255*4));
    memory->printRange("Array C: ", uint32_t(0x0C00), uint32_t(0x0C00 + 255*4));
    memory->printRange("Array D: ", uint32_t(0x1000), uint32_t(0x1000 + 255*4));
    

    return 0;
}



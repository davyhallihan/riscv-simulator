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

bool printDiagnostics = true;



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

    memory->printd = printDiagnostics;
    cpu1->printd = printDiagnostics;
    cpu2->printd = printDiagnostics;

    bool done1 = false;
    bool done2 = false;
    while(!done1 || !done2) {
        if(printDiagnostics && !(cpuclock->getClock() % 10)) { std::cout << "#################################################################" << std::endl; }
        //if(printDiagnostics && !(cpuclock->getClock() % 10)) { std::cout << "Overall Simulation - Simtick #" << cpuclock->getClock() << std::endl; }
        if(printDiagnostics && !(cpuclock->getClock() % 10)) { std::cout << "Overall Simulation - CPU Cycle #" << int(cpuclock->getClock() / 10) << std::endl; }
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
        if(printDiagnostics && !(cpuclock->getClock() % 10)) { std::cout << "#################################################################" << std::endl << std::endl; }
    }

    std::cout << "Simulation Complete" << std::endl;
    std::cout << "CPU0 Instruction Count: " << cpu1->instructioncount << std::endl;
    std::cout << "CPU1 Instruction Count: " << cpu2->instructioncount << std::endl;
    std::cout << "CPU0 Cycles: " << cpu1->cycles << std::endl;
    std::cout << "CPU1 Cycles: " << cpu2->cycles << std::endl;
    std::cout << "CPU0 CPI " << (float)cpu1->cycles / cpu1->instructioncount << std::endl;
    std::cout << "CPU1 CPI " << (float)cpu2->cycles / cpu2->instructioncount << std::endl;
    std::cout << std::endl;
    memory->printRange("Array A: ", uint32_t(0x0400), uint32_t(0x0400 + 255*4));
    memory->printRange("Array B: ", uint32_t(0x0800), uint32_t(0x0800 + 255*4));
    memory->printRange("Array C: ", uint32_t(0x0C00), uint32_t(0x0C00 + 255*4));
    memory->printRange("Array D: ", uint32_t(0x1000), uint32_t(0x1000 + 255*4));

    std::cout << std::endl << "Validating arrays C and D with vector addition: " << std::endl;

    
    

    return 0;
}



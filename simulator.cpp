#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>
#include "ram.h"
//#include <cpu.h>

RAM *memory = new RAM();
// CPU *cpu1 = new CPU();
// CPU *cpu2 = new CPU();
// MEMBUS *bus = new MEMBUS();



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
    if (argc < 2 || argc > 3) { std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl; return 1; }
    std::string filename = argv[1];
    std::cout << "Initializing Simulator With Filename: " << filename << std::endl;

    std::vector<int> instructions = instructionsFromFile(filename);
    std::cout << "Found: " << instructions.size() << " instructions" << std::endl;
    
    memory->initialize(instructions);
    

    return 0;
}



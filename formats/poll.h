#ifndef POLL_H
#define POLL_H

#include <cstdint>
#include <string>

struct POLL {
    std::string dest;
    bool rw; //true = read, false = write
    uint32_t address;
    uint32_t data;
    uint32_t result;
    bool done;
};

#endif
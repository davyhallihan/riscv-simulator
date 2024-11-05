#include <iostream>
#include "clock.h"

CPUCLOCK::CPUCLOCK() : clock(0) {}

void CPUCLOCK::increment() {
    clock++;
}

int CPUCLOCK::getClock() {
    return clock;
}
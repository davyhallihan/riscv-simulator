#include <iostream>
#include "clock.h"

class Clock {
private:
    int clock;

public:
    Clock() : clock(0) {}

    void increment() {
        clock++;
    }

    int getClock() const {
        return clock;
    }
};
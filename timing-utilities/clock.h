#ifndef CLOCK_H
#define CLOCK_H

class CPUCLOCK {
public:
    CPUCLOCK(); // Constructor
    void increment(); // Set the time
    int getClock(); // Get the time

private:
    int clock;
};

#endif // CLOCK_H
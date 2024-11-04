#ifndef CLOCK_H
#define CLOCK_H

class Clock {
public:
    Clock(); // Constructor
    void increment(); // Set the time
    int getClock(); // Get the time

private:
    int clock;
};

#endif // CLOCK_H
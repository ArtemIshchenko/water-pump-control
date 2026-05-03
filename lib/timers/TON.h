#pragma once
#include <Arduino.h>

class TON {
public:
    TON(unsigned long pt = 0);

    void setPT(unsigned long pt);
    void update(bool in);

    bool Q() const;          // вихід
    unsigned long ET() const; // elapsed time

    void reset();

private:
    unsigned long _pt;
    unsigned long _startTime = 0;
    unsigned long _et = 0;

    bool _in = false;
    bool _q = false;
    bool _timing = false;
};

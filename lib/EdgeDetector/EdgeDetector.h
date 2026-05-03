#pragma once
#include <Arduino.h>

class EdgeDetector {
public:
    bool rose = false;
    bool fell = false;

    EdgeDetector(bool initValue = false);

    void update(bool value);
    bool value() const;

private:
    bool _prev;
    bool _current;
};

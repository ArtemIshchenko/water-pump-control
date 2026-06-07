#pragma once

#include <Arduino.h>

class DisplayLink {
public:
    void init();
    bool readCommand(String& cmd);

    void sendSleep(bool sleeping);
    void sendWaterPumpStatus(bool pumpOn, bool waterMax, int baths);
    void sendPressureLimits(float limitMin, float limitMax);
    void flush();
};

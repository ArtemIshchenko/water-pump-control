#pragma once
#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>

class McuManage {
public:
    void init(int wakePin, unsigned long inactivityTimeoutMs);
    void registerActivity();
    bool wasWokenBySwitch() const;
    bool shouldSleep(bool canSleep) const;
    void enterDeepSleep();

private:
    int _wakePin = -1;
    unsigned long _inactivityTimeoutMs = 0;
    unsigned long _lastActivityTime = 0;
};

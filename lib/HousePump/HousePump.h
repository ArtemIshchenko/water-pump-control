#pragma once
#include <Arduino.h>

class HousePump {
public:
    void init(float defaultLimitMin = 1.0f, float defaultLimitMax = 3.0f);
    void init(int outputPin,
              int pressureSensorPin,
              float defaultLimitMin = 1.0f,
              float defaultLimitMax = 3.0f,
              int adcMax = 4095,
              float sensorVoltageMax = 5.0f);
    void setSettings(float limitMin, float limitMax);
    void setSensorScale(float voltageMin, float voltageMax, float pressureMin, float pressureMax);
    void setOutDebounce(unsigned long debounceMs);

    float pressure() const;
    int adc() const;
    float limitMin() const;
    float limitMax() const;
    bool out() const;

    bool update();
    void updatePressureFromVoltage(float sensorVoltage);
    void updatePressureFromAdc(int adcValue, int adcMax = 4095, float sensorVoltageMax = 5.0f);
    void updatePressure(float pressure);
    bool parsePressureLimits(const String& cmd, float& limitMin, float& limitMax);
    bool handleDisplayCommand(const String& cmd);

private:
    float _pressure = 0.0f;
    float _voltageMin = 0.0f;
    float _voltageMax = 5.0f;
    float _pressureMin = 0.0f;
    float _pressureMax = 6.0f;
    int _outputPin = -1;
    int _pressureSensorPin = -1;
    int _adc = 0;
    int _adcMax = 4095;
    float _sensorVoltageMax = 3.0f;
    float _limitMin = 1.2f;
    float _limitMax = 2.4f;
    bool _out = false;
    bool _outDebounceTiming = false;
    unsigned long _outDebounceStart = 0;
    unsigned long _outDebounceMs = 500;

    void updateOutput();
    void writeOutput();
};

#include "HousePump.h"
#include <Preferences.h>

namespace {
const char* SETTINGS_NAMESPACE = "housePump";
const char* LIMIT_MIN_KEY = "limitMin";
const char* LIMIT_MAX_KEY = "limitMax";
}

static float clampFloat(float value, float minValue, float maxValue) {
    if (value < minValue) {
        return minValue;
    }
    if (value > maxValue) {
        return maxValue;
    }
    return value;
}

void HousePump::init(float defaultLimitMin, float defaultLimitMax) {
    Preferences preferences;

    if (!preferences.begin(SETTINGS_NAMESPACE, false)) {
        _limitMin = defaultLimitMin;
        _limitMax = defaultLimitMax;
        Serial.println("HousePump settings: failed to open storage, using defaults");
        return;
    }

    _limitMin = preferences.getFloat(LIMIT_MIN_KEY, defaultLimitMin);
    _limitMax = preferences.getFloat(LIMIT_MAX_KEY, defaultLimitMax);
    preferences.end();
}

void HousePump::init(int outputPin,
                     int pressureSensorPin,
                     float defaultLimitMin,
                     float defaultLimitMax,
                     int adcMax,
                     float sensorVoltageMax) {
    _outputPin = outputPin;
    _pressureSensorPin = pressureSensorPin;
    _adcMax = adcMax;
    _sensorVoltageMax = sensorVoltageMax;

    pinMode(_outputPin, OUTPUT);
    pinMode(_pressureSensorPin, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(_pressureSensorPin, ADC_11db);
    writeOutput();

    init(defaultLimitMin, defaultLimitMax);
}

void HousePump::setSettings(float limitMin, float limitMax) {
    if (_limitMin == limitMin && _limitMax == limitMax) {
        return;
    }

    _limitMin = limitMin;
    _limitMax = limitMax;

    Preferences preferences;
    if (!preferences.begin(SETTINGS_NAMESPACE, false)) {
        Serial.println("HousePump settings: failed to open storage for write");
        return;
    }

    preferences.putFloat(LIMIT_MIN_KEY, _limitMin);
    preferences.putFloat(LIMIT_MAX_KEY, _limitMax);
    preferences.end();
}

void HousePump::setSensorScale(float voltageMin, float voltageMax, float pressureMin, float pressureMax) {
    if (voltageMin == voltageMax) {
        return;
    }

    _voltageMin = voltageMin;
    _voltageMax = voltageMax;
    _pressureMin = pressureMin;
    _pressureMax = pressureMax;
}

void HousePump::setOutDebounce(unsigned long debounceMs) {
    _outDebounceMs = debounceMs;
    _outDebounceTiming = false;
}

float HousePump::pressure() const {
    return _pressure;
}

int HousePump::adc() const {
    return _adc;
}

float HousePump::limitMin() const {
    return _limitMin;
}

float HousePump::limitMax() const {
    return _limitMax;
}

bool HousePump::out() const {
    return _out;
}

bool HousePump::update() {
    if (_pressureSensorPin < 0) {
        return false;
    }

    bool previousOut = _out;
    _adc = analogRead(_pressureSensorPin);
    updatePressureFromAdc(_adc, _adcMax, _sensorVoltageMax);
    updateOutput();

    return _out != previousOut;
}

bool HousePump::parsePressureLimits(const String& cmd, float& limitMin, float& limitMax) {
    String payload;

    if (cmd.startsWith("CMD:PRESSURE_LIMITS:")) {
        payload = cmd.substring(20);
    } else if (cmd.startsWith("CMD:LIMITS:")) {
        payload = cmd.substring(11);
    } else {
        return false;
    }

    int commaIndex = payload.indexOf(',');
    if (commaIndex < 0) {
        return false;
    }

    String minText = payload.substring(0, commaIndex);
    String maxText = payload.substring(commaIndex + 1);
    minText.trim();
    maxText.trim();

    char* minEnd = nullptr;
    char* maxEnd = nullptr;
    limitMin = strtof(minText.c_str(), &minEnd);
    limitMax = strtof(maxText.c_str(), &maxEnd);

    return minEnd != minText.c_str() &&
           maxEnd != maxText.c_str() &&
           *minEnd == '\0' &&
           *maxEnd == '\0' &&
           limitMin >= 0.0f &&
           limitMax > limitMin;
}

void HousePump::updatePressureFromVoltage(float sensorVoltage) {
    float scale = (sensorVoltage - _voltageMin) / (_voltageMax - _voltageMin);
    scale = clampFloat(scale, 0.0f, 1.0f);

    updatePressure(_pressureMin + scale * (_pressureMax - _pressureMin));
}

bool HousePump::handleDisplayCommand(const String& cmd) {
    float limitMin = 0.0f;
    float limitMax = 0.0f;

    if (parsePressureLimits(cmd, limitMin, limitMax)) {
        setSettings(limitMin, limitMax);
        Serial.printf("Pressure limits saved: %.2f / %.2f\n", limitMin, limitMax);
        return true;
    } else if (cmd == "CMD:GET_PRESSURE_LIMITS" || cmd == "CMD:GET_LIMITS") {
        return true;
    }

    return false;
}

void HousePump::updatePressureFromAdc(int adcValue, int adcMax, float sensorVoltageMax) {
    if (adcMax <= 0) {
        return;
    }

    float scale = static_cast<float>(adcValue) / static_cast<float>(adcMax);
    scale = clampFloat(scale, 0.0f, 1.0f);

    updatePressureFromVoltage(scale * sensorVoltageMax);
}

void HousePump::updatePressure(float pressure) {
    _pressure = pressure;
}

void HousePump::updateOutput() {
    bool desiredOut = _out;

    if (_out) {
        if (_pressure >= _limitMax) {
            desiredOut = false;
        }
    } else {
        if (_pressure <= _limitMin) {
            desiredOut = true;
        }
    }

    if (desiredOut == _out) {
        _outDebounceTiming = false;
        return;
    }

    unsigned long now = millis();

    if (!_outDebounceTiming) {
        _outDebounceStart = now;
        _outDebounceTiming = true;
    }

    if (now - _outDebounceStart >= _outDebounceMs) {
        _out = desiredOut;
        _outDebounceTiming = false;
        writeOutput();
    }
}

void HousePump::writeOutput() {
    if (_outputPin >= 0) {
        digitalWrite(_outputPin, _out ? HIGH : LOW);
    }
}

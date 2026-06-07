#include "WaterSupplyPump.h"

void WaterSupplyPump::init(int outputPin,
                           int manualCommandPin,
                           int isOnPin,
                           int waterLevelSensor1Pin,
                           int waterLevelSensor2Pin,
                           int waterLevelSensor3Pin,
                           unsigned long maxRunTimeMs,
                           unsigned long isOnTimeoutMs,
                           unsigned long sensorDebounceMs) {
    _outputPin = outputPin;
    _manualCommandPin = manualCommandPin;
    _isOnPin = isOnPin;
    _waterLevelSensor1Pin = waterLevelSensor1Pin;
    _waterLevelSensor2Pin = waterLevelSensor2Pin;
    _waterLevelSensor3Pin = waterLevelSensor3Pin;

    pinMode(_outputPin, OUTPUT);
    pinMode(_manualCommandPin, INPUT_PULLUP);
    pinMode(_isOnPin, INPUT_PULLUP);
    pinMode(_waterLevelSensor1Pin, INPUT_PULLUP);
    pinMode(_waterLevelSensor2Pin, INPUT_PULLUP);
    pinMode(_waterLevelSensor3Pin, INPUT_PULLUP);

    _isOnPinDebounce.setPT(sensorDebounceMs);
    _isOnDebounceOff.setPT(sensorDebounceMs);
    _manualCommandDebounce.setPT(sensorDebounceMs);
    _manualCommandDebounceOff.setPT(sensorDebounceMs);
    _sensorDebounce1.setPT(sensorDebounceMs);
    _sensorDebounce2.setPT(sensorDebounceMs);
    _sensorDebounce3.setPT(sensorDebounceMs);
    _isOnTimer.setPT(isOnTimeoutMs);
    _maxRunTimer.setPT(maxRunTimeMs);

    _manualCommandEdge = EdgeDetector(false);
    _waterLevelSensor1Edge = EdgeDetector(false);
    _waterLevelSensor2Edge = EdgeDetector(false);
    _waterLevelSensor3Edge = EdgeDetector(false);

    _firstScan = true;

    writeOutput();
}

void WaterSupplyPump::setWakeCommand() {
    _wakeCommand = true;
}

bool WaterSupplyPump::update(bool displayOnCommand, bool displayOffCommand) {
    bool manualCommand = digitalRead(_manualCommandPin) == LOW;
    bool isOn = digitalRead(_isOnPin) == HIGH;
    bool newWaterLevelMaxBaths1 = digitalRead(_waterLevelSensor1Pin) == HIGH;
    bool newWaterLevelMaxBaths2 = digitalRead(_waterLevelSensor2Pin) == HIGH;
    bool newWaterLevelMaxBaths3 = digitalRead(_waterLevelSensor3Pin) == HIGH;

    _isOnPinDebounce.update(isOn);
    _isOnDebounceOff.update(_isOnPinDebounce.Q());
    _manualCommandDebounce.update(manualCommand);
    _manualCommandDebounceOff.update(_manualCommandDebounce.Q());
    _sensorDebounce1.update(newWaterLevelMaxBaths1);
    _sensorDebounce2.update(newWaterLevelMaxBaths2);
    _sensorDebounce3.update(newWaterLevelMaxBaths3);

    _manualCommandEdge.update(_manualCommandDebounceOff.Q());
    _waterLevelSensor1Edge.update(_sensorDebounce1.Q());
    _waterLevelSensor2Edge.update(_sensorDebounce2.Q());
    _waterLevelSensor3Edge.update(_sensorDebounce3.Q());
    bool isOnConfirmed = _isOnDebounceOff.Q(); // NC: pin HIGH (contact open) = contactor energized = pump confirmed

    bool activity = _manualCommandEdge.rose || _manualCommandEdge.fell ||
                    _waterLevelSensor1Edge.rose || _waterLevelSensor1Edge.fell ||
                    _waterLevelSensor2Edge.rose || _waterLevelSensor2Edge.fell ||
                    _waterLevelSensor3Edge.rose || _waterLevelSensor3Edge.fell;

    if (_waterLevelSensor1Edge.rose) {
        _waterLevelMaxBaths1 = true;
        Serial.println("Water MAX");
    } else if (_waterLevelSensor1Edge.fell) {
        _waterLevelMaxBaths1 = false;
        Serial.println("Water OK");
    }

    if (_waterLevelSensor2Edge.rose) {
        _waterLevelMaxBaths2 = true;
        Serial.println("Water MAX");
    } else if (_waterLevelSensor2Edge.fell) {
        _waterLevelMaxBaths2 = false;
        Serial.println("Water OK");
    }

    if (_waterLevelSensor3Edge.rose) {
        _waterLevelMaxBaths3 = true;
        Serial.println("Water MAX");
    } else if (_waterLevelSensor3Edge.fell) {
        _waterLevelMaxBaths3 = false;
        Serial.println("Water OK");
    }

    // _isOnTimer: рахує поки насос увімкнений АЛЕ не підтверджений
    _isOnTimer.update(_out && !isOnConfirmed);
    // _maxRunTimer: рахує поки насос підтверджено працює → захист від перевищення часу
    _maxRunTimer.update(_out && isOnConfirmed);

    bool offCommand = displayOffCommand || _manualCommandEdge.fell;
    bool onCommand = ((displayOnCommand || _manualCommandEdge.rose) && !_firstScan) || _wakeCommand;
    bool waterMaxCommand = _waterLevelSensor1Edge.rose ||
                           _waterLevelSensor2Edge.rose ||
                           _waterLevelSensor3Edge.rose;
    bool isOnTimeout = _out && _isOnTimer.Q();
    bool maxRunTimeout = _out && _maxRunTimer.Q();


    if (offCommand || waterMaxCommand || isOnTimeout || maxRunTimeout) {
        bool wasOut = _out;
        _out = false;
        writeOutput();

        if (displayOffCommand) {
            Serial.println("Pump OFF — display");
        }
        if (_manualCommandEdge.fell) {
            Serial.println("Pump OFF — button");
        }
        if (waterMaxCommand) {
            Serial.println("Pump OFF — water max");
            _waterLevelMaxBaths1 = false;
            _waterLevelMaxBaths2 = false;
            _waterLevelMaxBaths3 = false;
        }
        if (isOnTimeout) {
            Serial.println("Pump OFF — isOn timeout");
        }
        if (maxRunTimeout) {
            Serial.println("Pump OFF — max run time");
        }

        _isOnTimer.reset();
        _maxRunTimer.reset();
        activity = activity || wasOut;
    } else if (onCommand && !_out) {
        _out = true;
        writeOutput();

        if (displayOnCommand) {
            Serial.println("Pump ON — display");
        }
        if (_manualCommandEdge.rose) {
            Serial.println("Pump ON — button");
        }
        if (_wakeCommand) {
            Serial.println("Pump ON — wake switch");
        }

        _wakeCommand = false;
        activity = true;
    }
    _firstScan = false;

    return activity;
}

bool WaterSupplyPump::out() const {
    return _out;
}

bool WaterSupplyPump::waterMax() const {
    return _waterLevelMaxBaths1 || _waterLevelMaxBaths2 || _waterLevelMaxBaths3;
}

int WaterSupplyPump::baths() const {
    return _baths;
}

void WaterSupplyPump::writeOutput() {
    if (_outputPin >= 0) {
        digitalWrite(_outputPin, _out ? HIGH : LOW);
    }
}

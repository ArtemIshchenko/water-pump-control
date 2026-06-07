#pragma once
#include <Arduino.h>
#include "EdgeDetector.h"
#include "TOF.h"
#include "TON.h"

class WaterSupplyPump {
public:
    void init(int outputPin,
              int manualCommandPin,
              int isOnPin,
              int waterLevelSensor1Pin,
              int waterLevelSensor2Pin,
              int waterLevelSensor3Pin,
              unsigned long maxRunTimeMs,
              unsigned long isOnTimeoutMs = 5000,
              unsigned long sensorDebounceMs = 400);

    void setWakeCommand();
    bool update(bool displayOnCommand, bool displayOffCommand);

    bool out() const;
    bool waterMax() const;
    int baths() const;

private:
    int _outputPin = -1;
    int _manualCommandPin = -1;
    int _isOnPin = -1;
    int _waterLevelSensor1Pin = -1;
    int _waterLevelSensor2Pin = -1;
    int _waterLevelSensor3Pin = -1;

    bool _out = false;
    bool _waterLevelMaxBaths1 = false;
    bool _waterLevelMaxBaths2 = false;
    bool _waterLevelMaxBaths3 = false;
    bool _wakeCommand = false;
    bool _firstScan = false;
    int _baths = 1;

    EdgeDetector _manualCommandEdge;
    EdgeDetector _waterLevelSensor1Edge;
    EdgeDetector _waterLevelSensor2Edge;
    EdgeDetector _waterLevelSensor3Edge;

    TON _manualCommandDebounce;
    TOF _manualCommandDebounceOff;
    TON _isOnPinDebounce;
    TOF _isOnDebounceOff;
    TON _sensorDebounce1;
    TON _sensorDebounce2;
    TON _sensorDebounce3;
    TON _isOnTimer;
    TON _maxRunTimer;

    void writeOutput();
};

#include "PumpSystem.h"
#include <Config.h>

void PumpSystem::init() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("=== START ===");

    _display.init();

    _mcuManage.init(MAN_COMMAND_WATER_PUMP_PIN, INACTIVITY_SLEEP_TIME_MS);

    _housePump.init(HOUSE_PUMP_ON_PIN, HOUSE_PRESSURE_SENSOR_PIN, 1.2f, 2.4f, 4095, 3.3f);
    _housePump.setSensorScale(0.0f, 2.5f, 0.0f, 6.9f);
    _waterSupplyPump.init(WATER_PUMP_ON_PIN,
                          MAN_COMMAND_WATER_PUMP_PIN,
                          WATER_PUMP_IS_ON_PIN,
                          WATER_LEVEL_SENSOR1_PIN,
                          WATER_LEVEL_SENSOR2_PIN,
                          WATER_LEVEL_SENSOR3_PIN,
                          WATER_PUMP_MAX_RUN_TIME_MS,
                          WATER_PUMP_IS_ON_TIMEOUT_MS);

    if (_mcuManage.wasWokenBySwitch()) {
        _waterSupplyPump.setWakeCommand();
        Serial.println("Wake up — switch command detected");
    }

    _display.sendSleep(false);
    _display.sendWaterPumpStatus(_waterSupplyPump.out(),
                                 _waterSupplyPump.waterMax(),
                                 _waterSupplyPump.baths());
    _display.sendPressureLimits(_housePump.limitMin(), _housePump.limitMax());
}

void PumpSystem::update() {
    if (_housePump.update()) {
        Serial.printf("House pump %s, pressure=%.2f bar, adc=%d\n",
                      _housePump.out() ? "ON" : "OFF",
                      _housePump.pressure(),
                      _housePump.adc());
        _mcuManage.registerActivity();
    }

    String cmd = "";
    if (_display.readCommand(cmd)) {
        if (_housePump.handleDisplayCommand(cmd)) {
            _display.sendPressureLimits(_housePump.limitMin(), _housePump.limitMax());
            _mcuManage.registerActivity();
        }
    }

    if (_waterSupplyPump.update(cmd == "CMD:ON", cmd == "CMD:OFF")) {
        _mcuManage.registerActivity();
        _display.sendWaterPumpStatus(_waterSupplyPump.out(),
                                     _waterSupplyPump.waterMax(),
                                     _waterSupplyPump.baths());
    }

    bool canSleep = !_waterSupplyPump.out() &&
                    !_housePump.out() &&
                    digitalRead(MAN_COMMAND_WATER_PUMP_PIN) == HIGH;
    if (_mcuManage.shouldSleep(canSleep)) {
        _display.sendSleep(true);
        _display.flush();
        _mcuManage.enterDeepSleep();
    }

    delay(100);
}

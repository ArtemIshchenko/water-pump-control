#include "DisplayLink.h"
#include <Config.h>

void DisplayLink::init() {
    Serial2.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
}

bool DisplayLink::readCommand(String& cmd) {
    if (!Serial2.available()) {
        return false;
    }

    cmd = Serial2.readStringUntil('\n');
    cmd.trim();
    Serial.println("CMD: " + cmd);
    return true;
}

void DisplayLink::sendSleep(bool sleeping) {
    Serial2.printf("SLEEP:%d\n", sleeping ? 1 : 0);
}

void DisplayLink::sendWaterPumpStatus(bool pumpOn, bool waterMax, int baths) {
    Serial2.printf("PUMP:%d,WATER:%d,BATHS:%d\n",
                   pumpOn ? 1 : 0,
                   waterMax ? 1 : 0,
                   baths);
    Serial.printf("Sent: PUMP:%d,WATER:%d,BATHS:%d\n",
                  pumpOn ? 1 : 0,
                  waterMax ? 1 : 0,
                  baths);
}

void DisplayLink::sendPressureLimits(float limitMin, float limitMax) {
    Serial2.printf("PRESSURE_LIMITS:%.2f,%.2f\n", limitMin, limitMax);
    Serial.printf("Sent: PRESSURE_LIMITS:%.2f,%.2f\n", limitMin, limitMax);
}

void DisplayLink::flush() {
    Serial2.flush();
}

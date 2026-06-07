#include <Arduino.h>
#include "PumpSystem.h"

PumpSystem app;

void setup() {
    app.init();
}

void loop() {
    app.update();
}

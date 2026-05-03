#include <Arduino.h>
#include <Preferences.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include "TOF.h"
#include "EdgeDetector.h"

// -----------------------------------------------------
// ПІНИ
// -----------------------------------------------------
#define WATER_PUMP_ON_PIN           13
#define MAN_COMMAND_WATER_PUMP_PIN  12
#define WATER_LEVEL_SENSOR1_PIN     14
#define WATER_LEVEL_SENSOR2_PIN     27
#define WATER_LEVEL_SENSOR3_PIN     26

#define INACTIVITY_SLEEP_TIME_MS    (1000UL * 60UL * 15UL)

// UART до дисплея
#define UART_TX_PIN 17
#define UART_RX_PIN 16

// -----------------------------------------------------
// ЗМІННІ
// -----------------------------------------------------
bool pumpState     = false;
bool waterLevelMaxBaths1 = false;
bool waterLevelMaxBaths2 = false;
bool waterLevelMaxBaths3 = false;
int baths = 1;
unsigned long lastActivityTime = 0;
bool wakePumpCommand = false;

// Детектори для всіх джерел керування
EdgeDetector manCommandEdge;   // фізична кнопка
EdgeDetector btnOnEdge;        // кнопка ON з дисплея
EdgeDetector btnOffEdge;       // кнопка OFF з дисплея
EdgeDetector waterLevelSensor1Edge;
EdgeDetector waterLevelSensor2Edge;
EdgeDetector waterLevelSensor3Edge;

// -----------------------------------------------------
// ВІДПРАВКА СТАНУ НА ДИСПЛЕЙ
// -----------------------------------------------------
void sendStatus() {
    Serial2.printf("PUMP:%d,WATER:%d,BATHS:%d\n",
                   pumpState ? 1 : 0,
                   waterLevelMaxBaths1 || waterLevelMaxBaths2 || waterLevelMaxBaths3 ? 1 : 0,
                   baths);
    Serial.printf("Sent: PUMP:%d,WATER:%d,BATHS:%d\n",
                  pumpState ? 1 : 0,
                  waterLevelMaxBaths1 || waterLevelMaxBaths2 || waterLevelMaxBaths3 ? 1 : 0,
                  baths);
}

// Таймер на відключення насосу
TOF pumpTOF(1000 * 60 * 15); // 15 хв.

// -----------------------------------------------------
// СОН / ПРОБУДЖЕННЯ
// -----------------------------------------------------
void enterDeepSleep() {
    Serial.println("Going to deep sleep");
    sendStatus();
    Serial.flush();
    Serial2.flush();

    rtc_gpio_init((gpio_num_t)MAN_COMMAND_WATER_PUMP_PIN);
    rtc_gpio_set_direction((gpio_num_t)MAN_COMMAND_WATER_PUMP_PIN, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pullup_en((gpio_num_t)MAN_COMMAND_WATER_PUMP_PIN);
    rtc_gpio_pulldown_dis((gpio_num_t)MAN_COMMAND_WATER_PUMP_PIN);

    esp_sleep_enable_ext0_wakeup((gpio_num_t)MAN_COMMAND_WATER_PUMP_PIN, 0); // wake when switch pulls pin LOW
    esp_deep_sleep_start();
}

// -----------------------------------------------------
// SETUP
// -----------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("=== START ===");

    // UART до дисплея
    Serial2.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

    pinMode(WATER_PUMP_ON_PIN, OUTPUT);
    pinMode(MAN_COMMAND_WATER_PUMP_PIN, INPUT_PULLUP);
    pinMode(WATER_LEVEL_SENSOR1_PIN, INPUT_PULLUP);
    pinMode(WATER_LEVEL_SENSOR2_PIN, INPUT_PULLUP);
    pinMode(WATER_LEVEL_SENSOR3_PIN, INPUT_PULLUP);

    lastActivityTime = millis();

    // Ініціалізація EdgeDetector з поточними станами
    manCommandEdge = EdgeDetector(digitalRead(MAN_COMMAND_WATER_PUMP_PIN));
    btnOnEdge      = EdgeDetector(false);
    btnOffEdge     = EdgeDetector(false);
    waterLevelSensor1Edge = EdgeDetector(digitalRead(WATER_LEVEL_SENSOR1_PIN));
    waterLevelSensor2Edge = EdgeDetector(digitalRead(WATER_LEVEL_SENSOR2_PIN));
    waterLevelSensor3Edge = EdgeDetector(digitalRead(WATER_LEVEL_SENSOR3_PIN));

    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0 &&
        digitalRead(MAN_COMMAND_WATER_PUMP_PIN) == LOW) {
        wakePumpCommand = true;
        Serial.println("Wake up — switch command detected");
    }

    // Відправити початковий стан
    sendStatus();

    Serial.println("Setup done");
}

// -----------------------------------------------------
// ГОЛОВНИЙ ЦИКЛ
// -----------------------------------------------------
void loop() {
    // -----------------------------------------------------
    // ЧИТАЄМО ВСІ ДЖЕРЕЛА
    // -----------------------------------------------------
    bool manCmd    = digitalRead(MAN_COMMAND_WATER_PUMP_PIN);
    bool newWaterLevelMaxBaths1 = digitalRead(WATER_LEVEL_SENSOR1_PIN);
    bool newWaterLevelMaxBaths2 = digitalRead(WATER_LEVEL_SENSOR2_PIN);
    bool newWaterLevelMaxBaths3 = digitalRead(WATER_LEVEL_SENSOR3_PIN);

    // Команда з дисплея
    String cmd = "";
    if (Serial2.available()) {
        cmd = Serial2.readStringUntil('\n');
        cmd.trim();
        Serial.println("CMD: " + cmd);
        lastActivityTime = millis();
    }

    // -----------------------------------------------------
    // ОНОВЛЮЄМО ВСІ ДЕТЕКТОРИ ФРОНТІВ
    // -----------------------------------------------------
    manCommandEdge.update(manCmd);
    btnOnEdge.update(cmd == "CMD:ON");
    btnOffEdge.update(cmd == "CMD:OFF");
    waterLevelSensor1Edge.update(newWaterLevelMaxBaths1);
    waterLevelSensor2Edge.update(newWaterLevelMaxBaths2);
    waterLevelSensor3Edge.update(newWaterLevelMaxBaths3);

    if (manCommandEdge.rose || manCommandEdge.fell ||
        waterLevelSensor1Edge.rose || waterLevelSensor1Edge.fell ||
        waterLevelSensor2Edge.rose || waterLevelSensor2Edge.fell ||
        waterLevelSensor3Edge.rose || waterLevelSensor3Edge.fell) {
        lastActivityTime = millis();
    }

    bool changed = false;

    if (waterLevelSensor1Edge.rose) {
        waterLevelMaxBaths1 = true;
        Serial.println("Water MAX");
        changed = true;
    } else if (waterLevelSensor1Edge.fell) {
        waterLevelMaxBaths1 = false;
        Serial.println("Water OK");
        changed = true;
    }
    if (waterLevelSensor2Edge.rose) {
        waterLevelMaxBaths2 = true;
        Serial.println("Water MAX");
        changed = true;
    } else if (waterLevelSensor2Edge.fell) {
        waterLevelMaxBaths2 = false;
        Serial.println("Water OK");
        changed = true;
    }
    if (waterLevelSensor3Edge.rose) {
        waterLevelMaxBaths3 = true;
        Serial.println("Water MAX");
        changed = true;
    } else if (waterLevelSensor3Edge.fell) {
        waterLevelMaxBaths3 = false;
        Serial.println("Water OK");
        changed = true;
    }

    // Таймер відключення насосу
    bool runCondition = !(waterLevelMaxBaths1 || waterLevelMaxBaths2 || waterLevelMaxBaths3);
    pumpTOF.update(runCondition);

    // Вимкнути насос — фізична кнопка або дисплей, Аварійне вимкнення — датчик досяг максимуму
    if (btnOffEdge.rose || manCommandEdge.rose || waterLevelSensor1Edge.rose || waterLevelSensor2Edge.rose || waterLevelSensor3Edge.rose || !pumpTOF.Q()) {
        pumpState = false;
        digitalWrite(WATER_PUMP_ON_PIN, LOW);
        if (btnOffEdge.rose)     Serial.println("Pump OFF — display");
        if (manCommandEdge.rose) Serial.println("Pump OFF — button");
        if (waterLevelSensor1Edge.rose || waterLevelSensor2Edge.rose || waterLevelSensor3Edge.rose) {
            Serial.println("Pump OFF — water max");
            waterLevelMaxBaths1 = waterLevelMaxBaths2 = waterLevelMaxBaths3 = false;
        }
        pumpTOF.reset();
        changed = true;
    } else if (btnOnEdge.rose || manCommandEdge.fell || wakePumpCommand) {
        // Увімкнути насос — фізична кнопка або дисплей
        pumpState = true;
        digitalWrite(WATER_PUMP_ON_PIN, HIGH);
        if (btnOnEdge.rose)      Serial.println("Pump ON — display");
        if (manCommandEdge.fell) Serial.println("Pump ON — button");
        if (wakePumpCommand)     Serial.println("Pump ON — wake switch");
        wakePumpCommand = false;
        changed = true;
    } 


    // Відправити стан на дисплей при змінах
    if (changed) {
        sendStatus();
    }

    if (!pumpState &&
        digitalRead(MAN_COMMAND_WATER_PUMP_PIN) == HIGH &&
        millis() - lastActivityTime >= INACTIVITY_SLEEP_TIME_MS) {
        enterDeepSleep();
    }

    delay(10);
}

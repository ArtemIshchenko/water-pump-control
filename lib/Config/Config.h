#pragma once

#include <Arduino.h>

constexpr int WATER_PUMP_ON_PIN = 13;
constexpr int HOUSE_PUMP_ON_PIN = 33;
constexpr int MAN_COMMAND_WATER_PUMP_PIN = 4;
constexpr int WATER_PUMP_IS_ON_PIN = 5;
constexpr int WATER_LEVEL_SENSOR1_PIN = 14;
constexpr int WATER_LEVEL_SENSOR2_PIN = 27;
constexpr int WATER_LEVEL_SENSOR3_PIN = 26;
constexpr int HOUSE_PRESSURE_SENSOR_PIN = 34;

constexpr unsigned long INACTIVITY_SLEEP_TIME_MS = 1000UL * 60UL * 10UL;
constexpr unsigned long WATER_PUMP_MAX_RUN_TIME_MS = 1000UL * 60UL * 10UL;
constexpr unsigned long WATER_PUMP_IS_ON_TIMEOUT_MS = 2000;

constexpr int UART_TX_PIN = 17;
constexpr int UART_RX_PIN = 16;

#include "McuManage.h"

void McuManage::init(int wakePin, unsigned long inactivityTimeoutMs) {
    _wakePin = wakePin;
    _inactivityTimeoutMs = inactivityTimeoutMs;
    _lastActivityTime = millis();
}

void McuManage::registerActivity() {
    _lastActivityTime = millis();
}

bool McuManage::wasWokenBySwitch() const {
    return esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0 &&
           digitalRead(_wakePin) == LOW;
}

bool McuManage::shouldSleep(bool canSleep) const {
    return canSleep && millis() - _lastActivityTime >= _inactivityTimeoutMs;
}

void McuManage::enterDeepSleep() {
    Serial.println("Going to deep sleep");
    Serial.flush();

    rtc_gpio_init((gpio_num_t)_wakePin);
    rtc_gpio_set_direction((gpio_num_t)_wakePin, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pullup_en((gpio_num_t)_wakePin);
    rtc_gpio_pulldown_dis((gpio_num_t)_wakePin);

    esp_sleep_enable_ext0_wakeup((gpio_num_t)_wakePin, 0);
    esp_deep_sleep_start();
}

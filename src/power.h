void powerOn() {
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);
}

void shutdown() {
  pinMode(PIN_POWER_ON, OUTPUT);
  pinMode(PIN_LCD_BL, OUTPUT);
  digitalWrite(PIN_POWER_ON, LOW);
  digitalWrite(PIN_LCD_BL, LOW);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BUTTON_2, 0);  // 1 = High, 0 = Low
  esp_deep_sleep_start();
}

void reboot(String opts) {
  ESP.restart();
}


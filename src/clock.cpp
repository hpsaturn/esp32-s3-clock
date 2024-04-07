#include "Arduino.h"
#include "OneButton.h"
#include "WiFi.h"
#include "gui.h"
#include "esp_sntp.h"
#include "time.h"
#include <ESP32WifiCLI.hpp>

OneButton button1(PIN_BUTTON_1, true);
OneButton button2(PIN_BUTTON_2, true);

void wifi_test(void);
void timeavailable(struct timeval *t);
void printLocalTime();
void SmartConfig();

class mESP32WifiCLICallbacks : public ESP32WifiCLICallbacks {
  void onWifiStatus(bool isConnected) {
    // Serial.println("onWifiStatus");
  }
  // Callback for extend the help menu.
  void onHelpShow() {
    // Serial.println("onHelpShow");
  }
  void onNewWifi(String ssid, String passw) {
  }
};

void reboot(String opts) {
  ESP.restart();
}

void setup() {
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);
  Serial.begin(115200);

  sntp_servermode_dhcp(1); // (optional)
  configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2);

  displayInit();
  
  button1.attachClick([]() {
    pinMode(PIN_POWER_ON, OUTPUT);
    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_POWER_ON, LOW);
    digitalWrite(PIN_LCD_BL, LOW);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BUTTON_2, 0); // 1 = High, 0 = Low
    esp_deep_sleep_start();
  });

  button2.attachClick([]() { ui_switch_page(); });
  configTime(7200, 0, "pool.ntp.org"); // set timezone to UTC+2
  showBootAnimation("");
  // wifi_test();
  wcli.setCallback(new mESP32WifiCLICallbacks());
  wcli.begin();
  wcli.term->add("reboot", &reboot, "\tperform a ESP32 reboot");
  wcli.term->add("bootanim", &showBootAnimation, "show boot animation");
  LV_DELAY(100);
  ui_begin();
  Serial.println("end setup");
}

void loop() {
  while(!wcli.isConfigured()) wcli.loop();
  lv_timer_handler();
  button1.tick();
  button2.tick();
  delay(3);
  static uint32_t last_tick;
  if (millis() - last_tick > 1000) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      lv_msg_send(MSG_NEW_HOUR, &timeinfo.tm_hour);
      lv_msg_send(MSG_NEW_MIN, &timeinfo.tm_min);
    }
    uint32_t volt = (analogRead(PIN_BAT_VOLT) * 2 * 3.3 * 1000) / 4096;
    lv_msg_send(MSG_NEW_VOLT, &volt);
    last_tick = millis();
  }
  wcli.loop();
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t) {
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
  WiFi.disconnect();
}

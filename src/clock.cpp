#include "Arduino.h"
#include "OneButton.h"
#include "WiFi.h"
#include "gui.h"
#include "power.h"
#include "esp_sntp.h"
#include "time.h"
#include <ESP32WifiCLI.hpp>

OneButton button1(PIN_BUTTON_1, true);
OneButton button2(PIN_BUTTON_2, true);

const char * key_ntp_server = "kntpserver";
const char * key_tzone = "ktzone";

// change these params via CLI:
const char * default_server = "pool.ntp.org";  
const char * default_tzone = "CET-1CEST,M3.5.0,M10.5.0/3";

bool wcli_setup_ready = false;

class mESP32WifiCLICallbacks : public ESP32WifiCLICallbacks {
  void onWifiStatus(bool isConnected) {
  }
  // Callback for extend the CLI help menu.
  void onHelpShow() {
    Serial.println("ntpserver <server>\tset NTP server. Default: pool.ntp.org");
    Serial.println("ntpzone <TZONE>\t\tset time zone. https://tinyurl.com/4s44uyzn");
    Serial.println("time\t\t\tprint the current time");
  }
  void onNewWifi(String ssid, String passw) { wcli_setup_ready = wcli.isConfigured(); }
};

void updateTimeSettings() {
  String server = wcli.getString(key_ntp_server, default_server );
  String tzone = wcli.getString(key_tzone, default_tzone);
  Serial.printf("ntp server: \t%s\r\ntimezone: \t%s\r\n",server.c_str(),tzone.c_str());
  configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, server.c_str(), NTP_SERVER2);
  setenv("TZ", tzone.c_str(), 1);  
  tzset();
}

void setNTPServer(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String server = operands.first();
  if (server.isEmpty()) {
    Serial.println(wcli.getString(key_ntp_server, default_server));
    return;
  }
  wcli.setString(key_ntp_server, server);
  updateTimeSettings();
}

void setTimeZone(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String tzone = operands.first();
  if (tzone.isEmpty()) {
    Serial.println(wcli.getString(key_tzone, default_tzone));
    return;
  }
  wcli.setString(key_tzone, tzone);
  updateTimeSettings();
}

void printLocalTime(char *args, Stream *response) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void setup() {
  Serial.begin(115200);
  displayInit();
  
  button1.attachClick([]() { shutdown(); });
  button2.attachClick([]() { ui_switch_page(); });

  showBootAnimation(NULL, &Serial);

  wcli.setCallback(new mESP32WifiCLICallbacks());
  wcli.begin();
  // NTP init
  updateTimeSettings();
  // CLI config  
  wcli.add("ntpserver", &setNTPServer, "set NTP server. Default: pool.ntp.org");
  wcli.add("ntpzone", &setTimeZone, "\tset TZONE. https://tinyurl.com/4s44uyzn");
  wcli.add("time", &printLocalTime, "\tprint the current time");
  wcli.add("reboot", &reboot, "\tperform a ESP32 reboot");
  wcli.add("bootanim", &showBootAnimation, "show boot animation");
  LV_DELAY(100);
  ui_begin();
  wcli_setup_ready = wcli.isConfigured();
  Serial.println("end setup\r\n");
}

void loop() {
  lv_timer_handler();
  button1.tick();
  button2.tick();
  delay(3);
  static uint32_t last_tick;
  if (millis() - last_tick > 1000) {
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    lv_msg_send(MSG_NEW_HOUR, &timeinfo.tm_hour);
    lv_msg_send(MSG_NEW_MIN, &timeinfo.tm_min);
    uint32_t volt = (analogRead(PIN_BAT_VOLT) * 2 * 3.3 * 1000) / 4096;
    lv_msg_send(MSG_NEW_VOLT, &volt);
    last_tick = millis();
  }
  while(!wcli_setup_ready) wcli.loop(); // only for fist setup
  wcli.loop();
}

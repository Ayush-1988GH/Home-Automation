#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>

void setupWiFi() {
  WiFiManager wm;
  wm.setConfigPortalTimeout(180); // 3 min

  bool res = wm.autoConnect("ALEXA", "12345678");

  if (!res) {
    Serial.println("Failed to connect and portal timed out.");
    Serial.println("Going to deep sleep until manually power-cycled...");
    ESP.deepSleep(0); // sleeps indefinitely, only true power-cycle wakes it
  } else {
    Serial.println("Connected!");
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
  }
}

// ---------- SinricPro Credentials ----------
#define APP_KEY           "SINRIC PRO APP KEY ID"
#define APP_SECRET        "SINRIC PRO APP SECRET ID"

#define SWITCH_ID_1       "Device_ID1"
#define SWITCH_ID_2       "Device_ID2"
#define SWITCH_ID_3       "Device_ID3"

#define BAUD_RATE         115200

// ---------- Relay Pins ----------
#define RELAY1 D1
#define RELAY2 D2
#define RELAY3 D5

bool onPowerState(const String &deviceId, bool &state) {
  int relayPin;

  if (deviceId == SWITCH_ID_1) relayPin = RELAY1;
  else if (deviceId == SWITCH_ID_2) relayPin = RELAY2;
  else if (deviceId == SWITCH_ID_3) relayPin = RELAY3;
  else return false;

  // Active-LOW relay: state=true (ON) -> LOW, state=false (OFF) -> HIGH
  digitalWrite(relayPin, state ? LOW : HIGH);

  Serial.printf("Device %s turned %s\n", deviceId.c_str(), state ? "ON" : "OFF");
  return true;
}

void setupSinricPro() {
  SinricProSwitch &mySwitch1 = SinricPro[SWITCH_ID_1];
  mySwitch1.onPowerState(onPowerState);

  SinricProSwitch &mySwitch2 = SinricPro[SWITCH_ID_2];
  mySwitch2.onPowerState(onPowerState);

  SinricProSwitch &mySwitch3 = SinricPro[SWITCH_ID_3];
  mySwitch3.onPowerState(onPowerState);

  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\n"); });
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\n"); });

  configTime(5.5 * 3600, 0, "pool.ntp.org", "time.google.com");

  SinricPro.begin(APP_KEY, APP_SECRET);
}

void setup() {
  Serial.begin(BAUD_RATE);

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);

  // Start all relays OFF (active-LOW, so HIGH = OFF)
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  digitalWrite(RELAY3, HIGH);

  setupWiFi();
  setupSinricPro();
}

void loop() {
  SinricPro.handle();
}
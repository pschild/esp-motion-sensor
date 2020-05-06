#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#ifndef WIFI_SSID
  #error "Missing WIFI_SSID"
#endif

#ifndef WIFI_PASSWORD
  #error "Missing WIFI_PASSWORD"
#endif

#ifndef VERSION
  #define VERSION "local-build"
#endif

unsigned long lastUpdateCheck = 0;
const long UPDATE_INTERVAL = 1 * 60 * 60 * 1000; // 1 hour

void connectToWifi() {
  //WiFi.persistent(false);
  //WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long wifiConnectStart = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (millis() - wifiConnectStart > 5000) {
      return;
    }
  }
}

void checkForUpdate(bool forceUpdate) {
  unsigned long now = millis();
  if (
    WiFi.status() == WL_CONNECTED
    && (now >= lastUpdateCheck + UPDATE_INTERVAL || forceUpdate == true)
  ) {
    WiFiClient client;
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    ESPhttpUpdate.update(client, "http://192.168.178.28:9042/ota", VERSION);
    lastUpdateCheck = now;
  }
}

void post() {
  HTTPClient http;
  http.begin("http://192.168.178.28:9052/movement");
  http.addHeader("Content-Type", "text/plain");
  http.POST("foo");
  String payload = http.getString();
  http.end();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(4, INPUT); // GPIO 4 => ESP12F

  connectToWifi();
  checkForUpdate(true);
}


void loop() {
  int state = digitalRead(4); // GPIO 4 => ESP12F
  if (state == HIGH) {
    digitalWrite(LED_BUILTIN, LOW); // LED on

    post();
    delay(5000);
  } else {
    digitalWrite(LED_BUILTIN, HIGH); // LED off
  }

  checkForUpdate(false);
}

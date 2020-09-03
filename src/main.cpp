#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <WifiHandler.h>
#include <MqttHandler.h>
#include <OTAUpdateHandler.h>

#ifndef WIFI_SSID
  #error "Missing WIFI_SSID"
#endif

#ifndef WIFI_PASSWORD
  #error "Missing WIFI_PASSWORD"
#endif

#ifndef VERSION
  #error "Missing VERSION"
#endif

const String CHIP_ID = String("ESP_") + String(ESP.getChipId());

void ping();
void onFooBar(char* payload);
void onOtaUpdate(char* payload);
void onMqttConnected();

WifiHandler wifiHandler(WIFI_SSID, WIFI_PASSWORD);
MqttHandler mqttHandler("192.168.178.28", CHIP_ID);
OTAUpdateHandler updateHandler("192.168.178.28:9042", VERSION);
Ticker pingTimer(ping, 60 * 1000);

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  pinMode(4, INPUT); // GPIO 4 => ESP12F

  wifiHandler.connect();
  mqttHandler.setOnConnectedCallback(onMqttConnected);
  mqttHandler.setup();
  pingTimer.start();

  // start OTA update immediately
  updateHandler.startUpdate();
}

void loop() {
  mqttHandler.loop();
  updateHandler.loop();
  pingTimer.update();
  
  int state = digitalRead(4); // GPIO 4 => ESP12F
  if (state == HIGH) {
    digitalWrite(LED_BUILTIN, LOW); // LED on
    
    HTTPClient http;
    http.begin("http://192.168.178.28:9052/movement");
    http.addHeader("Content-Type", "text/plain");
    http.POST("foo");
    String payload = http.getString();
    http.end();
    
    delay(5000);
  } else {
    digitalWrite(LED_BUILTIN, HIGH); // LED off
  }
}

void ping() {
  mqttHandler.publish("/devices/nodemcu/version", VERSION);
}

void onFooBar(char* payload) {
  if (strcmp(payload, "on") == 0) {
    digitalWrite(LED_BUILTIN, LOW);
  } else if (strcmp(payload, "off") == 0) {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void onOtaUpdate(char* payload) {
  updateHandler.startUpdate();
}

void onMqttConnected() {
  mqttHandler.subscribe("/foo/bar", onFooBar);
  mqttHandler.subscribe("/otaUpdate/all", onOtaUpdate);
}

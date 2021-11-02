#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <WifiHandler.h>
#include <MqttHandler.h>
#include <OTAUpdateHandler.h>
#include <ESP8266HTTPClient.h>

#ifndef WIFI_SSID
  #error "Missing WIFI_SSID"
#endif

#ifndef WIFI_PASSWORD
  #error "Missing WIFI_PASSWORD"
#endif

#ifndef VERSION
  #error "Missing VERSION"
#endif

const String CHIP_ID = String(ESP.getChipId());
const String CLIENT_NAME = String("ESP_") + CHIP_ID;

void ping();
void ledTurnOn();
void ledTurnOff();
void setLock();
void removeLock();
void onFooBar(char* payload);
void onOtaUpdate(char* payload);
void onMqttConnected();
void onMqttMessage(char* topic, char* message);

WifiHandler wifiHandler(WIFI_SSID, WIFI_PASSWORD);
MqttHandler mqttHandler("192.168.178.28", CLIENT_NAME);
OTAUpdateHandler updateHandler("192.168.178.28:9042", VERSION);

Ticker pingTimer(ping, 60 * 1000);
Ticker switchOffTimer(ledTurnOff, 500);
Ticker unlockTimer(removeLock, 5000);

boolean locked = false;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  pinMode(4, INPUT); // GPIO 4 => ESP12F

  wifiHandler.connect();
  mqttHandler.setup();
  mqttHandler.setOnConnectedCallback(onMqttConnected);
  mqttHandler.setOnMessageCallback(onMqttMessage);
  pingTimer.start();

  // start OTA update immediately
  updateHandler.startUpdate();
}

void loop() {
  mqttHandler.loop();
  updateHandler.loop();
  pingTimer.update();
  switchOffTimer.update();
  unlockTimer.update();

  int state = digitalRead(4); // GPIO 4 => ESP12F
  if (state == HIGH && !locked) {
    ledTurnOn();
    switchOffTimer.start();
    
    setLock();
    unlockTimer.start();

    /*
    HTTPClient http;
    http.begin("http://192.168.178.28:9052/movement");
    http.addHeader("Content-Type", "text/plain");
    http.POST("foo");
    String payload = http.getString();
    http.end();
    */

    const String channel = String("devices/") + CHIP_ID + String("/movement");
    mqttHandler.publish(channel.c_str(), NULL);
  } else {
    ledTurnOff();
  }
}

void ledTurnOn() {
  digitalWrite(LED_BUILTIN, LOW);
}

void ledTurnOff() {
  digitalWrite(LED_BUILTIN, HIGH);
}

void setLock() {
  locked = true; 
}

void removeLock() {
  locked = false;
}

void ping() {
  const String channel = String("devices/") + CHIP_ID + String("/version");
  mqttHandler.publish(channel.c_str(), VERSION);
}

void onFooBar(char* payload) {
  if (strcmp(payload, "on") == 0) {
    ledTurnOn();
  } else if (strcmp(payload, "off") == 0) {
    ledTurnOff();
  }
}

void onOtaUpdate(char* payload) {
  updateHandler.startUpdate();
}

void onMqttConnected() {
  mqttHandler.subscribe("foo/+/baz");

  const String otaDeviceChannel = String("ota/") + CHIP_ID;
  mqttHandler.subscribe(otaDeviceChannel.c_str());
  const String otaAllDevicesChannel = "ota/all";
  mqttHandler.subscribe(otaAllDevicesChannel.c_str());
}

void onMqttMessage(char* topic, char* message) {
  if (String(topic).startsWith("foo/")) {
    onFooBar(message);
  } else if (String(topic).startsWith("ota/")) {
    onOtaUpdate(message);
  }
}

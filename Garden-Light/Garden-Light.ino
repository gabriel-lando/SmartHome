#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#define LED_BUILTIN       2
#define LED_STATE         1
#else
#include <ESP8266WiFi.h>
#define LED_STATE         0
#endif
#include <fauxmoESP.h>
#include "credentials.h"

#define GardenLight      D3
#define IntGarden       D7

fauxmoESP fauxmo;

bool GardenState = 0, LastState = 0;

// -----------------------------------------------------------------------------
// Wifi

IPAddress ip(192, 168, 1, 54);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// -----------------------------------------------------------------------------

void wifiSetup() {

  // Set WIFI module to STA mode
  WiFi.mode(WIFI_STA);

  // Connect
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait
  int wait = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    if (wait++ > 15)
      ESP.restart();
  }

  // Connected!
}

void setup() {
  pinMode(GardenLight, OUTPUT);
  digitalWrite(GardenLight, LOW);

  pinMode(IntGarden, INPUT_PULLUP);
  LastState = digitalRead(IntGarden);

  attachInterrupt(IntGarden, ManualChange, CHANGE);
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LED_STATE);

  // Wifi
  wifiSetup();

  fauxmo.enable(true);

  // Add virtual devices
  fauxmo.addDevice("Garden Light");

  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state) {
    if (device_id == 0){
      digitalWrite(GardenLight, state);
      GardenState = state;
    }
  });

  // Callback to retrieve current state (for GetBinaryState queries)
  fauxmo.onGetState([](unsigned char device_id, const char * device_name) {
    if (device_id == 0){
      GardenState = digitalRead(GardenLight);
      return GardenState;
    }
  });

  digitalWrite(LED_BUILTIN, !LED_STATE);
}

void ManualChange(){
  static unsigned long LastTime = 0;
  unsigned long Time = millis();
  if(LastState != digitalRead(IntGarden)){
    if (Time - LastTime > 200){
      GardenState = !GardenState;
      digitalWrite(GardenLight, GardenState);
    }
    LastTime = Time;
  }
  LastState = digitalRead(IntGarden);
  
}

void loop() {
  fauxmo.handle();

  static unsigned long last = millis();
  if (millis() - last > 5000) {
    last = millis();
    if (WiFi.status() != WL_CONNECTED)
      ESP.restart();
  }
  delay(200);
}

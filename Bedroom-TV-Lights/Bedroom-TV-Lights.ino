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

//#define SERIAL_BAUDRATE   115200
#define BedroomLight      D1
#define TVLightLeft       D2
#define TVLightRight      D0
#define IntBedroom        D5
#define IntTVLeft         D6
#define IntTVRight        D7

fauxmoESP fauxmo;

bool BedroomState = 0, BedroomLastState = 0;
bool TVLeftState = 0, TVLeftLastState = 0;
bool TVRightState = 0, TVRightLastState = 0;

// -----------------------------------------------------------------------------
// Wifi

IPAddress ip(192, 168, 1, 51);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// -----------------------------------------------------------------------------

void wifiSetup() {
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait
  int wait = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    if (wait++ > 20)
      ESP.restart();
  }

}

void setPins(){
  pinMode(BedroomLight, OUTPUT);
  digitalWrite(BedroomLight, LOW);

  pinMode(TVLightLeft, OUTPUT);
  digitalWrite(TVLightLeft, LOW);

  pinMode(TVLightRight, OUTPUT);
  digitalWrite(TVLightRight, LOW);
  
  pinMode(IntBedroom, INPUT_PULLUP);
  BedroomLastState = digitalRead(IntBedroom);

  pinMode(IntTVLeft, INPUT_PULLUP);
  TVLeftLastState = digitalRead(IntTVLeft);

  pinMode(IntTVRight, INPUT_PULLUP);
  TVRightLastState = digitalRead(IntTVRight);

  attachInterrupt(IntBedroom, BedroomManualChange, CHANGE);
  attachInterrupt(IntTVLeft, TVLeftManualChange, CHANGE);
  attachInterrupt(IntTVRight, TVRightManualChange, CHANGE);
  
  pinMode(LED_BUILTIN, OUTPUT);
}

void fauxmoSetup(){
  fauxmo.enable(true);

  // Add virtual devices
  fauxmo.addDevice("Bedroom Light");
  fauxmo.addDevice("TV Light Left");
  fauxmo.addDevice("TV Light Right");

  // fauxmoESP 2.0.0 has changed the callback signature to add the device_id,
  // this way it's easier to match devices to action without having to compare strings.
  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state) {
    if (device_id == 0){
      digitalWrite(BedroomLight, state);
      BedroomState = state;
    }
    else if (device_id == 1){
      digitalWrite(TVLightLeft, state);
      TVLeftState = state;
    }
    else if (device_id == 2){
      digitalWrite(TVLightRight, state);
      TVRightState = state;
    }
  });

  // Callback to retrieve current state (for GetBinaryState queries)
  fauxmo.onGetState([](unsigned char device_id, const char * device_name) {
    if (device_id == 0){
      BedroomState = digitalRead(BedroomLight);
      return BedroomState;
    }
    else if (device_id == 1){
      TVLeftState = digitalRead(TVLightLeft);
      return TVLeftState;
    }
    else if (device_id == 2){
      TVRightState = digitalRead(TVLightRight);
      return TVRightState;
    }
  });  
}

void setup() {
  //Set Pins IO
  setPins();
  
  digitalWrite(LED_BUILTIN, LED_STATE);

  // Wifi
  wifiSetup();

  //Fauxmo Setup
  fauxmoSetup();
  
  digitalWrite(LED_BUILTIN, !LED_STATE);
}

void BedroomManualChange(){
  static unsigned long BedroomLastTime = 0;
  unsigned long Time = millis();
  if(BedroomLastState != digitalRead(IntBedroom)){
    if (Time - BedroomLastTime > 200){
      BedroomState = !BedroomState;
      digitalWrite(BedroomLight, BedroomState);
    }
    BedroomLastTime = Time;
  }
  BedroomLastState = digitalRead(IntBedroom);
}

void TVLeftManualChange(){
  static unsigned long TVLeftLastTime = 0;
  unsigned long Time = millis();
  if(TVLeftLastState != digitalRead(IntTVLeft)){
    if (Time - TVLeftLastTime > 200){
      TVLeftState = !TVLeftState;
      digitalWrite(TVLightLeft, TVLeftState);
    }
    TVLeftLastTime = Time;
  }
  TVLeftLastState = digitalRead(IntTVLeft);
}

void TVRightManualChange(){
  static unsigned long TVRightLastTime = 0;
  unsigned long Time = millis();
  if(TVRightLastState != digitalRead(IntTVRight)){
    if (Time - TVRightLastTime > 200){
      TVRightState = !TVRightState;
      digitalWrite(TVLightRight, TVRightState);
    }
    TVRightLastTime = Time;
  }
  TVRightLastState = digitalRead(IntTVRight);
}

void loop() {
  fauxmo.handle();

  static unsigned long last = millis();
  if (millis() - last > 5000) {
    last = millis();
    if (WiFi.status() != WL_CONNECTED)
      ESP.restart();
  }
  delay(500);
}

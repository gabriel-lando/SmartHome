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
#define LivingRoomLight      D2
#define DinnerRoomLight       D3
#define IntLivingRoom        D7
#define IntDinnerRoom         D5

fauxmoESP fauxmo;

bool LivingRoomState = 0, LivingRoomLastState = 0;
bool DinnerRoomState = 0, DinnerRoomLastState = 0;

// -----------------------------------------------------------------------------
// Wifi

IPAddress ip(192, 168, 1, 53);
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
    if (wait++ > 15)
      ESP.restart();
  }

}

void setPins(){
  pinMode(LivingRoomLight, OUTPUT);
  digitalWrite(LivingRoomLight, LOW);

  pinMode(DinnerRoomLight, OUTPUT);
  digitalWrite(DinnerRoomLight, LOW);
  
  pinMode(IntLivingRoom, INPUT_PULLUP);
  LivingRoomLastState = digitalRead(IntLivingRoom);

  pinMode(IntDinnerRoom, INPUT_PULLUP);
  DinnerRoomLastState = digitalRead(IntDinnerRoom);

  attachInterrupt(IntLivingRoom, LivingRoomManualChange, CHANGE);
  attachInterrupt(IntDinnerRoom, DinnerRoomManualChange, CHANGE);
  
  pinMode(LED_BUILTIN, OUTPUT);
}

void fauxmoSetup(){
  fauxmo.enable(true);

  // Add virtual devices
  fauxmo.addDevice("Living Room Light");
  fauxmo.addDevice("Dinner Room Light");

  // fauxmoESP 2.0.0 has changed the callback signature to add the device_id,
  // this way it's easier to match devices to action without having to compare strings.
  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state) {
    if (device_id == 0){
      digitalWrite(LivingRoomLight, state);
      LivingRoomState = state;
    }
    else if (device_id == 1){
      digitalWrite(DinnerRoomLight, state);
      DinnerRoomState = state;
    }
  });

  // Callback to retrieve current state (for GetBinaryState queries)
  fauxmo.onGetState([](unsigned char device_id, const char * device_name) {
    if (device_id == 0){
      LivingRoomState = digitalRead(LivingRoomLight);
      return LivingRoomState;
    }
    else if (device_id == 1){
      DinnerRoomState = digitalRead(DinnerRoomLight);
      return DinnerRoomState;
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

void LivingRoomManualChange(){
  static unsigned long LivingRoomLastTime = 0;
  unsigned long Time = millis();
  if(LivingRoomLastState != digitalRead(IntLivingRoom)){
    if (Time - LivingRoomLastTime > 200){
      LivingRoomState = !LivingRoomState;
      digitalWrite(LivingRoomLight, LivingRoomState);
    }
    LivingRoomLastTime = Time;
  }
  LivingRoomLastState = digitalRead(IntLivingRoom);
}

void DinnerRoomManualChange(){
  static unsigned long DinnerRoomLastTime = 0;
  unsigned long Time = millis();
  if(DinnerRoomLastState != digitalRead(IntDinnerRoom)){
    if (Time - DinnerRoomLastTime > 200){
      DinnerRoomState = !DinnerRoomState;
      digitalWrite(DinnerRoomLight, DinnerRoomState);
    }
    DinnerRoomLastTime = Time;
  }
  DinnerRoomLastState = digitalRead(IntDinnerRoom);
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

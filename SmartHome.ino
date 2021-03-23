/*************************************************************************************************************************************************
  SmartHome.ino
  For ESP8266 boards
  Written by Gabriel Lando
  Licensed under MIT license
**************************************************************************************************************************************************/

#if !defined(ESP8266)
  #error This code is designed to run on ESP8266 and ESP8266-based boards! Please check your Tools->Board setting.
#endif

#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <ArduinoOTA.h>

/*************************************************************************************************************************************************
   Besides the libraries already included with the Arduino Core for ESP8266, these libraries are also required to use fauxmoESP:
    => ESPAsyncTCP: https://github.com/me-no-dev/ESPAsyncTCP

    IMPORTANT: For ESP8266, before upload sketch, set LwIP to "v1.4 Higher Bandwidth" in Tools > LwIP Variant > "v1.4 Higher Bandwidth".
**************************************************************************************************************************************************/

#include "Settings.h"
#include "src/Lights.h"
#include "src/Switches.h"
#include "src/Alexa.h"
#include "src/NVMe.h"

Lights lights;
Switches switches;
Alexa alexa;
NVME nvme;
volatile byte lastState;

void setup() {
    if (DEBUG_ENABLED) {
        Serial.begin(115200);
        delay(250);
    }

    LoadCurrentState();
    SetupWiFi();
    SetupOTA();

    alexa.Initialize(lastState, AlexaStatusChanged);
}

void loop() {
    // fauxmoESP uses an async TCP server but a sync UDP server
    // Therefore, we have to manually poll for UDP packets
    alexa.Loop();
    ArduinoOTA.handle();

    byte switchStatus = switches.GetState();
    if (switchStatus) {
        byte currState = ProcessChanges(switchStatus);
        if (currState != lastState) {
            lastState = currState;
            
            nvme.SetState(lastState);
            lights.SetState(lastState);
            
            for (byte i = 0; i < NUM_DEVICES; i++) {
                if (DEBUG_ENABLED)
                    Serial.println((String)"[MAIN] Switch changed: " + i + " to " + ((lastState >> i) & 0x1) + " with " + lights.GetBrightness(i));
                alexa.SetState(i, ((lastState >> i) & 0x1), lights.GetBrightness(i));
            }
        }
    }

    // This is a sample code to output free heap every 5 seconds
    // This is a cheap way to detect memory leaks
    static unsigned long lastMillis = millis();
    if (millis() - lastMillis > 5000) {
        lastMillis = millis();
        ESP.getFreeHeap();
        if (!WiFi.isConnected() && !WiFi.reconnect()) {
            if (DEBUG_ENABLED)
                Serial.println("[WiFi] Connection lost. Restarting...");
            ESP.restart();
            delay(1000);
        }
    }
}

/******************************************************/
/****************** Setup Functions *******************/
/******************************************************/

void LoadCurrentState() {
    lastState = nvme.GetState();
    lights.SetState(lastState);

    if (DEBUG_ENABLED) {
        Serial.println("\n[MAIN] Current lights state: ");
        for (int i = 0; i < NUM_DEVICES; i++)
            Serial.println((String)"\t" + DEVICES[i] + ": " + ((lastState >> i) & 0x1));
    }
}

void SetupWiFi() {
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    WiFiManager wifiManager;

    // Setting AP SSID and Password
    String AP_MAC = WiFi.macAddress();
    String AP_SSID = "LandoAP-" + AP_MAC.substring(9);
    char C_AP_SSID[20];
    AP_SSID.toCharArray(C_AP_SSID, sizeof(C_AP_SSID));
    char C_AP_PWD[] = "12345678";

    bool res;
    wifiManager.setTimeout(180); // Set AP timeout to 180 seconds = 3 minutes
    res = wifiManager.autoConnect(C_AP_SSID, C_AP_PWD); // password protected ap

    if (!res) {
        ESP.restart();
        delay(5000);
    }
}

void SetupOTA() {
    ArduinoOTA.setPasswordHash(OTA_MD5_PASSWORD);
    ArduinoOTA.begin();
}

/******************************************************/
/******************* Loop Functions *******************/
/******************************************************/

byte ProcessChanges(byte changes) {
    // Process changes
    if (changes != 0) {
        byte currentState = lastState;

        for (int i = 0; i < NUM_DEVICES; i++) {
            if ((changes >> i) & 0x1)
                currentState ^= 0x1 << i;
        }
        return currentState;
    }
    return lastState;
}

void AlexaStatusChanged(byte id, bool state, byte value) {
    if (state)
        lastState |= (0x1 << id);
    else
        lastState &= ~(0x1 << id);

    lights.SetState(id, state, value);
    nvme.SetState(lastState);

    if (DEBUG_ENABLED)
        Serial.println((String)"[MAIN] Alexa changed: " + id + " to " + state + " with " + value);
}
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
#include <fauxmoESP.h>    // https://github.com/vintlabs/fauxmoESP
#include <ArduinoOTA.h>

#include <IRremoteESP8266.h> // https://github.com/crankyoldgit/IRremoteESP8266
#include <IRsend.h>          // https://github.com/crankyoldgit/IRremoteESP8266

/*************************************************************************************************************************************************
   Besides the libraries already included with the Arduino Core for ESP8266, these libraries are also required to use fauxmoESP:
    => ESPAsyncTCP: https://github.com/me-no-dev/ESPAsyncTCP

    IMPORTANT: For ESP8266, before upload sketch, set LwIP to "v1.4 Higher Bandwidth" in Tools > LwIP Variant > "v1.4 Higher Bandwidth".
**************************************************************************************************************************************************/

#include "Settings.h" // Choose Settings file

// Status for the JVC Stereo
enum changedState {
    none,
    powerOn,
    powerOff,
    turnOn,
    turnOff,
    putOnAux,
    volumeChanged
};

// Constans for JVC Stereo
const uint16_t kJVCPower = 0xC5E8;
const uint16_t kJVCAux = 0xC57C;
const uint16_t kJVCVolUp = 0xC578;
const uint16_t kJVCVolDown = 0xC5F8;

#define OTA_MD5_PASSWORD "8d2a859ad6c0f1027ec838626c71da70" // Generate a new MD5 hash password on: http://www.md5.cz/

fauxmoESP fauxmo;

bool currentState = false;
byte currentVolume = 0;
byte newVolume = 0;

changedState newState = none;

IRsend irsend(IR_PIN);

void setup() {
    if (DEBUG_ENABLED) {
        Serial.begin(115200);
        delay(250);
    }

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, currentState);

    irsend.begin();

    SetupWiFi();
    SetupOTA();

    fauxmoSetup();
}

void loop() {
    // fauxmoESP uses an async TCP server but a sync UDP server
    // Therefore, we have to manually poll for UDP packets
    fauxmo.handle();
    ArduinoOTA.handle();

    // This is a sample code to output free heap every 5 seconds
    // This is a cheap way to detect memory leaks
    static unsigned long lastHeap = millis();
    if (millis() - lastHeap > 5000) {
        lastHeap = millis();
        ESP.getFreeHeap();
        if (!WiFi.isConnected() && !WiFi.reconnect()) {
            if (DEBUG_ENABLED)
                Serial.println("[WiFi] Connection lost. Restarting...");
            ESP.restart();
            delay(1000);
        }
    }

    // Send commands every 200ms
    static unsigned long lastRead = millis();
    if (millis() - lastRead > 200) {
        lastRead = millis();
        ProcessChanges();
    }
}

/******************************************************/
/***************** Setup Functions ********************/
/******************************************************/

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

void fauxmoSetup() {
    // By default, fauxmoESP creates it's own webserver on the defined port
    // The TCP port must be 80 for gen3 devices (default is 1901)
    // This has to be done before the call to enable()
    fauxmo.createServer(true); // not needed, this is the default value
    fauxmo.setPort(80); // This is required for gen3 devices

    // You have to call enable(true) once you have a WiFi connection
    // You can enable or disable the library at any moment
    // Disabling it will prevent the devices from being discovered and switched
    fauxmo.enable(true);

    // Add virtual devices
    fauxmo.addDevice(DEVICE);

    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {

        // Callback when a command from Alexa is received.
        // You can use device_id or device_name to choose the element to perform an action onto (relay, LED,...)
        // State is a boolean (ON/OFF) and value a number from 0 to 255 (if you say "set kitchen light to 50%" you will receive a 128 here).
        // Just remember not to delay too much here, this is a callback, exit as soon as possible.
        // If you have to do something more involved here set a flag and process it in your main loop.

        // Checking for device_id is simpler if you are certain about the order they are loaded and it does not change.
        // Otherwise comparing the device_name is safer.

        if (DEBUG_ENABLED)
            Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d volume: %d\n", device_id, device_name, state ? "ON" : "OFF", value, MapVolume(value));

        if (strcmp(device_name, DEVICE) == 0) {
            if (state) {
                if (!currentState) { // Power On
                    newState = powerOn;
                    newVolume = MapVolume(value);
                }
                else { // Volume changed
                    newState = volumeChanged;
                    newVolume = MapVolume(value);
                }
            }
            else if (currentState) { // Power Off
                newState = turnOff;
                fauxmo.setState(DEVICE, state, 4);
            }
            else {
                newState = none;
            }
        }
    });

    fauxmo.setState(DEVICE, currentState, 4); // Minimum value
}

byte MapVolume(byte volume) {
    return map(volume, 4, 254, 1, 40);
}

/******************************************************/
/***************** Loop Functions *********************/
/******************************************************/

void ProcessChanges() {
    switch (newState) {
        case powerOn: // Turn ON relay
            digitalWrite(RELAY_PIN, true);
            delay(1000);
            newState = turnOn;
            break;

        case turnOn: // Turn ON Stereo using IR
            irsend.sendJVC(kJVCPower, 16, 1);
            delay(1000);
            newState = putOnAux;
            currentState = true;
            break;

        case putOnAux: // Put on Aux mode using IR
            irsend.sendJVC(kJVCAux, 16, 1);
            delay(1000);
            newState = volumeChanged;
            break;

        case volumeChanged: // Update volume using IR
            ChangeVolume();
            break;

        case turnOff: // Turn OFF Stereo using IR before turn OFF relay
            irsend.sendJVC(kJVCPower, 16, 1);
            delay(1000);
            newState = powerOff;
            currentVolume = 0;
            currentState = false;
            break;
          
        case powerOff: // Turn OFF relay
            digitalWrite(RELAY_PIN, false);
            newState = none;
            break;

        case none:
        default:
            break;
    }
}

void ChangeVolume() {
    if (currentVolume < newVolume) {
        irsend.sendJVC(kJVCVolUp, 16, 1);
        currentVolume++;
    }
    else if (currentVolume > newVolume) {
        irsend.sendJVC(kJVCVolDown, 16, 1);
        currentVolume--;
    }
    else {
        newState = none;
    }
}

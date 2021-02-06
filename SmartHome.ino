#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <fauxmoESP.h>    // https://github.com/vintlabs/fauxmoESP
#include <ArduinoOTA.h>

/*  Besides the libraries already included with the Arduino Core for ESP8266 or ESP32, these libraries are also required to use fauxmoESP:
    => ESP8266:  This library uses ESPAsyncTCP library by me-no-dev  -> https://github.com/me-no-dev/ESPAsyncTCP
    => ESP32:    This library uses AsyncTCP library by me-no-dev     -> https://github.com/me-no-dev/AsyncTCP

    IMPORTANT: For ESP8266, before upload sketch, set LwIP to "v1.4 Higher Bandwidth" in Tools > LwIP Variant > "v1.4 Higher Bandwidth".
*/

#include "NVMe.h"
#include "Settings.X.h" // Choose Settings file

fauxmoESP fauxmo;
NVME nvme;
byte currentState, lastState;

void setup() {
    if (DEBUG_ENABLED)
        Serial.begin(115200);

    SetPins();
    LoadCurrentState();
    SetupWiFi();
    SetupOTA();

    fauxmoSetup();

    if (DEBUG_ENABLED) {
        Serial.println("\nCurrent lights state: ");
        for (int i = 0; i < NUM_DEVICES; i++)
            Serial.println((String)DEVICES[i] + ": " + ((currentState >> i) & 0x1));
    }
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
        }
    }

    // Read GPIO every 200ms
    static unsigned long lastRead = millis();
    if (millis() - lastRead > 200) {
        lastRead = millis();
        ProcessChanges();
    }
}

/******************************************************/
/***************** Setup Functions ********************/
/******************************************************/

void LoadCurrentState() {
    lastState = currentState = nvme.GetState();
    for (int i = 0; i < NUM_DEVICES; i++)
        digitalWrite(LIGHT_PINS[i], (currentState >> i) & 0x1);
}

void SetPins() {
    for (int i = 0; i < NUM_DEVICES; i++) {
        pinMode(LIGHT_PINS[i], OUTPUT);
        pinMode(SWITCH_PINS[i], INPUT_PULLUP);
    }
}

void SetupWiFi() {
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    WiFiManager wm;

    // Setting AP SSID and Password
    String AP_MAC = WiFi.macAddress();
    String AP_SSID = "LandoAP-" + AP_MAC.substring(9);
    char C_AP_SSID[20];
    AP_SSID.toCharArray(C_AP_SSID, sizeof(C_AP_SSID));
    char C_AP_PWD[] = "12345678";

    bool res;
    res = wm.autoConnect(C_AP_SSID, C_AP_PWD); // password protected ap

    if (!res) {
        ESP.restart();
    }
}

void SetupOTA() {
    ArduinoOTA.setPasswordHash("8d2a859ad6c0f1027ec838626c71da70");

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_FS
            type = "filesystem";
        }

        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
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
    for (int i = 0; i < NUM_DEVICES; i++)
        fauxmo.addDevice(DEVICES[i]);

    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {

        // Callback when a command from Alexa is received.
        // You can use device_id or device_name to choose the element to perform an action onto (relay, LED,...)
        // State is a boolean (ON/OFF) and value a number from 0 to 255 (if you say "set kitchen light to 50%" you will receive a 128 here).
        // Just remember not to delay too much here, this is a callback, exit as soon as possible.
        // If you have to do something more involved here set a flag and process it in your main loop.

        // Checking for device_id is simpler if you are certain about the order they are loaded and it does not change.
        // Otherwise comparing the device_name is safer.

        for (int i = 0; i < NUM_DEVICES; i++) {
            if (strcmp(device_name, DEVICES[i]) == 0) {
                digitalWrite(LIGHT_PINS[i], state);
                if (state)
                    currentState |= 0x1 << i;
                else
                    currentState &= ~(0x1 << i);
            }
        }
    });

    for (int i = 0; i < NUM_DEVICES; i++)
        fauxmo.setState(DEVICES[i], (currentState >> i) & 0x1, 255);
}

/******************************************************/
/***************** Loop Functions *********************/
/******************************************************/

void ProcessChanges() {
    // Read GPIOs
    byte changes = ReadSwitchState();

    // Process changes
    if (changes != 0) {
        for (int i = 0; i < NUM_DEVICES; i++) {
            if ((changes >> i) & 0x1)
                currentState ^= 0x1 << i;
        }
    }

    // If there was any change, save it on EEPROM and update light status
    if (lastState != currentState) {
        nvme.SetState(currentState);

        if (changes != 0) {
            for (int i = 0; i < NUM_DEVICES; i++) {
                digitalWrite(LIGHT_PINS[i], (currentState >> i) & 0x1);
                fauxmo.setState(DEVICES[i], (currentState >> i) & 0x1, 255);
            }
        }
        lastState = currentState;
    }
}

byte ReadSwitchState() {
    static byte lastRead = 0xFF;
    byte currentRead = 0;
    for (int i = 0; i < NUM_DEVICES; i++) {
        if (digitalRead(SWITCH_PINS[i]))
            currentRead |= 0x1 << i;
        else
            currentRead &= ~(0x1 << i);
    }

    byte changes = 0;
    if (lastRead != 0xFF && lastRead != currentRead) {
        for (int i = 0; i < NUM_DEVICES; i++) {
            if ((currentRead >> i) & 0x1 != (lastRead >> i) & 0x1)
                changes |= 0x1 << i;
        }
    }

    lastRead = currentRead;
    return changes;
}

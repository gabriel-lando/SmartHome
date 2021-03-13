#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <fauxmoESP.h>    // https://github.com/vintlabs/fauxmoESP
#include <ArduinoOTA.h>

/*  Besides the libraries already included with the Arduino Core for ESP8266 or ESP32, these libraries are also required to use fauxmoESP:
    => ESP8266:  This library uses ESPAsyncTCP library by me-no-dev  -> https://github.com/me-no-dev/ESPAsyncTCP
    => ESP32:    This library uses AsyncTCP library by me-no-dev     -> https://github.com/me-no-dev/AsyncTCP

    IMPORTANT: For ESP8266, before upload sketch, set LwIP to "v1.4 Higher Bandwidth" in Tools > LwIP Variant > "v1.4 Higher Bandwidth".
*/

#include "NVMe.h"
#include "Dimmer.h"
#include "Settings.0.h" // Choose Settings file

#define OTA_MD5_PASSWORD "8d2a859ad6c0f1027ec838626c71da70" // Generate a new MD5 hash password on: http://www.md5.cz/

fauxmoESP fauxmo;
NVME nvme;
Dimmer dimmer(NUM_DEVICES, USE_DIMMER, ZC_DIMMER_PINS, LIGHT_PINS);
byte currentState, lastState;

void setup() {
    if (DEBUG_ENABLED) {
        Serial.begin(115200);
        delay(250);
    }

    SetPins();
    LoadCurrentState();
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

    // Read GPIO every 150ms but change lights every 2 reads (for debouncing)
    static unsigned long lastRead = millis();
    if (millis() - lastRead > 150) {
        lastRead = millis();
        ProcessChanges();
    }
}

/******************************************************/
/***************** Setup Functions ********************/
/******************************************************/

void LoadCurrentState() {
    lastState = currentState = nvme.GetState();
    for (int i = 0; i < NUM_DEVICES; i++) {
        bool currState = (currentState >> i) & 0x1;

        if (USE_DIMMER[i])
            dimmer.SetState(i, currState);
        else
            digitalWrite(LIGHT_PINS[i], currState);
    }

    if (DEBUG_ENABLED) {
        Serial.println("\nCurrent lights state: ");
        for (int i = 0; i < NUM_DEVICES; i++)
            Serial.println((String)DEVICES[i] + ": " + ((currentState >> i) & 0x1));
    }
}

void SetPins() {
    for (int i = 0; i < NUM_DEVICES; i++) {
        pinMode(SWITCH_PINS[i], INPUT_PULLUP);

        if (!USE_DIMMER[i])
            pinMode(LIGHT_PINS[i], OUTPUT);
    }

    dimmer.Initialize();
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

        if (DEBUG_ENABLED)
            Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);

        for (int i = 0; i < NUM_DEVICES; i++) {
            if (strcmp(device_name, DEVICES[i]) == 0) {
                if (state) {
                    currentState |= 0x1 << i;
                    if (USE_DIMMER[i])
                        dimmer.SetBrightness(i, value);
                    else
                        digitalWrite(LIGHT_PINS[i], HIGH);
                }
                else {
                    currentState &= ~(0x1 << i);

                    if (USE_DIMMER[i])
                        dimmer.TurnOff(i);
                    else
                        digitalWrite(LIGHT_PINS[i], LOW);
                }
            }
        }
    });

    // Update Alexa status to current status
    for (int i = 0; i < NUM_DEVICES; i++)
        fauxmo.setState(DEVICES[i], (currentState >> i) & 0x1, 254);
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
                bool state = (currentState >> i) & 0x1;
                if (USE_DIMMER[i]) {
                    dimmer.SetState(i, state);
                    fauxmo.setState(DEVICES[i], state, dimmer.GetBrightness(i));
                }
                else {
                    digitalWrite(LIGHT_PINS[i], state);
                    fauxmo.setState(DEVICES[i], state, 254);
                }
            }
        }
        lastState = currentState;
    }
}

byte ReadSwitchState() {
    static byte lastRead = 0xFF;
    static byte beforeLast = lastRead;
    static byte lastChange = 0;

    // Read current switch positions
    byte currentRead = 0;
    for (int i = 0; i < NUM_DEVICES; i++) {
        if (digitalRead(SWITCH_PINS[i]))
            currentRead |= 0x1 << i;
        else
            currentRead &= ~(0x1 << i);
    }

    // Compare corrent read with last read to check if there is any change
    byte changes = 0;
    if (lastRead != 0xFF && lastRead != currentRead) {
        for (int i = 0; i < NUM_DEVICES; i++) {
            if (((currentRead >> i) & 0x1) != ((lastRead >> i) & 0x1))
                changes |= 0x1 << i;
        }
    }

    // Debouncing: Compare if current read is equal to last, but different to a previous one.
    // If different, it means that the switch remains on its position, so the switch changed
    // and the current position is the final position
    if (lastRead == currentRead && lastRead != beforeLast) {
        beforeLast = lastRead;
        changes = lastChange;
    }
    else {
        lastChange = changes;
        changes = 0;
    }

    lastRead = currentRead;
    return changes;
}

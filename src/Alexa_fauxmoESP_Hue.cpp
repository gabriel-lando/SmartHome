#include "Alexa.h"

#ifdef USE_FAUXMOESP

#include <fauxmoESP.h>    // https://github.com/vintlabs/fauxmoESP

#define MIN_BRIGHTNESS_FAUXMO 0       // Min value for brigtness (min value accepted by fauxmoESP)
#define MAX_BRIGHTNESS_FAUXMO 254     // Max value for brigtness (max value accepted by fauxmoESP)

static TStatusChangedCallback StatusChanged = NULL;

void StatusChangedCallback(byte id, bool state, byte value) {
    StatusChanged(id, state, value);
}

fauxmoESP *fauxmo;

Alexa::Alexa(){
    fauxmo = new fauxmoESP();
}

void Alexa::Initialize(byte currentState, TStatusChangedCallback statusChanged) {

    StatusChanged = statusChanged;

    // By default, fauxmoESP creates it's own webserver on the defined port
    // The TCP port must be 80 for gen3 devices (default is 1901)
    // This has to be done before the call to enable()
    fauxmo->createServer(true); // not needed, this is the default value
    fauxmo->setPort(80); // This is required for gen3 devices

    // You have to call enable(true) once you have a WiFi connection
    // You can enable or disable the library at any moment
    // Disabling it will prevent the devices from being discovered and switched
    fauxmo->enable(true);

    // Add virtual devices
    for (int i = 0; i < NUM_DEVICES; i++)
        fauxmo->addDevice(DEVICES[i]);

    fauxmo->onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {

        // Callback when a command from Alexa is received.
        // You can use device_id or device_name to choose the element to perform an action onto (relay, LED,...)
        // State is a boolean (ON/OFF) and value a number from 0 to 255 (if you say "set kitchen light to 50%" you will receive a 128 here).
        // Just remember not to delay too much here, this is a callback, exit as soon as possible.
        // If you have to do something more involved here set a flag and process it in your main loop.

        // Checking for device_id is simpler if you are certain about the order they are loaded and it does not change.
        // Otherwise comparing the device_name is safer.

        if (DEBUG_ENABLED)
            Serial.printf("[ALEXA] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);

        for (int i = 0; i < NUM_DEVICES; i++) {
            if (strcmp(device_name, DEVICES[i]) == 0) {
                value = map(value, MIN_BRIGHTNESS_FAUXMO, MAX_BRIGHTNESS_FAUXMO, MIN_BRIGHTNESS_ALEXA, MAX_BRIGHTNESS_ALEXA);
                StatusChangedCallback(i, state, value);
            }
        }
    });

    // Update Alexa status to current status
    for (int i = 0; i < NUM_DEVICES; i++)
        fauxmo->setState(DEVICES[i], (currentState >> i) & 0x1, MAX_BRIGHTNESS_FAUXMO);
}

void Alexa::SetState(byte state){
    for (int i = 0; i < NUM_DEVICES; i++)
        fauxmo->setState(DEVICES[i], (state >> i) & 0x1, MAX_BRIGHTNESS_FAUXMO);
}

void Alexa::SetState(byte id, bool state, int value) {
    value = map(value, MIN_BRIGHTNESS_ALEXA, MAX_BRIGHTNESS_ALEXA, MIN_BRIGHTNESS_FAUXMO, MAX_BRIGHTNESS_FAUXMO);
    fauxmo->setState(DEVICES[id], state, value);
}

void Alexa::Loop(){
    fauxmo->handle();
}

#endif
#include "Alexa.h"

#ifdef USE_FAUXOESP_WEMO

static TStatusChangedCallback StatusChanged = NULL;
static byte lastState = 0;

void StatusChangedCallback(byte id, bool state, byte value) {
    StatusChanged(id, state, value);
}

#include <fauxmoESP.h>    // https://bitbucket.org/xoseperez/fauxmoesp/get/2.4.4.zip

fauxmoESP *fauxmo;

Alexa::Alexa(){
    fauxmo = new fauxmoESP();
}

void Alexa::Initialize(byte currentState, TStatusChangedCallback statusChanged) {

    lastState = currentState;
    StatusChanged = statusChanged;

    // You have to call enable(true) once you have a WiFi connection
    // You can enable or disable the library at any moment
    // Disabling it will prevent the devices from being discovered and switched
    fauxmo->enable(true);

    // Add virtual devices
    for (int i = 0; i < NUM_DEVICES; i++)
        fauxmo->addDevice(DEVICES[i]);

    // fauxmoESP 2.0.0 has changed the callback signature to add the device_id,
    // this way it's easier to match devices to action without having to compare strings.
    fauxmo->onSetState([](unsigned char device_id, const char * device_name, bool state) {
        if (DEBUG_ENABLED)
            Serial.printf("[ALEXA] Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");

        for (int i = 0; i < NUM_DEVICES; i++) {
            if (strcmp(device_name, DEVICES[i]) == 0) {
                if (state)
                    lastState |= 0x1 << i;
                else
                    lastState &= ~(0x1 << i);

                StatusChangedCallback(i, state, 0);
            }
        }
    });

    fauxmo->onGetState([](unsigned char device_id, const char * device_name) {
        return (lastState >> device_id) & 0x1;
    });

    // Update Alexa status to current status
    for (int i = 0; i < NUM_DEVICES; i++)
        fauxmo->setState(i, (currentState >> i) & 0x1);
}

void Alexa::SetState(byte state){
    lastState = state;
    for (int i = 0; i < NUM_DEVICES; i++)
        fauxmo->setState(i, (state >> i) & 0x1);
}

void Alexa::SetState(byte id, bool state, int value) {
    if (state)
        lastState |= 0x1 << id;
    else
        lastState &= ~(0x1 << id);

    fauxmo->setState(id, state);
}

void Alexa::Loop(){
    fauxmo->handle();
}

#endif
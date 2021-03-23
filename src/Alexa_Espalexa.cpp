#include "Alexa.h"

#ifdef USE_ESPALEXA

#include <Espalexa.h>    // https://github.com/Aircoookie/Espalexa/releases/tag/v2.7.0

Espalexa *espalexa;
EspalexaDevice *devices[NUM_DEVICES];

static TStatusChangedCallback StatusChanged = NULL;

void StatusChangedCallback(EspalexaDevice *device) {
    if (DEBUG_ENABLED)
        Serial.printf("[ALEXA] Device #%d (%s) state: %s value: %d\n", device->getId(), device->getName().c_str(), device->getState() ? "ON" : "OFF", device->getValue());
    StatusChanged(device->getId(), device->getState(), device->getValue());
}

Alexa::Alexa() {
    espalexa = new Espalexa();
}

void Alexa::Initialize(byte currentState, TStatusChangedCallback statusChanged) {

    StatusChanged = statusChanged;

    for (int i = 0; i < NUM_DEVICES; i++) {
        devices[i] = new EspalexaDevice(DEVICES[i], StatusChangedCallback);
        espalexa->addDevice(devices[i]);
    }

    espalexa->begin();

    // Update Alexa status to current status
    for (int i = 0; i < NUM_DEVICES; i++){
        int value = ((currentState >> i) & 0x1) ? MAX_BRIGHTNESS_ALEXA : MIN_BRIGHTNESS_ALEXA;
        devices[i]->setValue(value);
    }
}

void Alexa::SetState(byte state){
    for (int i = 0; i < NUM_DEVICES; i++){
        int value = ((state >> i) & 0x1) ? MAX_BRIGHTNESS_ALEXA : MIN_BRIGHTNESS_ALEXA;
        devices[i]->setValue(value);
    }
}

void Alexa::SetState(byte id, bool state, int value) {
    value = (state) ? value : MIN_BRIGHTNESS_ALEXA;
    devices[id]->setValue(value);
}

void Alexa::Loop(){
    espalexa->loop();
}

#endif
#include "Lights.h"

Lights::Lights() {
    bool setDimmer = false;

    for (int i = 0; i < NUM_DEVICES; i++) {
        if (!USE_DIMMER[i])
            pinMode(LIGHT_PINS[i], OUTPUT);
        else
            setDimmer = true;
    }

    if (setDimmer) {
        SetDimmer(NUM_DEVICES, USE_DIMMER, LIGHT_PINS, ZC_DIMMER_PIN);
        Dimmer_Initialize();
    }
}

int Lights::GetBrightness(byte id) {
    if (USE_DIMMER[id])
        return Dimmer_GetBrightness (id);
    return MAX_BRIGHTNESS_DIMMER;
}

void Lights::SetState(byte state) {
    for (int i = 0; i < NUM_DEVICES; i++) {
        bool currState = (state >> i) & 0x1;
        SetState(i, currState);
    }
}

void Lights::SetState(byte id, bool state) {
    if (DEBUG_ENABLED)
        Serial.println((String)"[Lights] Set: " + id + ". Bool state: " + state);

    if (USE_DIMMER[id])
        Dimmer_SetState(id, state);
    else
        digitalWrite(LIGHT_PINS[id], LOW_LEVEL_RELAY[id] ? !state : state);
}

void Lights::SetState(byte id, int state) {
    if (DEBUG_ENABLED)
        Serial.println((String)"[Lights] Set: " + id + ". Int state: " + state);
    
    if (USE_DIMMER[id])
        Dimmer_SetBrightness(id, state);
    else
        digitalWrite(LIGHT_PINS[id], LOW_LEVEL_RELAY[id] ? !state : state);
}

void Lights::SetState(byte id, bool state, byte value) {
    if (USE_DIMMER[id]) {
        if (state)
            Dimmer_SetBrightness(id, value);
        else
            Dimmer_SetState(id, state);
    }
    else
        digitalWrite(LIGHT_PINS[id], LOW_LEVEL_RELAY[id] ? !state : state);
}
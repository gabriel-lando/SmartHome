#include "Switches.h"

Switches::Switches() {
    for (int i = 0; i < NUM_DEVICES; i++) {
        pinMode(SWITCH_PINS[i], INPUT_PULLUP);
    }
}

byte Switches::GetState() {
    // Read GPIO every DEBOUNCE_TIME_MS, but change lights every 2 reads (for debouncing)
    static unsigned long lastRead = millis();
    if (millis() - lastRead > this->DEBOUNCE_TIME_MS) {
        lastRead = millis();
        return ReadSwitchState();
    }
    return 0;
}

byte Switches::ReadSwitchState() {
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


/*void Switches::ProcessChanges(byte changes) {
    // Process changes
    if (changes != 0) {
        for (int i = 0; i < NUM_DEVICES; i++) {
            if ((changes >> i) & 0x1)
                this->cState ^= 0x1 << i;
        }
    }
}*/

/*
 // If there was any change, save it on EEPROM and update light status
    if (*lState != *cState) {
        nvme.SetState(*cState);

        if (changes != 0) {
            for (int i = 0; i < NUM_DEVICES; i++) {
                bool state = ((*cState) >> i) & 0x1;
                if (USE_DIMMER[i]) {
                    Dimmer_SetState(i, state);
                    fauxmo.setState(DEVICES[i], state, Dimmer_GetBrightness(i));
                }
                else {
                    digitalWrite(LIGHT_PINS[i], state);
                    fauxmo.setState(DEVICES[i], state, 254);
                }
            }
        }
        *lState = *cState;
    }
    */

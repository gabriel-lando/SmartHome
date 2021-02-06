#pragma once

#include <EEPROM.h>

class NVME {
private:
    const unsigned int EEPROM_SIZE = 4096;
    const unsigned int CURR_STATE_INIT = 0;
    const unsigned int CURR_STATE_END = (EEPROM_SIZE - 1);

    byte current_state;
    int addr_curr;
    bool wasInitialized = false;

    void UpdateEEPROM(unsigned int addr, byte value) {
        if (EEPROM.read(addr) != value) {
            EEPROM.write(addr, value);
        }
    }

public:
    NVME();
    byte GetState();
    void SetState(byte state);
};

NVME::NVME() {
    this->current_state = 0;
    this->addr_curr = 0;

    if (!wasInitialized) {
        EEPROM.begin(EEPROM_SIZE);
        wasInitialized = true;
    }
}

byte NVME::GetState() {
    if (this->addr_curr)
        return this->current_state;

    for (int i = CURR_STATE_INIT; i <= CURR_STATE_END; i++) {
        this->current_state = EEPROM.read(i);
        if (this->current_state) {
            this->addr_curr = i;
            break;
        }
    }

    return this->current_state;
}

void NVME::SetState(byte state) {
    if (state == this->current_state)
        return;

    if (this->addr_curr) {
        UpdateEEPROM(this->addr_curr, 0);
        EEPROM.commit();
    }

    randomSeed(millis());
    this->addr_curr = random(CURR_STATE_INIT, CURR_STATE_END);
    UpdateEEPROM(this->addr_curr, state);
    EEPROM.commit();

    this->current_state = state;
    return;
}

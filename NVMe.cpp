#include "NVMe.h"

NVME::NVME() {
    this->current_state = 0;
    this->addr_curr = -1;

    if (!wasInitialized) {
        EEPROM.begin(EEPROM_SIZE);
        wasInitialized = true;
    }
}

byte NVME::GetState() {
    if (this->addr_curr != -1)
        return this->current_state;

    int count = 0;
    this->current_state = 0;

    for (int i = CURR_STATE_INIT; i <= CURR_STATE_END; i++) {
        byte state = EEPROM.read(i);
        if (state) {
            this->current_state = state;
            this->addr_curr = i;
            if (count++ > 1) break;
        }
    }

    if (count > 1) {
        ClearEEPROM();
        this->current_state = 0;
        this->addr_curr = -1;
    }

    return this->current_state;
}

void NVME::SetState(byte state) {
    Serial.println((String)"[NVMe] Current state: " + this->current_state + ". Addr: " + this->addr_curr);

    if (state == this->current_state)
        return;

    if (this->addr_curr != -1) {
        UpdateEEPROM(this->addr_curr, 0);
        EEPROM.commit();
    }

    randomSeed(millis());
    this->addr_curr = random(CURR_STATE_INIT, CURR_STATE_END);
    UpdateEEPROM(this->addr_curr, state);
    EEPROM.commit();

    this->current_state = state;

    Serial.println((String)"[NVMe] Updated state to: " + state + ". Addr: " + this->addr_curr);

    return;
}

void NVME::UpdateEEPROM(unsigned int addr, byte value) {
    if (EEPROM.read(addr) != value) {
        EEPROM.write(addr, value);
    }
}

void NVME::ClearEEPROM() {
    Serial.println("[NVMe] Cleaning EEPROM.");
    for (int i = CURR_STATE_INIT; i <= CURR_STATE_END; i++)
        UpdateEEPROM(i, 0);
    EEPROM.commit();
}

#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include "../Settings.h"

class NVME {
    private:
        const unsigned int EEPROM_SIZE = 4096;
        const unsigned int CURR_STATE_INIT = 0;
        const unsigned int CURR_STATE_END = (EEPROM_SIZE - 1);

        byte current_state;
        int addr_curr;
        bool wasInitialized = false;

    protected:
        void UpdateEEPROM(unsigned int addr, byte value);
        void ClearEEPROM();

    public:
        NVME();
        byte GetState();
        void SetState(byte state);
};

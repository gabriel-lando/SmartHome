#pragma once

#include <Arduino.h>
#include "../Settings.h"

class Switches {
    private:
        const int DEBOUNCE_TIME_MS = 100;
        byte ReadSwitchState();

    public:
        Switches();
        byte GetState();
};
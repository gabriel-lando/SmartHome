#pragma once

#include <Arduino.h>
#include "../Settings.h"
#include "Dimmer.h"

class Lights {
    public:
        Lights();
        int  GetBrightness(byte id);
        void SetState(byte state);
        void SetState(byte id, bool state);
        void SetState(byte id, int state);
        void SetState(byte id, bool state, byte value);
};
#pragma once

#include <Arduino.h>
#include <functional>
#include "../Settings.h"

#define MIN_BRIGHTNESS_ALEXA 0       // Min value for brigtness (received from Alexa)
#define MAX_BRIGHTNESS_ALEXA 255     // Max value for brigtness (received from Alexa)

typedef std::function<void (byte, bool, byte)> TStatusChangedCallback;

class Alexa {
    public:
        Alexa();
        void Initialize(byte currentState, TStatusChangedCallback statusChanged);
        void SetState(byte state);
        void SetState(byte id, bool state, int value);
        void Loop();
};
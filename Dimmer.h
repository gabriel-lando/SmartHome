#pragma once

#include <algorithm>
#include <ESP8266TimerInterrupt.h>

ESP8266Timer ITimer;

class Dimmer {
    private:
        const int FREQ_STEP_MS = 16;        // 16 us for each tick on timer
        const int MIN_BRIGHTNESS = 0;       // Min value for brigtness (received from FauxmoESP)
        const int MAX_BRIGHTNESS = 254;     // Max value for brigtness (received from FauxmoESP)

        const int MIN_DIMMER = 483;         // Dimmer value tr Turn Off the Light
        const int MAX_DIMMER = 0;           // Dimmer value tr Turn On the Light

        static volatile bool *zeroCross;    // Passed on a Zero Cross
        static volatile int *tickCounter;       // Count ticks on Timer

        static volatile int *currentDimmerValue;
        static int *lastDimmerValue;

        static int *currentBrightness;
        static int *pinsZC;
        static int *pinsDim;

        static int qtyDevices;
        static bool *useDimmer;

        static void ICACHE_RAM_ATTR HandleZeroCross();
        static void ICACHE_RAM_ATTR OnTimerISR();

    public:
        Dimmer(int qty, const bool *useDimmer, const int *pinsZC, const int *pinsDim);
        void Initialize();
        void SetBrightness(int id, int value);
        int GetBrightness(int id);
        void TurnOn(int id);
        void TurnOff(int id);
        void SetState(int id, bool state);
};

Dimmer::Dimmer(int qty, const bool *useDimmer, const int *pinsZC, const int *pinsDim) {

    Dimmer::qtyDevices = qty;

    /*  Dimmer::useDimmer = useDimmer;
        Dimmer::pinsZC = pinsZC;
        Dimmer::pinsDim = pinsDim;*/
    Dimmer::useDimmer = (bool*)malloc(qty * sizeof(bool));
    Dimmer::pinsZC = (int*)malloc(qty * sizeof(int));
    Dimmer::pinsDim = (int*)malloc(qty * sizeof(int));

    std::copy(useDimmer, useDimmer + qty, Dimmer::useDimmer);
    std::copy(pinsZC, pinsZC + qty, Dimmer::pinsZC);
    std::copy(pinsDim, pinsDim + qty, Dimmer::pinsDim);

    Dimmer::currentBrightness = (int*)malloc(qty * sizeof(int));
    Dimmer::currentDimmerValue = (int*)malloc(qty * sizeof(int));
    Dimmer::lastDimmerValue = (int*)malloc(qty * sizeof(int));

    Dimmer::zeroCross = (bool*)malloc(qty * sizeof(bool));
    Dimmer::tickCounter = (int*)malloc(qty * sizeof(int));
}

void Dimmer::Initialize() {
    bool setISRs = false;
    int zc_pin = 0;

    for (int i = 0; i < Dimmer::qtyDevices; i++) {
        if (!Dimmer::useDimmer[i])
            continue;

        setISRs = true;
        zc_pin = Dimmer::pinsZC[i];

        Dimmer::currentBrightness[i] = MIN_BRIGHTNESS;
        Dimmer::currentDimmerValue[i] = MIN_DIMMER;
        Dimmer::lastDimmerValue[i] = MAX_DIMMER;

        Dimmer::zeroCross[i] = false;
        Dimmer::tickCounter[i] = 0;

        pinMode(Dimmer::pinsDim[i], OUTPUT);
    }

    if (setISRs) {
        ITimer.attachInterruptInterval(this->FREQ_STEP_MS, OnTimerISR);
        attachInterrupt(digitalPinToInterrupt(zc_pin), HandleZeroCross, RISING);
    }
}

void Dimmer::SetBrightness(int id, int value) {
    if (value > MAX_BRIGHTNESS)
        value = MAX_BRIGHTNESS;
    else if (value < MIN_BRIGHTNESS)
        value = MIN_BRIGHTNESS;

    int new_value = map(value, MIN_BRIGHTNESS, MAX_BRIGHTNESS, MIN_DIMMER, MAX_DIMMER);

    Dimmer::currentBrightness[id] = value;
    Dimmer::currentDimmerValue[id] = new_value;
}

int Dimmer::GetBrightness(int id) {
    return Dimmer::currentBrightness[id];
}

void Dimmer::TurnOn(int id) {
    Dimmer::currentDimmerValue[id] = Dimmer::lastDimmerValue[id];
}

void Dimmer::TurnOff(int id) {
    Dimmer::lastDimmerValue[id] = Dimmer::currentDimmerValue[id];
    Dimmer::currentDimmerValue[id] = MIN_DIMMER;
}

void Dimmer::SetState(int id, bool state) {
    if (state)
        TurnOn(id);
    else
        TurnOff(id);
}

void Dimmer::HandleZeroCross() {
    // set the boolean to true to tell our dimming function that a zero cross has occured

    for (int i = 0; i < Dimmer::qtyDevices; i++) {
        if (!Dimmer::useDimmer[i])
            continue;

        Dimmer::zeroCross[i] = true;
        Dimmer::tickCounter[i] = 0;
        digitalWrite(Dimmer::pinsDim[i], LOW);
    }
}

void Dimmer::OnTimerISR() {
    for (int i = 0; i < Dimmer::qtyDevices; i++) {
        if (!Dimmer::useDimmer[i])
            continue;

        if (Dimmer::zeroCross[i] == true) {
            if (Dimmer::tickCounter[i] >= Dimmer::currentDimmerValue[i]) {
                digitalWrite(Dimmer::pinsDim[i], HIGH); // turn on light
                Dimmer::tickCounter[i] = 0;             // reset time step counter
                Dimmer::zeroCross[i] = false;           //reset zero cross detection
            }
            else {
                Dimmer::tickCounter[i]++; // increment time step counter
            }
        }
    }
}

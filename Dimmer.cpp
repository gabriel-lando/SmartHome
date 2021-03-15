#include "Dimmer.h"
#include <algorithm>

ESP8266Timer *ITimer;

volatile bool* zeroCross = nullptr;    // Passed on a Zero Cross
volatile int* tickCounter = nullptr;       // Count ticks on Timer

volatile int* currentDimmerValue = nullptr;
int* lastDimmerValue = nullptr;

int* currentBrightness = nullptr;
int* pinsDim = nullptr;
int pinZC = 0;

int qtyDevices = 0;
bool* useDimmer = nullptr;

void ICACHE_RAM_ATTR HandleZeroCross();
void ICACHE_RAM_ATTR OnTimerISR();

void SetDimmer(int qty, const bool* _useDimmer, const int* _pinsDim, const int _pinZC) {
    qtyDevices = qty;
    pinZC = _pinZC;

    useDimmer = (bool*)malloc(qty * sizeof(bool));
    pinsDim = (int*)malloc(qty * sizeof(int));

    std::copy(_useDimmer, _useDimmer + qty, useDimmer);
    std::copy(_pinsDim, _pinsDim + qty, pinsDim);

    currentBrightness = (int*)malloc(qty * sizeof(int));
    currentDimmerValue = (int*)malloc(qty * sizeof(int));
    lastDimmerValue = (int*)malloc(qty * sizeof(int));

    zeroCross = (bool*)malloc(qty * sizeof(bool));
    tickCounter = (int*)malloc(qty * sizeof(int));
}

void Dimmer_Initialize() {
    bool setISRs = false;

    for (int i = 0; i < qtyDevices; i++) {
        if (!useDimmer[i])
            continue;

        setISRs = true;

        currentBrightness[i] = MIN_BRIGHTNESS;
        currentDimmerValue[i] = MAX_DIMMER;
        lastDimmerValue[i] = MAX_DIMMER;

        zeroCross[i] = false;
        tickCounter[i] = 0;

        pinMode(pinsDim[i], OUTPUT);
    }

    if (setISRs) {
        ITimer = new ESP8266Timer();
        ITimer->attachInterruptInterval(FREQ_STEP_MS, OnTimerISR);
        attachInterrupt(digitalPinToInterrupt(pinZC), HandleZeroCross, RISING);
    }
}

void Dimmer_SetBrightness(int id, int value) {
    if (value > MAX_BRIGHTNESS)
        value = MAX_BRIGHTNESS;
    else if (value < MIN_BRIGHTNESS)
        value = MIN_BRIGHTNESS;

    int new_value = map(value, MIN_BRIGHTNESS, MAX_BRIGHTNESS, MIN_DIMMER, MAX_DIMMER);

    currentBrightness[id] = value;
    currentDimmerValue[id] = new_value;
}

int Dimmer_GetBrightness(int id) {
    return currentBrightness[id];
}

void Dimmer_TurnOn(int id) {
    currentDimmerValue[id] = lastDimmerValue[id];
}

void Dimmer_TurnOff(int id) {
    lastDimmerValue[id] = currentDimmerValue[id];
    currentDimmerValue[id] = MIN_DIMMER;
}

void Dimmer_SetState(int id, bool state) {
    if (state)
        Dimmer_TurnOn(id);
    else
        Dimmer_TurnOff(id);
}

void HandleZeroCross() {
    // set the boolean to true to tell our dimming function that a zero cross has occured

    for (int i = 0; i < qtyDevices; i++) {
        if (!useDimmer[i])
            continue;

        zeroCross[i] = true;
        tickCounter[i] = 0;
        digitalWrite(pinsDim[i], LOW);
    }
}

void OnTimerISR() {
    for (int i = 0; i < qtyDevices; i++) {
        if (!useDimmer[i])
            continue;

        if (zeroCross[i] == true) {
            if (tickCounter[i] >= currentDimmerValue[i]) {
                digitalWrite(pinsDim[i], HIGH); // turn on light
                tickCounter[i] = 0;             // reset time step counter
                zeroCross[i] = false;           //reset zero cross detection
            }
            else {
                tickCounter[i]++; // increment time step counter
            }
        }
    }
}

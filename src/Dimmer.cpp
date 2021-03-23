#include "Dimmer.h"

ESP8266Timer *ITimer;

volatile bool* zeroCross = nullptr;    // Passed on a Zero Cross
volatile int* tickCounter = nullptr;       // Count ticks on Timer

volatile int* currentDimmerValue = nullptr;
volatile int* lastDimmerValue = nullptr;

volatile int* currentBrightness = nullptr;
volatile int* lastBrightness = nullptr;

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
    lastBrightness = (int*)malloc(qty * sizeof(int));
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

        currentBrightness[i] = MAX_BRIGHTNESS_DIMMER;
        lastBrightness[i] = MAX_BRIGHTNESS_DIMMER;
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
    if (DEBUG_ENABLED)
        Serial.println((String)"[Dimmer] Set Brightness: " + id + ". Value: " + value);

    int new_value = map(value, MIN_BRIGHTNESS_DIMMER, MAX_BRIGHTNESS_DIMMER, MIN_DIMMER, MAX_DIMMER);

    lastBrightness[id] = currentBrightness[id];
    currentBrightness[id] = value;
    currentDimmerValue[id] = new_value;
}

int Dimmer_GetBrightness(int id) {
    if (DEBUG_ENABLED)
        Serial.println((String)"[Dimmer] Current state: " + id + ". Value: " + currentBrightness[id] + ". Last value: " + lastBrightness[id]);
    
    if (currentBrightness[id] == MIN_BRIGHTNESS_DIMMER)
        return lastBrightness[id];
    return currentBrightness[id];
}

void Dimmer_TurnOn(int id) {
    currentBrightness[id] = lastBrightness[id];
    currentDimmerValue[id] = lastDimmerValue[id];
}

void Dimmer_TurnOff(int id) {
    lastBrightness[id] = currentBrightness[id];
    lastDimmerValue[id] = currentDimmerValue[id];
    currentDimmerValue[id] = MIN_DIMMER;
    currentBrightness[id] = MIN_BRIGHTNESS_DIMMER;
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
        if (!useDimmer[i] || currentDimmerValue[i] == MIN_DIMMER)
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

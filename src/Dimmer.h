#pragma once

#include <Arduino.h>
#include <ESP8266TimerInterrupt.h>
#include <algorithm>
#include "../Settings.h"

#define FREQ_STEP_MS 16        // 16 us for each tick on timer
#define MIN_BRIGHTNESS_DIMMER 0       // Min value for brigtness (received from Alexa)
#define MAX_BRIGHTNESS_DIMMER 255     // Max value for brigtness (received from Alexa)

#define MIN_DIMMER 448         // Dimmer value to Turn Off the Light
#define MAX_DIMMER 0           // Dimmer value to Turn On the Light

void SetDimmer(int qty, const bool* _useDimmer, const int* _pinsDim, const int _pinZC);
void Dimmer_Initialize();
void Dimmer_SetBrightness(int id, int value);
int  Dimmer_GetBrightness(int id);
void Dimmer_TurnOn(int id);
void Dimmer_TurnOff(int id);
void Dimmer_SetState(int id, bool state);

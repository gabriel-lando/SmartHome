#pragma once

#include <Arduino.h>
#include <ESP8266TimerInterrupt.h>

#define FREQ_STEP_MS 16        // 16 us for each tick on timer
#define MIN_BRIGHTNESS 0       // Min value for brigtness (received from FauxmoESP)
#define MAX_BRIGHTNESS 254     // Max value for brigtness (received from FauxmoESP)

#define MIN_DIMMER 483         // Dimmer value tr Turn Off the Light
#define MAX_DIMMER 0           // Dimmer value tr Turn On the Light

void SetDimmer(int qty, const bool* _useDimmer, const int* _pinsZC, const int* _pinsDim);
void Dimmer_Initialize();
void Dimmer_SetBrightness(int id, int value);
int Dimmer_GetBrightness(int id);
void Dimmer_TurnOn(int id);
void Dimmer_TurnOff(int id);
void Dimmer_SetState(int id, bool state);

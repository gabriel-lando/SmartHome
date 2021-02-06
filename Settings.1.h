#pragma once
#define DEBUG_ENABLED false
#define NUM_DEVICES 3

char DEVICES[][25] = { "Bedroom Light", "TV Light Left", "TV Light Right" };
int LIGHT_PINS[] = { D1, D2, D0 };
int SWITCH_PINS[] = { D5, D6, D7 };

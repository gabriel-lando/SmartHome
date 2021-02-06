#pragma once
#define DEBUG_ENABLED false
#define NUM_DEVICES 2

char DEVICES[][25] = { "Living Room Light", "Dinner Room Light" };
int LIGHT_PINS[] = { D2, D3 };
int SWITCH_PINS[] = { D7, D5 };

/*
 * ESP8366 pinout: https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
 */

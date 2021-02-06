#pragma once
#define DEBUG_ENABLED false
#define NUM_DEVICES 1

char DEVICES[][25] = { "Door Light" };
int LIGHT_PINS[] = { D3 };
int SWITCH_PINS[] = { D7 };

/*
 * ESP8366 pinout: https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
 */

/*
 Name:		AlexaESP.ino
 Created:	5/14/2020 11:42:41 AM
 Author:	Gabriel Lando
*/

#include "wifi.h"

WIFI wifi = WIFI();

// the setup function runs once when you press reset or power the board
void setup() {
    delay(1500);
    Serial.begin(115200);
    Serial.println();

    if (wifi.Connect()) {
        Serial.print("Connected. IP: ");
        Serial.println(wifi.localIP());
    }
    else
        ESP.restart();
}

// the loop function runs over and over again until power down or reset
void loop() {
    delay(10000);
}

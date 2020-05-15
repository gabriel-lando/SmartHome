#include <EEPROM.h>

#define EEPROM_SIZE 4096

void setup() {
  Serial.begin(115200);//Inicia a comunicação Serial.
  EEPROM.begin(EEPROM_SIZE);//Inicia a EEPROM com tamanho de 4 Bytes (minimo).

  delay(5000);
  Serial.println();
  Serial.println("Starting EEPROM Read");

  for (int idx = 0; idx < EEPROM_SIZE; idx++) {
    byte last = EEPROM.read(idx);
    if (!(idx % 75))
      Serial.println();
    Serial.print(last);
    Serial.print(" ");
  }
}

void loop() {}

#include <EEPROM.h>

#define EEPROM_SIZE 4096

void setup() {
  Serial.begin(115200);//Inicia a comunicação Serial.
  EEPROM.begin(EEPROM_SIZE);//Inicia a EEPROM com tamanho de 4 Bytes (minimo).

  delay(1000);
  Serial.println();
  Serial.println();
  Serial.println("Cleaning EEPROM...");

  double st = micros();
  for (int idx = 0; idx < EEPROM_SIZE; idx++) {
    byte last = EEPROM.read(idx);
    if (last != 0)
      EEPROM.write(idx, 0);
  }
  EEPROM.commit();
  double ed = micros() - st;

  Serial.print("EEPROM Cleaned in ");
  Serial.print(ed);
  Serial.println(" us.");
  Serial.println();
}

void loop() {}

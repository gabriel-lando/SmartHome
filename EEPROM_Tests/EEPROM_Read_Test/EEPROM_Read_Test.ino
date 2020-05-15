#include <EEPROM.h>

#define EEPROM_SIZE 4096

void setup() {
  Serial.begin(115200);//Inicia a comunicação Serial.
  EEPROM.begin(EEPROM_SIZE);//Inicia a EEPROM com tamanho de 4 Bytes (minimo).

  delay(5000);
  Serial.println();
  Serial.println("Starting EEPROM Read");

  double st = micros();
  for (int idx = 0; idx < EEPROM_SIZE; idx++)
    EEPROM.read(idx);
  double ed = micros() - st;

  Serial.println();
  Serial.print("Time Read: ");
  Serial.println(ed);
  Serial.println();

  delay(5000);
  Serial.println("Starting EEPROM Read/Write");

  double st2 = micros();
  for (int idx = 0; idx < EEPROM_SIZE; idx++) {
    byte last = EEPROM.read(idx);
    if (last != 0)
      EEPROM.write(idx, 0);
  }
  EEPROM.commit();
  double ed2 = micros() - st2;

  Serial.println();
  Serial.print("Time Write: ");
  Serial.println(ed2);
  Serial.println();

  delay(5000);


  for (int idx = 0; idx < EEPROM_SIZE; idx++) {
    byte last = EEPROM.read(idx);
    if (!(idx % 100))
      Serial.println();
    Serial.print(last);
    Serial.print(" ");
  }
}

void loop() {}

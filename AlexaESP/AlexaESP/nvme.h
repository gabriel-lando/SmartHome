#pragma once

#include <EEPROM.h>

class NVME {
private:
    const unsigned int EEPROM_SIZE = 4096;
    const unsigned int WIFI_USR_INIT = 4000;
    const unsigned int WIFI_PWD_INIT = 4032;
    const unsigned int CURR_STATE_INIT = 10;
    const unsigned int CURR_STATE_END = (WIFI_USR_INIT - 1);

    byte current_state;
    int addr_curr;
    bool wasInitialized = false;

    void UpdateEEPROM(unsigned int addr, byte value) {
        if (EEPROM.read(addr) != value) {
            EEPROM.write(addr, value);
        }
    }

public:
    NVME();
    byte GetState();
    void SetState(byte state);
    String GetWiFiUser();
    bool SetWiFiUser(String data);
    String GetWiFiPwd();
    bool SetWiFiPwd(String data);

};

NVME::NVME() {
    this->current_state = 0;
    this->addr_curr = 0;

    if (!wasInitialized) {
        EEPROM.begin(EEPROM_SIZE);
        wasInitialized = true;
    }
}

byte NVME::GetState() {
    if (this->addr_curr)
        return this->current_state;

    for (int i = CURR_STATE_INIT; i <= CURR_STATE_END; i++) {
        this->current_state = EEPROM.read(i);
        if (this->current_state) {
            this->addr_curr = i;
            break;
        }
    }

    return this->current_state;
}

void NVME::SetState(byte state) {
    if (state == this->current_state)
        return;

    if (this->addr_curr) {
        UpdateEEPROM(this->addr_curr, 0);
        EEPROM.commit();
    }

    randomSeed(millis());
    this->addr_curr = random(CURR_STATE_INIT, CURR_STATE_END);
    UpdateEEPROM(this->addr_curr, state);
    EEPROM.commit();
    return;
}

String NVME::GetWiFiUser() {
    const unsigned int _size = WIFI_PWD_INIT - WIFI_USR_INIT;
    
    char *data = (char*)malloc(_size * sizeof(char));

    int len = 0;
    unsigned char k;
    k = EEPROM.read(WIFI_USR_INIT);
    while ((k != '\0' && k != 0) && len < _size) {
        k = EEPROM.read(WIFI_USR_INIT + len);
        data[len] = k;
        len++;
    }
    data[len] = '\0';
    return String(data);
}

bool NVME::SetWiFiUser(String data) {
    const unsigned int _size = WIFI_PWD_INIT - WIFI_USR_INIT;

    if (data.length() > _size)
        return false;

    for (int i = 0; i < data.length(); i++)
        UpdateEEPROM(WIFI_USR_INIT + i, data[i]);
    EEPROM.commit();

    return true;
}

String NVME::GetWiFiPwd() {
    const unsigned int _size = EEPROM_SIZE - WIFI_PWD_INIT;

    char* data = (char*)malloc(_size * sizeof(char));

    int len = 0;
    unsigned char k;
    k = EEPROM.read(WIFI_PWD_INIT);
    while ((k != '\0' && k != 0) && len < _size) {
        k = EEPROM.read(WIFI_PWD_INIT + len);
        data[len] = k;
        len++;
    }
    data[len] = '\0';
    return String(data);
}

bool NVME::SetWiFiPwd(String data) {
    const unsigned int _size = EEPROM_SIZE - WIFI_PWD_INIT;

    if (data.length() > _size)
        return false;

    for (int i = 0; i < data.length(); i++)
        UpdateEEPROM(WIFI_PWD_INIT + i, data[i]);
    EEPROM.commit();

    return true;
}

// MAC Address: 5C:CF:7F:78:11:40

#pragma once

#define DEBUG_ENABLED false
#define NUM_DEVICES 1
#define USE_FAUXMOESP

const char DEVICES[][25] = { "Door Light" };
const int SWITCH_PINS[] = { 13 };
const int LIGHT_PINS[] = { 0 };
const bool LOW_LEVEL_RELAY[] = { false };

const bool USE_DIMMER[] = { false };
const int ZC_DIMMER_PIN = 0;


/************************************************************************************************************************************
 * Alexa ESP Libraries:                                                                                                             *
 * => USE_FAUXMOESP:                                                                                                                *
 *      vintlabs/fauxmoESP v3.2: https://github.com/vintlabs/fauxmoESP/releases/tag/3.2                                             *
 * => USE_ESPALEXA:                                                                                                                 *
 *      Aircoookie/Espalexa v2.7.0: https://github.com/Aircoookie/Espalexa/releases/tag/v2.7.0                                      *
 * => USE_FAUXOESP_WEMO:                                                                                                            *
 *      xoseperez/fauxmoESP v2.4.4: https://bitbucket.org/xoseperez/fauxmoesp/get/2.4.4.zip                                         *
 *      - THIS LIBRARY DOESN'T SUPPORT DIMMER: Make sure you set all USE_DIMMER[] as false when using this library                  *
 *                                                                                                                                  *
 ************************************************************************************************************************************
 *                                                                                                                                  *
 * ESP8366 pinout: https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/                                                  *
 *                                                                                                                                  *
 * Label  GPIO      Input           Output                  Notes                                                                   *
 * -------------------------------------------------------------------------------------------------------------------------------- *
 * D0     GPIO16    no interrupt    no PWM or I2C support   HIGH at boot - used to wake up from deep sleep                          *
 * D1     GPIO5     OK              OK                      often used as SCL (I2C)                                                 *
 * D2     GPIO4     OK              OK                      often used as SDA (I2C)                                                 *
 * D3     GPIO0     pulled up       OK                      connected to FLASH button, boot fails if pulled LOW                     *
 * D4     GPIO2     pulled up       OK                      HIGH at boot - connected to on-board LED, boot fails if pulled LOW      *
 * D5     GPIO14    OK              OK                      SPI (SCLK)                                                              *
 * D6     GPIO12    OK              OK                      SPI (MISO)                                                              *
 * D7     GPIO13    OK              OK                      SPI (MOSI)                                                              *
 * D8     GPIO15    pulled to GND   *OK*                    SPI (CS) Boot fails if pulled HIGH                                      *
 * RX     GPIO3     *OK*            RX pin (X)              HIGH at boot                                                            *
 * TX     GPIO1     TX pin (X)      *OK*                    HIGH at boot - debug output at boot, boot fails if pulled LOW           *
 * A0     ADC0      Analog Input    (X)                                                                                             *
 *                                                                                                                                  *
 ***********************************************************************************************************************************/

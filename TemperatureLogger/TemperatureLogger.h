
#ifndef TemperatureLogger_H_
#define TemperatureLogger_H_
#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif

#define DEBUG 1

// Sampling configuration

#define SEC *1000L
#define MIN *1000L*60L
#define SAMPLE_INTERVAL (5 MIN)

// Analogue/Digital input logging configuration

#define ADC_CHANNEL_COUNT 1
//#define DIGITAL_CHANNEL_COUNT 0

// Alarm configuration

#define SD_ALARM_PIN 3
// Temperature sensor configuration

#define ONE_WIRE_BUS_PIN 2
#define TEMPERATURE_PRECISION 12

// SD configuration

#define ENABLE_SD
#define SD_CS_PIN 4

// Nimbits configuration

//#define ENABLE_NIMBITS

#define MAC_ADDRESS { 0, 0, 0, 0, 0, 0, 0, 0 }

#define NIMBITS_SERVER "nimbits1"
#define NIMBITS_EMAIL  "pelrun@gmail.com"
#define NIMBITS_KEY    "temptestkey"

// timezone offset in seconds from UTC

#define UTC 3600*
#define TIMEZONE_OFFSET (UTC +10)

// LCD configuration

//#define ENABLE_LCD

#define LCD_UPDATE_INTERVAL 100

#define LCD_ROWS 2
#define LCD_COLUMNS 16

#define LCD_RS 0
#define LCD_ENABLE 0
#define LCD_D0 0
#define LCD_D1 0
#define LCD_D2 0
#define LCD_D3 0

#define LCD_BUTTON_UP 3
#define LCD_BUTTON_DOWN 4

#define LCD_BUTTON_DEBOUNCE_TIMEOUT 100

#endif /* TemperatureLogger_H_ */

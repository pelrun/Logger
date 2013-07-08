#include "TemperatureLogger.h"

#include <PString.h>

#include "TempSensors.h"

static TempSensors tSensors;

//#define ENABLE_SD
#ifdef ENABLE_SD
#include <SD.h>

// 2013/12/31/235959.txt - filename length always 21 chars
char logFilename[22];
#endif

#include <DigitalIO.h>
#include <RTClib.h>

#define ENABLE_HARDWARE_RTC
#ifdef ENABLE_HARDWARE_RTC
RTC_DS1307 rtc;
#else
RTC_Millis rtc;
#endif

//#define ENABLE_NIMBITS
#ifdef ENABLE_NIMBITS
#include <SPI.h>
#include <Ethernet.h>
#include <Nimbits.h>

Nimbits nimbits(NIMBITS_SERVER, NIMBITS_EMAIL, NIMBITS_KEY);
#endif

//#define ENABLE_LCD
#ifdef ENABLE_LCD

#include <LiquidCrystal.h>

LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_D0, LCD_D1, LCD_D2, LCD_D3);

uint8_t lcd_page = 0;

#endif

static void setupNimbits()
{
#ifdef ENABLE_NIMBITS
  uint8_t mac[8] = MAC_ADDRESS;
  Ethernet.begin(mac);
#endif
}

static void setupRTC()
{
  // Use hardware RTC if enabled
  // Else get nimbits time if enabled (signal error if request failed)
  // fall back to compile time if neither available (for testing only!)

#ifdef ENABLE_HARDWARE_RTC
  // Turn D13 into Vcc for the rtc :D
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  rtc.begin();
//  rtc.adjust(DateTime(__DATE__, __TIME__));

#else
#ifdef ENABLE_NIMBITS
  {
    long unixtime = nimbits.getTime();
    if (unixtime > 0)
    {
      rtc.begin(DateTime(unixtime+TIMEZONE_OFFSET));
    }
    else
    {
      // no other time source available
      // signal error condition
    }
  }
#else
  rtc.begin(DateTime(__DATE__, __TIME__));
#endif // ENABLE_NIMBITS
#endif // ENABLE_HARDWARE_RTC
}

static void setupLCD()
{
#ifdef ENABLE_LCD
  lcd.begin(LCD_COLUMNS, LCD_ROWS, 0);
#endif
}

static void setupSD()
{
#ifdef ENABLE_SD
  PString filename(logFilename, sizeof(logFilename));
  DateTime now = rtc.now();

  SD.begin(SD_CS_PIN);

  filename.print(now.year());
  filename.print(F("/"));
  if (now.month() < 10) filename.print(F("0"));
  filename.print(now.month());
  filename.print(F("/"));
  if (now.day() < 10) filename.print(F("0"));
  filename.print(now.day());

  SD.mkdir(logFilename);

  filename.print(F("/"));
  if (now.hour() < 10) filename.print(F("0"));
  filename.print(now.hour());
  if (now.minute() < 10) filename.print(F("0"));
  filename.print(now.minute());
  if (now.second() < 10) filename.print(F("0"));
  filename.print(now.second());
  filename.print(F(".txt"));

  Serial.print(F("Log filename: "));
  Serial.println(filename);

#endif

  pinMode(SD_ALARM_PIN, OUTPUT);
  digitalWrite(SD_ALARM_PIN, LOW);
}

static void updateLogFile(const String &text)
{
#ifdef ENABLE_SD
  SD.begin(SD_CS_PIN);

  File logFile = SD.open(logFilename, O_CREAT | O_WRITE | O_SYNC);

  if (logFile)
  {
    logFile.println(text);
    logFile.close();

    if (!logFile.getWriteError())
    {
      digitalWrite(SD_ALARM_PIN, LOW);
      return;
    }
  }

  // There was an error, SD card missing or full
  Serial.println(F("SD fail"));
  // Set alarm pin
  digitalWrite(SD_ALARM_PIN, HIGH);
#endif
}

static String address_to_string(DeviceAddress &address)
{
  char addressString[17];
  PString string(addressString, sizeof(addressString));

  for (int i = 0; i < 16; i++)
  {
    string.print(address[i], HEX);
  }

  return String(string);
}

static float adc_to_voltage(int adc_value)
{
  return (float)(adc_value * 5) / 1023;
}

unsigned long next_measurement_time;
unsigned long next_lcd_update_time;

void setup()
{
  String header = "seconds,";
  uint8_t index;

  Serial.begin(57600);
  tSensors.begin();

  setupNimbits();
  setupRTC();
  setupLCD();
  setupSD();

#if ADC_CHANNEL_COUNT > 0
    for (int i = 0; i < ADC_CHANNEL_COUNT; i++)
    {
      header += "A";
      header += i;
      header += ",";
    }
#endif

  for (index = 0; index < tSensors.getDeviceCount(); index++)
  {
    DeviceAddress address;

    if (tSensors.getAddress(index, address))
    {
      String addr_string = address_to_string(address);
      header += addr_string;

#ifdef ENABLE_NIMBITS
      nimbits.createPoint(addr_string);
#endif
    }

      header += ",";
  }

  header = header.substring(0,header.length()-1);

  updateLogFile(header);

  Serial.println(header);

  next_measurement_time = millis();
  next_lcd_update_time = millis();
}

void loop()
{
  char temp[10];

  if (millis() > next_measurement_time)
  {
    uint8_t index;
    String output;

    next_measurement_time += SAMPLE_INTERVAL;

    output = String(millis()/1000);
    output += ",";

#if ADC_CHANNEL_COUNT > 0
    for (int adc_channel = 0; adc_channel < ADC_CHANNEL_COUNT; adc_channel++)
    {
      float adc_read = adc_to_voltage(analogRead(adc_channel));

      output += dtostrf(adc_read, 1, 2, temp);
      output += ",";

#ifdef ENABLE_NIMBITS
      {
        String adc_name = "ADC";
        adc_name += adc_channel;
        nimbits.recordValue(adc_name, adc_read);
      }
#endif
    }
#endif

    tSensors.querySensors();

    for (index = 0; index < tSensors.getDeviceCount(); index++)
    {
      DeviceAddress address;
      float temperature;

      tSensors.getTemperature(index, address, temperature);

      output += dtostrf(temperature, 1, 2, temp);

#ifdef ENABLE_NIMBITS
      nimbits.recordValue(address_to_string(address), temperature);
#endif

        output += ",";
    }

    output = output.substring(0,output.length()-1);

    // write output to SD card
    updateLogFile(output);

    Serial.println(output);
  }

#ifdef ENABLE_LCD
  // Update LCD

  if (millis() > next_lcd_update_time)
  {
    next_lcd_update_time += LCD_UPDATE_INTERVAL;

    if (lcd_page < ADC_CHANNEL_COUNT)
    {
      // update LCD with ADC reading
      lcd.clear();
      lcd.print(F("A"));
      lcd.print(lcd_page);
      lcd.print(F(": "));
      lcd.print(adc_to_voltage(analogRead(lcd_page)));
      lcd.print(F(" V"));
    }
    else
    {
      // update LCD with current temp
      DeviceAddress address;
      float temperature;

      uint8_t index = lcd_page - ADC_CHANNEL_COUNT;

      tSensors.getTemperature(index, address, temperature);

      lcd.clear();
      lcd.print(F("T"));
      lcd.print(index);
      lcd.print(F(": "));
      lcd.print(temperature);
      lcd.print(F(" C"));
      lcd.setCursor(0, 1);
      lcd.print(address_to_string(address));
    }
  }

  // read LCD buttons

  {
    static uint8_t buttonPressed = 0;
    static unsigned long buttonTimeout = 0;

    if (buttonPressed == 0)
    {
      if (digitalRead(LCD_BUTTON_UP))
      {
        buttonPressed = LCD_BUTTON_UP;
      }
      else if (digitalRead(LCD_BUTTON_DOWN))
      {
        buttonPressed = LCD_BUTTON_DOWN;
      }

      if (buttonPressed != 0)
      {
        buttonTimeout = millis() + LCD_BUTTON_DEBOUNCE_TIMEOUT;
      }
    }
    else
    {
      if (!digitalRead(buttonPressed))
      {
        buttonPressed = 0;

        if (millis() > buttonTimeout)
        {
          if (buttonPressed == LCD_BUTTON_UP)
          {
            lcd_page = (lcd_page > 0) ? lcd_page-1 : tSensors.getDeviceCount() + ADC_CHANNEL_COUNT - 1;
          }
          else
          {
            lcd_page = (lcd_page > tSensors.getDeviceCount() + ADC_CHANNEL_COUNT - 1) ? 0 : lcd_page + 1;
          }
        }
      }
    }
  }
#endif
}

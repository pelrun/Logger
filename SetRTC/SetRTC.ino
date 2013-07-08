#include <DigitalIO.h>
#include <DigitalPin.h>
#include <I2cConstants.h>
#include <PinIO.h>
#include <SoftI2cMaster.h>
#include <SoftSPI.h>

#include "SetRTC.h"
#include <RTClib.h>

RTC_DS1307 rtc;

void setup()
{
  // Turn D13 into Vcc for the rtc :D
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  rtc.begin();

  // Pull the battery momentarily before running this
  // Only sets the RTC if it's not already set
  if (!rtc.isrunning())
  {
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
}

void loop()
{
}

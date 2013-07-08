#include "TemperatureLogger.h"

#include <OneWire.h>
#include <DallasTemperature.h>

#include "TempSensors.h"

static OneWire oneWire(ONE_WIRE_BUS_PIN);
static DallasTemperature sensors(&oneWire);

typedef struct
{
  DeviceAddress id;
  float temperature;
} SensorData_T;

static SensorData_T *sensorData;

bool TempSensors::begin()
{
  sensors.begin();
  sensors.setResolution(TEMPERATURE_PRECISION);
  sensors.setCheckForConversion(false);

  #if 0
  // locate devices on the bus
  Serial.print(F("Found "));
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(F(" devices."));

  // report parasite power requirements
  Serial.print(F("Parasite power is: "));
  if (sensors.isParasitePowerMode()) Serial.println(F("ON"));
  else Serial.println(F("OFF"));
  #endif

  sensorData = (SensorData_T *)calloc(sensors.getDeviceCount(), sizeof(SensorData_T));
  if (sensorData != NULL)
  {
    for (uint8_t i = 0; i < sensors.getDeviceCount(); i++)
    {
      sensors.getAddress(sensorData[i].id, i);
    }
    return true;
  }

  return false;

}

uint8_t TempSensors::getDeviceCount() const
{
  return sensors.getDeviceCount();
}

void TempSensors::querySensors()
{
  sensors.requestTemperatures();

  for (uint8_t i = 0; i < sensors.getDeviceCount(); i++)
  {
    sensorData[i].temperature = sensors.getTempC(sensorData[i].id);
  }
}

bool TempSensors::getTemperature(uint8_t index, DeviceAddress &address, float &temperature)
{
  if (index < sensors.getDeviceCount())
  {
    for (int i = 0; i < 8; i++)
    {
      address[i] = sensorData[index].id[i];
    }
    temperature = sensorData[index].temperature;
    return true;
  }
  return false;
}

bool TempSensors::getAddress(uint8_t index, DeviceAddress &address)
{
  if (index < sensors.getDeviceCount())
  {
    for (int i = 0; i < 8; i++)
    {
      address[i] = sensorData[index].id[i];
    }
    return true;
  }
  return false;
}

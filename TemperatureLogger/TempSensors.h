/*
 * TempSensors.h
 *
 *  Created on: 01/02/2013
 *      Author: James
 */

#ifndef TEMPSENSORS_H_
#define TEMPSENSORS_H_

#ifndef OneWire_h
typedef uint8_t DeviceAddress[8];
#endif

class TempSensors
{
public:
  TempSensors() {};

  bool begin();

  uint8_t getDeviceCount() const;

  void querySensors();

  bool getTemperature(uint8_t index, DeviceAddress &address, float &temperature);

  bool getAddress(uint8_t index, DeviceAddress &address);

};

#endif /* TEMPSENSORS_H_ */

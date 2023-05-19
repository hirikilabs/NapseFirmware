#ifndef _NAPSE_BLE_H
#define _NAPSE_BLE_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "napse.h"

class NapseBLE {
  int numCh;

  public:
    BLEServer* pServer;
    BLEService* pService;
    BLECharacteristic* pCharacteristicData1;
    BLECharacteristic* pCharacteristicData2;
    BLECharacteristic* pCharacteristicData3;
    BLECharacteristic* pCharacteristicData4;
    BLECharacteristic* pCharacteristicData5;
    BLECharacteristic* pCharacteristicData6;
    BLECharacteristic* pCharacteristicData7;
    BLECharacteristic* pCharacteristicData8;
    BLECharacteristic* pCharacteristicData;
    BLECharacteristic* pCharacteristicStartStop;
    BLECharacteristic* pCharacteristicBatt;
    BLEDescriptor* dataDescriptor;
    BLEDescriptor* startStopDescriptor;
    BLEDescriptor* battDescriptor;
    BLEAdvertising* pAdvertising;

  bool setup(int _numCh);
  void updateData(uint32_t data[]);
  void updateBatt(float battv);

};


#endif // _NAPSE_BLE_H


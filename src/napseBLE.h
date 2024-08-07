#ifndef _NAPSE_BLE_H
#define _NAPSE_BLE_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "napse.h"

class NapseServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param);
    void onDisconnect(BLEServer* pServer);
};


class NapseBLE {
    int num_ch;
    NapseServerCallbacks* serverCallbacks;
    
  public:
    BLEServer* pServer;
    BLEService* pService;
    BLECharacteristic* pCharacteristicInfo;
    BLECharacteristic* pCharacteristicData;
    BLECharacteristic* pCharacteristicStartStop;
    BLECharacteristic* pCharacteristicBatt;
    BLECharacteristic* pCharacteristicConfig;
    BLEDescriptor* infoDescriptor;
    BLEDescriptor* dataDescriptor;
    BLEDescriptor* startStopDescriptor;
    BLEDescriptor* battDescriptor;
    BLEDescriptor* confDescriptor;
    BLEAdvertising* pAdvertising;

  bool setup(int _num_ch);
  void updateData(uint32_t data[]);
  void updateBatt(float battv);
};


#endif // _NAPSE_BLE_H


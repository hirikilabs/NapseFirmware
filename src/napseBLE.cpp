#include <Esp.h>
#include "napseBLE.h"

extern uint16_t start_stop;
extern bool start_stop_changed;

class StartStopCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    uint8_t* rxData;
    //rxData = (uint8_t*) malloc(pCharacteristic->getLength());
    rxData = pCharacteristic->getData();
    if (pCharacteristic->getLength() > 0) {
      Serial.print("rxData: (");
      Serial.print(pCharacteristic->getLength());
      Serial.print(") ");
      Serial.println(rxData[0], HEX);
      if (rxData[0] == 0) {
        start_stop = 0;
        start_stop_changed = true;
      }
      else if (rxData[0] == 1) {
        start_stop = 1;
        start_stop_changed = true;
      }
    }
  }
};

bool NapseBLE::setup(int _numCh) {
  // check channels
  if (_numCh < 4 || _numCh > 8) {
    return false;
  }

  numCh = _numCh;

  // create BL name
  char blid[23];
  uint64_t chipId = ESP.getEfuseMac();
  snprintf(blid, 23, "NIT-BL-%llX", chipId);

  // init BLE
  BLEDevice::init(blid);
  pServer = BLEDevice::createServer();
  pService = pServer->createService(SERVICE_UUID);
  
  // create characteristics
  //
  pCharacteristicData = pService->createCharacteristic(
      CHARACTERISTIC_DATA_UUID,
      BLECharacteristic::PROPERTY_READ
      );
  dataDescriptor = new BLEDescriptor ("2901", 100);
  dataDescriptor->setValue("All data as uint8");
  pCharacteristicData->addDescriptor(dataDescriptor);
  
  pCharacteristicData1 = pService->createCharacteristic(
      CHARACTERISTIC_DATA1_UUID,
      BLECharacteristic::PROPERTY_READ
      );
  pCharacteristicData2 = pService->createCharacteristic(
      CHARACTERISTIC_DATA2_UUID,
      BLECharacteristic::PROPERTY_READ
      );
  pCharacteristicData3 = pService->createCharacteristic(
      CHARACTERISTIC_DATA3_UUID,
      BLECharacteristic::PROPERTY_READ
      );
  pCharacteristicData4 = pService->createCharacteristic(
      CHARACTERISTIC_DATA4_UUID,
      BLECharacteristic::PROPERTY_READ
      );

  if (numCh > 4) {
    pCharacteristicData5 = pService->createCharacteristic(
        CHARACTERISTIC_DATA5_UUID,
        BLECharacteristic::PROPERTY_READ
        );
    pCharacteristicData6 = pService->createCharacteristic(
        CHARACTERISTIC_DATA6_UUID,
        BLECharacteristic::PROPERTY_READ
        );
  }

  if (numCh > 6) {
    pCharacteristicData7 = pService->createCharacteristic(
        CHARACTERISTIC_DATA7_UUID,
        BLECharacteristic::PROPERTY_READ
        );

    pCharacteristicData8 = pService->createCharacteristic(
        CHARACTERISTIC_DATA8_UUID,
        BLECharacteristic::PROPERTY_READ
        );
  }

  pCharacteristicStartStop = pService->createCharacteristic(
      CHARACTERISTIC_START_STOP_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
      );

  startStopDescriptor = new BLEDescriptor ("2901", 100);
  startStopDescriptor->setValue("Start or stop logging");
  pCharacteristicStartStop->addDescriptor(startStopDescriptor);

  pCharacteristicBatt = pService->createCharacteristic(
      CHARACTERISTIC_BATT_UUID,
      BLECharacteristic::PROPERTY_READ
      );

  battDescriptor = new BLEDescriptor ("2901", 100);
  battDescriptor->setValue("Battery value (string)");
  pCharacteristicBatt->addDescriptor(battDescriptor);

// initial values
  uint32_t initVal = 0;
  pCharacteristicData1->setValue(initVal);
  pCharacteristicData2->setValue(initVal);
  pCharacteristicData3->setValue(initVal);
  pCharacteristicData4->setValue(initVal);
  if (numCh > 4) {
    pCharacteristicData5->setValue(initVal);
    pCharacteristicData6->setValue(initVal);
  }
  if (numCh > 6) {
    pCharacteristicData7->setValue(initVal);
    pCharacteristicData8->setValue(initVal);
  }
  pCharacteristicStartStop->setValue(initVal);

  // callbacks
  pCharacteristicStartStop->setCallbacks(new StartStopCallback());
  // start service
  pService->start();

  // advertising
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  return true;
}

void NapseBLE::updateData(uint32_t data[]) {
  pCharacteristicData1->setValue(data[0]);
  pCharacteristicData2->setValue(data[1]);
  pCharacteristicData3->setValue(data[2]);
  pCharacteristicData4->setValue(data[3]);
  pCharacteristicData->setValue((uint8_t*)data, 4*9);
}

void NapseBLE::updateBatt(float battv) {
  pCharacteristicBatt->setValue(battv);
}

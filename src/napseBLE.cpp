#include <Esp.h>
#include "napseBLE.h"

extern uint16_t start_stop;
extern bool start_stop_changed;
extern uint8_t* channel_config;

class StartStopCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    uint8_t* rx_data;
    //rxData = (uint8_t*) malloc(pCharacteristic->getLength());
    rx_data = pCharacteristic->getData();
    if (pCharacteristic->getLength() > 0) {
      Serial.print("StartStop rx_data: (");
      Serial.print(pCharacteristic->getLength());
      Serial.print(") ");
      Serial.println(rx_data[0], HEX);
      if (rx_data[0] == 0) {
        start_stop = 0;
        start_stop_changed = true;
      }
      else if (rx_data[0] == 1) {
        start_stop = 1;
        start_stop_changed = true;
      }
    }
  }
};

class ConfigCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    uint8_t* rx_data;
    uint8_t data_len;
    //rxData = (uint8_t*) malloc(pCharacteristic->getLength());
    rx_data = pCharacteristic->getData();
    data_len = pCharacteristic->getLength();
    if (data_len > 0) {
      Serial.print("Config rxData: (");
      Serial.print(data_len);
      Serial.print(") ");
      Serial.println(rx_data[0], HEX);
      for (int i = 0; i < data_len; i++) {
        if (i>7) 
          break;
        channel_config[i] = rx_data[i];
      }
    }
  }
};

bool NapseBLE::setup(int _num_ch) {
  // check channels
  if (_num_ch < 4 || _num_ch > 8) {
    return false;
  }

  num_ch = _num_ch;

  // create BL name
  char blid[23];
  uint64_t chip_id = ESP.getEfuseMac();
  snprintf(blid, 23, "NIT-BL-%llX", chip_id);

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

  pCharacteristicConfig = pService->createCharacteristic(
      CHARACTERISTIC_CONF_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
      );

  confDescriptor = new BLEDescriptor ("2901", 100);
  confDescriptor->setValue("Channels configuration");
  pCharacteristicConfig->addDescriptor(confDescriptor);



  // initial values
  uint32_t init_val = 0;
  pCharacteristicStartStop->setValue(init_val);

  // callbacks
  pCharacteristicStartStop->setCallbacks(new StartStopCallback());
  pCharacteristicConfig->setCallbacks(new ConfigCallback());
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
  pCharacteristicData->setValue((uint8_t*)data, 4*10);
}

void NapseBLE::updateBatt(float battv) {
  pCharacteristicBatt->setValue(battv);
}

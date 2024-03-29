#include <Esp.h>
#include "BLEDevice.h"
#include "napseBLE.h"
#include "napse.h"

extern uint16_t start_stop;
extern bool start_stop_changed;
extern channel_config_t* channel_config;
extern bool config_changed;

bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {
      deviceConnected = true;
      pServer->updateConnParams(param->connect.remote_bda, 0x06, 0x06, 0, 100);
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      pServer->startAdvertising();
    }
};

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
      if (data_len >= 8) {
	for (int i=0; i<data_len; i++) {
	  channel_config[i] = rx_data[i];
	}
	config_changed = true;
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
  //BLEDevice::setMTU(1024);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  pService = pServer->createService(SERVICE_UUID);
  
  // create characteristics
  pCharacteristicInfo = pService->createCharacteristic(
      CHARACTERISTIC_INFO_UUID,
      BLECharacteristic::PROPERTY_READ
      );
  infoDescriptor = new BLEDescriptor ("2901", 100);
  infoDescriptor->setValue("Device ID and number of channels");
  infoDescriptor->setAccessPermissions(ESP_GATT_PERM_READ);
  pCharacteristicInfo->addDescriptor(infoDescriptor);
  uint8_t* info;
  info = (uint8_t*) malloc(9);
  memcpy(info, &chip_id, 8);
  info[8] = num_ch;
  pCharacteristicInfo->setValue(info, 9);
  free(info);

  
  pCharacteristicData = pService->createCharacteristic(
      CHARACTERISTIC_DATA_UUID,
      BLECharacteristic::PROPERTY_READ
      );
  dataDescriptor = new BLEDescriptor ("2901", 100);
  dataDescriptor->setValue("All data as uint8");
  dataDescriptor->setAccessPermissions(ESP_GATT_PERM_READ);
  pCharacteristicData->addDescriptor(dataDescriptor);

  pCharacteristicStartStop = pService->createCharacteristic(
      CHARACTERISTIC_START_STOP_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
      );

  startStopDescriptor = new BLEDescriptor ("2901", 100);
  startStopDescriptor->setValue("Start or stop logging");
  startStopDescriptor->setAccessPermissions(ESP_GATT_PERM_READ);
  pCharacteristicStartStop->addDescriptor(startStopDescriptor);

  pCharacteristicBatt = pService->createCharacteristic(
      CHARACTERISTIC_BATT_UUID,
      BLECharacteristic::PROPERTY_READ
      );

  battDescriptor = new BLEDescriptor ("2901", 100);
  battDescriptor->setValue("Battery value (float32)");
  battDescriptor->setAccessPermissions(ESP_GATT_PERM_READ);
  pCharacteristicBatt->addDescriptor(battDescriptor);

  pCharacteristicConfig = pService->createCharacteristic(
      CHARACTERISTIC_CONF_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
      );

  confDescriptor = new BLEDescriptor ("2901", 100);
  confDescriptor->setValue("Channels configuration");
  confDescriptor->setAccessPermissions(ESP_GATT_PERM_READ);
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
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMaxPreferred(0x06);
  BLEDevice::startAdvertising();

  return true;
}

void NapseBLE::updateData(uint32_t data[]) {
  pCharacteristicData->setValue((uint8_t*)data, 4*10);
}

void NapseBLE::updateBatt(float battv) {
  pCharacteristicBatt->setValue(battv);
}

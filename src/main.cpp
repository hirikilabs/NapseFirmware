#include <Arduino.h>
#include <cstdint>
#include <stdlib.h>
#include <ADS1299.h>
#include <Adafruit_NeoPixel.h>
#include "esp32-hal-psram.h"
#include "napse.h"
#include "napseBLE.h"
#include "leds.h"

//#define DEBUG_ON

channel_data_t last_data;
boolean device_id_returned = false;
uint16_t start_stop = 0;
boolean start_stop_changed = false;
uint32_t channel_data[10];
channel_config_t* channel_config;
boolean config_changed = false;

ADS1299 ADS;
NapseBLE ble;
NapseLEDs leds;

void printADSConfig()
{
  Serial.println(ADS.getDeviceID(), BIN); // Function to return Device ID

  // prints dashed line to separate serial print sections
  Serial.println("----------------------------------------------");

  // Read ADS1299 Register at address 0x00 (see Datasheet pg. 35 for more info on SPI commands)
  int reg;
  reg = ADS.readRegister(0x00);
  ADS.printRegisterName(0x00);
  ADS.printRegister(reg);
  Serial.println("----------------------------------------------");

  for (int addr = 0x01; addr < 0x18; addr++)
  {
    reg = ADS.readRegister(addr);
    ADS.printRegisterName(addr);
    ADS.printRegister(reg);
  }
  Serial.println("----------------------------------------------");
}

float get_batt() {
  uint16_t analog = analogRead(BATT_PIN);
  // make conversion, reference is 3.3V
  float voltage_measured = (analog * 3.3)/4096.0;
  return voltage_measured * BATT_DIVISION * BATT_CORRECTION;
}

void parseConfig(channel_config_t* config) {
  for (int i=0; i<MAX_CHANNELS; i++) {
    // shortcircuit mode xxxxxxx1 = shortcircuit
    if (config[i] & 0x01) {
      ADS.channelInputShorted(i+1);
    } else {
      ADS.channelInputNormal(i+1);
    }
    // gain xxxxnnnx
    uint8_t gain = (config[i] << 3) | 0b10001111;
    ADS.channelGainSet(i+1, gain);
  }
  // ok, configured
  config_changed = false;
}

void configureADS() {
  // Channels mode
  for (int i = 1; i <= ADS.numCh; i++) {
    ADS.channelInputNormal(i);
    ADS.channelGainSet(i, PGA_GAIN_6);
  }
  // referential mode, datasheet pg 68
  ADS.writeRegister(ADS1299_MISC1, SRB1_NEG_INPUTS);
}


void setup()
{
  // data
  channel_config = (channel_config_t*) malloc(sizeof(channel_config_t) * MAX_CHANNELS);
  
  // NeoPX
  // start on red
  leds.setup(NEOPX_PIN);
  leds.set(COLOR_ERR);

  delay(3000);

  Serial.begin(115200);

    // ADS reset
  pinMode(ADS_RST_PIN, OUTPUT);
  digitalWrite(ADS_RST_PIN, HIGH);

  // BATT PIN
  pinMode(BATT_PIN, INPUT);

  Serial.println();
  Serial.println("⚡ ADS1299-bridge has started!");

  ADS.setup(DRDY_PIN, ADS1299_4CH); // (DRDY pin, num of channels);
  delay(10);                  // delay to ensure connection

  Serial.println("⚙ ADS1299-bridge configured!");

  ADS.RESET();
  Serial.println("✅ ADS1299-bridge reset!");
  ADS.STOP();
  configureADS();
  // ok, show it (green)
  leds.set(COLOR_INIT);

  // BLEServer
  Serial.println("🔌 Starting BLE...");
  ble.setup(ADS1299_4CH);
  Serial.println("📡 Started BLE...");
  // update battery value
  ble.updateBatt(get_batt());
}

void loop()
{
  if (device_id_returned == false)
  {

    Serial.print("🪪 ID: ");
    Serial.println(ADS.getDeviceID(), BIN); // Function to return Device ID
#ifdef DEBUG_ON
    // prints dashed line to separate serial print sections
    Serial.println("----------------------------------------------");

    // Read ADS1299 Register at address 0x00 (see Datasheet pg. 35 for more info on SPI commands)
    int reg;
    reg = ADS.readRegister(0x00);
    ADS.printRegisterName(0x00);
    ADS.printRegister(reg);
    Serial.println("----------------------------------------------");

    for (int addr = 0x01; addr < 0x18; addr++)
    {
      reg = ADS.readRegister(addr);
      ADS.printRegisterName(addr);
      ADS.printRegister(reg);
    }
    Serial.println("----------------------------------------------");

    // Write register command (see Datasheet pg. 38 for more info about writeRegister)
    ADS.writeRegister(ADS1299_CONFIG1, 0b11010110);
    Serial.print("Register 0x");
    Serial.print(ADS1299_CONFIG1, HEX);
    Serial.println(" modified.");
    Serial.println("----------------------------------------------");

    // Repeat PRINT ALL REGISTERS to verify changes 
    for (int addr = 0x01; addr < 0x18; addr++)
    {
      reg = ADS.readRegister(addr);
      ADS.printRegisterName(addr);
      ADS.printRegister(reg);
    }
    Serial.println("----------------------------------------------");
#endif    
    Serial.print("🔋 Batt: ");
    Serial.println(get_batt());
    Serial.println("----------------------------------------------");
    

    // Start data conversions command
    device_id_returned = true;
  }

  // serial commands
  if (Serial.available() > 0)
  {
    int command = Serial.read();
    switch (command)
    {
    case 'l':
      Serial.println("Log start");
      leds.set(COLOR_READ);
      ADS.START();
      break;
    case 's':
      Serial.println("Log stop");
      leds.set(COLOR_INIT);
      ADS.STOP();
      break;
    case 'p':
      printADSConfig();
      break;
    }
  }

  if (config_changed) {
    parseConfig(channel_config);
  }
  
  // BLE commands
  if (start_stop_changed) {
    if (start_stop == 0) {
      leds.set(COLOR_INIT);
      ADS.STOP();
      ble.updateBatt(get_batt());
    } else {
      ble.updateBatt(get_batt());
      leds.set(COLOR_READ);
      ADS.START();
    }
    start_stop_changed = false;
  }

  // get data if available
  if (ADS.updateData(&last_data))
  {
    //ADS.printData(last_data);
    channel_data[0] = last_data.numPacket;
    channel_data[1] = last_data.status;
    channel_data[2] = last_data.chan1;
    channel_data[3] = last_data.chan2;
    channel_data[4] = last_data.chan3;
    channel_data[5] = last_data.chan4;
    if (ADS.numCh > 4) {
      channel_data[6] = last_data.chan5;
      channel_data[7] = last_data.chan6;
    }
    if (ADS.numCh > 6) {
      channel_data[8] = last_data.chan7;
      channel_data[9] = last_data.chan8;
    }
    ble.updateData(channel_data);
  }
}

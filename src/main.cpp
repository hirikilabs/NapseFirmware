#include <Arduino.h>
#include <ADS1299.h>
#include <Adafruit_NeoPixel.h>
#include "napse.h"
#include "napseBLE.h"
#include "leds.h"

channel_data_t last_data;
boolean device_id_returned = false;
uint16_t start_stop = 0;
boolean start_stop_changed = false;
uint32_t channel_data[10];
uint8_t channel_config[8];

ADS1299 ADS;
NapseBLE ble;
NapseLEDs leds;

void printADSConfig()
{
  Serial.println(ADS.getDeviceID(), BIN); // Funciton to return Device ID

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

void setup()
{
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
  Serial.println("ADS1299-bridge has started!");

  ADS.setup(DRDY_PIN, ADS1299_4CH); // (DRDY pin, num of channels);
  delay(10);                  // delay to ensure connection

  Serial.println("ADS1299-bridge configured!");

  ADS.RESET();
  Serial.println("ADS1299-bridge reset!");
  ADS.STOP();
  // ok, show it (green)
  leds.set(COLOR_INIT);

  // BLEServer
  Serial.println("Starting BLE...");
  ble.setup(ADS1299_4CH);
  Serial.println("Started BLE...");
  // update battery value
  ble.updateBatt(get_batt());
}

void loop()
{
  if (device_id_returned == false)
  {

    Serial.print("ID: ");
    Serial.println(ADS.getDeviceID(), BIN); // Funciton to return Device ID

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

    // Channel mode
    for (int i = 1; i <= ADS.numCh; i++) {
      ADS.channelInputNormal(i);
      ADS.channelGainSet(i, PGA_GAIN_6);
    }
    // ADS.channelInputShorted(1);
    // ADS.channelInputShorted(2);
    // ADS.channelInputShorted(3);
    // ADS.channelInputShorted(4);
    
    //ADS.channelGainSet(1, PGA_GAIN_6);

    // Repeat PRINT ALL REGISTERS to verify changes 
    for (int addr = 0x01; addr < 0x18; addr++)
    {
      reg = ADS.readRegister(addr);
      ADS.printRegisterName(addr);
      ADS.printRegister(reg);
    }
    Serial.println("----------------------------------------------");
    Serial.print("Batt: ");
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

  // print data if available
  if (ADS.updateData(&last_data))
  {
    ADS.printData(last_data);
    channel_data[0] = last_data.numPacket;
    channel_data[1] = last_data.status;
    channel_data[2] = last_data.chan1;
    channel_data[3] = last_data.chan2;
    channel_data[4] = last_data.chan3;
    channel_data[5] = last_data.chan4;

    ble.updateData(channel_data);
  }
}

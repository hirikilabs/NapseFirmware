//
//  ADS1299.h
//  
//  Created by Conor Russomanno on 6/17/13.
//

#ifndef ____ADS1299__
#define ____ADS1299__

#include <inttypes.h>
#include <stdio.h>
#include <Arduino.h>
#include "Definitions.h"

#include <SPI.h>


typedef struct {
  uint32_t numPacket;
  uint32_t status;
  uint32_t chan1;
  uint32_t chan2;
  uint32_t chan3;
  uint32_t chan4;
  uint32_t chan5;
  uint32_t chan6;
  uint32_t chan7;
  uint32_t chan8;
} channel_data_t;

class ADS1299 {
public:
    
  float tCLK;
  int DRDY; // pin number for "Data Ready" (DRDY)
  int numCh;
  uint64_t outputCount;

  void setup(int _DRDY, int _numCh);
    
  //ADS1299 SPI Command Definitions (Datasheet, Pg. 35)
  //System Commands
  void WAKEUP();
  void STANDBY();
  void RESET();
  void START();
  void STOP();
    
  // Data Read Commands
  void RDATAC();
  void SDATAC();
  void RDATA();
    
  // Register Read/Write Commands
  byte getDeviceID();
  byte readRegister(byte address);
  void writeRegister(byte address, byte value);

  void setSampleRate(byte rate);
  // channel data read
  // returns true if new data is read from the ADC
  boolean updateData(channel_data_t* data);

  void channelPowerDown(byte channel);
  void channelPowerUp(byte channel);
    
  // input mode selection
  void channelInputMode(byte channel, byte mode);

  // channel gain 
  void channelGainSet(byte channel, byte gain);

  // connection to SRB2
  void channelUseSRB2(byte channel, bool use);
  
  // DEBUG
  void printRegister(byte reg);
  void printRegisterName(byte address);
  void printData(channel_data_t data);
    
  // SPI transfer
  byte transfer(byte _data);

  //------------------------//
  // void attachInterrupt();
  // void detachInterrupt(); // Default
  // void begin(); // Default
  // void end();
  // void setBitOrder(uint8_t);
  // void setDataMode(uint8_t);
  // void setClockDivider(uint8_t);
  //------------------------//
    
        
  //    vector<String> registers;
    
};

#endif

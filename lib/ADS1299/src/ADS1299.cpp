//
//  ADS1299.cpp
//
//  Created by Conor Russomanno on 6/17/13.
//

#include "pins_arduino.h"
#include "ADS1299.h"
#include <SPI.h>

#define VSPI_MISO MISO
#define VSPI_MOSI MOSI
#define VSPI_SCLK SCK
#define VSPI_SS SS

static const int spiClk = 1000000; // 1 MHz

void ADS1299::setup(int _DRDY, int _numCh)
{
    // **** ----- SPI Setup ----- **** //
    // ESP32 SCLK = 18, MISO = 19, MOSI = 23, SS = 5
    // ESP32C3 SCLK = 4, MISO = 5, MOSI = 6, SS = 7
    SPI.begin();

    // number of channels
    numCh = _numCh;

    // initalize the  data ready and chip select pins:
    pinMode(SS, OUTPUT);
    DRDY = _DRDY;
    pinMode(DRDY, INPUT_PULLUP);

    tCLK = 0.000666; // 666 ns (Datasheet, pg. 8)
    outputCount = 0;
}

// System Commands
void ADS1299::WAKEUP()
{
    digitalWrite(SS, LOW);
    transfer(ADS1299_WAKEUP);
    digitalWrite(SS, HIGH);
    // delay(4.0*tCLK);  //must way at least 4 tCLK cycles before sending another command (Datasheet, pg. 35)
    delay(50);
}
void ADS1299::STANDBY()
{
    digitalWrite(SS, LOW);
    transfer(ADS1299_STANDBY);
    digitalWrite(SS, HIGH);
}
void ADS1299::RESET()
{
    digitalWrite(SS, LOW);
    transfer(ADS1299_RESET);
    delay(10);
    //    delay(18.0*tCLK); //must wait 18 tCLK cycles to execute this command (Datasheet, pg. 35)
    digitalWrite(SS, HIGH);
}
void ADS1299::START()
{
    digitalWrite(SS, LOW);
    transfer(ADS1299_START);
    digitalWrite(SS, HIGH);
}
void ADS1299::STOP()
{
    digitalWrite(SS, LOW);
    transfer(ADS1299_STOP);
    digitalWrite(SS, HIGH);
}
// Data Read Commands
void ADS1299::RDATAC()
{
    digitalWrite(SS, LOW);
    transfer(ADS1299_RDATAC);
    digitalWrite(SS, HIGH);
}
void ADS1299::SDATAC()
{
    digitalWrite(SS, LOW);
    transfer(ADS1299_SDATAC);
    digitalWrite(SS, HIGH);
}
void ADS1299::RDATA()
{
    digitalWrite(SS, LOW);
    transfer(ADS1299_RDATA);
    digitalWrite(SS, HIGH);
}

// Register Read/Write Commands
byte ADS1299::getDeviceID()
{
    digitalWrite(SS, LOW);
    transfer(ADS1299_SDATAC);   // SDATAC
    transfer(ADS1299_RREG);     // readRegister
    transfer(0x00);             // Asking for 1 byte
    byte data = transfer(0x00); // byte to read (hopefully 0b???11110)
    transfer(ADS1299_RDATAC);   // turn read data continuous back on
    digitalWrite(SS, HIGH);
    return data;
}

byte ADS1299::readRegister(byte address)
{
    byte opcode1 = ADS1299_RREG + address; // 001rrrrr; _readRegister = 00100000 and _address = rrrrr
    digitalWrite(SS, LOW);
    transfer(ADS1299_SDATAC);   // SDATAC
    transfer(opcode1);          // readRegister
    transfer(0x00);             // opcode2
    byte data = transfer(0x00); // returned byte should match default of register map unless edited manually (Datasheet, pg.39)
    transfer(ADS1299_RDATAC);   // turn read data continuous back on
    digitalWrite(SS, HIGH);
    return data;
}

void ADS1299::writeRegister(byte address, byte value)
{
    byte opcode1 = ADS1299_WREG + address; // 001rrrrr; _readRegister = 00100000 and _address = rrrrr
    digitalWrite(SS, LOW);
    transfer(ADS1299_SDATAC); // SDATAC
    transfer(opcode1);
    transfer(0x00);
    transfer(value);
    transfer(ADS1299_RDATAC);
    digitalWrite(SS, HIGH);
}

boolean ADS1299::updateData(channel_data_t *data)
{
    if (digitalRead(DRDY) == LOW)
    {
        digitalWrite(SS, LOW);
        uint32_t output[9];
        uint32_t dataPacket;
        for (int i = 0; i <= numCh; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                byte dataByte = transfer(0x00);
                dataPacket = (dataPacket << 8) | dataByte;
            }
            output[i] = dataPacket;
            dataPacket = 0;
        }
        digitalWrite(SS, HIGH);
        outputCount++;


        // store data in struct
        data->numPacket = outputCount;
        data->status = output[0];
        data->chan1 = output[1];
        data->chan2 = output[2];
        data->chan3 = output[3];
        data->chan4 = output[4];
        if (numCh > 4)
        {
            data->chan5 = output[5];
            data->chan6 = output[6];
        }
        if (numCh > 6)
        {
            data->chan5 = output[7];
            data->chan6 = output[8];
        }

        return true;
    }
    else
    {
        // not ready
        return false;
    }
}


// changes channel mode to normal input
void ADS1299::channelInputMode(byte channel, byte mode) 
{
    // check channel
    if (channel > numCh) {
        return;
    }
    // read channel mode
    byte chan_mode;
    chan_mode = readRegister(ADS1299_LOFF + channel);

    // apply it
    chan_mode = chan_mode & 0b11111000;
    chan_mode = chan_mode | mode;
    writeRegister(ADS1299_LOFF + channel, chan_mode);
}

// sets gain for channel n
void ADS1299::channelGainSet(byte channel, byte gain) {
    // check channel
    if (channel > numCh) {
        return;
    }
    // read channel mode
    byte chan_mode;
    chan_mode = readRegister(ADS1299_LOFF + channel);

    chan_mode = chan_mode | 0b01110000;
    chan_mode = chan_mode & gain;

    writeRegister(ADS1299_LOFF + channel, chan_mode);
}

void ADS1299::channelUseSRB2(byte channel, bool use) {
    // check channel
    if (channel > numCh) {
        return;
    }
    // read channel config
    byte chan_conf;
    chan_conf = readRegister(ADS1299_LOFF + channel);
    
    if (use) {
      chan_conf = chan_conf | CH_SRB2;
    } else {
      chan_conf = chan_conf & ~(CH_SRB2);
    }
    writeRegister(ADS1299_LOFF + channel, chan_conf);
}

// prints register
void ADS1299::printRegister(byte reg)
{
    Serial.print("0x");
    if (reg < 16)
        Serial.print("0");
    Serial.print(reg, HEX);
    Serial.print(", ");
    for (byte j = 0; j < 8; j++)
    {
        Serial.print(bitRead(reg, 7 - j), BIN);
        if (j != 7)
            Serial.print(", ");
    }
    Serial.println();
}

// prints channel data content
void ADS1299::printData(channel_data_t data)
{
    Serial.print("0x");
    Serial.print(data.status, HEX);
    Serial.print(", 0x");
    Serial.print(data.chan1, HEX);
    Serial.print(", 0x");
    Serial.print(data.chan2, HEX);
    Serial.print(", 0x");
    Serial.print(data.chan3, HEX);
    Serial.print(", 0x");
    Serial.print(data.chan4, HEX);
    if (numCh > 4)
    {
        Serial.print(", 0x");
        Serial.print(data.chan5, HEX);
        Serial.print(", 0x");
        Serial.print(data.chan6, HEX);
    }
    if (numCh > 6)
    {
        Serial.print(", 0x");
        Serial.print(data.chan7, HEX);
        Serial.print(", 0x");
        Serial.print(data.chan8, HEX);
    }
    Serial.println();
}

// String-Byte converter
void ADS1299::printRegisterName(byte address)
{
    switch (address)
    {
    case ADS1299_ID:
        Serial.print("ID, ");
        break;
    case ADS1299_CONFIG1:
        Serial.print("CONFIG1, ");
        break;
    case ADS1299_CONFIG2:
        Serial.print("CONFIG2, ");
        break;
    case ADS1299_CONFIG3:
        Serial.print("CONFIG3, ");
        break;
    case ADS1299_LOFF:
        Serial.print("LOFF, ");
        break;
    case ADS1299_CH1SET:
        Serial.print("CH1SET, ");
        break;
    case ADS1299_CH2SET:
        Serial.print("CH2SET, ");
        break;
    case ADS1299_CH3SET:
        Serial.print("CH2SET, ");
        break;
    case ADS1299_CH4SET:
        Serial.print("CH4SET, ");
        break;
    case ADS1299_CH5SET:
        Serial.print("CH5SET, ");
        break;
    case ADS1299_CH6SET:
        Serial.print("CH6SET, ");
        break;
    case ADS1299_CH7SET:
        Serial.print("CH7SET, ");
        break;
    case ADS1299_CH8SET:
        Serial.print("CH8SET, ");
        break;
    case ADS1299_BIAS_SENSP:
        Serial.print("BIAS_SENSP, ");
        break;
    case ADS1299_BIAS_SENSN:
        Serial.print("BIAS_SENSN, ");
        break;
    case ADS1299_LOFF_SENSP:
        Serial.print("LOFF_SENSP, ");
        break;
    case ADS1299_LOFF_SENSN:
        Serial.print("LOFF_SENSN, ");
        break;
    case ADS1299_LOFF_FLIP:
        Serial.print("LOFF_FLIP, ");
        break;
    case ADS1299_LOFF_STATP:
        Serial.print("LOFF_STATP, ");
        break;
    case ADS1299_LOFF_STATN:
        Serial.print("LOFF_STATN, ");
        break;
    case ADS1299_GPIO:
        Serial.print("GPIO, ");
        break;
    case ADS1299_MISC1:
        Serial.print("MISC1, ");
        break;
    case ADS1299_MISC2:
        Serial.print("MISC2, ");
        break;
    case ADS1299_CONFIG4:
        Serial.print("CONFIG4, ");
        break;
    }
}

// SPI communication methods
byte ADS1299::transfer(byte data)
{
    byte recv;

    SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE1));
    recv = SPI.transfer(data);
    SPI.endTransaction();
    return recv;
}

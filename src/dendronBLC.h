#ifndef _DENDRON_BLC_H
#define _DENDRON_BLC_H

#include "BluetoothSerial.h"
#include "napse.h"

#define BTC_BUFFER_SIZE 48

class DendronBLC {
  int num_ch;
  byte buf[BTC_BUFFER_SIZE];
    
  public:
    BluetoothSerial SerialBT;

    bool setup(int _num_ch);
    void sendData(uint32_t data[]);
};


#endif // _DENDRON_BLC_H

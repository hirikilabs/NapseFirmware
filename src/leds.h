#ifndef _LEDS_H
#define _LEDS_H

#include <Adafruit_NeoPixel.h>

class NapseLEDs {
  uint8_t pin;  
  Adafruit_NeoPixel strip;

  public: 
    void setup(uint8_t _pin);
    void set(uint32_t color);
};

#endif // _LEDS_H

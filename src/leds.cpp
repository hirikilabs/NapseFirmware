#include "leds.h"

// start NeoPixels
void NapseLEDs::setup(uint8_t _pin) {
  pin = _pin;
  strip = Adafruit_NeoPixel(NAPSE_LED_NUMBER, pin, NEO_GRB + NEO_KHZ800);
  strip.begin();
  strip.clear();
  strip.setBrightness(100);
}

// Set all LEDs
void NapseLEDs::set(uint32_t color) {
  for(int i=0; i<NAPSE_LED_NUMBER; i++) { 
    strip.setPixelColor(i, color);
    strip.show();
  }
}

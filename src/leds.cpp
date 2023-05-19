#include "leds.h"

void NapseLEDs::setup(uint8_t _pin) {
  pin = _pin;
  strip = Adafruit_NeoPixel(1, pin, NEO_GRB + NEO_KHZ800);
  strip.begin();
  strip.clear();
  strip.setBrightness(100);
}

void NapseLEDs::set(uint32_t color) {
  strip.setPixelColor(0, color);
  strip.show();
}

#ifndef NAPSE_H
#define NAPSE_H

// Configuration
#define NUM_CHANNELS    4

// GPIO and utility
#define DRDY_PIN        10
#define ADS_RST_PIN     3
#define BATT_PIN        1
#define BATT_DIVISION   3.0
#define BATT_CORRECTION 0.965
#define NEOPX_PIN       0

// values
#define COLOR_ERR       0xCC0000
#define COLOR_INIT      0x006600
#define COLOR_READ      0x0000FF


// BLE Configuration
#define SERVICE_UUID              "98abbe75-b810-4fe4-83a5-aea7dd9a915c"
#define CHARACTERISTIC_DATA_UUID  "9d746b90-552c-4eef-b3f0-506d1b3a48b2"
#define CHARACTERISTIC_START_STOP_UUID "5c804a1f-e0c0-4e30-b068-55a47b8a60c7"
#define CHARACTERISTIC_BATT_UUID  "50f9da7d-8dd4-4354-956d-3d1b5d68322e"


// Types

#endif


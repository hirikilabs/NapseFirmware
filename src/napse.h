#ifndef _NAPSE_H
#define _NAPSE_H

#include <inttypes.h>

// Configuration
#define NUM_CHANNELS    4
#define MAX_CHANNELS    8

// GPIO and utility
#define DRDY_PIN        10
#define ADS_RST_PIN     3
#define BATT_PIN        1
#define BATT_DIVISION   3.0
#define BATT_CORRECTION 0.965
#define NEOPX_PIN       0

// values
#define COLOR_ERR       0xCC0000
#define COLOR_READ      0x006600
#define COLOR_INIT_BLE  0x0000FF
#define COLOR_INIT_STA  0xFF00FF
#define COLOR_INIT_AP   0xFF6600


// BLE Configuration
#define SERVICE_UUID              "98abbe75-b810-4fe4-83a5-aea7dd9a915c"
#define CHARACTERISTIC_INFO_UUID  "71eee1da-2cef-4702-9d3e-4729e19232b5"
#define CHARACTERISTIC_DATA_UUID  "9d746b90-552c-4eef-b3f0-506d1b3a48b2"
#define CHARACTERISTIC_START_STOP_UUID "5c804a1f-e0c0-4e30-b068-55a47b8a60c7"
#define CHARACTERISTIC_BATT_UUID  "50f9da7d-8dd4-4354-956d-3d1b5d68322e"
#define CHARACTERISTIC_CONF_UUID  "aa766dda-0889-42ec-81f7-53cf26ad05ce"


// Types
typedef uint8_t channel_config_t;

#endif


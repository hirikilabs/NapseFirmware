#ifndef _NAPSE_H
#define _NAPSE_H

#include <inttypes.h>

// Configuration
#define NUM_CHANNELS    6
#define MAX_CHANNELS    8

#define LED_TYPE_BRG

// GPIO and utility
#define DRDY_PIN        16
#define ADS_RST_PIN     17
#define BATT_PIN        26
#define BATT_DIVISION   3.2
#define BATT_CORRECTION 1
#define NEOPX_PIN       4

#define ADC_SCLK        18
#define ADC_MISO        19
#define ADC_MOSI        25
#define ADC_CS          5

// LED color values
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

// packet fields
#define DATA_FIELD_NUMBER      0
#define DATA_FIELD_STATUS      1
#define DATA_FIELD_CHANNEL1    2
#define DATA_FIELD_CHANNEL2    3
#define DATA_FIELD_CHANNEL3    4
#define DATA_FIELD_CHANNEL4    5
#define DATA_FIELD_CHANNEL5    6
#define DATA_FIELD_CHANNEL6    7
#define DATA_FIELD_CHANNEL7    8
#define DATA_FIELD_CHANNEL8    9
#define DATA_FIELD_MARKER      10
#define DATA_FIELD_BATT        11


// WiFi TCP Commands
#define WIFI_COMMAND_STOP      0x00
#define WIFI_COMMAND_INFO      0x11
#define WIFI_COMMAND_MARK      0x33
#define WIFI_COMMAND_START     0x55
#define WIFI_COMMAND_GROUND    0x66
#define WIFI_COMMAND_TEST      0x77
#define WIFI_COMMAND_PWDN      0x88
#define WIFI_COMMAND_PWUP      0x99
#define WIFI_COMMAND_NORMAL    0xAA
#define WIFI_COMMAND_BATT      0xBB
#define WIFI_COMMAND_CLIENT    0xCC
#define WIFI_COMMAND_IMPEDANCE 0xDD


// BLC Commands
#define BLC_COMMAND_STOP       's'
#define BLC_COMMAND_MARK       'm'
#define BLC_COMMAND_START      'l'
#define BLC_COMMAND_GROUND     'g'
#define BLC_COMMAND_TEST       't'
#define BLC_COMMAND_PWDN       'd'
#define BLC_COMMAND_PWUP       'u'
#define BLC_COMMAND_NORMAL     'n'
#define BLC_COMMAND_BATT       'b'


// Data Logging
#define NAPSE_DATA_STOP        0
#define NAPSE_DATA_START       1

// Types
typedef uint8_t channel_config_t;

typedef struct {
    bool ok;
    String ssid;
    String psk;
    String client;
} napse_wifi_credentials_t;


typedef struct {
    boolean device_id_returned; // ADS ID ok?
    uint32_t init_color;        // LED colors
    bool do_delay;              // need to do delay after wifi command?
    float batt;                             // battery voltage
    String client_ip;                       // client UDP address
    napse_wifi_credentials_t wifi_creds;    // credentials for wifi network
} napse_t;

typedef struct {
    uint32_t field[10];
    float batt;
} dendron_data_t;

#endif

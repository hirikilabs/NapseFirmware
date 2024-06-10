#ifndef __NAPSE_WIFI
#define __NAPSE_WIFI

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <WiFiManager.h> 

#include "napse.h"

#define NAPSE_TCP_PORT 1337
#define NAPSE_UDP_PORT 31337

#define PACKET_STOP   0x00
#define PACKET_INFO   0x11
#define PACKET_START  0x55
#define PACKET_BATT   0xBB
#define PACKET_CONFIG 0xCC
#define PACKET_DST    0xDD

#define WIFI_SSID "HIRIKILABS"
#define WIFI_PSK  "Hal.2023"

#define WIFI_AP_PSK "napse1234"

#define WIFI_CREDS_FILE "/wificreds.txt"
#define WIFI_CLIENT_FILE "/wificlient.txt"

typedef enum {
    NAPSE_WIFI_MODE_AP,
    NAPSE_WIFI_MODE_STA
} napse_wifi_mode_t;

class NapseWifi {
    WiFiManager *wifiManager;
    void createAPPortal();
public:
    String clientIP;
    bool saveConfig;
    napse_wifi_mode_t wifi_mode;
    WebServer *webServer;
    bool init(String client_ip);
    void sendData(uint32_t data[]);
    WiFiClient client();
};

#endif

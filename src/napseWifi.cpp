#include "napseWifi.h"
#include "Arduino.h"
#include "HWCDC.h"
#include "esp_wifi_types.h"
#include <cstdint>
#include <stdio.h>
#include "FS.h"
#include "SPIFFS.h"

WiFiUDP udp;
WiFiServer tcp;

char client_ip[16] = "0.0.0.0";

//flag for saving data
bool shouldSaveConfig = false;
bool enteredConfigMode = false;

// callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void configModeCallback (WiFiManager *myWiFiManager) {
    //wifi_mode = NAPSE_WIFI_MODE_AP;
    enteredConfigMode = true;
    Serial.println("\n🛜 Can't connect to AP, creating wifi: ");
}

bool NapseWifi::init() {
    // try to conenct to WiFi
    wifi_mode = NAPSE_WIFI_MODE_STA;
    int timeout = 0;

    // create wifimanager
    wifiManager = new WiFiManager();

    // create Wifi AP name
    char ssid[25];
    uint64_t chip_id = ESP.getEfuseMac();
    snprintf(ssid, 25, "NAPSE-%llX", chip_id);

    // configure WifiManager
    wifiManager->setTitle("🧠 Napse Board");
    wifiManager->setAPCallback(configModeCallback);
    WiFiManagerParameter client_ip_param("clientip", "Client IP", client_ip, 15);
    wifiManager->addParameter(&client_ip_param);
    wifiManager->setClass("invert"); // dark theme
    wifiManager->setCustomHeadElement("<style>body{color: #ff9bfd; background-color: #494c88; font-family: sans-serif; font-size: 2vh;}\
    h1{font-size: 5vh;} h2{font-size: 3vh;} input{font-size: 2vh;}</style>");
    
    // try to connect in STA mode
    // if conection fails, create the AP Portal.
    Serial.println("❓ Trying to connect to saved WiFis: ");
    wifiManager->autoConnect(ssid);

    // if we connected normally
    if (enteredConfigMode == false) {
        wifi_mode = NAPSE_WIFI_MODE_STA;
        Serial.println("");
        Serial.print("✅ Connected to ");
        Serial.println(WiFi.SSID());
        Serial.print("💻 IP address: ");
        Serial.println(WiFi.localIP());
        tcp.begin(NAPSE_TCP_PORT);
        udp.begin(WiFi.localIP(), NAPSE_UDP_PORT);
    } else {
        wifi_mode = NAPSE_WIFI_MODE_AP;
        
        tcp.begin(NAPSE_TCP_PORT);
        udp.begin(WiFi.softAPIP(), NAPSE_UDP_PORT);
    }

    // mDNS (napse.local)
    if(!MDNS.begin("napse")) {
        Serial.println("❌ Error starting mDNS");
        return false;
    }
    
    // init the webserver
    webServer = new WebServer(80);
    
    return true;
}

// Create a WiFi network in AP mode
void NapseWifi::createAPPortal() {
    Serial.print("\n🛜 Can't connect to AP, creating wifi: ");
    // create Wifi name
    char ssid[25];
    uint64_t chip_id = ESP.getEfuseMac();
    snprintf(ssid, 25, "NAPSE-%llX", chip_id);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, WIFI_AP_PSK);
    Serial.println(WiFi.softAPSSID());
    Serial.print("💻 AP IP address: ");
    Serial.println(WiFi.softAPIP());
    wifi_mode = NAPSE_WIFI_MODE_AP;
}

// Send data via UDP
void NapseWifi::sendData(uint32_t data[]) {
    udp.beginPacket(udpAddress, 31337);
    udp.write((uint8_t *)data, 44);
    udp.endPacket();
}

// check if we have TCP clients
WiFiClient NapseWifi::client() {
    return tcp.available();
}


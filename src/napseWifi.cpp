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

// flags
bool shouldSaveConfig = false;
bool enteredConfigMode = false;

// vars
char temp_ip[15];

// callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("⚠️ Should save config");
  shouldSaveConfig = true;
}

void configModeCallback (WiFiManager *myWiFiManager) {
    //wifi_mode = NAPSE_WIFI_MODE_AP;
    enteredConfigMode = true;
    Serial.print("\n🛜 Can't connect to AP, creating wifi: ");
    Serial.print(myWiFiManager->getConfigPortalSSID());
    Serial.print(" : ");
    Serial.println(WiFi.softAPIP());
}

bool NapseWifi::init(String client_ip) {
    // try to conenct to WiFi
    wifi_mode = NAPSE_WIFI_MODE_STA;
    int timeout = 0;
    saveConfig = false;

    // create wifimanager
    wifiManager = new WiFiManager();

    // create Wifi AP name
    char ssid[25];
    uint64_t chip_id = ESP.getEfuseMac();
    snprintf(ssid, 25, "NAPSE-%llX", chip_id);

    // configure WifiManager
    wifiManager->setDebugOutput(false);
    wifiManager->setTitle("🧠 Napse Board");
    wifiManager->setAPCallback(configModeCallback);
    wifiManager->setSaveConfigCallback(saveConfigCallback);
    WiFiManagerParameter client_ip_param("clientip", "Client IP", temp_ip, 15);
    wifiManager->addParameter(&client_ip_param);
    wifiManager->setClass("invert"); // dark theme
    wifiManager->setCustomHeadElement("<style>body{color: #ff9bfd; background-color: #494c88; font-family: sans-serif; font-size: 2vh;}\
    h1{font-size: 5vh;} h2{font-size: 3vh;} input{font-size: 2vh;}</style>");
    
    // try to connect in STA mode
    // if conection fails, create the AP Portal.
    Serial.println("❓ Trying to connect to saved WiFis...");
    wifiManager->autoConnect(ssid);

    Serial.println("");
    Serial.print("✅ Connected to ");
    Serial.println(wifiManager->getWiFiSSID(false));
    Serial.print("💻 IP address: ");
    Serial.println(WiFi.localIP());
    clientIP = client_ip;
    tcp.begin(NAPSE_TCP_PORT);
    udp.begin(WiFi.localIP(), NAPSE_UDP_PORT);

    // need to save client ip?
    if (shouldSaveConfig) {
        saveConfig = true;
        clientIP = client_ip_param.getValue();
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

// clear wifimanager saved credentials
void NapseWifi::clearSettings() {
    wifiManager->resetSettings();
}


// Send data via UDP
void NapseWifi::sendData(uint32_t data[]) {
    udp.beginPacket(clientIP.c_str(), 31337);
    udp.write((uint8_t *)data, 44);
    udp.endPacket();
}

// check if we have TCP clients
WiFiClient NapseWifi::client() {
    return tcp.available();
}


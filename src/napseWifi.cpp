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

#define FORMAT_SPIFFS_IF_FAILED true
//#define SAVE_TEST_CREDS_FILE

bool NapseWifi::init() {
    Serial.println("💾 Starting filesystem.");
    // init SPIFFS for credentials and IP data
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("❌ An Error has occurred while mounting SPIFFS");
        return false;
    }

#ifdef SAVE_TEST_CREDS_FILE
    napse_wifi_credentials_t crds;
    crds.ssid = WIFI_SSID;
    crds.psk = WIFI_PSK;
    crds.client = "0.0.0.0";
    crds.ok = true;
    saveCredentials(crds);
#endif
    
    // try to conenct to WiFi
    wifi_mode = NAPSE_WIFI_MODE_STA;
    int timeout = 0;

    // get credentials from SPIFFS
    creds = getCredentials();
    // if we have saved credentials, try to connect in STA mode
    // if conection fails, create the AP Portal.
    if (creds.ok) {
        Serial.print("❓ Trying to connect to: ");
        Serial.print(creds.ssid);
        Serial.print(" : ");
        Serial.println(creds.psk);
        WiFi.begin(creds.ssid, creds.psk);
        // Wait for connection
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
            timeout++;
            if (timeout > 10) {
                createAPPortal();
                break;
            }
        }
    } else {
        createAPPortal();
    }

    // if we connected normally
    if (wifi_mode == NAPSE_WIFI_MODE_STA) {
        Serial.println("");
        Serial.print("✅ Connected to ");
        Serial.println(WiFi.SSID());
        Serial.print("💻 IP address: ");
        Serial.println(WiFi.localIP());
        tcp.begin(NAPSE_TCP_PORT);
        udp.begin(WiFi.localIP(), NAPSE_UDP_PORT);
    } else {
        tcp.begin(NAPSE_TCP_PORT);
        udp.begin(WiFi.softAPIP(), NAPSE_UDP_PORT);
    }

    if(!MDNS.begin("napse")) {
        Serial.println("❌ Error starting mDNS");
        return false;
    }
    
    // init the webserver
    webServer = new WebServer(80);
    
    return true;
}

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

napse_wifi_credentials_t NapseWifi::getCredentials() {
    napse_wifi_credentials_t creds;

    // try to read credentials
    File file = SPIFFS.open(WIFI_CREDS_FILE);
    if(!file || file.isDirectory()){
       Serial.println("❌ failed to open WiFi credentials file");
       creds.psk = WIFI_PSK;
       creds.ssid = WIFI_SSID;
       creds.client = "0.0.0.0";
       creds.ok = false;
       return creds;
    } else {
        creds.ssid = file.readStringUntil('\n');
        creds.psk = file.readStringUntil('\n');
        creds.client = file.readStringUntil('\n');
        file.close();
        // we have a SSID?
        if (creds.ssid == "") {
            creds.ok = false;
            return creds;
        }

        creds.ok = true;
        return creds;
    }
}

bool NapseWifi::saveCredentials(napse_wifi_credentials_t credentials) {
    File file = SPIFFS.open(WIFI_CREDS_FILE, FILE_WRITE);
    if(!file){
      Serial.println("❌ failed to open WiFi credentials file for writing");
      return false;
    } else {
        file.print(credentials.ssid);
        file.write('\n');
        file.print(credentials.psk);
        file.write('\n');
        file.print(credentials.client);
        file.write('\n');
        file.close();
        return true;
    }
}

void NapseWifi::sendData(uint32_t data[]) {
    const char * udpAddress = creds.client.c_str();
    udp.beginPacket(udpAddress, 31337);
    udp.write((uint8_t *)data, 40);
    udp.endPacket();
    udp.flush();
}

WiFiClient NapseWifi::client() {
    return tcp.available();
}

void NapseWifi::updateBatt(float vbatt) {

}

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

bool NapseWifi::init(napse_wifi_credentials_t creds) {
    // try to conenct to WiFi
    wifi_mode = NAPSE_WIFI_MODE_STA;
    int timeout = 0;

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
        // ok, connected
        wifi_credentials = creds;
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
    const char * udpAddress = wifi_credentials.client.c_str();
    udp.beginPacket(udpAddress, 31337);
    udp.write((uint8_t *)data, 44);
    udp.endPacket();
    udp.flush();
}

// check if we have TCP clients
WiFiClient NapseWifi::client() {
    return tcp.available();
}


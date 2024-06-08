#include "filesystem.h"

bool NapseFilesystem::init() {
// init SPIFFS for credentials and client IP data
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
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

    // ok?
    return true;
}

napse_wifi_credentials_t NapseFilesystem::getCredentials() {
    napse_wifi_credentials_t creds;

    // try to read credentials
    File file = SPIFFS.open(WIFI_CREDS_FILE);
    if(!file || file.isDirectory()){
       Serial.println("❌ failed to open WiFi credentials file");
       creds.psk = "DEFAULT";
       creds.ssid = "none";
       creds.client = "0.0.0.0";
       creds.ok = false;            // return false so the previous values are discarded
       return creds;
    } else {
        // read the values
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

String NapseFilesystem::getClientIP() {
    String client;

    // try to read credentials
    File file = SPIFFS.open(WIFI_CLIENT_FILE);
    if(!file || file.isDirectory()){
       Serial.println("❌ failed to open Client IP file");
       client = "0.0.0.0";
       return client;
    } else {
        // read the values
        client = file.readStringUntil('\n');
        file.close();
        // we have an IP?
        if (client == "") {
            client = "0.0.0.0";
            return client;
        }
        
        return client;
    }
}


bool NapseFilesystem::saveCredentials(napse_wifi_credentials_t credentials) {
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

bool NapseFilesystem::saveClientIP(String client) {
    File file = SPIFFS.open(WIFI_CLIENT_FILE, FILE_WRITE);
    if(!file){
      Serial.println("❌ failed to open Client IP file for writing");
      return false;
    } else {
        file.print(client);
        file.write('\n');
        file.close();
        return true;
    }
}

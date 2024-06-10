#include "filesystem.h"

bool NapseFilesystem::init() {
// init SPIFFS for client IP data
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        return false;
    }

#ifdef SAVE_TEST_CREDS_FILE
    String client = "0.0.0.0";
    saveClientIP(client);
#endif

    // ok?
    return true;
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

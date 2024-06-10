#ifndef __NAPSE_FILESYSTEM_H
#define __NAPSE_FILESYSTEM_H

#include <SPIFFS.h>
#include "napseWifi.h"

#define FORMAT_SPIFFS_IF_FAILED true    // try to format the filesystem if mounting fails
//#define SAVE_TEST_CREDS_FILE          // create a fixed credential file (FOR TESTING)

class NapseFilesystem {
public:
    bool init();
    String getClientIP();
    bool saveClientIP(String ip);
};

#endif

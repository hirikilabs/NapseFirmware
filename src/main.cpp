//#define DEBUG_ON
//#define USE_BLE

#include <Arduino.h>
#include <cstdint>
#include <stdlib.h>
#include <ADS1299.h>
#include "HWCDC.h"
#include "esp_wifi_types.h"
#include "napse.h"
#include <Adafruit_NeoPixel.h>

#ifdef USE_BLE
#include "esp32-hal-psram.h"
#include "esp_bt.h"
#include "napseBLE.h"
#else
#include "napseWifi.h"
#include "webs.h"
#endif

#include "leds.h"


channel_data_t last_data;               // last data received from the ADS
uint32_t channel_data[11];              // data to send
boolean device_id_returned = false;     // for checking if the ADS is OK
uint16_t start_stop = 0;                // start/stop logging flag
boolean start_stop_changed = false;     // for BLE
channel_config_t* channel_config;       // channel configuration
boolean config_changed = false;         // for BLE
uint32_t init_color;                    // initial LED color
bool do_delay;                          // need to do delay? (TCP)
float batt;                             // battery voltage
String client_ip;                       // client UDP address

ADS1299 ADS;                            // ADS object
#ifdef USE_BLE
NapseBLE bl;                            // BLE object
#else
NapseWifi wi;                           // WiFi object
#endif
NapseLEDs leds;

// Print current ADS config to the serial port
void printADSConfig() {
    Serial.println(ADS.getDeviceID(), BIN); // Function to return Device ID

    // prints dashed line to separate serial print sections
    Serial.println("----------------------------------------------");

    // Read ADS1299 Register at address 0x00 (see Datasheet pg. 35 for more info on SPI commands)
    int reg;
    reg = ADS.readRegister(0x00);
    ADS.printRegisterName(0x00);
    ADS.printRegister(reg);
    Serial.println("----------------------------------------------");

    for (int addr = 0x01; addr < 0x18; addr++) {
            reg = ADS.readRegister(addr);
            ADS.printRegisterName(addr);
            ADS.printRegister(reg);
    }
    Serial.println("----------------------------------------------");
}

// read battery
float get_batt() {
    uint16_t analog = analogRead(BATT_PIN);
    // make conversion, reference is 3.3V
    float voltage_measured = (analog * 3.3)/4096.0;
    return voltage_measured * BATT_DIVISION * BATT_CORRECTION;
}

// parse and apply channel configuration
void parseConfig(channel_config_t* config) {
    for (int i=0; i<MAX_CHANNELS; i++) {
        // shortcircuit mode
        if (config[i] & 0x01) {
            ADS.channelInputMode(i+1, CH_SHORTED);
        } else {
            ADS.channelInputMode(i+1, CH_NORMAL);
        }
        // gain xxxxnnnx
        uint8_t gain = (config[i] << 3) | 0b10001111;
        ADS.channelGainSet(i+1, gain);
    }
    // ok, configured
    config_changed = false;
}

// Initial ADS configuration
void configureADS() {
    // Basic configuration
    //ADS.writeRegister(ADS1299_CONFIG1, 0b11010110); // No daisy, Clock output disabled, 250SPS
    
    // Channels mode
    for (int i = 1; i <= ADS.numCh; i++) {
        ADS.channelInputMode(i, CH_NORMAL);
        //ADS.channelUseSRB2(i, true);
        ADS.channelGainSet(i, PGA_GAIN_24);
    }

    // Sample Rate
    ADS.setSampleRate(ADS1299_DR_500SPS);
    
    // Bias configuration
    ADS.writeRegister(ADS1299_CONFIG3, 0b11101100);

    // add channels to bias generation
    if (ADS.numCh == ADS1299_4CH) {
        ADS.writeRegister(ADS1299_BIAS_SENSP, 0b00001111);
    } else if (ADS.numCh == ADS1299_6CH) {
        ADS.writeRegister(ADS1299_BIAS_SENSP, 0b00111111);
    } else {
        ADS.writeRegister(ADS1299_BIAS_SENSP, 0b11111111);
    }
  
    // SRB1 (bit 5)
    ADS.writeRegister(ADS1299_MISC1, 0b00100000);

    // Test signal mode
    ADS.writeRegister(ADS1299_CONFIG2, 0b11010101);
  
    // referential mode, datasheet pg 68
    //ADS.writeRegister(ADS1299_MISC1, SRB1_NEG_INPUTS);
}

#ifndef USE_BLE
// WebServer handles
void handleRoot() {
    serverRootHTML.replace("%%SSID%%", wi.creds.ssid);
    serverRootHTML.replace("%%PSK%%", wi.creds.psk);
    serverRootHTML.replace("%%CLIENT%%", wi.creds.client);
    wi.webServer->send(200, "text/html", serverRootHTML);
}

// handle web configuration form data
void handleConfig() {
    if (wi.webServer->args() == 3) {
        napse_wifi_credentials_t creds;
        creds.ssid = wi.webServer->arg("fssid");
        creds.psk = wi.webServer->arg("fpsk");
        creds.client = wi.webServer->arg("fclient");
        bool ans = wi.saveCredentials(creds);
        if (ans) {
            wi.webServer->send(200, "text/html", "<h1>OK Rebooting...</h1>");
            delay(5000);
            ESP.restart();
        } else {
            wi.webServer->send(500, "text/plain", "Problem saving credentials");
        }    
    } else {
        wi.webServer->send(500, "text/plain", "Problem with parameters");
    }
}
#endif

void setup() {
    // data
    channel_config = (channel_config_t*) malloc(sizeof(channel_config_t) * MAX_CHANNELS);
  
    // NeoPixels
    // start on red
    leds.setup(NEOPX_PIN);
    leds.set(COLOR_ERR);

    // ???
    delay(3000);

    Serial.begin(115200);

    // ADS hardware reset
    pinMode(ADS_RST_PIN, OUTPUT);
    digitalWrite(ADS_RST_PIN, HIGH);

    // BATT PIN
    pinMode(BATT_PIN, INPUT);

    Serial.println();
    Serial.println("⚡ ADS1299-bridge has started!");

    // Start ADS
    ADS.setup(DRDY_PIN, ADS1299_4CH);   // (DRDY pin, num of channels);
    delay(10);                          // delay to ensure connection

    Serial.println("⚙️ ADS1299-bridge configured!");

    ADS.RESET();    // Send reset command
    Serial.println("✅ ADS1299-bridge reset!");
    ADS.STOP();     // Stop data capture
  
    // BLEServer
#ifdef USE_BLE
    Serial.println("🔌 Starting BLE...");
    bl.setup(ADS1299_4CH);
    Serial.println("📡 Started BLE...");
    // update battery value
    bl.updateBatt(get_batt());
#else
    Serial.println("🔌 Starting WiFi...");
    wi.init();
    Serial.println("📡 Started WiFi...");
    // start webserver
    wi.webServer->on("/", handleRoot);
    wi.webServer->on("/config", handleConfig);
    wi.webServer->begin();
    Serial.print("🪧 Client address: ");
    Serial.println(wi.creds.client);
#endif

    // ok, show it
#ifdef USE_BLE
    init_color = COLOR_INIT_BLE;
#else
    if (wi.wifi_mode == NAPSE_WIFI_MODE_STA) {
        init_color = COLOR_INIT_STA;
    } else {
        init_color = COLOR_INIT_AP;
    }
#endif
    leds.set(init_color);
}

void loop() {
    // First run
    if (device_id_returned == false)
    {
        Serial.print("🪪 ID: ");
        Serial.println(ADS.getDeviceID(), BIN); // Function to return Device ID
#ifdef DEBUG_ON
        // prints dashed line to separate serial print sections
        Serial.println("----------------------------------------------");
        // Read ADS1299 Register at address 0x00 (see Datasheet pg. 35 for more info on SPI commands)
        int reg;
        reg = ADS.readRegister(0x00);
        ADS.printRegisterName(0x00);
        ADS.printRegister(reg);
        Serial.println("----------------------------------------------");

        for (int addr = 0x01; addr < 0x18; addr++)
        {
            reg = ADS.readRegister(addr);
            ADS.printRegisterName(addr);
            ADS.printRegister(reg);
        }
        Serial.println("----------------------------------------------");

        // Write register command (see Datasheet pg. 38 for more info about writeRegister)
        ADS.writeRegister(ADS1299_CONFIG1, 0b11010110);
        Serial.print("Register 0x");
        Serial.print(ADS1299_CONFIG1, HEX);
        Serial.println(" modified.");
        Serial.println("----------------------------------------------");

        // Repeat PRINT ALL REGISTERS to verify changes
        for (int addr = 0x01; addr < 0x18; addr++)
        {
            reg = ADS.readRegister(addr);
            ADS.printRegisterName(addr);
            ADS.printRegister(reg);
        }
        Serial.println("----------------------------------------------");
#endif
        // initial configuration
        configureADS();

        Serial.print("🔋 Batt: ");
        Serial.println(get_batt());

        Serial.println("----------------------------------------------");

        // Start data conversions command
        device_id_returned = true;
    }

    // reset marker
    channel_data[10] = 0;
    
    // serial commands
    if (Serial.available() > 0) {
            int command = Serial.read();
            switch (command) {
                case 'l':
                    Serial.println("Log start");
                    leds.set(COLOR_READ);
                    ADS.START();
                    break;
                case 's':
                    Serial.println("Log stop");
                    leds.set(init_color);
                    ADS.STOP();
                    break;
                case 'p':
                    printADSConfig();
                    break;
            }
    }
  
    // BLE commands
    if (start_stop_changed) {
        if (start_stop == 0) {
            leds.set(init_color);
            ADS.STOP();
#ifdef USE_BLE
            bl.updateBatt(get_batt());
#endif
        } else {
#ifdef USE_BLE
            bl.updateBatt(get_batt());
#endif
            leds.set(COLOR_READ);
            ADS.START();
        }
        start_stop_changed = false;
    }


// WiFi commands
#ifndef USE_BLE
    WiFiClient c = wi.client();
    if (c) {
        while (c.connected()) {
            do_delay = true;
            while (c.available()>0) {
                char ch = c.read();
                switch (ch) {
                case WIFI_COMMAND_MARK:
                    // need to be fast
                    ch = c.read();
                    channel_data[10] = ch;
                    do_delay = false;
                    break;
                case WIFI_COMMAND_START:
                    // start
                    leds.set(COLOR_READ);
                    ADS.START();
                    break;
                case WIFI_COMMAND_STOP:
                    // stop
                    leds.set(init_color);
                    ADS.STOP();
                    break;
                case WIFI_COMMAND_INFO:
                    // send info
                    c.write(NUM_CHANNELS);
                    break;
                case WIFI_COMMAND_BATT:
                    batt = get_batt();
                    c.write((uint8_t*) &batt, sizeof(float));
                    break;
                case WIFI_COMMAND_CLIENT:
                    client_ip = c.readStringUntil('\n');
                    wi.creds.client = client_ip;
                    wi.saveCredentials(wi.creds);
                    break;
                default:
                    break;
                }
                c.write(ch);
            }
            if (do_delay) {
                delay(10);
            }
        }
        //c.flush();
        c.stop();
        //Serial.println("Client disconnected");
    }
#endif

    // if BLE or WiFi changed device configuration, apply it
    if (config_changed) {
        parseConfig(channel_config);
    }

    // get data if available (only when ADS is in START mode)
    if (ADS.updateData(&last_data)) {
            ADS.printData(last_data);
            channel_data[0] = last_data.numPacket;
            channel_data[1] = last_data.status;
            channel_data[2] = last_data.chan1;
            channel_data[3] = last_data.chan2;
            channel_data[4] = last_data.chan3;
            channel_data[5] = last_data.chan4;
            if (ADS.numCh > 4) {
                channel_data[6] = last_data.chan5;
                channel_data[7] = last_data.chan6;
            }
            if (ADS.numCh > 6) {
                channel_data[8] = last_data.chan7;
                channel_data[9] = last_data.chan8;
            }
#ifdef USE_BLE
            bl.updateData(channel_data);
#else
            wi.sendData(channel_data);
#endif
    }

#ifndef USE_BLE
    // if we are in WiFi Mode, handle the webserver
    wi.webServer->handleClient();
#endif
  
}

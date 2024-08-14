#include "dendronBLC.h"
#include "napse.h"

bool DendronBLC::setup(int _num_ch) {
  // check channels
  if (_num_ch < 4 || _num_ch > 8) {
    return false;
  }

  num_ch = _num_ch;

  // create BL name
  char blid[26];
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_BT);
  sprintf(blid, "DENDRON-%02X%02X%02X%02X%02X%02X", (int)mac[0], (int)mac[1],
           (int)mac[2], (int)mac[3], (int)mac[4], (int)mac[5]);

  SerialBT.begin(blid);

  return true;

}

void DendronBLC::sendData(uint32_t data[]) {
    SerialBT.write((uint8_t *)data, 48);
}

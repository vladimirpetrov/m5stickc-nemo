// ===== Settings ===== //
const uint8_t channels[] = {1, 6, 11}; // used Wi-Fi channels (available: 1-14)
const bool wpa2 = true; // WPA2 networks
int spamtype = 1; // 1 = funny, 2 = rickroll, maybe more later

/*
  SSIDs:
  - don't forget the \n at the end of each SSID!
  - max. 32 characters per SSID
  - don't add duplicates! You have to change one character at least
*/
char ssids[]={};

const char funnyssids[] PROGMEM = {
  "Mom Use This One\n"
  "Abraham Linksys\n"
  "Benjamin FrankLAN\n"
  "Martin Router King\n"
  "John Wilkes Bluetooth\n"
  "Pretty Fly for a Wi-Fi\n"
  "Bill Wi the Science Fi\n"
  "I Believe Wi Can Fi\n"
  "Tell My Wi-Fi Love Her\n"
  "No More Mister Wi-Fi\n"
  "LAN Solo\n"
  "The LAN Before Time\n"
  "Silence of the LANs\n"
  "House LANister\n"
  "Winternet Is Coming\n"
  "Ping’s Landing\n"
  "The Ping in the North\n"
  "This LAN Is My LAN\n"
  "Get Off My LAN\n"
  "The Promised LAN\n"
  "The LAN Down Under\n"
  "FBI Surveillance Van 4\n"
  "Area 51 Test Site\n"
  "Drive-By Wi-Fi\n"
  "Planet Express\n"
  "Wu Tang LAN\n"
  "Darude LANstorm\n"
  "Never Gonna Give You Up\n"
  "Hide Yo Kids, Hide Yo Wi-Fi\n"
  "Loading…\n"
  "Searching…\n"
  "VIRUS.EXE\n"
  "Virus-Infected Wi-Fi\n"
  "Starbucks Wi-Fi\n"
  "Text ###-#### for Password\n"
  "Yell ____ for Password\n"
  "The Password Is 1234\n"
  "Free Public Wi-Fi\n"
  "No Free Wi-Fi Here\n"
  "Get Your Own Damn Wi-Fi\n"
  "It Hurts When IP\n"
  "Dora the Internet Explorer\n"
  "404 Wi-Fi Unavailable\n"
  "Porque-Fi\n"
  "Titanic Syncing\n"
  "Test Wi-Fi Please Ignore\n"
  "Drop It Like It’s Hotspot\n"
  "Life in the Fast LAN\n"
  "The Creep Next Door\n"
  "Ye Olde Internet\n"
};

const char rickrollssids[] PROGMEM = {
  "01 Never gonna give you up\n"
  "02 Never gonna let you down\n"
  "03 Never gonna run around\n"
  "04 and desert you\n"
  "05 Never gonna make you cry\n"
  "06 Never gonna say goodbye\n"
  "07 Never gonna tell a lie\n"
  "08 and hurt you\n"
};

const char foobar[] PROGMEM = {
  "abh\nfoooo\nbarrr\nbaz\nbat\ngarply\nquux\nheyfuckface\n"
};

#define SSIDLEN 375 /* Change to whatever length you need */

char* randomSSID(){
  /* Change to allowable characters */
  const char possible[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 -.!)(,?%";
  static char ssid[SSIDLEN + 1];
  for(int p = 0, i = 0; i < SSIDLEN; i++){
    int r = random(0, strlen(possible));
    if(r % 7 == 0){
      ssid[p++] = '\n'; // inject newlines occasionally :D
    }
    ssid[p++] = possible[r];
  }
  ssid[SSIDLEN] = '\n';  M5.Lcd.setTextSize(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0, 1);
  M5.Lcd.println("Spamming Random SSIDs:");
  // Maximum broadcast SSID length is 32, but the strings might show longer in the output. Sorry.
  M5.Lcd.print(ssid);
  return ssid;
}

#include <WiFi.h>

extern "C" {
#include "esp_wifi.h"
  esp_err_t esp_wifi_set_channel(uint8_t primary, wifi_second_chan_t second);
  esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
}

// run-time variables
char emptySSID[32];
uint8_t channelIndex = 0;
uint8_t macAddr[6];
uint8_t wifi_channel = 1;
uint32_t currentTime = 0;
uint32_t packetSize = 0;
uint32_t packetCounter = 0;
uint32_t attackTime = 0;
uint32_t packetRateTime = 0;

// beacon frame definition
uint8_t beaconPacket[109] = {
  /*  0 - 3  */ 0x80, 0x00, 0x00, 0x00, // Type/Subtype: managment beacon frame
  /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: broadcast
  /* 10 - 15 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source
  /* 16 - 21 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source

  // Fixed parameters
  /* 22 - 23 */ 0x00, 0x00, // Fragment & sequence number (will be done by the SDK)
  /* 24 - 31 */ 0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, // Timestamp
  /* 32 - 33 */ 0xe8, 0x03, // Interval: 0x64, 0x00 => every 100ms - 0xe8, 0x03 => every 1s
  /* 34 - 35 */ 0x31, 0x00, // capabilities Tnformation

  // Tagged parameters

  // SSID parameters
  /* 36 - 37 */ 0x00, 0x20, // Tag: Set SSID length, Tag length: 32
  /* 38 - 69 */ 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, // SSID

  // Supported Rates
  /* 70 - 71 */ 0x01, 0x08, // Tag: Supported Rates, Tag length: 8
  /* 72 */ 0x82, // 1(B)
  /* 73 */ 0x84, // 2(B)
  /* 74 */ 0x8b, // 5.5(B)
  /* 75 */ 0x96, // 11(B)
  /* 76 */ 0x24, // 18
  /* 77 */ 0x30, // 24
  /* 78 */ 0x48, // 36
  /* 79 */ 0x6c, // 54

  // Current Channel
  /* 80 - 81 */ 0x03, 0x01, // Channel set, length
  /* 82 */      0x01,       // Current Channel

  // RSN information
  /*  83 -  84 */ 0x30, 0x18,
  /*  85 -  86 */ 0x01, 0x00,
  /*  87 -  90 */ 0x00, 0x0f, 0xac, 0x02,
  /*  91 -  92 */ 0x02, 0x00,
  /*  93 - 100 */ 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04, /*Fix: changed 0x02(TKIP) to 0x04(CCMP) is default. WPA2 with TKIP not supported by many devices*/
  /* 101 - 102 */ 0x01, 0x00,
  /* 103 - 106 */ 0x00, 0x0f, 0xac, 0x02,
  /* 107 - 108 */ 0x00, 0x00
};

// goes to next channel
void nextChannel() {
  if (sizeof(channels) > 1) {
    uint8_t ch = channels[channelIndex];
    channelIndex++;
    if (channelIndex > sizeof(channels)) channelIndex = 0;

    if (ch != wifi_channel && ch >= 1 && ch <= 14) {
      wifi_channel = ch;
      //wifi_set_channel(wifi_channel);
      esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
    }
  }
}

// generates random MAC
void randomMac() {
  for (int i = 0; i < 6; i++)
    macAddr[i] = random(256);
}

void beaconSpam(const char list[]){
  attackTime = currentTime;

  // temp variables
  int i = 0;
  int j = 0;
  int ssidNum = 1;
  char tmp;
  int ssidsLen = strlen_P(list);
  bool sent = false;

  // go to next channel
  nextChannel();

  while (i < ssidsLen) {
    // read out next SSID
    j = 0;
    do {
      tmp = pgm_read_byte(list + i + j);
      j++;
    } while (tmp != '\n' && j <= 32 && i + j < ssidsLen);

    uint8_t ssidLen = j - 1;

    // set MAC address
    macAddr[5] = ssidNum;
    ssidNum++;

    // write MAC address into beacon frame
    memcpy(&beaconPacket[10], macAddr, 6);
    memcpy(&beaconPacket[16], macAddr, 6);

    // reset SSID
    memcpy(&beaconPacket[38], emptySSID, 32);

    // write new SSID into beacon frame
    memcpy_P(&beaconPacket[38], &list[i], ssidLen);

    // set channel for beacon frame
    beaconPacket[82] = wifi_channel;

    // send packet
    for (int k = 0; k < 3; k++) {
      packetCounter += esp_wifi_80211_tx(WIFI_IF_STA, beaconPacket, packetSize, 0) == 0;
      delay(1);
    }
    i += j;
  }
}

#include "main.hpp"
#include "Autopairing.hpp"

uint8_t broadcastAddress[] = BOARD;
GButton cloudButtonESP(CLOUD_PIN);
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

int counter = 0;

void setup() {
  boardInitialisation();
  buttonInitialize();
  #ifdef BOARD2
    screenInitialize();
  #endif
}

void loop() {
  messageSend();

  #ifdef BOARD1
    masterPairing();
    uploadCheckMaster();
  #endif
  #ifdef BOARD2
    slavePairing();
    // drawPairingAddr();
  #endif
  timersCheck();
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {}
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  messageRecv(mac, incomingData, len);
}
#ifdef BOARD2

void screenInitialize() {
  oled.init();
  oled.clear();
  oled.setScale(2);
  oled.home();
}
#endif
void buttonInitialize() {
  cloudButtonESP.setDebounce(30);
  cloudButtonESP.setTimeout(1000);
  cloudButtonESP.setClickTimeout(500);

  cloudButtonESP.setType(LOW_PULL);
  cloudButtonESP.setDirection(NORM_OPEN);
}
void boardInitialisation() {
  Serial.begin(115200);
  Serial.println("Serial Begin");

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  esp_now_register_recv_cb(OnDataRecv);

  #ifdef BOARD2
    pinMode(RESET_32, OUTPUT);
    pinMode(BOOT_32, OUTPUT);

    pinMode(LED_BUILTIN, OUTPUT);
    // digitalWrite(LED_BUILTIN, LOW);
  #endif
  #ifdef BOARD1
    pinMode(DTR, INPUT);
    pinMode(RTS, INPUT);

    pinMode(LED_BUILTIN, OUTPUT);
  #endif
}
void drawPairingAddr() {
  oled.setCursor(5, 10);
  oled.printf("%x:%x:%x:%x:%x:%x", peerAddr[0], peerAddr[1], peerAddr[2], peerAddr[3], peerAddr[4], peerAddr[5]);
}
#include "main.hpp"
#include "Autopairing.hpp"

uint8_t broadcastAddress[] = BOARD;

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
void buttonInitialize() {
  cloudButton.setDebounce(30);
  cloudButton.setTimeout(1000);
  cloudButton.setClickTimeout(500);

  cloudButton.setType(LOW_PULL);
  cloudButton.setDirection(NORM_OPEN);
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
    digitalWrite(LED_BUILTIN, LOW);
  #endif
  #ifdef BOARD1
    pinMode(DTR, INPUT);
    pinMode(RTS, INPUT);
  #endif
}

#endif
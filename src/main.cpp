#include "main.hpp"
#include "Autopairing.hpp"

uint8_t broadcastAddress[] = BOARD;
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

int counter = 0;

void setup() {
  boardInitialisation();
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
    drawInfoOnScreen();
    slavePairing();
  #endif
  timersCheck();
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  lastPackageTime = millis();
}
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  messageRecv(mac, incomingData, len);
  lastPackageTime = millis();
}
#ifdef BOARD2

void screenInitialize() {
  oled.init();
  oled.clear();
  oled.setScale(2);
  oled.home();

  oled.printf("%s\n", paired == 0 ? "Not paired" : "Paired");
  oled.setCursor(0, 2);
  oled.printf("Ch: %d%%\n", getChargeProcent());
}
#endif

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
    pinMode(CLOUD_PIN, INPUT);
    // digitalWrite(LED_BUILTIN, LOW);
  #endif
  #ifdef BOARD1
    pinMode(DTR, INPUT);
    pinMode(RTS, INPUT);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(CLOUD_PIN, INPUT);
  #endif
}
void drawInfoOnScreen() {
  static uint32_t timer = millis() + 1000;
  static int prevParams[2] = {paired, getChargeProcent()};

  if((millis() > timer)) {
    int charge = getChargeProcent();
    if(prevParams[0] != paired || prevParams[1] != charge) {
      oled.clear();
      oled.home();
      oled.printf("%s\n", paired == 0 ? "Not paired" : "Paired");
      oled.setCursor(0, 2);
      oled.printf("Ch: %d%%\n", charge);
      oledDrawChargeBlock();

      prevParams[0] = paired;
      prevParams[1] = charge;
    }
    timer = millis() + 1000;
  }
}
int getChargeProcent() {
  int ADCvalue = analogRead(A0); 
  return (((ADCvalue > 840 ? 840 : (ADCvalue < 700 ? 700 : ADCvalue)) - 700) * 10) / 14;
}
void oledDrawChargeBlock() {
  oled.line(10, 20, 60, 20);
  oled.line(10, 50, 60, 50);
  oled.line(10, 20, 10, 50);

  oled.line(60, 20, 60, 30);
  oled.line(60, 50, 60, 40);

  oled.line(60, 30, 70, 30);
  oled.line(60, 40, 70, 40);
  oled.line(70, 30, 70, 40);
}
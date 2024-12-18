#include "main.hpp"
#include "Autopairing.hpp"

uint8_t broadcastAddress[] = BOARD;
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

uint8_t charge = 0;

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
    ADCpowerTimer();
    drawInfoOnScreen();
    slavePairing();
  #endif
  timersCheck();
  pingConnection();
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  lastSendPackageTime = millis();
}
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  messageRecv(mac, incomingData, len);
  lastRecvPackageTime = millis();
}
#ifdef BOARD2

void screenInitialize() {
  oled.init();
  oled.clear();
  oled.setScale(2);
  oled.home();

  oled.printf("%s\n", paired == 0 ? "Not paired" : "Paired");
  oled.setCursor(0, 2);
  charge = getChargeProcent();
  oled.printf("Ch: %d%%\n", charge);
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

    pinMode(D1, OUTPUT);
  #endif

  EEPROM.begin(4096);
  getMacFromEEPROM();
}
void drawInfoOnScreen() {
  static int prevParams[2] = {paired, getChargeProcent()};

  if(prevParams[0] != paired) {
    oledCustomClear(0, 0, 10);
    oled.home();
    oled.printf("%s\n", paired == 0 ? "Not paired" : "Paired");


    prevParams[0] = paired;
  }
  if(prevParams[1] != charge) {
    oledCustomClear(0, 2, 10);
    oled.setCursor(0, 2);
    oled.printf("Ch: %d%%\n", charge);

    // oledDrawChargeBlock();
    prevParams[1] = charge;
  }
  oled.setCursor(0, 3);
  oled.printf("%d", Serial.baudRate());
}
int getChargeProcent() {
  int ADCvalue = analogRead(A0); 
  return (((ADCvalue > 840 ? 840 : (ADCvalue < 700 ? 700 : ADCvalue)) - 700) * 10) / 14;
}
void oledDrawChargeBlock() {
  // Draw bitmaps
}
void ADCpowerTimer() {
  static uint32_t timer = millis();

  if(millis() > timer + 5000) {
    charge = getChargeProcent();
    timer = millis();
  }
}
void oledCustomClear(int x, int y, int width) {
  oled.setCursor(x, y);
  for(int i = 0; i < width; i++) oled.printf(" ");
}
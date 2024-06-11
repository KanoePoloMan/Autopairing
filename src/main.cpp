#include "main.hpp"

const uint32_t timeout_micros = (int)(1.0 / 74880 * 1E6) * 20;

uint8_t broadcastAddress[] = BOARD;

uint8_t buf_recv[BUFFER_SIZE];
uint8_t buf_send[BUFFER_SIZE];
uint8_t buf_size = 0;
uint32_t send_timeout = 0;

bool paired = 0;

void setup() {
  #ifdef BOARD1
    masterPairing();
  #endif
  #ifdef BOARD2
    slavePairing();
  #endif
  boardInitialisation();
}
uint32_t reset_timer = 0;
uint32_t boot_timer = 0;

void loop() {
  if (Serial.available() > 0) {
    while (Serial.available() > 0 && buf_size < BUFFER_SIZE) {
      buf_send[buf_size] = Serial.read();
      send_timeout = micros() + timeout_micros;
      buf_size++;
    }
  }
    if (buf_size == BUFFER_SIZE || (buf_size > 0 && micros() >= send_timeout)) {
      esp_now_send(broadcastAddress, (uint8_t *) &buf_send, buf_size);
      buf_size = 0;
  }
  #ifdef BOARD1
    if(digitalRead(RTS) == 0){
      uint8_t temp_buff[BUFFER_SIZE];
      for(int i = 0; i < BUFFER_SIZE; i++) temp_buff[i] = 1;
      uint8_t temp_buff_size = 250;
      esp_now_send(broadcastAddress, (uint8_t *) &temp_buff, temp_buff_size);
    } 
    if (digitalRead(DTR) == 0) {
      uint8_t temp_buff[BUFFER_SIZE];
      for(int i = 0; i < BUFFER_SIZE; i++) temp_buff[i] = 2;
      uint8_t temp_buff_size = 250;
      esp_now_send(broadcastAddress, (uint8_t *) &temp_buff, temp_buff_size);
    }
  #endif
  #ifdef BOARD2
    if(millis() > reset_timer){
      digitalWrite(RESET_32, 0);
    }
    if(millis() > boot_timer){
      digitalWrite(BOOT_32, 0);
    }
  #endif

  if(digitalRead(CLOUD_PIN) == 1) {
    #ifdef BOARD1
      masterPairing();
    #endif
    #ifdef BOARD2
      slavePairing();
    #endif
    
    boardInitialisation();
  }
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
//  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
//    Serial.println("success");
  }
  else{
//    Serial.println("fail");
  }
}
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&buf_recv, incomingData, sizeof(buf_recv));
//  Serial.printf("Bytes received: %s ", buf_recv);
  Serial.write(buf_recv, len);
  #ifdef BOARD2
    bool flag = 0;
    for(int i = 0; i < BUFFER_SIZE; i++){
      if(buf_recv[i] != 1) break;
      if(i == BUFFER_SIZE - 1){
        digitalWrite(RESET_32, 1);
        boot_timer = millis() + 300;
      }
    }
    for(int i = 0; i < BUFFER_SIZE; i++){
      if(buf_recv[i] != 2) break;
      if(i == BUFFER_SIZE - 1){
        digitalWrite(BOOT_32, 1);
        reset_timer = millis() + 200;
      }
    }
    
  #endif
    if(paired == 0) {
      memcpy(broadcastAddress, incomingData, 6);
      paired = 1;
    }
}
void masterPairing() {
    uint32_t time = millis() + 10000;
    paired = 0;

    uint32_t led_time = millis() + 200;
    bool led_state = 0;

    WiFi.softAP("PERVOPROHODETS_AP", "PERVOPROHODETS_AP", 1, true, 1);

    if (esp_now_init() != 0) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);

    while(millis() < time || paired != 1){
      uint8_t broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
      uint8_t mymac[6];
      wifi_get_macaddr(STATION_IF, mymac);
      esp_now_send(broadcast, mymac, 6);

      if(millis() > led_time) {
        digitalWrite(LED_BUILTIN, led_state);
        led_state = !led_state;
        led_time = millis() + 200;
      }
      digitalWrite(LED_BUILTIN, 0);
    }

    esp_now_unregister_send_cb();
    esp_now_unregister_recv_cb();
    esp_now_deinit();
    WiFi.disconnect();
}
void slavePairing() {
  uint32_t time = millis() + 10000;
  paired = 0;

  uint32_t led_time = millis() + 200;
  bool led_state = 0;

  WiFi.begin("PERVOPROHODETS_AP", "PERVOPROHODETS_AP", 1);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  while(millis() < time || paired != 1){
    uint8_t broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint8_t mymac[6];
    wifi_get_macaddr(STATION_IF, mymac);
    esp_now_send(broadcast, mymac, 6);

    if(millis() > led_time) {
      digitalWrite(LED_BUILTIN, led_state);
      led_state = !led_state;
      led_time = millis() + 200;
    }
    digitalWrite(LED_BUILTIN, 0);
  }

  esp_now_unregister_send_cb();
  esp_now_unregister_recv_cb();
  esp_now_deinit();
  WiFi.disconnect();
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
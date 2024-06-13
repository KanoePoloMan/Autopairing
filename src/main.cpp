#include "main.hpp"
#include "Autopairing.hpp"

uint8_t broadcastAddress[] = BOARD;

int counter = 0;

void setup() {
  #ifdef BOARD1
    // masterPairing();
  #endif
  #ifdef BOARD2
    screenInitialize();

    // slavePairing();
  #endif
  boardInitialisation();
}
uint32_t reset_timer = 0;
uint32_t boot_timer = 0;

void loop() {
  messageSend();

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
  timersCheck();

  // if(digitalRead(CLOUD_PIN) == 1) {
  //   #ifdef BOARD1
  //     masterPairing();
  //   #endif
  //   #ifdef BOARD2
  //     slavePairing();
  //   #endif
    
  //   boardInitialisation();
  // }
  uint8_t broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  esp_now_send(broadcast, (uint8_t *)broadcast, 6);
  delay(1000);

  #ifdef BOARD2

  // oled.clear();
  oled.home();
  oled.printf("%04d", counter);
  #endif
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
//----------------------
    Serial.printf("%d: MAC: %02x:%02x:%02x:%02x:%02x:%02x\n\r", counter, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    counter++;
//----------------------
}
#ifdef BOARD2

void screenInitialize() {
  oled.init();
  oled.clear();
  oled.setScale(2);
  oled.home();
}

#endif
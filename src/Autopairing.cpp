#include "Autopairing.hpp"

const uint32_t timeout_micros = (int)(1.0 / 115200 * 1E6) * 20;
uint32_t send_timeout = 0;

uint32_t pinsTimer[2];

uint8_t peerAddr[6];
int paired = 0;

uint32_t lastSendPackageTime = millis();
uint32_t lastRecvPackageTime = millis();

UartBuffer uartBuffer;

MessageStruct::MessageStruct() {
    msgType = MessageType::DATA;
    command = 0;
    senderType = SenderType::FLASH;

    uint8_t mac[6];
    wifi_get_macaddr(0, mac);
    password = mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5];
    msgLen = 0;
}

void masterPairing() {
    static uint32_t timer = millis();
    static bool hold = false;

    if(digitalRead(CLOUD_PIN) == 1 && hold == false) {
        timer = millis() + 5000;
        hold = true;
    } else if(digitalRead(CLOUD_PIN) == 0) hold = false;
    if(millis() > timer && hold == true) paired = 0;

    if(paired == 0) {
        MessageStruct pairingMsg;
        pairingMsg.msgType = MessageType::PAIRING;
        pairingMsg.msgLen = 6;
        uint8_t mymac[6];
        wifi_get_macaddr(0, mymac);
        memcpy(pairingMsg.message, mymac, 6);

        uint32_t timeout = millis() + 10000;
        uint32_t led_timeout = millis() + 200;
        bool led_state = 0;

        uint8_t broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        while(millis() < timeout && paired != 1) {
            if(millis() > led_timeout) {
                esp_now_send(broadcast, (uint8_t *)&pairingMsg, pairingMsg.msgLen + 5);
                Serial.println("Send pairing message");

                led_state = !led_state;
                digitalWrite(LED_BUILTIN, led_state);
                led_timeout = millis() + 200;
                ESP.wdtFeed();
            }
        }
        digitalWrite(LED_BUILTIN, HIGH);
    }
}
void slavePairing() {
    static uint32_t timer = millis();
    static bool hold = false;

    if(digitalRead(CLOUD_PIN) == 1 && hold == false) {
        timer = millis() + 5000;
        hold = true;
    } else if(digitalRead(CLOUD_PIN) == 0) hold = false;
    if(millis() > timer && hold == true) paired = 0;
}
void messageSend() {
    static MessageStruct sendMsg;
    #ifdef BOARD1
        sendMsg.senderType = SenderType::FLASH;
    #endif
    #ifdef BOARD2
        sendMsg.senderType = SenderType::PERVOPROHODETS;
    #endif
    if(Serial.available() > 0) {
        while(Serial.available() > 0 && sendMsg.msgLen < BUFFER_SIZE) {
            #ifdef BOARD1
                uint8_t currSym = Serial.peek();
                uartBuffer.addSym(currSym);
                uint8_t sequence[] = {0xC0, 0x00, 0x0F, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00};
                int16_t result = -1;
                if(currSym == 0xC0) result = uartBuffer.findSequence(sequence, 9);
                if(result != -1) {
                    digitalWrite(5, HIGH);

                    uint8_t speed[4];

                    for(int i = 0; i < 4; i++) {
                        speed[i] = uartBuffer.getUartBuffer()[result + 9 + i];
                    }
                    MessageStruct command;

                    command.msgType = MessageType::UPLOAD_COMMAND;
                    command.command = SubcommandType::CHANGE_BAUDRATE;
                    command.senderType = SenderType::PERVOPROHODETS;
                    command.msgLen = 4;

                    for(int i = 0; i < 4; i++) command.message[i] = result.second[i];

                    esp_now_send(peerAddr, (uint8_t *)&command, command.msgLen + 5);

                    uint32_t speed32 = 0;
                    speed32 |= result.second[0] << 24; 
                    speed32 |= result.second[1] << 16; 
                    speed32 |= result.second[2] << 8; 
                    speed32 |= result.second[3] << 0;

                    Serial.updateBaudRate(speed32);
                }
            #endif
            sendMsg.message[sendMsg.msgLen] = Serial.read();
            send_timeout = micros() + timeout_micros;
            sendMsg.msgLen++;
        }
    }
    if (sendMsg.msgLen == BUFFER_SIZE || (sendMsg.msgLen > 0 && micros() >= send_timeout)) {
        esp_now_send(peerAddr, (uint8_t *)&sendMsg, sendMsg.msgLen + 5);
        sendMsg.msgLen = 0;
    }
}
void messageRecv(uint8_t *mac, uint8_t *incomiingData, uint8_t len) {
    MessageStruct incoming;
    memcpy(&incoming, incomiingData, sizeof(incoming));

    switch(incoming.msgType) {
        case MessageType::DATA:
            if(!checkPassword(mac, incoming.password)) return;
            Serial.write(incoming.message, incoming.msgLen);
        break;
        case MessageType::PAIRING:
            if(paired != 0) return;
            if(incoming.senderType == SenderType::FLASH) {
                #ifdef BOARD2
                memcpy(peerAddr, incoming.message, 6);
                MessageStruct answer;
                answer.msgType = MessageType::PAIRING;
                answer.senderType = SenderType::PERVOPROHODETS;
                answer.msgLen = 6;

                uint8_t mymac[6];
                wifi_get_macaddr(0, mymac);
                memcpy(answer.message, mymac, 6);

                esp_now_send(peerAddr, (uint8_t *)&answer, answer.msgLen + 5);
                #endif
            } else if(incoming.senderType == SenderType::PERVOPROHODETS) {
                #ifdef BOARD1
                memcpy(peerAddr, incoming.message, 6);

                MessageStruct testPackage;
                testPackage.msgType = MessageType::UPLOAD_COMMAND;
                testPackage.command = SubcommandType::TEST;
                testPackage.msgLen = 5;

                testPackage.message[0] = 5;
                testPackage.message[1] = 4;
                testPackage.message[2] = 3;
                testPackage.message[3] = 2;
                testPackage.message[4] = 1;

                esp_now_send(peerAddr, (uint8_t *)&testPackage, testPackage.msgLen + 5);
                #endif
            }
        break;
        case MessageType::UPLOAD_COMMAND:
            if(!checkPassword(mac, incoming.password)) return;
            #ifdef BOARD2
            if(incoming.command == SubcommandType::RESET) {
                digitalWrite(RESET_32, 1);
                pinsTimer[SubcommandType::RESET] = millis() + 300;
            } else if(incoming.command == SubcommandType::BOOT) {
                digitalWrite(BOOT_32, 1);
                pinsTimer[SubcommandType::BOOT] = millis() + 200;
            } else if(incoming.command == SubcommandType::TEST) {
                MessageStruct answer;
                answer.senderType = SenderType::PERVOPROHODETS;
                answer.command = SubcommandType::TEST;
                answer.msgType = MessageType::UPLOAD_COMMAND;
                answer.msgLen = incoming.msgLen;
                memcpy(answer.message, incoming.message, incoming.msgLen);

                esp_now_send(peerAddr, (uint8_t *)&answer, answer.msgLen + 5);
            } else if(incoming.command == SubcommandType::PAIRED_IS_OK) {
                paired = 1;
                for(int i = 0; i < 6; i++) if(EEPROM[i] != mac[i]) EEPROM[i] = mac[i];
                EEPROM.commit();
            } else if(incoming.command == SubcommandType::PING) {
                MessageStruct pingAnswer;
                pingAnswer.msgType = MessageType::UPLOAD_COMMAND;
                pingAnswer.command = SubcommandType::PING;
                pingAnswer.senderType = SenderType::PERVOPROHODETS;

                esp_now_send(peerAddr, (uint8_t *)&pingAnswer, pingAnswer.msgLen + 5);
            } else if(incoming.command == SubcommandType::CHANGE_BAUDRATE) {
                uint32_t speed = 0;

                speed |= incoming.message[0] << 24;
                speed |= incoming.message[1] << 16;
                speed |= incoming.message[2] << 8;
                speed |= incoming.message[3] << 0;

                Serial.updateBaudRate(speed);
            }
            #endif
            #ifdef BOARD1
            if(incoming.command == SubcommandType::TEST) {
                if(incoming.senderType == SenderType::PERVOPROHODETS) {
                    if(incoming.msgLen == 5 && incoming.message[0] == 5 
                                            && incoming.message[1] == 4 
                                            && incoming.message[2] == 3 
                                            && incoming.message[3] == 2 
                                            && incoming.message[4] == 1){
                        paired = 1;
                        MessageStruct goodPairing;
                        goodPairing.msgType = MessageType::UPLOAD_COMMAND;
                        goodPairing.command = SubcommandType::PAIRED_IS_OK;

                        esp_now_send(peerAddr, (uint8_t *)&goodPairing, 5);
                        
                        for(int i = 0; i < 6; i++) if(EEPROM[i] != mac[i]) EEPROM[i] = mac[i];
                        EEPROM.commit();
                    }
                }
            }
            #endif
            if(incoming.command == SubcommandType::SETUP_PING) {
                if(incoming.msgLen == 0 && paired == 1 && mac[0] == peerAddr[0] &&
                                                          mac[1] == peerAddr[1] &&
                                                          mac[2] == peerAddr[2] &&
                                                          mac[3] == peerAddr[3] &&
                                                          mac[4] == peerAddr[4] &&
                                                          mac[5] == peerAddr[5]) {
                    MessageStruct setupPingAnswer;
                    setupPingAnswer.msgType = MessageType::UPLOAD_COMMAND;
                    setupPingAnswer.command = SubcommandType::SETUP_PING;
                    #ifdef BOARD1
                        setupPingAnswer.senderType = SenderType::FLASH;
                    #endif
                    #ifdef BOARD2
                        setupPingAnswer.senderType = SenderType::PERVOPROHODETS;
                    #endif
                    setupPingAnswer.msgLen = 1;

                    esp_now_send(peerAddr, (uint8_t *)&setupPingAnswer, 5);
                } else if(incoming.msgLen == 1) paired = 1;
            }
        break;
        case MessageType::OTHER_DATA:
            
        break;
    }
    // Serial.printf("Msg type: %d \n\r", incoming.msgType);
    // Serial.printf("Command: %d \n\r", incoming.command);
    // Serial.printf("senderType: %d \n\r", incoming.senderType);
    // Serial.printf("Passwors: %d \n\r", incoming.password);
    // Serial.printf("Msg Len: %d \n\r", incoming.msgLen);
    // Serial.printf("Data:");
    // for(int i = 0; i < incoming.msgLen; i++) {
    //     Serial.printf("%d ", incoming.message[i]);
    //     if(i % 20 == 0) Serial.printf("\n\r");
    // }
    // Serial.printf("\n\r\n\r\n\r");
}
bool checkPassword(uint8_t *mac, uint8_t password) {
    uint8_t realPassword = mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5];
    
    return realPassword == password;
}
void timersCheck() {
    #ifdef BOARD2
    if(millis() > pinsTimer[SubcommandType::RESET]) digitalWrite(RESET_32, 0);
    if(millis() > pinsTimer[SubcommandType::BOOT]) digitalWrite(BOOT_32, 0);
    #endif
}
void uploadCheckMaster() {
    if(digitalRead(RTS) == 0) {
        MessageStruct resetCommand;
        resetCommand.msgType = MessageType::UPLOAD_COMMAND;
        resetCommand.command = SubcommandType::RESET;

        esp_now_send(peerAddr, (uint8_t *)&resetCommand, 5);
    }
    if(digitalRead(DTR) == 0) {
        MessageStruct bootCommand;
        bootCommand.msgType = MessageType::UPLOAD_COMMAND;
        bootCommand.command = SubcommandType::BOOT;

        esp_now_send(peerAddr, (uint8_t *)&bootCommand, 5);
    }
}
void pingConnection() {
    #ifdef BOARD1
    if(millis() > lastRecvPackageTime + 3000 && millis() > lastSendPackageTime + 3000) {
        MessageStruct ping;
        ping.msgType = MessageType::UPLOAD_COMMAND;
        ping.command = SubcommandType::PING;
        esp_now_send(peerAddr, (uint8_t *)&ping, ping.msgLen + 5);
    }
    #endif
    if(millis() > lastRecvPackageTime + 10000) paired = 0;
}
void getMacFromEEPROM() {
    for(int i = 0; i < 6; i++) peerAddr[i] = EEPROM[i];

    MessageStruct setupPing;
    setupPing.msgType = MessageType::UPLOAD_COMMAND;
    setupPing.command = SubcommandType::SETUP_PING;
    #ifdef BOARD1
        setupPing.senderType = SenderType::FLASH;
    #endif
    #ifdef BOARD2
        setupPing.senderType = SenderType::PERVOPROHODETS;
    #endif
    esp_now_send(peerAddr, (uint8_t *)&setupPing, 5);
}
#ifdef BOARD1
std::pair<boolean, uint8_t[4]> commandAnalysier(std::pair<uint8_t, uint8_t[18]> command) {
    std::pair<boolean, uint8_t[4]> returned;
    returned.first = false;

    if(command.second[0] != 0xC0) return returned;
    if(command.second[1] != 0x00) return returned;
    if(command.second[2] != 0x0F) return returned;
    
    returned.first = true;
    for(int i = 0; i < 4; i++) {
        returned.second[i] = command.second[command.first - 6 - i];
    }
    return returned;
}
#endif
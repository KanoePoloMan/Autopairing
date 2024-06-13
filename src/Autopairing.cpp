#include "Autopairing.hpp"

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

    if(cloudButton.isDouble()) timer = millis() + 2000;
    if(millis() < timer && cloudButton.isHolded()) paired = 0;

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
            esp_now_send(broadcast, (uint8_t *)&pairingMsg, pairingMsg.msgLen + 5);
            if(millis() > led_timeout) {
                led_state = !led_state;
                digitalWrite(LED_BUILTIN, led_state);
                led_timeout = millis();
            }
        }
        digitalWrite(LED_BUILTIN, LOW);
    }
}
void slavePairing() {
    static uint32_t timer = millis();

    if(cloudButton.isDouble()) timer = millis() + 2000;
    if(millis() < timer && cloudButton.isHolded()) paired = 0;

    if(paired == 0) {
        uint32_t timeout = millis() + 10000;
        uint32_t led_timeout = millis() + 200;
        bool led_state = 0;

        uint8_t broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        while(millis() < timeout && paired != 1) {
            if(millis() > led_timeout) {
                led_state = !led_state;
                digitalWrite(LED_BUILTIN, led_state);
                led_timeout = millis();
            }
        }
        digitalWrite(LED_BUILTIN, HIGH);
    }
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
    memcpy(&incoming, incomiingData, len);

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
            #ifdef BOARD2
            if(!checkPassword(mac, incoming.password)) return;
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
            }
            #endif
            #ifdef BOARD1
            if(incoming.command == SubcommandType::TEST) {
                if(incoming.senderType == SenderType::PERVOPROHODETS) {
                    if(incoming.msgLen == 5 && incoming.message[0] == 5 
                                            && incoming.message[1] == 4 
                                            && incoming.message[2] == 3 
                                            && incoming.message[1] == 2 
                                            && incoming.message[4] == 1){
                        paired = 1;
                        MessageStruct goodPairing;
                        goodPairing.msgType = MessageType::UPLOAD_COMMAND;
                        goodPairing.command = SubcommandType::PAIRED_IS_OK;

                        esp_now_send(peerAddr, (uint8_t *)&goodPairing, 5);
                    }
                }
            }
            #endif
        break;
        case MessageType::OTHER_DATA:
            
        break;
    }
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
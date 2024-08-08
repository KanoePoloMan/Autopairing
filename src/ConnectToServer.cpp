#include "ConnectToServer.hpp"

void checkCommands(int sym) {
    if(sym == '~' && Serial.peek() == '/') {
        Serial.read();
        char command[8]; 
        for(int i = 0; i < 8 && Serial.available() > 0; i++) command[i] = Serial.read();
        if(strcmp(command, "getMacAd")) {
            getMacAddrSerial();
        } else if(strcmp(command, "WiFiCon")) {
            WifiCon();
        }
    }
}
void getMacAddrSerial() {
    uint8_t mac[6];
    wifi_get_macaddr(0, mac);
    Serial.printf("Flash mac: %02X:%02X:%02X:%02X:%02X:%02X\n\r", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Serial.printf("PPS mac: %02X:%02X:%02X:%02X:%02X:%02X\n\r", peerAddr[0], peerAddr[1], peerAddr[2], peerAddr[3], peerAddr[4], peerAddr[5]);
}
void WifiCon() {
    char netName[32];
    char pass[32];
    readNameAndPassSerial(netName, pass);

    MessageStruct package;
    package.msgType = MessageType::UPLOAD_COMMAND;
    package.command = SubcommandType::CONNECT_TO_WIFI;
    package.msgLen = 64;
    memcpy(package.message, netName, 32);
    memcpy(package.message + 32, pass, 32);

    esp_now_send(peerAddr, (uint8_t *)&package, package.msgLen + 5);
}
void readNameAndPassSerial(char *name, char *pass) {
    Serial.read();
    char *pointer = name;
    for(int i = 0; Serial.available() > 0; i++) {
        if(Serial.peek() == ':') {
            Serial.read();
            pointer = pass;
            i = 0;
        }
        pointer[i] = Serial.read();
    }
}
void connectToWifi(char *name, char *pass) {
    WiFi.disconnect(true);
    WiFi.begin(name, pass);
}
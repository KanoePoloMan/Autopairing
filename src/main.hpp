#ifndef __MAIN_HPP__
#define __MAIN_HPP__

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <GyverOLED.h>
#include <EEPROM.h>

// #define BOARD2//slave
#define BOARD1//programmer

#ifdef BOARD2 //programmer bc:ff:4d:f8:04:f8 COM3 white
  #define BOARD {0xb4, 0xe6, 0x2d, 0x37, 0x16, 0xbf}
#endif
#ifdef BOARD1 //slave bc:ff:4d:f9:d0:23 COM5 black
  #define BOARD {0xBC, 0xFF, 0x4d, 0xf9, 0xee, 0x54}
#endif

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus);
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len);

void boardInitialisation();


#ifdef BOARD2

extern GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

void screenInitialize();
void drawInfoOnScreen();
#endif


#define CLOUD_PIN 14

extern uint8_t charge;

int getChargeProcent();
void ADCpowerTimer();
void oledDrawChargeBlock();
void oledCustomClear(int x, int y, int width);

void getMacFromEEPROM();

#ifdef BOARD1
//Фнккция анализа команды смены скорости
//Передается массив команды, если не вмена скорости, возвращается false, иначе 
//true и 4 8ми битных числа, являющиеся 32х битной скоростью сериала
std::pair<boolean, uint8_t[4]> commandAnalysier(std::pair<uint8_t, uint8_t[18]>);
#endif

#endif